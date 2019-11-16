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
// Date: Fri Nov 15 22:59:57 CST 2019

#ifndef TESLA_ALLOCATOR_METADATA_ALLOCATOR_H_
#define TESLA_ALLOCATOR_METADATA_ALLOCATOR_H_

#include <cstddef>
#include <cstdint>

namespace tesla {
namespace allocator {

// Return pointer pointed to a chunk of memory with a size of bytes
// if success, NULL otherwise.
void* MetaDataAlloc(size_t bytes);

// Return number of bytes taken from system.
uint64_t metadata_system_bytes();

}  // namespace allocator
}  // namespace tesla

#endif  // TESLA_ALLOCATOR_METADATA_ALLOCATOR_H_
