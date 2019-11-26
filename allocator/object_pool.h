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
#include <pthread.h>
#include "tutil/compiler_specific.h"

namespace tesla {
namespace allocator {

static constexpr size_t kPageSize = 4UL << 10;
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

template <typename T> struct ObjectPoolBlockMaxSize {
    static const size_t value = 1UL << 16; // bytes
};
template <typename T> struct ObjectPoolBlockMaxItem {
    static const size_t value = 256;
};

template <typename T>
class ObjectPoolBlockItemNum {
    static const size_t N1 = ObjectPoolBlockMaxSize<T>::value / sizeof(T);
    static const size_t N2 = (N1 < 1 ? 1 : N1);
public:
    static const size_t value = (N2 > ObjectPoolBlockMaxItem<T>::value ?
                                 ObjectPoolBlockMaxItem<T>::value : N2);
};

template <typename T>
class ObjectPool {
 public:
  static constexpr size_t kNumItemsInBlock = ObjectPoolBlockItemNum<T>::value;
  static constexpr size_t kNumItemsInFreeChunk = kNumItemsInBlock;
  static constexpr size_t kMaxNumBlockGroup = 1UL << 16;
  static constexpr size_t kNumBlocksInGroup = 1UL << 16;
  static constexpr size_t kInitialFreeListSize = 1UL << 10;

  using FreeChunk = ObjectPoolFreeChunk<T, kNumItemsInFreeChunk>;
  using DynamicFreeChunk = ObjectPoolFreeChunk<T, 0>;

  struct TESLA_CACHELINE_ALIGNMENT Block {
    char items[sizeof(T) * kNumItemsInBlock];
    size_t num_items;

    Block() : num_items(0) {}
  };

  struct BlockGroup {
    std::atomic<size_t> num_blocks;
    std::atomic<Block*> blocks[kNumBlocksInGroup]; 

    BlockGroup() : num_blocks(0) {
      memset(blocks, 0, sizeof(std::atomic<Block*>) * kNumBlocksInGroup);
    }
  };
 
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
      
      if (local_block_ != nullptr && local_block_->num_items < kNumItemsInBlock) {
        T* object = new ((T*)local_block_->items + local_block_->num_items) T;
        ++local_block_->num_items; 
        return object;
      }

      local_block_ = AddBlock();
      if (local_block_ != nullptr) {
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
    size_t index{0};
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

  inline LocalPool* GetLocalPool() {
    static thread_local LocalPool local_pool(this);
    return &local_pool;
  }

 private:
  ObjectPool() {
    free_chunks_.reserve(kInitialFreeListSize);
  }

  ~ObjectPool() {

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
    Block* const new_block = new (std::nothrow) Block;
    if (new_block == nullptr) {
      return nullptr;
    }

    size_t num_block_groups;
    do {
      num_block_groups = num_block_groups_.load(std::memory_order_acquire);
      if (num_block_groups >= 1) {
        BlockGroup* group =
            block_groups_[num_block_groups - 1].load(std::memory_order_relaxed);
        size_t block_index =
            group->num_blocks.fetch_add(1, std::memory_order_relaxed);
        if (block_index < kNumBlocksInGroup) {
          group->blocks[block_index].store(new_block, std::memory_order_release);
          return new_block;
        }
        group->num_blocks.fetch_sub(1, std::memory_order_relaxed);
      }
    } while (AddGroup(num_block_groups));

    // Fail to add BlockGroup
    delete new_block;
    return nullptr;
  }

  static bool AddGroup(size_t num_block_groups) {
    BlockGroup* new_group{nullptr};

    std::lock_guard<std::mutex> guard(block_groups_mutex_); 
    const size_t group_index = num_block_groups_.load(std::memory_order_relaxed);
    if (num_block_groups != group_index) {
      // Other thread has already got the lock and added a BlockGroup.
      return true;
    }

    if (group_index < kMaxNumBlockGroup) {
      new_group = new(std::nothrow) BlockGroup;
      if (new_group != nullptr) {
        // use std::memory_order_consume to prevent other threads calling AddBlock() from seeing
        // **un-constructed** `new_group' ?
        block_groups_[group_index].store(new_group, std::memory_order_relaxed);
        num_block_groups_.store(group_index + 1, std::memory_order_release);
      }
    }
    return new_group != nullptr;
  }

  //static std::atomic<ObjectPool*> object_pool_{nullptr};
  //static std::mutex init_mutex_;

  static std::atomic<BlockGroup*> block_groups_[kMaxNumBlockGroup]; 
  static std::atomic<size_t> num_block_groups_;
  static std::mutex block_groups_mutex_;

  std::vector<DynamicFreeChunk*> free_chunks_;
  std::mutex free_chunks_mutex_;

  static std::atomic<size_t> num_local_pools_;
};

template <typename T>
std::atomic<typename ObjectPool<T>::BlockGroup*> ObjectPool<T>::block_groups_[kMaxNumBlockGroup];

template <typename T>
std::atomic<size_t> ObjectPool<T>::num_block_groups_{0};

template <typename T>
std::mutex ObjectPool<T>::block_groups_mutex_;

template <typename T>
std::atomic<size_t> ObjectPool<T>::num_local_pools_{0};

}  // namespace allocator
}  // namespace tesla
