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
// Date: Wed Aug 14 07:42:58 CST 2019

#ifndef TESLA_TUTIL_EXTENSIONS_INL_H_
#define TESLA_TUTIL_EXTENSIONS_INL_H_

namespace tesla {
namespace tutil {

template <typename T>
Extension<T>& Extension<T>::GetInstance() {
  // a fast, thread-safe singleton.
  static Extension<T> extension;
  return extension;
}

template <typename T>
void Extension<T>::RegisterOrDir(const std::string& name, const CreateFunc& create_func) {
  if (!name.empty()) {
    // if name was used, exit.
    if (map_.insert(std::make_pair(name, create_func)).second) {
      return;
    } 
  }
  exit(1);
}

template <typename T>
bool Extension<T>::Find(const std::string& name, CreateFunc& create_func) const {
  if (!name.empty()) {
    auto iter = map_.find(name);
    if (iter != map_.end()) {
      create_func = iter->second;
      return true;
    }
  }
  return false;
}

}  // namespace tutil
}  // namespace tesla

#endif  // TESLA_TUTIL_EXTENSIONS_INL_H_
