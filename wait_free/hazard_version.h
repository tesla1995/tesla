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
// Date: Sat Jul  6 10:28:59 CST 2019

#ifndef TESLA_WAIT_FREE_HAZARD_VERSION_H_
#define TESLA_WAIT_FREE_HAZARD_VERSION_H_

#include <stdlib.h>
#include <stdio.h>
#include <limits>
#include <atomic>

#include "wait_free/common.h"
#include "wait_free/util.h"
//#include "wait_free/spin_lock.h"
#include "tutil/timestamp.h"

namespace tesla {
namespace wait_free {
namespace hazard_version {

class ThreadLocalStorage;

// TODO(tesla): why not use `struct' ?
class HazardNode {
 friend class hazard_version::ThreadLocalStorage;

 public:
  HazardNode()
      : version_(std::numeric_limits<uint64_t>::max()),
        next_(nullptr) {}

  virtual ~HazardNode() {}
 public:
  virtual void Retire() = 0;
 private:
   void set_version(uint64_t version) { version_ = version; }
   uint64_t version() const { return version_; }
   void set_next(HazardNode* next) { next_ = next; }
   HazardNode* next() const { return next_; }
   HazardNode*& mutable_next() { return next_; }
 private:
  uint64_t version_;
  HazardNode* next_;
};

class DummyHazardNode : public HazardNode {
 public:
  void Retire() {};
};

// Note:
//   1. handle must be released in the thread which it was acquired.
//   2. release->release and acquire->acquire order is not allowded.
struct VersionHandle {
  union {
    struct {
      uint16_t tid;  // always used in the same thread
      uint16_t _;    // placeholder
      uint32_t seq;  // constraint order
    };
    uint64_t u64;
  };
  VersionHandle(uint64_t uv) : u64(uv) {}
};

class ThreadLocalStorage {
 public:
  ThreadLocalStorage();
  ~ThreadLocalStorage();
 public:
  void set_enabled(const uint16_t tid);
  bool enabled() const;
  uint16_t tid() const;

  void set_next(ThreadLocalStorage* next);
  ThreadLocalStorage* next() const;
  ThreadLocalStorage*& mutable_next();

  void AbortNotInSameThread(uint16_t tid);

  // Always called from the thread owning the `ThreadLocalStorage'.
  int Acquire(const uint64_t version, VersionHandle& handle);

  // Always called from the thread owning the `ThreadLocalStorage'.
  void Release(VersionHandle& handle);

  // Always called from the thread owning the `ThreadLocalStorage'.
  int AddNode(const uint64_t version, HazardNode* node);
  
  // [ThreadSafe] May be called from anywhere.
  int64_t Retire(const uint64_t version, ThreadLocalStorage& node_receiver);

  int64_t hazard_waiting_count() const;
  uint64_t current_version() const;

 private:
   bool ReceiverIsSelf(const ThreadLocalStorage& node_receiver);
   void AddNodes(HazardNode* head, HazardNode* tail, const int64_t count);
 private:
  bool enabled_;
  uint16_t tid_;  // identifies which thread this object belong to.
  uint64_t last_retire_version_;

  // Put variables that work together into the same cacheline.
  struct HAZARD_CACHELINE_ALIGNMENT
  {
    uint32_t current_seq_;
    uint64_t current_version_;
  };
  
  HAZARD_CACHELINE_ALIGNMENT std::atomic<HazardNode*> hazard_waiting_list_;
  // Note: moidifying `hazard_waiting_list_' and `hazard_waiting_count_' is
  // not an atomic operation, so `hazard_waiting_count_' may be negative.
  HAZARD_CACHELINE_ALIGNMENT std::atomic<int64_t> hazard_waiting_count_;
  HAZARD_CACHELINE_ALIGNMENT ThreadLocalStorage* next_;
};

///////////////////////////////////////////////////////////////////////////////

template <uint16_t MaxThreadCount>
class HazardVersionT {
 public:
  HazardVersionT(const int64_t thread_waiting_threshold = 64,
                 const int64_t min_version_cache_timeus = 200000);
  ~HazardVersionT();
 public:
  int AddNode(HazardNode* node);
  int Acquire(uint64_t& handle);
  void Release(const uint64_t& handle);
  void Retire();
  int64_t hazard_waiting_count() const;
 private:
   int GetThreadLocalStorage(hazard_version::ThreadLocalStorage*& ts);
   uint64_t min_version(const bool force_flush);
 private:
  int64_t thread_waiting_threshold_;
  int64_t min_version_cache_timeus_;

