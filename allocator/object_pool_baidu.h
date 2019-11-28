// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

// bthread - A M:N threading library to make applications more concurrent.

// Author: Ge,Jun (gejun@baidu.com)
// Date: Sun Jul 13 15:04:18 CST 2014

#ifndef BUTIL_OBJECT_POOL_H
#define BUTIL_OBJECT_POOL_H

#include <cstring>                       // memcpy, memset
#include <cstddef>                       // size_t
#include <iostream>                      // std::ostream
#include <pthread.h>                     // pthread_mutex_t
#include <algorithm>                     // std::max, std::min
#include <atomic>
#include <mutex>
//#include "baidu/atomicops.h"              // baidu::atomic
//#include "baidu/macros.h"                 // BAIDU_CACHELINE_ALIGNMENT
//#include "baidu/scoped_lock.h"            // BAIDU_SCOPED_LOCK
//#include "baidu/thread_local.h"           // BAIDU_THREAD_LOCAL
#include <vector>

#define BAIDU_CACHELINE_ALIGNMENT alignas(64)
#define BAIDU_LIKELY(expr) (__builtin_expect((bool)(expr), true))

#ifdef BUTIL_OBJECT_POOL_NEED_FREE_ITEM_NUM
#define BAIDU_OBJECT_POOL_FREE_ITEM_NUM_ADD1                    \
    (_global_nfree.fetch_add(1, std::memory_order_relaxed))
#define BAIDU_OBJECT_POOL_FREE_ITEM_NUM_SUB1                    \
    (_global_nfree.fetch_sub(1, std::memory_order_relaxed))
#else
#define BAIDU_OBJECT_POOL_FREE_ITEM_NUM_ADD1
#define BAIDU_OBJECT_POOL_FREE_ITEM_NUM_SUB1
#endif

// ObjectPool is a derivative class of ResourcePool to allocate and
// reuse fixed-size objects without identifiers.

namespace baidu {

// Specialize following classes to override default parameters for type T.
//   namespace baidu {
//     template <> struct ObjectPoolBlockMaxSize<Foo> {
//       static const size_t value = 1024;
//     };
//   }

// Memory is allocated in blocks, memory size of a block will not exceed:
//   min(ObjectPoolBlockMaxSize<T>::value,
//       ObjectPoolBlockMaxItem<T>::value * sizeof(T))
template <typename T> struct ObjectPoolBlockMaxSize {
    static const size_t value = 64 * 1024; // bytes
};
template <typename T> struct ObjectPoolBlockMaxItem {
    static const size_t value = 256;
};

// Free objects of each thread are grouped into a chunk before they are merged
// to the global list. Memory size of objects in one free chunk will not exceed:
//   min(ObjectPoolFreeChunkMaxItem<T>::value(),
//       ObjectPoolBlockMaxSize<T>::value,
//       ObjectPoolBlockMaxItem<T>::value * sizeof(T))
template <typename T> struct ObjectPoolFreeChunkMaxItem {
    static size_t value() { return 256; }
};

// ObjectPool calls this function on newly constructed objects. If this
// function returns false, the object is destructed immediately and
// get_object() shall return NULL. This is useful when the constructor
// failed internally(namely ENOMEM).
template <typename T> struct ObjectPoolValidator {
    static bool validate(const T*) { return true; }
};

}  // namespace baidu

