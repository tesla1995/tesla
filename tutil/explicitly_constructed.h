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
// Date: Wed Sep  4 23:00:17 CST 2019

#ifndef TESLA_TUTIL_EXPLICITLY_CONSTRUCTED_H_
#define TESLA_TUTIL_EXPLICITLY_CONSTRUCTED_H_

#include <type_traits>
#include <utility>

namespace tesla {
namespace tutil {

// This type wraps a variable whose constructor and destructor are explicitly
// called. It is particularly useful for a global variable, without its
// constructor and destructor run on start and end of the program lifetime.
// This circumvents the initial construction order fiasco, while keeping
// the address of the empty string a compile time constant.
//
// Pay special attention to the initialization state of the object.
// 1. The object is "uninitialized" to begin with.
// 2. Call Construct() or DefaultConstruct() only if the object is
//    uninitialized. After the call, the object becomes "initialized".
// 3. Call get() and get_mutable() only if the object is initialized.
// 4. Call Destruct() only if the object is initialized.
//    After the call, the object becomes uninitialized.
template <typename T>
class ExplicitlyConstructed {
 public:
  void DefaultConstruct() { new (&buf_) T(); }  

  template <typename... Args>
  void Construct(Args&&... args) {
    new (&buf_) T(std::forward<Args>(args)...);
  }

  const T& get() { return reinterpret_cast<const T&>(buf_); }

  T* get_mutable() { return reinterpret_cast<T*>(&buf_); }

  void Destruct() { get_mutable()->~T(); }

 private:
  // The alignment value of type `BufferType' is consistent with type `T'.
  using BufferType = std::aligned_storage_t<sizeof(T), alignof(T)>;
  BufferType buf_;
};

} // namespace tutil
} // namespace tesla

#endif // TESLA_TUTIL_EXPLICITLY_CONSTRUCTED_H_
