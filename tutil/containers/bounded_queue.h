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
// Date: Mon Apr 15 19:48:33 CST 2019

#ifndef TESLA_TUTIL_CONTAINERS_BOUNDED_QUEUE_H_
#define TESLA_TUTIL_CONTAINERS_BOUNDED_QUEUE_H_

#include <stdlib.h>
#include <stddef.h>
#include <utility>
#include <iostream>

#include "tutil/macros.h"

namespace tesla {
namespace tutil {

// A thread-unsafe bounded queue(ring buffer).
// It can push/pop from both sides.
//
// Example
// [Create a on-heap queue]
//   char* p = (char*)malloc(1024);
//   if (p == NULL) {
//     return false;
//   }
//   BoundedQueue<int> queue(p, 1024, StorageOwnership::kOwnsStorage);
//   queue.push_back(1);
//   queue.push_back(2);
//
// [Initialize a class-member queue]
//   class Foo {
//     ...
//     size_t capacity_;
//     BoundedQueue<int> queue_;
//   };
//
//   bool Foo::Init() {
//     BoundedQueue<int> tmp(capacity_); 
//     if (!tmp.initialized()) {
//       return false;
//     }
//     tmp.swap(queue_); 
//     return true;
//   }

enum class StorageOwnership {
  kOwnsStorage,
  kNotOwnStorage
};

template<typename T>
class BoundedQueue {
 public:
  BoundedQueue() = default;

  BoundedQueue(char* memory, size_t capacity, StorageOwnership ownership)
      : memory_(memory),
        capacity_(capacity / sizeof(T)),
        ownership_(ownership) {}

  BoundedQueue(size_t capacity)
      : memory_((char*)malloc(capacity * sizeof(T))),
        capacity_(capacity),
        ownership_(StorageOwnership::kOwnsStorage) {}

  ~BoundedQueue() {
    clear();

    if (ownership()) {
      free(memory_);
      memory_ = nullptr;
    }
  }

  DISALLOW_COPY_AND_ASSIGN(BoundedQueue);  

  bool initialized() {
    return memory_ != nullptr;
  }

  bool ownership() {
    return ownership_ == StorageOwnership::kOwnsStorage;
  }

  size_t size() {
    return size_;
  }

  size_t capacity() {
    return capacity_;
  }

  bool empty() {
    return size_ == 0;
  }

  bool full() {
    return size_ == capacity_;
  }

  T* front() {
    return empty() ? nullptr : ((T*)memory_ + start_);
  }

  // Randomly access element from front side.
  // front(0) == front(), front(size()-1) == back()
  // Returns nullptr if |index| is out of range.
  T* front(size_t index) {
    return out_of_range(index) ? nullptr : ((T*)memory_ + mod(start_ + index, capacity_));
  }

  T* back() {
    return empty() ? nullptr : ((T*)memory_ + mod(start_ + size_ - 1, capacity_));
  }

  // Randomly access element from back side.
  // back(0) == back(), back(size()-1) == front()
  // Returns nullptr if |index| is out of range.
  T* back(size_t index) {
    return out_of_range(index) ?
           nullptr : ((T*)memory_ + mod(start_ + size_ - 1 - index, capacity_));
  }

  // Pushs a new element into back side of this queue constructed in-place
  // with the given args. The constructor of the new element is called with
  // exactly the same args as supplied to push_back, forwarded via 
  // `std::forward<Args>(args)...'.
  //
  // Returns true on success, false if queue is full.
  template <typename... Args>
  bool push_back(Args... args) {
    if (!full()) {
      new ((T*)memory_ + mod(start_ + size_, capacity_)) T(std::forward<Args>(args)...);
      size_++;
      return true;
    }
    return false;
  }

  // Pushs a new element into front side of this queue constructed in-place
  // with the given args. The constructor of the new element is called with
  // exactly the same args as supplied to push_back, forwarded via 
  // `std::forward<Args>(args)...'.
  //
  // Returns true on success, false if queue is full.
  template <typename... Args>
  bool push_front(Args... args) {
    if (!full()) {
      start_ = (start_ == 0) ? (capacity_ - 1) : (start_ - 1);
      new ((T*)memory_ + start_) T(std::forward<Args>(args)...);
      size_++;
      return true;
    }
    return false;
  }

  // Push a new element into back side of this queue. If the queue is full,
  // pop frontmost element first.
  void eliminate_push(const T& element) {
    if (!full()) {
      new ((T*)memory_ + mod(start_ + size_, capacity_)) T(element);
      size_++;
    } else {
      // trick.
      ((T*)memory_)[start_] = element;  
      start_ = mod(start_ + 1, capacity_);
    }
  }

  // Pops back element from this queue.
  // Returns true on success, false if queue is empty.
  bool pop_back() {
    if (!empty()) {
      T* p = (T*)memory_ + mod(start_ + size_ - 1, capacity_);  
      p->T::~T();
      size_--;
      return true;
    }
    return false;
  }

  // Pops back element from this queue and copy into |value|.
  // Returns true on success, false if queue is empty.
  bool pop_back(T& value) {
    if (!empty()) {
      T* p = (T*)memory_ + mod(start_ + size_ - 1, capacity_);  
      value = *p;
      p->T::~T();
      size_--;
      return true;
    }
    return false;
  }

  // Pops front element from this queue.
  // Returns true on success, false if queue is empty.
  bool pop_front() {
    if (!empty()) {
      ((T*)memory_)[start_].~T();
      start_ = mod(start_ + 1, capacity_);
      size_--;
      return true;
    }
    return false;
  }

  // Pops front element from this queue and copy into |value|.
  // Returns true on success, false if queue is empty.
  bool pop_front(T& value) {
    if (!empty()) {
      value = ((T*)memory_)[start_];
      ((T*)memory_)[start_].~T();
      start_ = mod(start_ + 1, capacity_);
      size_--;
      return true;
    }
    return false;
  }

  void clear() {
    for (size_t i = 0; i < size(); i++) {
      ((T*)memory_ + mod(start_ + i, capacity_))->~T();
    }
    size_ = 0;
    start_ = 0;
  }

  void swap(BoundedQueue& rhs) {
    std::swap(memory_, rhs.memory_);
    std::swap(capacity_, rhs.capacity_);
    std::swap(size_, rhs.size_);
    std::swap(start_, rhs.start_);
    std::swap(ownership_, rhs.ownership_);
  }

 private:
  // mod() is faster than operator % because most |off| are smaller
  // than |capacity|.
  size_t mod(size_t off, size_t capacity) {
    while (off >= capacity) {
      off -= capacity;
    } 
    return off;
  }

  bool out_of_range(size_t index) {
    return index >= size_;
  }

  char* memory_{nullptr};
  size_t capacity_{0};
  size_t size_{0};
  size_t start_{0};
  StorageOwnership ownership_{StorageOwnership::kNotOwnStorage};
};

} // namespace tutil
} // namespace tesla

#endif // TESLA_TUTIL_CONTAINERS_BOUNDED_QUEUE_H_

