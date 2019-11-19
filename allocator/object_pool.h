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

#include <atomic>
#include <mutex>
#include <vector>
#include "tutil/compiler_specific.h"

namespace tesla {
namespace allocator {

static constexpr size_t kPageSize = 1UL << 20;
static constexpr size_t kPageMask = kPageSize - 1;

template <typename T, size_t NUM_ITEMS>
struct ObjectPoolFreeChunk {
  size_t num_ptrs;
  T* ptrs[NUM_ITEMS];
};

template <typename T>
struct ObjectPoolFreeChunk<T, 0> {
  size_t num_ptrs;
  T* ptrs[0];
};

template <typename T>
struct ObjectPoolBlockMaxSize {
  static constexpr size_t value = kPageSize;
  static_assert((value & kPageMask) == 0);
};

template <typename T>
struct ObjectPoolBlockItemNum {
  static constexpr size_t value =
    (ObjectPoolBlockMaxSize<T>::value - sizeof(size_t)) / sizeof(T);
};

template <typename T>
class ObjectPool {
 public:
  static constexpr size_t kNumItemsInBlock = ObjectPoolBlockItemNum<T>::value;
  static constexpr size_t kNumItemsInFreeChunk = kNumItemsInBlock;
  static constexpr size_t kMaxNumBlockGroup = 65536;
  static constexpr size_t kNumBlocksInGroup = 65536;

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
    }

    ~LocalPool() {

    }

    T* New() {
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

      local_block_ = AddBlock(&local_block_index_);
      if (local_block_) {
        T* object = new ((T*)local_block_->items + local_block_->num_items) T;
        ++local_block_->num_items; 
        return object;
      }

      return nullptr;
    }

    void Delete(T* ptr) {
          
    }

   private:
    ObjectPool* object_pool_{nullptr};
    size_t local_block_index_{0};
    Block* local_block_{nullptr};
    size_t num_items_{0};
    FreeChunk local_free_chunk_;
  };

  static T* New() {

  }

  static void Delete(T*) {

  }
 private:
  bool PopFreeChunk(FreeChunk& c) {
    DynamicFreeChunk* p{nullptr};

    // Critical for the case that most Delete are called in
    // different threads.
    if (free_chunks_.empty()) {
      return false;
    }

    {
      std::lock_guard<std::mutex> guard(free_chunks_mutex_);
      if (free_chunks_.empty()) {
        return false;
      }
      p = free_chunks_.p(); 
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

  static Block* AddBlock(size_t* index) {
    Block* new_block = new(std::nothrow) Block;
    if (!new_block) {
      return nullptr;
    }

    std::lock_guard<std::mutex> guard(block_groups_mutex_); 

    do {
      if (num_block_groups >= 1) {
        BlockGroup* group = block_groups[num_block_groups - 1];
        if (group->num_blocks < kNumBlocksInGroup) {
          block_groups[group->num_blocks] = new_block;
          *index = (num_block_groups - 1) * kNumBlocksInGroup + group->num_blocks;
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

  ObjectPool() {}

  ~ObjectPool() {}

  static ObjectPool* Singleton() {
#ifdef USE_ATOMIC_SINGLETON
    ObjectPool* object_pool = object_pool_.load(std::memory_order_consume);
    if (!object_pool) {
      std::lock_guard<std::mutex> guard(init_mutex_);
      object_pool = object_pool_.load(std::memory_order_relaxed); // protected by lock.
      if (!object_pool) {
        object_pool = new (std::nothrow) ObjectPool;
        object_pool_.store(object_pool, std::memory_order_release);
      }
    }
    return object_pool;
#else
    static ObjectPool object_pool;
    return &object_pool;
#endif
  }

#ifdef USE_ATOMIC_SINGLETON
  static std::atomic<ObjectPool*> object_pool_{nullptr};
  static std::mutex init_mutex_;
#endif

  static BlockGroup block_groups[kMaxNumBlockGroup]; 
  static size_t num_block_groups;

  static std::vector<DynamicFreeChunk*> free_chunks_;

  static std::mutex free_chunks_mutex_;
  static std::mutex block_groups_mutex_;
};

}  // namespace allocator
}  // namespace tesla
