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
// Date: Sat Apr  6 21:32:10 CST 2019

#ifndef TESLA_TVAR_DETAIL_IS_ATOMICAL_H_
#define TESLA_TVAR_DETAIL_IS_ATOMICAL_H_

#include <type_traits>

namespace tesla {
namespace tvar {
namespace detail {

template <typename T>
struct is_atomical : std::integral_constant<bool, (std::is_integral<T>::value ||
                                                  std::is_floating_point<T>::value)
                                           > {};

template <typename T>
struct is_atomical<const T> : is_atomical<T> {};

template <typename T>
struct is_atomical<volatile T> : is_atomical<T> {};

template <typename T>
struct is_atomical<const volatile T> : is_atomical<T> {};

} // namespace detail
} // namespace tvar
} // namespace tesla

#endif //TESLA_TVAR_DETAIL_IS_ATOMICAL_H_
