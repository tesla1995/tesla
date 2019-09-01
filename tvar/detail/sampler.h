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
// Date: Sun Apr 14 00:01:21 CST 2019

#ifndef TESLA_TVAR_DETAIL_SAMPLER_H_
#define TESLA_TVAR_DETAIL_SAMPLER_H_

#include <mutex>

#include "tutil/timestamp.h"
#include "tutil/containers/linked_list.h"
#include "log/logging.h"

namespace tesla {
namespace tvar {
namespace detail {

template <typename T>
struct Sample {
  T data;
  tutil::Timestamp time; 

  Sample() = default;
  Sample(const T& data2, tutil::Timestamp time2) : data(data2), time(time2) {}
};

// The base class for all samplers whose take_sample() are called periodically.
class Sampler : public tutil::LinkNode<Sampler> {
 friend class SamplerCollector;

 public:
  Sampler();
  virtual ~Sampler();

  // This function will be called every second(approximately) in
  // dedicated thread if schedule() is called.
  virtual void TakeSample() = 0;

  // Register this sampler globally so that TakeSample() will be called
  // periodically.
  void Schedule();

  // Call this function instead of delete operator to destroy the sampler.
  // Deletion of the sampler may be delayed for seconds.
  void Destroy();

 protected:
  bool used_; 
  std::mutex mutex_; // used to synchronize Destroy() and TakeSample().
};

// Representing a non-existing operator so that we can test
// is_same<Op, VoidOp>::value to write code for different branches.
// The false branch should be removed by compiler at compile-time.
struct VoidOp {
  template <typename T>
  T operator()(const T&, const T&) const {
    LOG_FATAL << "This function should never be called, abort";
  };
};

} // namespace detail
} // namespace tvar
} // namespace tesla

#endif // TESLA_TVAR_DETAIL_SAMPLER_H_