namespace baidu {

template <typename T, size_t NITEM>
struct ObjectPoolFreeChunk {
    size_t nfree;
    T* ptrs[NITEM];
}; 
// for gcc 3.4.5
template <typename T>
struct ObjectPoolFreeChunk<T, 0> {
    size_t nfree;
    T* ptrs[0];
}; 

struct ObjectPoolInfo {
    size_t local_pool_num;
    size_t block_group_num;
    size_t block_num;
    size_t item_num;
    size_t block_item_num;
    size_t free_chunk_item_num;
    size_t total_size;
#ifdef BUTIL_OBJECT_POOL_NEED_FREE_ITEM_NUM
    size_t free_item_num;
#endif
};

static const size_t OP_MAX_BLOCK_NGROUP = 65536;
static const size_t OP_GROUP_NBLOCK_NBIT = 16;
static const size_t OP_GROUP_NBLOCK = (1UL << OP_GROUP_NBLOCK_NBIT);
static const size_t OP_INITIAL_FREE_LIST_SIZE = 1024;

template <typename T>
class ObjectPoolBlockItemNum {
    static const size_t N1 = ObjectPoolBlockMaxSize<T>::value / sizeof(T);
    static const size_t N2 = (N1 < 1 ? 1 : N1);
public:
    static const size_t value = (N2 > ObjectPoolBlockMaxItem<T>::value ?
                                 ObjectPoolBlockMaxItem<T>::value : N2);
};

template <typename T>
class BAIDU_CACHELINE_ALIGNMENT ObjectPool {
public:
    static const size_t BLOCK_NITEM = ObjectPoolBlockItemNum<T>::value;
    static const size_t FREE_CHUNK_NITEM = BLOCK_NITEM;

    // Free objects are batched in a FreeChunk before they're added to
    // global list(_free_chunks).
    typedef ObjectPoolFreeChunk<T, FREE_CHUNK_NITEM>    FreeChunk;
    typedef ObjectPoolFreeChunk<T, 0> DynamicFreeChunk;

    // When a thread needs memory, it allocates a Block. To improve locality,
    // items in the Block are only used by the thread.
    // To support cache-aligned objects, align Block.items by cacheline.
    struct BAIDU_CACHELINE_ALIGNMENT Block {
        char items[sizeof(T) * BLOCK_NITEM];
        size_t nitem;

        Block() : nitem(0) {}
    };

    // An Object addresses at most OP_MAX_BLOCK_NGROUP BlockGroups,
    // each BlockGroup addresses at most OP_GROUP_NBLOCK blocks. So an
    // object addresses at most OP_MAX_BLOCK_NGROUP * OP_GROUP_NBLOCK Blocks.
    struct BlockGroup {
        std::atomic<size_t> nblock;
        std::atomic<Block*> blocks[OP_GROUP_NBLOCK];

        BlockGroup() : nblock(0) {
            // We fetch_add nblock in add_block() before setting the entry,
            // thus address_resource() may sees the unset entry. Initialize
            // all entries to NULL makes such address_resource() return NULL.
            memset(blocks, 0, sizeof(std::atomic<Block*>) * OP_GROUP_NBLOCK);
        }
    };

    // Each thread has an instance of this class.
    class BAIDU_CACHELINE_ALIGNMENT LocalPool {
    public:
        explicit LocalPool(ObjectPool* pool)
            : _pool(pool)
            , _cur_block(NULL)
            , _cur_block_index(0) {
            _cur_free.nfree = 0;
        }

        ~LocalPool() {
            // Add to global _free if there're some free objects
            if (_cur_free.nfree) {
                _pool->push_free_chunk(_cur_free);
            }

            _pool->clear_from_destructor_of_local_pool();
        }

        static void delete_local_pool(void* arg) {
            delete (LocalPool*)arg;
        }

