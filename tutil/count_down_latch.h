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
// Date: Fri Apr 12 22:50:22 CST 2019

#ifndef TESLA_TUTIL_COUNTDOWNLATCH_H_
#define TESLA_TUTIL_COUNTDOWNLATCH_H_

#include <mutex>
#include <condition_variable>
#include "tutil/macros.h"

namespace tesla {
namespace tutil {

class CountDownLatch {
 public:
  explicit CountDownLatch(int count)
    : count_(count) {} 
  
  ~CountDownLatch() = default;

  DISALLOW_COPY_AND_ASSIGN(CountDownLatch);

  void Wait()
  {
    std::unique_lock<std::mutex> lock(mutex_); 
    // method 1:
    //while (count_)
    //{
    //  condition_.wait(lock);
    //}

    // method 2:
    condition_.wait(lock, [&]{ return count_ == 0; });
  }

  void CountDown()
  {
    std::lock_guard<std::mutex> lock(mutex_);

    --count_;
    if (!count_)
    {
      condition_.notify_all();
    }
  }

 private:
  mutable std::mutex mutex_;
  std::condition_variable condition_; 
  int count_;
};

} // namespace tutil
} // namespace tesla

#endif // TESLA_TUTIL_COUNTDOWNLATCH_H_
