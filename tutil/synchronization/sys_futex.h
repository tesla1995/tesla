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

#ifndef TESLA_TUTIL_SYNCHRONIZATION_SYS_FUTEX_H_
#define TESLA_TUTIL_SYNCHRONIZATION_SYS_FUTEX_H_

#include <unistd.h>       // syscall
#include <syscall.h>      // SYS_futex
#include <linux/futex.h>  // FUTEX_WAIT, FUTEX_WAKE

namespace tesla {
namespace tutil {

int futex_wait_private(int *uaddr, int expected_value) {
  return syscall(SYS_futex, uaddr, FUTEX_WAIT_PRIVATE,
                 expected_value, NULL, NULL, 0);
}

int futex_wake_private(int *uaddr, int waiters_value) {
  return syscall(SYS_futex, uaddr, FUTEX_WAKE_PRIVATE, waiters_value,
                 NULL, NULL, 0);
}

} // namespace tutil
} // namespace tesla

#endif // TESLA_TUTIL_SYNCHRONIZATION_SYS_FUTEX_H_
