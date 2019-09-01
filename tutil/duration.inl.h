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

#ifndef TESLA_TUTIL_DURATIONS_INL_H_
#define TESLA_TUTIL_DURATIONS_INL_H_

namespace tesla {
namespace tutil {

inline Duration::Duration(int64_t nanoseconds)
    : nanoseconds_(nanoseconds) {}

inline Duration::Duration(double seconds)
    : nanoseconds_(static_cast<int64_t>(seconds * kSecond)) {}

inline int64_t Duration::Nanoseconds() const {
  return nanoseconds_; 
}

inline double Duration::Microseconds() const {
  return static_cast<double>(nanoseconds_) / kMicrosecond; 
}

inline double Duration::Milliseconds() const {
  return static_cast<double>(nanoseconds_) / kMillisecond; 
}

inline double Duration::Seconds() const {
  return static_cast<double>(nanoseconds_) / kSecond; 
}

inline double Duration::Minutes() const {
  return static_cast<double>(nanoseconds_) / kMinute; 
}

inline double Duration::Hours() const {
  return static_cast<double>(nanoseconds_) / kHour; 
}

inline bool Duration::IsZero() const {
  return nanoseconds_ == 0;
}

inline struct timeval Duration::TimeVal() const {
  struct timeval t; 
  To(&t);
  return t;
}

inline void Duration::To(struct timeval* t) const {
  t->tv_sec = static_cast<long>(nanoseconds_ / kSecond);
  t->tv_usec = static_cast<long>(nanoseconds_ % kSecond) /
               static_cast<long>(kMicrosecond);
}

inline struct timespec Duration::TimeSpec() const {
  struct timespec t;
  To(&t);
  return t;
}

inline void Duration::To(struct timespec* t) const {
  t->tv_sec = static_cast<time_t>(nanoseconds_ / kSecond);
  t->tv_nsec = static_cast<long>(nanoseconds_ % kSecond);
}

inline bool Duration::operator< (const Duration& rhs) const {
  return nanoseconds_ < rhs.nanoseconds_;
}

inline bool Duration::operator<=(const Duration& rhs) const {
  return nanoseconds_ <= rhs.nanoseconds_;
}

inline bool Duration::operator> (const Duration& rhs) const {
  return nanoseconds_ > rhs.nanoseconds_;
}

inline bool Duration::operator>=(const Duration& rhs) const {
  return nanoseconds_ >= rhs.nanoseconds_;
}

inline bool Duration::operator==(const Duration& rhs) const {
  return nanoseconds_ == rhs.nanoseconds_;
}

inline Duration& Duration::operator+=(const Duration& rhs) {
  nanoseconds_ += rhs.nanoseconds_;
  return *this;
}

inline Duration& Duration::operator-=(const Duration& rhs) {
  nanoseconds_ -= rhs.nanoseconds_;
  return *this;
}

inline Duration& Duration::operator*=(int n) {
  nanoseconds_ *= n;
  return *this;
}

inline Duration& Duration::operator/=(int n) {
  nanoseconds_ /= n;
  return *this;
}

} // namespace tutil
} // namespace tesla

#endif // TESLA_TUTIL_DURATIONS_INL_H_
