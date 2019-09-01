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

#ifndef TESLA_TUTIL_TIMESTAMP_INL_H_
#define TESLA_TUTIL_TIMESTAMP_INL_H_

namespace tesla {
namespace tutil {

inline Timestamp::Timestamp(int64_t nanoseconds)
    : nanoseconds_(nanoseconds) {}

// `struct timespec' is specified in <time.h>
//   struct timespec {
//     time_t   tv_sec;        /* seconds */
//     long     tv_nsec;       /* nanoseconds */
//   };
inline Timestamp::Timestamp(const struct timespec& t)
    : nanoseconds_(static_cast<int64_t>(t.tv_sec) * Duration::kSecond +
                   t.tv_nsec) {}

// `struct timespec' is specified in <sys/time.h>
//    struct timeval {
//      time_t      tv_sec;     /* seconds */
//      suseconds_t tv_usec;    /* microseconds */
//    };
inline Timestamp::Timestamp(const struct timeval& t)
    : nanoseconds_(static_cast<int64_t>(t.tv_sec) * Duration::kSecond +
                   t.tv_usec * Duration::kMicrosecond) {}

inline Timestamp Timestamp::Now() {
  // gettimeofday is fast than clock_gettime, but its
  // resolution is lower than clock_gettime's.
  struct timeval now;
  gettimeofday(&now, NULL);
  return Timestamp(now);
}

inline Timestamp Timestamp::Invalid() {
  return Timestamp();
}

inline void Timestamp::Add(Duration d) {
  nanoseconds_ += d.Nanoseconds();
}

inline bool Timestamp::IsValid() const {
  return nanoseconds_ > 0;
}

inline void Timestamp::To(struct timeval* t) const {
  t->tv_sec = static_cast<time_t>(nanoseconds_ / Duration::kSecond); 
  t->tv_usec = static_cast<suseconds_t>(nanoseconds_ % Duration::kSecond) /
               static_cast<suseconds_t>(Duration::kMicrosecond);
}

inline struct timeval Timestamp::TimeVal() const {
  struct timeval t; 
  To(&t); 
  return t;
}

inline void Timestamp::To(struct timespec* t) const {
  t->tv_sec = static_cast<time_t>(nanoseconds_ / Duration::kSecond); 
  t->tv_nsec = static_cast<long>(nanoseconds_ % Duration::kSecond);
}

inline struct timespec Timestamp::TimeSpec() const {
  struct timespec t; 
  To(&t); 
  return t;
}

int64_t Timestamp::UnixNanoseconds() const {
  return nanoseconds_;
}

int64_t Timestamp::UnixMicroseconds() const {
  return nanoseconds_ / Duration::kMicrosecond;
}

int64_t Timestamp::UnixMilliseconds() const {
  return nanoseconds_ / Duration::kMillisecond;
}

int64_t Timestamp::UnixSeconds() const {
  return nanoseconds_ / Duration::kSecond;
}

bool Timestamp::operator< (const Timestamp& rhs) const {
  return nanoseconds_ < rhs.nanoseconds_;
}

bool Timestamp::operator> (const Timestamp& rhs) const {
  return nanoseconds_ > rhs.nanoseconds_;
}

bool Timestamp::operator==(const Timestamp& rhs) const {
  return nanoseconds_ == rhs.nanoseconds_;
}

Timestamp Timestamp::operator+ (const Duration& rhs) const {
  return Timestamp(nanoseconds_ + rhs.Nanoseconds());
}

Timestamp& Timestamp::operator+=(const Duration& rhs) {
  nanoseconds_ += rhs.Nanoseconds();
  return *this;
}

Timestamp Timestamp::operator- (const Duration& rhs) const {
  return Timestamp(nanoseconds_ - rhs.Nanoseconds());
}

Timestamp& Timestamp::operator-=(const Duration& rhs) {
  nanoseconds_ -= rhs.Nanoseconds();
  return *this;
}

Duration Timestamp::operator- (const Timestamp& rhs) const {
  return Duration(nanoseconds_ - rhs.nanoseconds_);
}

} // namespace tutil
} // namespace tesla

#endif // TESLA_TUTIL_TIMESTAMP_INL_H_
