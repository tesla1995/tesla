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
// Date: Sat Apr 13 00:31:49 CST 2019

#ifndef TESLA_TUTIL_TYPE_TRAITS_H_
#define TESLA_TUTIL_TYPE_TRAITS_H_

#include <type_traits>

namespace tesla {
namespace tutil {

template <typename T>
struct add_cr_non_integral {
  using type = typename std::conditional<std::is_integral<T>::value, T,
    typename std::add_lvalue_reference<typename std::add_const<T>::type>::type>::type;
};

// Calculate base^exponent.
template <unsigned base, unsigned exponent>
struct Power {
  enum {value = (exponent % 2 == 0) ?
                Power<Power<base, exponent / 2>::value, 2>::value :
                base * Power<Power<base, (exponent - 1) / 2>::value, 2>::value
       };
};
template <unsigned base>
struct Power<base, 0> {
  enum {value = 1};
};
template <unsigned base>
struct Power<base, 1> {
  enum {value = base};
};
template <unsigned base>
struct Power<base, 2> {
  enum {value = base * base};
};

} // namespace tutil
} // namespace tesla

#endif // TESLA_TUTIL_TYPE_TRAITS_H_
