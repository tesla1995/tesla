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
// Date: Sat Jul  6 09:52:52 CST 2019

#ifndef TESLA_WAIT_FREE_COMMON_H_
#define TESLA_WAIT_FREE_COMMON_H_

#if __cplusplus < 201103L
  #error "Should use C++11 implementation"
#endif

#define HAZARD_CACHELINE_SIZE 64
#define HAZARD_CACHELINE_ALIGNMENT alignas(HAZARD_CACHELINE_SIZE)

#define PAUSE() asm("pause\n")

#endif  // TESLA_WAIT_FREE_COMMON_H_