        // We need following macro to construct T with different CTOR_ARGS
        // which may include parenthesis because when T is POD, "new T()"
        // and "new T" are different: former one sets all fields to 0 which
        // we don't want.
#define BAIDU_OBJECT_POOL_GET(CTOR_ARGS)                                \
        /* Fetch local free ptr */                                      \
        if (_cur_free.nfree) {                                          \
            BAIDU_OBJECT_POOL_FREE_ITEM_NUM_SUB1;                       \
            return _cur_free.ptrs[--_cur_free.nfree];                   \
        }                                                               \
        /* Fetch a FreeChunk from global.                               \
           TODO: Popping from _free needs to copy a FreeChunk which is  \
           costly, but hardly impacts amortized performance. */         \
        if (_pool->pop_free_chunk(_cur_free)) {                         \
            BAIDU_OBJECT_POOL_FREE_ITEM_NUM_SUB1;                       \
            return _cur_free.ptrs[--_cur_free.nfree];                   \
        }                                                               \
        /* Fetch memory from local block */                             \
        if (_cur_block && _cur_block->nitem < BLOCK_NITEM) {            \
            T* obj = new ((T*)_cur_block->items + _cur_block->nitem) T CTOR_ARGS; \
            if (!ObjectPoolValidator<T>::validate(obj)) {               \
                obj->~T();                                              \
                return NULL;                                            \
            }                                                           \
            ++_cur_block->nitem;                                        \
            return obj;                                                 \
        }                                                               \
        /* Fetch a Block from global */                                 \
        _cur_block = add_block(&_cur_block_index);                      \
        if (_cur_block != NULL) {                                       \
            T* obj = new ((T*)_cur_block->items + _cur_block->nitem) T CTOR_ARGS; \
            if (!ObjectPoolValidator<T>::validate(obj)) {               \
                obj->~T();                                              \
                return NULL;                                            \
            }                                                           \
            ++_cur_block->nitem;                                        \
            return obj;                                                 \
        }                                                               \
        return NULL;                                                    \
 

        inline T* get() {
            BAIDU_OBJECT_POOL_GET();
        }

        template <typename A1>
        inline T* get(const A1& a1) {
            BAIDU_OBJECT_POOL_GET((a1));
        }

        template <typename A1, typename A2>
        inline T* get(const A1& a1, const A2& a2) {
            BAIDU_OBJECT_POOL_GET((a1, a2));
        }

#undef BAIDU_OBJECT_POOL_GET

        inline int return_object(T* ptr) {
            // Return to local free list
            if (_cur_free.nfree < ObjectPool::free_chunk_nitem()) {
                _cur_free.ptrs[_cur_free.nfree++] = ptr;
                BAIDU_OBJECT_POOL_FREE_ITEM_NUM_ADD1;
                return 0;
            }
            // Local free list is full, return it to global.
            // For copying issue, check comment in upper get()
            if (_pool->push_free_chunk(_cur_free)) {
                _cur_free.nfree = 1;
                _cur_free.ptrs[0] = ptr;
                BAIDU_OBJECT_POOL_FREE_ITEM_NUM_ADD1;
                return 0;
            }
            return -1;
        }

    private:
        ObjectPool* _pool;
        Block* _cur_block;
        size_t _cur_block_index;
        FreeChunk _cur_free;
    };

    inline T* get_object() {
        LocalPool* lp = get_or_new_local_pool();
        if (BAIDU_LIKELY(lp != NULL)) {
            return lp->get();
        }
        return NULL;
    }

    template <typename A1>
    inline T* get_object(const A1& arg1) {
        LocalPool* lp = get_or_new_local_pool();
        if (BAIDU_LIKELY(lp != NULL)) {
            return lp->get(arg1);
        }
        return NULL;
    }

    template <typename A1, typename A2>
    inline T* get_object(const A1& arg1, const A2& arg2) {
        LocalPool* lp = get_or_new_local_pool();
        if (BAIDU_LIKELY(lp != NULL)) {
            return lp->get(arg1, arg2);
        }
        return NULL;
    }

    inline int return_object(T* ptr) {
        LocalPool* lp = get_or_new_local_pool();
        if (BAIDU_LIKELY(lp != NULL)) {
            return lp->return_object(ptr);
        }
        return -1;
    }

    void clear_objects() {
        LocalPool* lp = _local_pool;
        if (lp) {
            _local_pool = NULL;
            //baidu::thread_atexit_cancel(LocalPool::delete_local_pool, lp);
            delete lp;
        }
    }

    inline static size_t free_chunk_nitem() {
        const size_t n = ObjectPoolFreeChunkMaxItem<T>::value();
        return (n < FREE_CHUNK_NITEM ? n : FREE_CHUNK_NITEM);
    }

