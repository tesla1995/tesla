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

#include "tutil/synchronization/fast_pthread_mutex.h"

#include <errno.h>
#include <atomic>

#include "tutil/synchronization/sys_futex.h"

namespace tesla {
namespace tutil {

struct MutexInternal {
  std::atomic<char> locked;
  std::atomic<char> contended;
  uint16_t padding;
};
static_assert(sizeof(MutexInternal) == sizeof(int),
                "size of MutexInternal must equal to size of int");
static_assert(sizeof(std::atomic<int>) == sizeof(int),
                "size of std::atomic<int> must equal to size of int");

constexpr MutexInternal MUTEX_LOCKED_RAW = {{1}, {0}, 0};
constexpr MutexInternal MUTEX_CONTENDED_RAW = {{1}, {1}, 0};

#define TUTIL_LOCKED_RAW (*(const int*)(&MUTEX_LOCKED_RAW))
#define TUTIL_CONTENDED_RAW (*(const int*)(&MUTEX_CONTENDED_RAW))

int FastPthreadMutex::lock_contended()  {
  std::atomic<int>* whole = (std::atomic<int>*)(&futex_word_);
  while (whole->exchange(TUTIL_CONTENDED_RAW) & TUTIL_LOCKED_RAW) {
    if (futex_wait_private(&futex_word_, TUTIL_CONTENDED_RAW) < 0 &&
        errno != EAGAIN) {
      return -1;
    }
  }
  return 0;
}

void FastPthreadMutex::lock() {
  MutexInternal* split = (MutexInternal*)(&futex_word_);
  if (split->locked.exchange(ACQUIRED, std::memory_order_acquire) != RELEASED) {
    (void)lock_contended();
  }
}

bool FastPthreadMutex::trylock() {
  MutexInternal* split = (MutexInternal*)(&futex_word_);
  return split->locked.exchange(ACQUIRED, std::memory_order_acquire) == RELEASED;
}

void FastPthreadMutex::unlock() {
  std::atomic<int>* whole = (std::atomic<int>*)(&futex_word_);
  if (whole->exchange(RELEASED, std::memory_order_release) != TUTIL_LOCKED_RAW) {
    futex_wake_private(&futex_word_, 1);
  }
}

} // namespace tutil
} // namespace tesla
