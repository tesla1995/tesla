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
// Date: Sun Sep  1 17:23:22 CST 2019

#ifndef TESLA_TUTIL_SYNCHRONIZATION_FAST_PTHREAD_MUTEX_H_
#define TESLA_TUTIL_SYNCHRONIZATION_FAST_PTHREAD_MUTEX_H_

#include <atomic>

namespace tesla {
namespace tutil {

class FastPthreadMutex {
 public:
  enum LockState {
    RELEASED,
    ACQUIRED,
  };
 public:
  FastPthreadMutex() = default;
  ~FastPthreadMutex() = default;
 public:
  void lock();
  bool trylock(); 
  void unlock(); 
 private:
  int lock_contended();
 private:
  int futex_word_{RELEASED};
};

static_assert(sizeof(FastPthreadMutex) == 4,
              "size of std::atomic<int> must equal to 4");
} // namespace tutil
} // namespace tesla

#endif // TESLA_TUTIL_SYNCHRONIZATION_FAST_PTHREAD_MUTEX_H_