    // Number of all allocated objects, including being used and free.
    ObjectPoolInfo describe_objects() const {
        ObjectPoolInfo info;
        info.local_pool_num = _nlocal.load(std::memory_order_relaxed);
        info.block_group_num = _ngroup.load(std::memory_order_acquire);
        info.block_num = 0;
        info.item_num = 0;
        info.free_chunk_item_num = free_chunk_nitem();
        info.block_item_num = BLOCK_NITEM;
#ifdef BUTIL_OBJECT_POOL_NEED_FREE_ITEM_NUM
        info.free_item_num = _global_nfree.load(std::memory_order_relaxed);
#endif

        for (size_t i = 0; i < info.block_group_num; ++i) {
            BlockGroup* bg = _block_groups[i].load(std::memory_order_consume);
            if (NULL == bg) {
                break;
            }
            size_t nblock = std::min(bg->nblock.load(std::memory_order_relaxed),
                                     OP_GROUP_NBLOCK);
            info.block_num += nblock;
            for (size_t j = 0; j < nblock; ++j) {
                Block* b = bg->blocks[j].load(std::memory_order_consume);
                if (NULL != b) {
                    info.item_num += b->nitem;
                }
            }
        }
        info.total_size = info.block_num * info.block_item_num * sizeof(T);
        return info;
    }

    static inline ObjectPool* singleton() {
        ObjectPool* p = _singleton.load(std::memory_order_consume);
        if (p) {
            return p;
        }
        pthread_mutex_lock(&_singleton_mutex);
        p = _singleton.load(std::memory_order_consume);
        if (!p) {
            p = new ObjectPool();
            _singleton.store(p, std::memory_order_release);
        }
        pthread_mutex_unlock(&_singleton_mutex);
        return p;
    }

private:
    ObjectPool() {
        _free_chunks.reserve(OP_INITIAL_FREE_LIST_SIZE);
        pthread_mutex_init(&_free_chunks_mutex, NULL);
    }

    ~ObjectPool() {
        pthread_mutex_destroy(&_free_chunks_mutex);
    }

    // Create a Block and append it to right-most BlockGroup.
    static Block* add_block(size_t* index) {
        Block* const new_block = new(std::nothrow) Block;
        if (NULL == new_block) {
            return NULL;
        }
        size_t ngroup;
        do {
            ngroup = _ngroup.load(std::memory_order_acquire);
            if (ngroup >= 1) {
                BlockGroup* const g =
                    _block_groups[ngroup - 1].load(std::memory_order_consume);
                const size_t block_index =
                    g->nblock.fetch_add(1, std::memory_order_relaxed);
                if (block_index < OP_GROUP_NBLOCK) {
                    g->blocks[block_index].store(
                        new_block, std::memory_order_release);
                    *index = (ngroup - 1) * OP_GROUP_NBLOCK + block_index;
                    return new_block;
                }
                g->nblock.fetch_sub(1, std::memory_order_relaxed);
            }
        } while (add_block_group(ngroup));

        // Fail to add_block_group.
        delete new_block;
        return NULL;
    }

    // Create a BlockGroup and append it to _block_groups.
    // Shall be called infrequently because a BlockGroup is pretty big.
    static bool add_block_group(size_t old_ngroup) {
        BlockGroup* bg = NULL;
        //BAIDU_SCOPED_LOCK(_block_group_mutex);
        std::lock_guard<std::mutex> guard(_block_group_mutex);
        const size_t ngroup = _ngroup.load(std::memory_order_acquire);
        if (ngroup != old_ngroup) {
            // Other thread got lock and added group before this thread.
            return true;
        }
        if (ngroup < OP_MAX_BLOCK_NGROUP) {
            bg = new(std::nothrow) BlockGroup;
            if (NULL != bg) {
                // Release fence is paired with consume fence in add_block()
                // to avoid un-constructed bg to be seen by other threads.
                _block_groups[ngroup].store(bg, std::memory_order_release);
                _ngroup.store(ngroup + 1, std::memory_order_release);
            }
        }
        return bg != NULL;
    }

