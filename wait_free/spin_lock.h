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
// Date: Sat Jul  6 10:02:13 CST 2019

#ifndef TESLA_WAIT_FREE_HAZARD_SPIN_LOCK_H_
#define TESLA_WAIT_FREE_HAZARD_SPIN_LOCK_H_

#include <atomic>
#include <thread>
#include "wait_free/common.h"

namespace tesla {
namespace wait_free {

// The purpose of a spin lock is to prevent multiple threads from
// concurrently accessing a shared data structure.
class SpinLock {
 public:
  SpinLock() = default;
  ~SpinLock() = default;

  void Lock() {
    while (flag_.test_and_set(std::memory_order_acquire)) {
      std::this_thread::yield();
    }
  }

  void Unlock() {
    flag_.clear(std::memory_order_release);
  }

 private:
  HAZARD_CACHELINE_ALIGNMENT std::atomic_flag flag_;
};

}  // wait_free
}  // tesla

#endif  // TESLA_WAIT_FREE_HAZARD_SPIN_LOCK_H_
