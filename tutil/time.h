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

#ifndef TESLA_TUTIL_TIME_H_
#define TESLA_TUTIL_TIME_H_

#include <stdint.h>
#include <time.h>

namespace tesla {
namespace tutil {

inline int64_t clock_ns() {
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  return now.tv_sec * 1000000000L + now.tv_nsec;
}

// Count elapses
class Timer {
 public:
  Timer() = default;
  ~Timer() = default;

  // Start this timer.
  void start() {
    start_ = clock_ns();
    stop_ = start_;
  }

  // Stop this timer
  void stop() {
    stop_ = clock_ns();
  }

  // Get the elapse from start() to stop(), in various units.
  int64_t n_elapsed() const { return stop_ - start_; }
  int64_t u_elapsed() const { return n_elapsed() / 1000L; }
  int64_t m_elapsed() const { return u_elapsed() / 1000L; }
  int64_t s_elapsed() const { return m_elapsed() / 1000L; }

  double n_elapsed(double) const { return static_cast<double>(stop_ - start_); }
  double u_elapsed(double) const { return static_cast<double>(n_elapsed()) / 1000.0; }
  double m_elapsed(double) const { return static_cast<double>(u_elapsed()) / 1000.0; }
  double s_elapsed(double) const { return static_cast<double>(m_elapsed()) / 1000.0; }

 private:
  int64_t start_{0};
  int64_t stop_{0};
};

} // namespace tutil
} // namespace tesla

#endif // TESLA_TUTIL_TIME_H_