    inline LocalPool* get_or_new_local_pool() {
        LocalPool* lp = _local_pool;
        if (BAIDU_LIKELY(lp != NULL)) {
            return lp;
        }
        lp = new(std::nothrow) LocalPool(this);
        if (NULL == lp) {
            return NULL;
        }
        //BAIDU_SCOPED_LOCK(_change_thread_mutex); //avoid race with clear()
        std::lock_guard<std::mutex> guard(_change_thread_mutex);
        _local_pool = lp;
        //std::thread_atexit(LocalPool::delete_local_pool, lp);
        _nlocal.fetch_add(1, std::memory_order_relaxed);
        return lp;
    }

    void clear_from_destructor_of_local_pool() {
        // Remove tls
        _local_pool = NULL;

        // Do nothing if there're active threads.
        if (_nlocal.fetch_sub(1, std::memory_order_relaxed) != 1) {
            return;
        }

        // Can't delete global even if all threads(called ObjectPool
        // functions) quit because the memory may still be referenced by 
        // other threads. But we need to validate that all memory can
        // be deallocated correctly in tests, so wrap the function with 
        // a macro which is only defined in unittests.
#ifdef BAIDU_CLEAR_OBJECT_POOL_AFTER_ALL_THREADS_QUIT
        //BAIDU_SCOPED_LOCK(_change_thread_mutex);  // including acquire fence.
        std::lock_guard<std::mutex> guard(_change_thread_mutex);
        // Do nothing if there're active threads.
        if (_nlocal.load(std::memory_order_relaxed) != 0) {
            return;
        }
        // All threads exited and we're holding _change_thread_mutex to avoid
        // racing with new threads calling get_object().

        // Clear global free list.
        FreeChunk dummy;
        while (pop_free_chunk(dummy));

        // Delete all memory
        const size_t ngroup = _ngroup.exchange(0, std::memory_order_relaxed);
        for (size_t i = 0; i < ngroup; ++i) {
            BlockGroup* bg = _block_groups[i].load(std::memory_order_relaxed);
            if (NULL == bg) {
                break;
            }
            size_t nblock = std::min(bg->nblock.load(std::memory_order_relaxed),
                                     OP_GROUP_NBLOCK);
            for (size_t j = 0; j < nblock; ++j) {
                Block* b = bg->blocks[j].load(std::memory_order_relaxed);
                if (NULL == b) {
                    continue;
                }
                for (size_t k = 0; k < b->nitem; ++k) {
                    T* const objs = (T*)b->items;
                    objs[k].~T();
                }
                delete b;
            }
            delete bg;
        }

        memset(_block_groups, 0, sizeof(BlockGroup*) * OP_MAX_BLOCK_NGROUP);
#endif
    }

private:
    bool pop_free_chunk(FreeChunk& c) {
        // Critical for the case that most return_object are called in
        // different threads of get_object.
        if (_free_chunks.empty()) {
            return false;
        }
        pthread_mutex_lock(&_free_chunks_mutex);
        if (_free_chunks.empty()) {
            pthread_mutex_unlock(&_free_chunks_mutex);
            return false;
        }
        DynamicFreeChunk* p = _free_chunks.back();
        _free_chunks.pop_back();
        pthread_mutex_unlock(&_free_chunks_mutex);
        c.nfree = p->nfree;
        memcpy(c.ptrs, p->ptrs, sizeof(*p->ptrs) * p->nfree);
        free(p);
        return true;
    }

    bool push_free_chunk(const FreeChunk& c) {
        DynamicFreeChunk* p = (DynamicFreeChunk*)malloc(
            offsetof(DynamicFreeChunk, ptrs) + sizeof(*c.ptrs) * c.nfree);
        if (!p) {
            return false;
        }
        p->nfree = c.nfree;
        memcpy(p->ptrs, c.ptrs, sizeof(*c.ptrs) * c.nfree);
        pthread_mutex_lock(&_free_chunks_mutex);
        _free_chunks.push_back(p);
        pthread_mutex_unlock(&_free_chunks_mutex);
        return true;
    }
    
    static std::atomic<ObjectPool*> _singleton;
    static pthread_mutex_t _singleton_mutex;
    static __thread LocalPool* _local_pool;
    static std::atomic<long> _nlocal;
    static std::atomic<size_t> _ngroup;
    static std::mutex _block_group_mutex;
    static std::mutex _change_thread_mutex;
    static std::atomic<BlockGroup*> _block_groups[OP_MAX_BLOCK_NGROUP];

