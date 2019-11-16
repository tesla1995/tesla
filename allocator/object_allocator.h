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
// Date: Sat Nov 16 10:23:10 CST 2019

#ifndef TESLA_ALLOCATOR_OBJECT_ALLOCATOR_H_
#define TESLA_ALLOCATOR_OBJECT_ALLOCATOR_H_

#include <cstdio>   // for fprintf, stderr
#include <cstdlib>  // for abort
#include "allocator/metadata_allocator.h"  // for MetaDataAlloc

namespace tesla {
namespace allocator {

// External locking is required before accessing one of these objects.
template <typename T>
class ObjectAllocator {
 public:
  void Init() {
    static_assert(sizeof(T) >= sizeof(void*),
      "The size of type T is little than pointer");

    static_assert(sizeof(T) <= kAllocIncrement,
      "The size of type T is greater than maximum object allocated by ObjectAllocator");

    free_area_ = nullptr;
    free_avail_ = 0;
    free_list_ = nullptr;
    inuse_ = 0;
    // Reserve some space at the beginning to avoid fragmentation.
    Delete(New());
  }

  T* New() {
    void* result = nullptr;
    if (free_list_) {
      result = free_list_;
      free_list_ = *reinterpret_cast<void**>(free_list_);
    } else {
      if (free_avail_ < sizeof(T)) {
        free_area_ = reinterpret_cast<char*>(MetaDataAlloc(kAllocIncrement));
        if (free_area_ == nullptr) {
          // TODO: Print stacktrace to stderr.
          fprintf(stderr, "%s %d FATAL ERROR: Out of memory trying to "
                  "allocate bytes[%zd] for object-size[%zd]!", __FILE__,
                  __LINE__, kAllocIncrement, sizeof(T));
          abort();
        }
        free_avail_ = kAllocIncrement;
      }

      result = free_area_;
      free_area_ += sizeof(T); 
      free_avail_ -= sizeof(T);
    }
    inuse_++;
    return reinterpret_cast<T*>(result);
  }

  // Note that using void as the type of `p' is not type-safe.
  void Delete(T* p) {
    *reinterpret_cast<void**>(p) = free_list_;
    free_list_ = p;
    inuse_--;
  }

  int inuse() { return inuse_; }

 private:
  // How much to allocate from system at a time
  static constexpr size_t kAllocIncrement = 128 << 10;

  // Free area from which to carve new objects
  char* free_area_{ nullptr };
  size_t free_avail_{ 0 }; 

  // Free list of already carved objects
  void* free_list_{ nullptr };

  // Number of allocated but unfreed objects
  int inuse_{ 0 };
};

}  // namespace allocator
}  // namespace tesla

#endif  // TESLA_ALLOCATOR_OBJECT_ALLOCATOR_H_
