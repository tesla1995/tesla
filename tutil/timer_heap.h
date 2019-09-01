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

#ifndef TESLA_TUTIL_TIMER_HEAP_H_
#define TESLA_TUTIL_TIMER_HEAP_H_

#include <vector>
#include <queue>
#include <functional>
#include <utility>
#include <atomic>
#include <memory>
#include <mutex>
#include <unordered_map>

#include "tutil/timestamp.h"

// A high-performance timer implemented with the min binary heap.
// 
// Example:
//   TimerHeap timer;
//   TimerId id = timer.AddTimer(Timestamp::Now + Duration(3.0),
//                               []{ std::cout << "timeout" << std::endl; });
//
//   // After 3.0 second,
//   if (timer.HasNextTimeout()) {
//     timer.ExecuteNextTimeout();
//   }
//
//   // If you want to remove it before timing out.
//   timer.RemoveTimer(id);
//
// Note: Before calling ExecuteNextTimeout(), you need to call HasNextTimeout() to
// check whether the timer timeout.
//
// Thread-safe:
//   TimerHeap is intened to run in process whose every thread has a single
//   event loop, so it's member functions are not thread-safe
//   which can't directly used in multi-thread processes.
namespace tesla {
namespace tutil {

class TimerObject;
using TimerObjectPtr = std::unique_ptr<TimerObject>;

using TimerId = int64_t;
using TimerCallback = std::function<void()>;

class TimerHeap {
 public:
  TimerHeap();
  ~TimerHeap();

  TimerId AddTimer(Timestamp expiration_time, const TimerCallback& callback);

  void RemoveTimer(TimerId id);

  bool HasNextTimeout();
  void ExecuteNextTimeout();

  // used to determine sleep time in epoll.
  // Return
  //   >0: next timeout.
  //   0 : timer timeout.
  //   -1: empty
  int GetNextTimeout();

  // only for debug.
  void Traversal(); 

 private:
  // Fix heap from bottom up.
  void FixUp(int index);

  // Fix heap from top down.
  void FixDown(int index);

  // swap two elements in `heap_' and their indexes in `indexes_'.
  void Swap(int from, int to) noexcept;
  void SwapToLast(int from) noexcept;

  // Delete the last element in `heap_' and it's index in `indexes_'.
  TimerObjectPtr DeleteLastElement();


  std::vector<TimerObjectPtr> heap_;
  std::unordered_map<TimerId, int64_t> indexes_;
  std::queue<TimerObjectPtr> free_list_;  // reuse TimerObject.
};

} // tutil
} // namespace tesla

#endif // TESLA_TUTIL_TIMER_HEAP_H_
