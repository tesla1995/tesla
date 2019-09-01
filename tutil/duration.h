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
// Date: Wed Apr  3 00:25:15 CST 2019

#ifndef TESLA_TUTIL_DURATIONS_H_
#define TESLA_TUTIL_DURATIONS_H_

#include <stdint.h>
#include <time.h>

namespace tesla {
namespace tutil {

// A Duration represents the elapsed time between two time point
// with an nanosecond count.
//
// Example:
//   Duration d(3.5);     // nanoseconds_ = 3.5 * 1000000000L;
//   Duration d(5000000); // nanoseconds_ = 5000000;
class Duration {
 public:
  static constexpr int64_t kNanosecond = 1LL;
  static constexpr int64_t kMicrosecond = 1000;
  static constexpr int64_t kMillisecond = 1000 * kMicrosecond;
  static constexpr int64_t kSecond = 1000 * kMillisecond;
  static constexpr int64_t kMinute = 60 * kSecond;
  static constexpr int64_t kHour = 24 * kMinute;

 public:
  Duration() = default;
  explicit Duration(int64_t nanoseconds);
  explicit Duration(double seconds);

  ~Duration() = default;

  // These methods return double because the dominant
  // use case is for printing a floating point number
  // like 1.5s.
  
  int64_t Nanoseconds() const;
  double Microseconds() const;
  double Milliseconds() const;
  double Seconds() const;
  double Minutes() const;
  double Hours() const;

  struct timeval TimeVal() const;
  void To(struct timeval* t) const;

  struct timespec TimeSpec() const;
  void To(struct timespec* t) const;

  bool IsZero() const;
  bool operator< (const Duration& rhs) const;
  bool operator<=(const Duration& rhs) const;
  bool operator> (const Duration& rhs) const;
  bool operator>=(const Duration& rhs) const;
  bool operator==(const Duration& rhs) const;

  Duration& operator+=(const Duration& rhs);
  Duration& operator-=(const Duration& rhs);
  Duration& operator*=(int n);
  Duration& operator/=(int n);
 
 private:
  int64_t nanoseconds_{0};
};

} // namespace tutil
} // namespace tesla

#include "tutil/duration.inl.h"

#endif // TESLA_TUTIL_DURATIONS_H_
