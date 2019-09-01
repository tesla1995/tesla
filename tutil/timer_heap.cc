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
// Date: Thu Apr  4 17:46:09 CST 2019

#include "tutil/timer_heap.h"
#include "log/logging.h"
#include "tutil/compiler_specific.h" // TESLA_LIKELY,TESLA_UNLIKELY

#include <iostream>

namespace tesla {
namespace tutil {

class TimerObject {
 public:
  TimerObject(Timestamp expiration_time, const TimerCallback& callback);

  TimerId id() { return id_; }
  Timestamp expiration_time() { return expiration_time_; }

  void run() { callback_(); }
  void clear();
  void reset(Timestamp expiration_time, const TimerCallback& callback);

  bool operator> (const TimerObject& obj) const;
  bool operator< (const TimerObject& obj) const;
  bool operator==(const TimerObject& obj) const;

 private:
  static std::atomic_int_fast64_t  global_id_;

  int64_t id_;
  Timestamp expiration_time_; 
  TimerCallback callback_; 
};

std::atomic_int_fast64_t TimerObject::global_id_{1};

TimerObject::TimerObject(Timestamp expiration_time, const TimerCallback& callback)
    : id_(global_id_++),
      expiration_time_(expiration_time),
      callback_(callback) {

}

void TimerObject::clear() {
  expiration_time_ = Timestamp::Invalid();
  callback_ = nullptr;
}

void TimerObject::reset(Timestamp expiration_time, const TimerCallback& callback) {
  expiration_time_ = expiration_time;
  callback_ = callback;
}

bool TimerObject::operator>(const TimerObject& obj) const {
  return expiration_time_ > obj.expiration_time_;
}

bool TimerObject::operator<(const TimerObject& obj) const {
  return expiration_time_ < obj.expiration_time_;
}

bool TimerObject::operator==(const TimerObject& obj) const {
  return expiration_time_ == obj.expiration_time_;
}

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

TimerHeap::TimerHeap() {

}

TimerHeap::~TimerHeap() {

}

void TimerHeap::Swap(int from, int to) noexcept {
  std::swap(heap_[from], heap_[to]); 
  std::swap(indexes_[heap_[from]->id()], indexes_[heap_[to]->id()]);
}

void TimerHeap::SwapToLast(int from) noexcept {
  Swap(from, heap_.size() - 1);
}

// TimerHeap Uses min heap to store timer. Min heap is implemented with
// vector that starts from zero but not one.
//
// For node k in heap_(assume all node exist),
//   - the index of it's parent node is (k-1)/2, heap_[k] > heap_[(k-1)/2]
//   - the index of it's left child node is 2*k+1, heap_[k] <= heap_[2*k+1]
//   - the index of it's right child node is 2*k+2, heap_[k] <= heap_[2*k+2]
void TimerHeap::FixUp(int index) {
  int parent = 0;

  // When does the loop exit?
  // 1. `index' node is equal to or larger than it's parent node.
  // 2. reaches the top of `heap_'.
  while (index > 0) {
    parent = (index - 1) / 2;

    if (*heap_[parent] > *heap_[index]) {
      Swap(index, parent);
      index = parent;
    } else {
      break;
    }
  }
}

void TimerHeap::FixDown(int index) {
  int min_pos = 0;
  int left_child = 0;
  int right_child = 0;
  int max_pos = heap_.size() - 1;

  // When does the loop exit?
  // 1. `index' node is the min node.
  // 2. reaches the bottom of `heap_'.
  while (true) {
    min_pos = index;
    left_child = 2 * index + 1;
    right_child = 2 * index + 2; 

    if (left_child <= max_pos && *heap_[index] > *heap_[left_child]) {
      min_pos = left_child;
    }

    if (right_child <= max_pos && *heap_[min_pos] > *heap_[right_child]) {
      min_pos = right_child;
    }

    if (min_pos == index) {
      break;
    }

    Swap(index, min_pos);

    index = min_pos;
  }
}

TimerObjectPtr TimerHeap::DeleteLastElement() {
  TimerObjectPtr last;
  std::swap(last, heap_[heap_.size() - 1]);
  heap_.pop_back();
  indexes_.erase(last->id());
  return last;
}

TimerId TimerHeap::AddTimer(Timestamp expiration_time, const TimerCallback& callback) {
  TimerId id;
  TimerObjectPtr timer;

  if (free_list_.empty() == false) {
    std::swap(timer, free_list_.front());
    free_list_.pop();
    timer->reset(expiration_time, callback);
  } else {
    timer = std::make_unique<TimerObject>(expiration_time, callback);
    if (TESLA_UNLIKELY(timer == nullptr)) {
      LOG_FATAL << "TimerHeap::AddTimer std::make_unique<TimerObject>";
    }
  }
  
  id = timer->id();
  heap_.push_back(std::move(timer));
  indexes_[id] = heap_.size() - 1;

  return id;
}

void TimerHeap::RemoveTimer(TimerId id) {
  auto it = indexes_.find(id);
  if (it == indexes_.end()) {
    LOG_WARN << "invalid TimerId[" << id << "]";
    return;  // do nothing for invalid `id'.
  }

  int index = it->second;
  TimerObjectPtr last;

  if (heap_.size() == 1) {
    last = DeleteLastElement();
  } else {
    SwapToLast(index);
    last = DeleteLastElement();

    if (*heap_[index] < *last) {
      FixUp(index);    
    } else if (*heap_[index] > *last) {
      FixDown(index);
    }
  }

  free_list_.push(std::move(last));
}

bool TimerHeap::HasNextTimeout() {
  if (heap_.empty() == false) {
    return Timestamp::Now() > heap_[0]->expiration_time();
  }
  return false;
}

int TimerHeap::GetNextTimeout() {
  if (heap_.empty() == false) {
    int timeout = heap_[0]->expiration_time().UnixSeconds() -
                  Timestamp::Now().UnixSeconds();
    if (timeout < 0) {
      timeout = 0;
    }
    return timeout;
  }
  return -1;
}

void TimerHeap::ExecuteNextTimeout() {
  // TODO(tesla): Add check here ?
  
  auto& first = heap_[0];
  first->run();
  RemoveTimer(first->id());
}

void TimerHeap::Traversal() {
  std::cout << "Traversal: ";
  for (auto& it : heap_) {
    std::cout << it->expiration_time().UnixSeconds() << " ";
  }
  std::cout << std::endl;
}

} // namespace tutil
} // namespace tesla
