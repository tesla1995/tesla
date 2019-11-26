// Copyright (c) 2019 Tesla, Inc.
//
// Licensed under the Apache License, Version 2.0 (the License);
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an AS IS BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Author: Michael Tesla (michaeltesla1995@gmail.com)
// Date: Mon Nov 18 20:14:33 CST 2019

#include <cstring>
#include <atomic>
#include <mutex>
#include <vector>
#include <memory>
#include "tutil/compiler_specific.h"

namespace tesla {
namespace allocator {

static constexpr size_t kPageSize = 1UL << 20;
static constexpr size_t kPageMask = kPageSize - 1;

template <typename T, size_t NUM_ITEMS>
struct ObjectPoolFreeChunk {
  size_t num_ptrs{0};
  T* ptrs[NUM_ITEMS];
};

template <typename T>
struct ObjectPoolFreeChunk<T, 0> {
  size_t num_ptrs{0};
  T* ptrs[0];
};

template <typename T>
struct ObjectPoolBlockMaxSize {
  static constexpr size_t value = kPageSize - sizeof(size_t);
};

template <typename T>
struct ObjectPoolBlockItemNum {
  static constexpr size_t value =
    ObjectPoolBlockMaxSize<T>::value / sizeof(T);
};

template <typename T>
class ObjectPool {
 public:
  static constexpr size_t kNumItemsInBlock = ObjectPoolBlockItemNum<T>::value;
  static constexpr size_t kNumItemsInFreeChunk = kNumItemsInBlock;
  static constexpr size_t kMaxNumBlockGroup = 65536;
  static constexpr size_t kNumBlocksInGroup = 65536;
  static constexpr size_t kInitialFreeListSize = 1024;

  using FreeChunk = ObjectPoolFreeChunk<T, kNumItemsInFreeChunk>;
  using DynamicFreeChunk = ObjectPoolFreeChunk<T, 0>;

  struct TESLA_CACHELINE_ALIGNMENT Block {
    char items[ObjectPoolBlockMaxSize<T>::value];
    size_t num_items;

    Block() : num_items(0) {}
  };

  struct BlockGroup {
    Block* blocks[kNumBlocksInGroup]; 
    size_t num_blocks;

    BlockGroup() : num_blocks(0) {
      memset(blocks, 0, sizeof(Block*) * kNumBlocksInGroup);
    }
  };
 
  static_assert((sizeof(Block) & kPageMask) == 0);

  // Each thread has an instance of this class.
  class TESLA_CACHELINE_ALIGNMENT LocalPool {
   public:
    LocalPool(ObjectPool* object_pool) {
      object_pool_ = object_pool;
      object_pool_->num_local_pools_.fetch_add(1, std::memory_order_relaxed);
    }

    ~LocalPool() {
      while (local_block_->num_items < kNumItemsInBlock) {
        T* object = new ((T*)local_block_->items + local_block_->num_items) T;
        ++local_block_->num_items; 
        Delete(object);
      }

      if (local_free_chunk_.num_ptrs) {
        object_pool_->PushFreeChunk(local_free_chunk_);
      }

      object_pool_->num_local_pools_.fetch_add(-1, std::memory_order_relaxed);
    }

    inline T* New() {
      if (local_free_chunk_.num_ptrs) {
        return local_free_chunk_.ptrs[--local_free_chunk_.num_ptrs];
      }

      if (object_pool_->PopFreeChunk(local_free_chunk_)) {
        return local_free_chunk_.ptrs[--local_free_chunk_.num_ptrs];
      }
      
      if (local_block_ && local_block_->num_items < kNumItemsInBlock) {
        T* object = new ((T*)local_block_->items + local_block_->num_items) T;
        ++local_block_->num_items; 
        return object;
      }

      local_block_ = AddBlock();
      if (local_block_) {
        T* object = new ((T*)local_block_->items + local_block_->num_items) T;
        ++local_block_->num_items; 
        return object;
      }

      return nullptr;
    }

    inline void Delete(T* ptr) {
      if (local_free_chunk_.num_ptrs < kNumItemsInFreeChunk) {
        local_free_chunk_.ptrs[local_free_chunk_.num_ptrs] = ptr;
        ++local_free_chunk_.num_ptrs;
        return;
      }

      if (object_pool_->PushFreeChunk(local_free_chunk_)) {
        local_free_chunk_.ptrs[0] = ptr;
        local_free_chunk_.num_ptrs = 1;
      }
    }

    inline size_t NumFreeItems() {
      return local_free_chunk_.num_ptrs;
    }

