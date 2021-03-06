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
// Date: Wed Aug 14 07:42:39 CST 2019

#ifndef TESLA_TUTIL_EXTENSIONS_H_
#define TESLA_TUTIL_EXTENSIONS_H_

#include <string>
#include <unordered_map>
#include <mutex>
#include <functional>

#include "tutil/synchronization/fast_pthread_mutex.h"

namespace tesla {
namespace tutil {

template <typename T>
class Extension {
 public:
  // Note: Don't call GetInstance() in destructor of other static objects.
  static Extension* GetInstance();
  void RegisterOrDir(const char* name, T* default_instance);
  T* Find(const char* name);
 private:
  Extension();
  ~Extension();
 private:
  std::unordered_map<const char*, T*> map_;
  FastPthreadMutex map_mutex_;
};

}  // namespace tutil
}  // namespace tesla

#include "tutil/extension_inl.h"

#endif  // TESLA_TUTIL_EXTENSIONS_H_
