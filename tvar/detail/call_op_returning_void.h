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
// Date: Sat Apr  6 21:53:57 CST 2019

#ifndef TESLA_TVAR_DETAIL_CALL_OP_RETURNING_VOID_H_
#define TESLA_TVAR_DETAIL_CALL_OP_RETURNING_VOID_H_

namespace tesla {
namespace tvar {
namespace detail {

// For performance issue, we don't let Op return value, instead it shall
// set the result to the first parameter in-place. Namely to add two values,
// "+=" should be implemented rather than "+".
template <typename Operator, typename T1, typename T2>
inline void call_op_returning_void(const Operator& op, T1& v1, T2& v2) {
  return op(v1, v2);
}

} // namespace detail
} // namespace tvar
} // namespace tesla

#endif // TESLA_TVAR_DETAIL_CALL_OP_RETURNING_VOID_H_