    inline size_t NumItems() {
      return local_block_->num_items;
    }

   private:
    ObjectPool* object_pool_{nullptr};
    Block* local_block_{nullptr};
    FreeChunk local_free_chunk_;
  };

  inline T* New() {
    return GetLocalPool()->New();
  }

  inline void Delete(T* obj) {
    return GetLocalPool()->Delete(obj);
  }

  static inline ObjectPool* Singleton() {
    //ObjectPool* object_pool = object_pool_.load(std::memory_order_consume);
    //if (!object_pool) {
    //  std::lock_guard<std::mutex> guard(init_mutex_);
    //  object_pool = object_pool_.load(std::memory_order_relaxed); // protected by lock.
    //  if (!object_pool) {
    //    object_pool = new (std::nothrow) ObjectPool;
    //    object_pool_.store(object_pool, std::memory_order_release);
    //  }
    //}
    //return object_pool;

    static ObjectPool object_pool;
    return &object_pool;
  }

  size_t GetLocalPoolNumItems() {
    return GetLocalPool()->NumItems();
  }
 
  size_t GetLocalPoolNumFreeItems() {
    return GetLocalPool()->NumFreeItems();
  }

 private:
  ObjectPool() {
    free_chunks_.reserve(kInitialFreeListSize);
  }

  ~ObjectPool() {

  }

  inline LocalPool* GetLocalPool() {
    static thread_local LocalPool local_pool_(this);
    return &local_pool_;
  }

  bool PopFreeChunk(FreeChunk& c) {
    // Critical for the case that most Delete are called in
    // different threads.
    if (free_chunks_.empty()) {
      return false;
    }

    DynamicFreeChunk* p{nullptr};
    {
      std::lock_guard<std::mutex> guard(free_chunks_mutex_);
      if (free_chunks_.empty()) {
        return false;
      }
      p = free_chunks_.back(); 
      free_chunks_.pop_back();
    }

    c.num_ptrs = p->num_ptrs;
    memcpy(c.ptrs, p->ptrs, p->num_ptrs * sizeof(p->ptrs[0]));
    free(p);
    return true;
  }

  bool PushFreeChunk(const FreeChunk& c) {
    DynamicFreeChunk* p = (DynamicFreeChunk*)malloc(
      sizeof(DynamicFreeChunk) + sizeof(c.ptrs[0]) * c.num_ptrs);
    if (!p) {
      return false;
    }

    p->num_ptrs = c.num_ptrs;
    memcpy(p->ptrs, c.ptrs, sizeof(c.ptrs[0]) * c.num_ptrs);

    std::lock_guard<std::mutex> guard(free_chunks_mutex_);
    free_chunks_.push_back(p);
    return true;
  }

  static Block* AddBlock() {
    Block* new_block = new(std::nothrow) Block;
    if (new_block == nullptr) {
      return nullptr;
    }

    std::lock_guard<std::mutex> guard(block_groups_mutex_); 

    do {
      if (num_block_groups >= 1) {
        BlockGroup* group = block_groups[num_block_groups - 1];
        if (group->num_blocks < kNumBlocksInGroup) {
          group->blocks[group->num_blocks] = new_block;
          ++group->num_blocks;
          return new_block;
        }
      }
    } while (AddGroup(num_block_groups));

    // Fail to add new BlockGroup
    delete new_block;
    return nullptr;
  }

  static bool AddGroup(size_t group_index) {
    (void)(group_index);

    if (num_block_groups < kMaxNumBlockGroup) {
      BlockGroup* new_group = new(std::nothrow) BlockGroup;
      if (new_group) {
        block_groups[num_block_groups] = new_group; 
        ++num_block_groups;
        return true;
      }
    }
    return false;
  }

  //static std::atomic<ObjectPool*> object_pool_{nullptr};
  //static std::mutex init_mutex_;

  static BlockGroup* block_groups[kMaxNumBlockGroup]; 
  static size_t num_block_groups;
  static std::mutex block_groups_mutex_;

  std::vector<DynamicFreeChunk*> free_chunks_;
  std::mutex free_chunks_mutex_;

  static std::atomic<size_t> num_local_pools_;
};

template <typename T>
typename ObjectPool<T>::BlockGroup* ObjectPool<T>::block_groups[kMaxNumBlockGroup];

template <typename T>
size_t ObjectPool<T>::num_block_groups{0};

template <typename T>
std::mutex ObjectPool<T>::block_groups_mutex_;

template <typename T>
std::atomic<size_t> ObjectPool<T>::num_local_pools_{0};

}  // namespace allocator
}  // namespace tesla
