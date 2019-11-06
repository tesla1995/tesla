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
// Date: Tue Nov  5 21:45:50 CST 2019

#ifndef TESLA_TUTIL_PACKED_CACHE_H_
#define TESLA_TUTIL_PACKED_CACHE_H_

#include <stdint.h>

#include "glog/logging.h"
#include "tutil/compiler_specific.h"

// A safe way of doing "(1 << n) - 1" -- without worrying about overflow.
#define N_ONES_(IntType, N)    \
  ( (N) == 0 ? 0 : ((static_cast<IntType>(1) << ((N)-1))-1 +  \
                    (static_cast<IntType>(1) << ((N)-1))) )

namespace tesla {
namespace tutil {

template <int kKeybits>
class PackedCache {
 public:
  using T = uintptr_t;
  using K = uintptr_t;
  using V = uint32_t;

  static constexpr int kHashbits = 16;
  static constexpr int kValuebits = 7;
  static constexpr int kInvalidMask = 0x80;

  PackedCache() {
    static_assert(kKeybits + kValuebits + 1 <= 8 * sizeof(T), "use whole keys");
    static_assert(kHashbits <= kKeybits, "hash function");
    static_assert(kHashbits >= kValuebits + 1, "small value space");
    Clear();
  }

  void Clear() {
    memset(const_cast<T*>(array_), kInvalidMask, sizeof(array_));
  }

  void Put(K key, V value) {
    CHECK(key == (key & kKeyMask)) << "The key is out of range"; 
    CHECK(value == (value & kValueMask)) << "The value is out of range"; 
    array_[Hash(key)] = KeyToUpper(key) | value;
  }

  void Invalidate(K key) {
    CHECK(key == (key & kKeyMask)) << "The key is out of range"; 
    array_[Hash(key)] = KeyToUpper(key) | kInvalidMask;
  }

  bool TryGet(K key, V* out) const {
    CHECK(key == (key & kKeyMask)) << "The key is out of range"; 
    T entry = array_[Hash(key)] ^ KeyToUpper(key);
    if (TESLA_UNLIKELY(entry >= (1 << kValuebits))) {
      return false;
    }
    *out = static_cast<V>(entry);
    return true;
  }

 private:
  // we just clear lower kHashbits of the key.
  static T KeyToUpper(K key) {
    return static_cast<T>(key) ^ Hash(key);
  }
  
  // we simply takes the low kHashbits of the key.
  static T Hash(K key) {
    return static_cast<T>(key) & N_ONES_(T, kHashbits);
  }

  // For masking a K.
  static constexpr K kKeyMask = N_ONES_(K, kKeybits);

  // For masking a V or a T.
  static constexpr V kValueMask = N_ONES_(K, kValuebits);

  volatile T array_[kHashbits];
};

}  // tutil
}  // tesla

#undef N_ONES_  // #define N_ONES_

#endif  // TESLA_TUTIL_PACKED_CACHE_H_