    std::vector<DynamicFreeChunk*> _free_chunks;
    pthread_mutex_t _free_chunks_mutex;

#ifdef BUTIL_OBJECT_POOL_NEED_FREE_ITEM_NUM
    static baidu::static_atomic<size_t> _global_nfree;
#endif
};

// Declare template static variables:

template <typename T>
const size_t ObjectPool<T>::FREE_CHUNK_NITEM;

template <typename T>
__thread typename ObjectPool<T>::LocalPool*
ObjectPool<T>::_local_pool = NULL;

template <typename T>
std::atomic<ObjectPool<T>*> ObjectPool<T>::_singleton(NULL);

template <typename T>
pthread_mutex_t ObjectPool<T>::_singleton_mutex = PTHREAD_MUTEX_INITIALIZER;

template <typename T>
std::atomic<long> ObjectPool<T>::_nlocal(0);

template <typename T>
std::atomic<size_t> ObjectPool<T>::_ngroup(0);

template <typename T>
std::mutex ObjectPool<T>::_block_group_mutex;

template <typename T>
std::mutex ObjectPool<T>::_change_thread_mutex;

template <typename T>
std::atomic<typename ObjectPool<T>::BlockGroup*>
ObjectPool<T>::_block_groups[OP_MAX_BLOCK_NGROUP] = {};

#ifdef BUTIL_OBJECT_POOL_NEED_FREE_ITEM_NUM
template <typename T>
baidu::static_atomic<size_t> ObjectPool<T>::_global_nfree = BUTIL_STATIC_ATOMIC_INIT(0);
#endif

inline std::ostream& operator<<(std::ostream& os,
                                ObjectPoolInfo const& info) {
    return os << "local_pool_num: " << info.local_pool_num
              << "\nblock_group_num: " << info.block_group_num
              << "\nblock_num: " << info.block_num
              << "\nitem_num: " << info.item_num
              << "\nblock_item_num: " << info.block_item_num
              << "\nfree_chunk_item_num: " << info.free_chunk_item_num
              << "\ntotal_size: " << info.total_size
#ifdef BUTIL_OBJECT_POOL_NEED_FREE_ITEM_NUM
              << "\nfree_num: " << info.free_item_num
#endif
        ;
}
}  // namespace baidu

namespace baidu {

// Get an object typed |T|. The object should be cleared before usage.
// NOTE: T must be default-constructible.
template <typename T> inline T* get_object() {
    return ObjectPool<T>::singleton()->get_object();
}

// Get an object whose constructor is T(arg1)
template <typename T, typename A1>
inline T* get_object(const A1& arg1) {
    return ObjectPool<T>::singleton()->get_object(arg1);
}

// Get an object whose constructor is T(arg1, arg2)
template <typename T, typename A1, typename A2>
inline T* get_object(const A1& arg1, const A2& arg2) {
    return ObjectPool<T>::singleton()->get_object(arg1, arg2);
}

// Return the object |ptr| back. The object is NOT destructed and will be
// returned by later get_object<T>. Similar with free/delete, validity of
// the object is not checked, user shall not return a not-yet-allocated or
// already-returned object otherwise behavior is undefined.
// Returns 0 when successful, -1 otherwise.
template <typename T> inline int return_object(T* ptr) {
    return ObjectPool<T>::singleton()->return_object(ptr);
}

// Reclaim all allocated objects typed T if caller is the last thread called
// this function, otherwise do nothing. You rarely need to call this function
// manually because it's called automatically when each thread quits.
template <typename T> inline void clear_objects() {
    ObjectPool<T>::singleton()->clear_objects();
}

// Get description of objects typed T.
// This function is possibly slow because it iterates internal structures.
// Don't use it frequently like a "getter" function.
template <typename T> ObjectPoolInfo describe_objects() {
    return ObjectPool<T>::singleton()->describe_objects();
}

}  // namespace baidu

#endif  // BUTIL_OBJECT_POOL_H
