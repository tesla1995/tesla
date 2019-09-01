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

#ifndef TESLA_TUTIL_TIMESTAMP_H_
#define TESLA_TUTIL_TIMESTAMP_H_

#include <stdint.h>
#include <sys/time.h>
#include <time.h>

#include "tutil/duration.h"

namespace tesla {
namespace tutil {

class Timestamp {
 public:
  Timestamp() = default; 

  explicit Timestamp(int64_t nanoseconds);
  explicit Timestamp(const struct timespec& t);
  explicit Timestamp(const struct timeval& t);
  
  ~Timestamp() = default;
  
  // returns the current local time.
  static Timestamp Now();

  // returns a invalid timestamp.
  static Timestamp Invalid();

  bool IsValid() const;

  struct timespec TimeSpec() const;
  void To(struct timespec* t) const;

  struct timeval TimeVal() const;
  void To(struct timeval* t) const;

  void Add(Duration d);

  int64_t UnixNanoseconds() const;
  int64_t UnixMicroseconds() const;
  int64_t UnixMilliseconds() const;
  int64_t UnixSeconds() const;

  bool operator< (const Timestamp& rhs) const;
  bool operator> (const Timestamp& rhs) const;
  bool operator==(const Timestamp& rhs) const;

  Timestamp  operator+ (const Duration& rhs) const;
  Timestamp& operator+=(const Duration& rhs);
  Timestamp  operator- (const Duration& rhs) const;
  Timestamp& operator-=(const Duration& rhs);

  Duration   operator- (const Timestamp& rhs) const;
  
 private:
  // `nanoseconds_' gives the number of nanoseconds elapsed since the Epoch
  // 1970-01-01 00:00:00 +0000(UTC).
  int64_t nanoseconds_{0};
};

} // namespace tutil
} // namespace tesla

#include "tutil/timestamp.inl.h"

#endif // TESLA_TUTIL_TIMESTAMP_H_