  HAZARD_CACHELINE_ALIGNMENT std::atomic<uint64_t> global_version_;

  //HAZARD_CACHELINE_ALIGNMENT SpinLock thread_lock_;

  // inner object is aligned.
  hazard_version::ThreadLocalStorage threads_[MaxThreadCount];
  std::atomic<hazard_version::ThreadLocalStorage*> thread_list_;
  std::atomic<int64_t> thread_count_;

  HAZARD_CACHELINE_ALIGNMENT std::atomic<int64_t> hazard_waiting_count_;

  HAZARD_CACHELINE_ALIGNMENT std::atomic<uint64_t> current_min_version_;
  HAZARD_CACHELINE_ALIGNMENT std::atomic<int64_t> current_min_version_timestamp_;
};

constexpr static uint16_t kMaxThreadCount = 1024;
using HazardVersion = HazardVersionT<kMaxThreadCount>;

///////////////////////////////////////////////////////////////////////////////

ThreadLocalStorage::ThreadLocalStorage()
    : enabled_(false),
      tid_(0),
      last_retire_version_(0),
      current_seq_(0),
      current_version_(std::numeric_limits<uint64_t>::max()),
      hazard_waiting_list_(nullptr),
      hazard_waiting_count_(0),
      next_(nullptr) {
}

ThreadLocalStorage::~ThreadLocalStorage() {
  // FIXME: no free?
  HazardNode* list = hazard_waiting_list_.load();
  while (nullptr != list) {
    list->Retire();
    list = list->next();
  }
}

void ThreadLocalStorage::set_enabled(const uint16_t tid) {
  enabled_ = true;
  tid_ = tid;  // Set which thread this object belongs to
}

bool ThreadLocalStorage::enabled() const {
  return enabled_;
}

uint16_t ThreadLocalStorage::tid() const {
  return tid_;
}

void ThreadLocalStorage::set_next(ThreadLocalStorage* next) {
  next_ = next;
}

ThreadLocalStorage* ThreadLocalStorage::next() const {
  return next_;
}

ThreadLocalStorage*& ThreadLocalStorage::mutable_next() {
  return next_;
}

void ThreadLocalStorage::AbortNotInSameThread(uint16_t tid) {
  if (tid != GetCurrentThreadId()) {
    fprintf(stderr, "tid[%u] != current thread id[%ld]\n", tid, GetCurrentThreadId());
    abort();
  }
}

int ThreadLocalStorage::Acquire(const uint64_t version,
                                VersionHandle& handle) {
  AbortNotInSameThread(tid_);
  int ret = 0;
  if (current_version_ != std::numeric_limits<uint64_t>::max()) {
      fprintf(stderr,
              "current thread has already assigned a version handle, seq=%u\n",
              current_seq_);
      ret = -1;
  } else {
    current_version_ = version;
    handle.tid = tid_;
    handle._ = 0;
    handle.seq = current_seq_;
  }
  return ret;
}

void ThreadLocalStorage::Release(VersionHandle& handle) {
  AbortNotInSameThread(tid_);
  if (handle.tid != tid_ && handle.seq != current_seq_) {
    fprintf(stderr, "invalid handle, seq=%u tid=%hu\n", handle.seq, handle.tid);
  } else {
    current_version_ = std::numeric_limits<uint64_t>::max();
    current_seq_++;
  }
}

int ThreadLocalStorage::AddNode(const uint64_t version, HazardNode* node) {
  AbortNotInSameThread(tid_);
  int ret = 0;
  node->set_version(version);
  AddNodes(node, node, 1);
  return ret;
}

int64_t ThreadLocalStorage::Retire(const uint64_t version,
                                   ThreadLocalStorage& node_receiver) {
  // If node_receiver is self, check whether current thread id is the same as
  // `tid_'.
  if (ReceiverIsSelf(node_receiver)) {
    AbortNotInSameThread(tid_);
  }
  
  if (last_retire_version_ == version) {
    return 0;
  }
  last_retire_version_ = version;

  HazardNode* current = hazard_waiting_list_.exchange(nullptr);
  
  HazardNode* list2retire = nullptr;
  int64_t retire_count = 0;
  int64_t move_count = 0;
  DummyHazardNode pseudo_head;
  pseudo_head.set_next(current);
  HazardNode* iter = &pseudo_head;
  while (iter->next() != nullptr) {
    // Note: `=' is not allowd!
    if (iter->next()->version() < version) {
      // delete nodes from old waiting list and add them to retire list.
      retire_count++;
      HazardNode* tmp = iter->next();
      iter->set_next(iter->next()->next());

      tmp->set_next(list2retire);
      list2retire = tmp;
    } else {
      move_count++;
      iter = iter->next();
    }
  }

  // move retire list to node_receiver.
  HazardNode* move_list_head = nullptr;
  HazardNode* move_list_tail = nullptr;
  if (nullptr != (move_list_head = pseudo_head.next())) {
    move_list_tail = iter;
  }
  node_receiver.AddNodes(move_list_head, move_list_tail, move_count);
  hazard_waiting_count_.fetch_add(-(retire_count + move_count));

  // Retire ndoes which no other threads access.
  while (nullptr != list2retire) {
    HazardNode* current = list2retire;
    list2retire = list2retire->next();
    // FIXME: no free?
    current->Retire();
    delete current;
  }

  return retire_count;
}

int64_t ThreadLocalStorage::hazard_waiting_count() const {
  return hazard_waiting_count_.load();
}

uint64_t ThreadLocalStorage::current_version() const {
  return current_version_;
}

void ThreadLocalStorage::AddNodes(HazardNode* head, HazardNode* tail,
                                 const int64_t count) {
  AbortNotInSameThread(tid_);
  if (0 < count) {
    HazardNode* old = hazard_waiting_list_.load();
    tail->set_next(old);
    while (!hazard_waiting_list_.compare_exchange_weak(tail->mutable_next(), head));
    hazard_waiting_count_.fetch_add(count);
  }
}

bool ThreadLocalStorage::ReceiverIsSelf(const ThreadLocalStorage& node_receiver) {
  return (this == &node_receiver);
}

///////////////////////////////////////////////////////////////////////////////

template <uint16_t MaxThreadCount>
HazardVersionT<MaxThreadCount>::HazardVersionT(
    const int64_t thread_waiting_threshold,
    const int64_t min_version_cache_timeus)
    : thread_waiting_threshold_(thread_waiting_threshold),
      min_version_cache_timeus_(min_version_cache_timeus),
      global_version_(0),
      thread_list_(nullptr),
      thread_count_(0),
      hazard_waiting_count_(0),
      current_min_version_(0),
      current_min_version_timestamp_(0) {

}

template <uint16_t MaxThreadCount>
HazardVersionT<MaxThreadCount>::~HazardVersionT() {
  Retire();
  fprintf(stdout, "~HazardVersionT\n");
}

template <uint16_t MaxThreadCount>
int HazardVersionT<MaxThreadCount>::Acquire(uint64_t& handle) {
  int ret = 0;
  
  hazard_version::ThreadLocalStorage* tls = nullptr;
  if (0 != (ret = GetThreadLocalStorage(tls))) {
    fprintf(stderr, "GetThreadLocalStorage fail, ret=%d\n", ret);
  } else {
    while (true) {
      const uint64_t version = global_version_.load();
      hazard_version::VersionHandle version_handle(0);
      if (0 != (ret = tls->Acquire(version, version_handle))) {
        fprintf(stderr, "tls Acquire fail, ret=%d\n", ret);
        break;
      } else if (version != global_version_.load()) {
        tls->Release(version_handle);
      } else {
        handle = version_handle.u64;
        break;
      }
    }
  }
  return ret;
}

template <uint16_t MaxThreadCount>
int HazardVersionT<MaxThreadCount>::AddNode(HazardNode* node) {
  int ret = 0;
  hazard_version::ThreadLocalStorage* tls = nullptr;

  if (nullptr == node) {
    fprintf(stderr, "invalid parameter, node nullptr\n");
    return -1;
  } else if (0 != (ret = GetThreadLocalStorage(tls))) {
    fprintf(stderr, "GetThreadLocalStorage fail, ret=%d\n", ret);
  } else if (0 != (ret = tls->AddNode(global_version_.fetch_add(1), node))) {
    fprintf(stderr, "tls AddNode fail, ret=%d\n", ret);
  } else {
    hazard_waiting_count_.fetch_add(1);
  }
  return ret;
}

template <uint16_t MaxThreadCount>
void HazardVersionT<MaxThreadCount>::Release(const uint64_t& handle) {
  hazard_version::VersionHandle version_handle(handle);
  if (MaxThreadCount > version_handle.tid) {
    hazard_version::ThreadLocalStorage* tls = &threads_[version_handle.tid];
    tls->Release(version_handle);
    if (thread_waiting_threshold_ < tls->hazard_waiting_count()) {
      uint64_t version = min_version(false);
      int64_t retire_count = tls->Retire(version, *tls);
      hazard_waiting_count_.fetch_add(-retire_count);
    } else if (thread_waiting_threshold_ * thread_count_.load() <
        hazard_waiting_count_.load()) {
      Retire();
    }
  }
}

template <uint16_t MaxThreadCount>
int HazardVersionT<MaxThreadCount>::GetThreadLocalStorage(
  hazard_version::ThreadLocalStorage*& tls) {
  int ret = 0;
  uint16_t thread_id = static_cast<uint16_t>(GetCurrentThreadId());

  if (MaxThreadCount <= thread_id) {
    fprintf(stderr, "thread number overflow, thread_id=%hu\n", thread_id);
    return -1;
  } else {
    tls = &threads_[thread_id];
    if (!tls->enabled()) {
        tls->set_enabled(thread_id);
        tls->set_next(thread_list_.load());
        while (!thread_list_.compare_exchange_weak(tls->mutable_next(), tls));
        thread_count_.fetch_add(1);
        fprintf(stdout, "thread_count_[%ld]\n", thread_count_.load());
    }
  }
  return ret;
}

template <uint16_t MaxThreadCount>
uint64_t HazardVersionT<MaxThreadCount>::min_version(const bool force_flush) {
  uint64_t min_version = 0;
  // To use the cache, you need to satisfy three conditions as follw:
  //   1. no forced reflush.
  //   2. `current_min_version_' was initialized.
  //   3. `current_min_version_' has not expired.
  if (!force_flush
      && 0 != (min_version = current_min_version_.load())
      && (current_min_version_timestamp_.load() + min_version_cache_timeus_)
          > tutil::Timestamp::Now().UnixMicroseconds()) {
    // from cache
  } else {
    hazard_version::ThreadLocalStorage* iter = thread_list_.load();
    while (nullptr != iter) {
      uint64_t tls_version = iter->current_version();
      if (min_version > tls_version) {
        min_version = tls_version;
      }
      iter = iter->next();
    }
    current_min_version_.store(min_version);
    current_min_version_timestamp_.store(
        tutil::Timestamp::Now().UnixMicroseconds());
  }

  return min_version;
}

template <uint16_t MaxThreadCount>
void HazardVersionT<MaxThreadCount>::Retire() {
  int ret = 0;
  hazard_version::ThreadLocalStorage* tls = nullptr;
  if (0 != (ret = GetThreadLocalStorage(tls))) {
    fprintf(stderr, "GetThreadLocalStorage fail, ret=%d\n", ret);
  } else {
    uint64_t version = min_version(true);

    int64_t retire_count = tls->Retire(version, *tls);
    hazard_waiting_count_.fetch_add(-retire_count);

    hazard_version::ThreadLocalStorage* iter = thread_list_.load();
    while (nullptr != iter) {
      if (tls != iter) {
        // Note: add unretired nodes of other threads to retire list of local thread.
        int64_t retire_count = iter->Retire(version, *tls);
        hazard_waiting_count_.fetch_add(-retire_count);
      }
      iter = iter->next();
    }
  }
}

template <uint16_t MaxThreadCount>
int64_t HazardVersionT<MaxThreadCount>::hazard_waiting_count() const {
  return hazard_waiting_count_.load();
}

}  // hazard_version
}  // wait_free
}  // tesla

#endif  // TESLA_WAIT_FREE_HAZARD_VERSION_H_
