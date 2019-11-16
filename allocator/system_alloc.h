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
// Date: Thu Nov 14 22:50:08 CST 2019

#ifndef TESLA_ALLOCATOR_SYSTEM_ALLOCATE_H_
#define TESLA_ALLOCATOR_SYSTEM_ALLOCATE_H_

#include <cstddef>
#include <cstdint>

#define BASE_ALIGNMENT_SIZE 64

namespace tesla {
namespace allocator {

// [Thread-safe]
void* TeslaMalloc_SystemAlloc(size_t bytes, size_t *actual_bytes,
        size_t alignment = 0);

// [Thread-safe]
bool TeslaMalloc_SystemRelease(void* addr, size_t length);

// [Thread-safe]
uint64_t TeslaMalloc_Taken();

}  // namespace allocator
}  // namespace tesla

#endif  // TESLA_ALLOCATOR_SYSTEM_ALLOCATE_H_
