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
// Date: Fri Sep  6 00:54:00 CST 2019

#ifndef TESLA_TUTIL_COMM_H_
#define TESLA_TUTIL_COMM_H_

namespace tesla {
namespace tutil {

template <typename T>
void tesla_destruct_object(void* object) {
  reinterpret_cast<T*>(object)->~T();
}

template <typename T>
void tesla_delete_object(void* object) {
  delete reinterpret_cast<T*>(object);
}

}  // namespace tutil
}  // namespace tesla

#include "tutil/extension_inl.h"

#endif  // TESLA_TUTIL_COMM_H_
