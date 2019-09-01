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
// Date: Sun Apr 21 23:43:48 CST 2019

#ifndef TESLA_TVAR_REDUCER_H_
#define TESLA_TVAR_REDUCER_H_

#include "tvar/variable.h"
#include "tvar/detail/sampler.h"
#include "tvar/detail/series.h"
#include "tvar/detail/combiner.h"

#include "tutil/type_traits.h"
#include "tutil/compiler_specific.h"

#include "log/logging.h"

namespace tesla {
namespace tvar {

template <typename T, typename Op, typename InvOp = detail::VoidOp>
class Reducer : public Variable {
 public:
  using combiner_type = typename detail::AgentCombiner<T, T, Op>;
  using agent_type    = typename combiner_type::Agent;
  // TODO(tesla): implement ReducerSampler.
  //using sampler_type  = detail::ReducerSampler<Reducer, T, Op, InvOp>;
  
  class SeriesSampler : public detail::Sampler {
   public:
    SeriesSampler(Reducer* owner, const Op& op)
        : owner_(owner), series_(op) {} 
 
    ~SeriesSampler() = default;

    void TakeSample() { series_.Append(owner_->GetValue()); }

    void Describe(std::ostream& os) { series_.Describe(os); }

   private:
    Reducer* owner_{nullptr};
    detail::Series<T, Op> series_;
  };

 public:
  Reducer(typename tutil::add_cr_non_integral<T>::type identity = T(),
          const Op& op = Op(),
          const InvOp& inv_op = InvOp())
      : combiner_(identity, identity, op),
        series_sampler_(NULL),
        inv_op_(inv_op) {
  } 

  ~Reducer() {
    //hide();
    //if (series_sampler_) {
    //  series_sampler_->Destroy();
    //  series_sampler_ = nullptr;
    //}
  }

  // Add a value.
  // Returns self reference for chaining.
  Reducer& operator<<(typename tutil::add_cr_non_integral<T>::type value);

  // Get reduced value.
  // Notice that this function walks through threads that ever add values
  // into this reducer. You should avoid calling it frequently.
  T GetValue() const {
    //CHECK(!(std::is_same<InvOp, detail::VoidOp>::value));
    return combiner_.CombineAllAgents();
  }

  // Reset the reduced value to T().
  // Returns the reduced value before reset.
  T Reset() { return combiner_.ResetAllAgents(); }

  // Implement this method to print the variable into ostream.
  void describe(std::ostream& os, bool quote_string) const override {
    if (std::is_same<T, std::string>::value && quote_string) {
      os << '"' << GetValue() << '"';
    } else {
      os << GetValue();
    }
  }

  // True if this reducer is constructed successfully.
  bool Valid() const { return combiner_.Valid(); }

  // Get instance of Op.
  const Op& op() const { return combiner_.op(); }
  const InvOp& inv_op() const { return inv_op_; }

 private:
  combiner_type combiner_;
  //sampler_type* sampler_;
  SeriesSampler* series_sampler_;
  InvOp inv_op_;
}; // class Reducer

template <typename T, typename Op, typename InvOp>
inline Reducer<T, Op, InvOp>& Reducer<T, Op, InvOp>::operator<<(
    typename tutil::add_cr_non_integral<T>::type value) {
  agent_type* agent = combiner_.GetOrCreateTlsAgent();
  if (TESLA_UNLIKELY(agent == nullptr)) {
    LOG_ERROR << "Fail to create agent";
    return *this;
  }
  agent->element.modify(combiner_.op(), value);
  return *this;
}

// ========================= common reducers ====================
namespace detail {

template <typename T>
struct AddTo {
  void operator()(T& lhs,
      typename tutil::add_cr_non_integral<T>::type rhs) const {
    lhs += rhs;
  }
};

template <typename T>
struct MinusFrom {
  void operator()(T& lhs,
      typename tutil::add_cr_non_integral<T>::type rhs) const {
    lhs -= rhs;
  }
};

}  // namespace detail

template <typename T>
class Adder : public Reducer<T, detail::AddTo<T>, detail::MinusFrom<T>> {
 public:
  using Base = Reducer<T, detail::AddTo<T>, detail::MinusFrom<T>>;
  using value_type = T;
 public:
  Adder() : Base() {}
  ~Adder() {}
};

}  // namespace tvar
}  // namespace tesla

#endif  // TESLA_TVAR_REDUCER_H_
