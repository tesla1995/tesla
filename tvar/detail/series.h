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
// Date: Sun Apr 21 12:40:43 CST 2019

#ifndef TESLA_TVAR_DETAIL_SERIES_H_
#define TESLA_TVAR_DETAIL_SERIES_H_

#include <math.h>
#include <type_traits>
#include <mutex>
#include <ostream>
#include "tvar/detail/call_op_returning_void.h"

namespace tesla {
namespace tvar {
namespace detail {

// Judge whether Op can be used to add two values of type T.
// It works for integral/floating point.
template <typename T, typename Op>
struct ProbablyAddition {
  ProbablyAddition(const Op& op) {
    T res(32); 
    call_op_returning_void(op, res, T(64));
    ok_ = (res == T(96));
  };

  operator bool() const { return ok_; }

 private:
  bool ok_{false};
};

template <typename T, typename Op, typename Enable = void>
struct DivideOnAddition {
  static void inplace_divide(T& /*obj*/, const Op*, int /*number*/) {
    // do nothing in default.
  }
};

template <typename T, typename Op>
struct DivideOnAddition <T, Op, typename std::enable_if<
                                  std::is_integral<T>::value>::type>
{
  static void inplace_divide(T& obj, const Op* op, int number) {
    // static object is to avoid constructing or destructing
    // the same obj repeatly.
    static ProbablyAddition<T, Op> probably_add(op);
    if (probably_add) {
      obj = (T)round(obj / (double)number);
    }
  }
};

template <typename T, typename Op>
struct DivideOnAddition <T, Op, typename std::enable_if<
                                  std::is_floating_point<T>::value>::type>
{
  static void inplace_divide(T& obj, const Op* op, int number) {
    // static object is to avoid constructing or destructing
    // the same obj repeatly.
    static ProbablyAddition<T, Op> probably_add(op);
    if (probably_add) {
      obj /= number;
    }
  }
};

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

template <typename T, typename Op>
class SeriesBase {
 public:
  explicit SeriesBase(const Op& op) : op_(op) {}
  ~SeriesBase() = default;

  void Append(const T& value) {
    std::lock_guard<std::mutex> guard(mutex_);
    AppendSecond(value); 
  }
  
 private:
  void AppendSecond(const T& value);
  void AppendMinute(const T& value);
  void AppendHour(const T& value);
  void AppendDay(const T& value);

  class Data {
   public:
    Data() {
      if (std::is_pod<T>::value) {
        memset(array_, 0, sizeof(array_));
      }
    }

    T& Second(int index) { return array_[index]; }
    const T& Second(int index) const { return array_[index]; }

    T& Minute(int index) { return array_[60 + index]; }
    const T& Minute(int index) const { return array_[60 + index]; }

    T& Hour(int index) { return array_[120 + index]; }
    const T& Hour(int index) const { return array_[120 + index]; }

    T& Day(int index) { return array_[144 + index]; }
    const T& Day(int index) const { return array_[144 + index]; }

   private:
    T array_[60 + 60 + 24 + 30];
  };

 protected:
  Op op_;
  mutable std::mutex mutex_;
  char next_second_{0};  // 0 ~ 59 
  char next_minute_{0};  // 0 ~ 59
  char next_hour_{0};    // 0 ~ 23
  char next_day_{0};     // 0 ~ 29
  Data data_;
};

template <typename T, typename Op>
void SeriesBase<T, Op>::AppendSecond(const T& value) {
  data_.Second(next_second_) = value;
  ++next_second_;
  if (next_second_ >= 60) {
    next_second_ = 0;
    T result = data_.Second(0);
    for (int i = 1; i < 60; ++i) {
      call_op_returning_void(op_, result, data_.Second(i));
    }
    DivideOnAddition<T, Op>::inplace_divide(result, op_, 60);
    AppendMinute(result, op_);
  }
}

template <typename T, typename Op>
void SeriesBase<T, Op>::AppendMinute(const T& value) {
  data_.Minute(next_minute_) = value; 
  ++next_minute_;
  if (next_minute_ >= 60) {
    next_minute_ = 0;
    T result = data_.Minute(0);
    for (int i = 1; i < 60; ++i) {
      call_op_returning_void(op_, result, data_.Minute(i));
    }
    DivideOnAddition<T, Op>::inplace_divide(result, op_, 60);
    AppendHour(result, op_); 
  }
}

template <typename T, typename Op>
void SeriesBase<T, Op>::AppendHour(const T& value) {
  data_.Hour(next_hour_) = value;
  ++next_hour_;
  if (next_hour_ >= 24) {
    next_hour_ = 0;
    T result = data_.Hour(0);
    for (int i = 1; i < 24; ++i) {
      call_op_returning_void(op_, result, data_.Hour(i));
    }
    DivideOnAddition<T, Op>::inplace_divide(result, op_, 24);
    AppendDay(result, op_); 
  }
}

template <typename T, typename Op>
void SeriesBase<T, Op>::AppendDay(const T& value) {
  data_.Day(next_day_) = value;
  ++next_day_;
  if (next_day_ >= 30) {
    next_day_ = 0;
  }
}

template <typename T, typename Op>
class Series : public SeriesBase<T, Op> {
 public:
  explicit Series(const Op& op) : Base(op) {}
  void Describe(std::ostream& os, const std::string* name) const;

 private:
  using Base = SeriesBase<T, Op>;
};

template <typename T, typename Op>
void Series<T, Op>::Describe(std::ostream& os,
                             const std::string* name) const {
  // TODO(tesla): check name not null ? 
  
  this->mutex_.lock();
  const int second_begin = this->next_second_;
  const int minute_begin = this->next_minute_;
  const int hour_begin = this->next_hour_;
  const int day_begin = this->next_day_;
  this->mutex_.unlock();

  int c = 0;
  os << "{\"label\":\"trend\",\"data\":[";
  for (int i = 0; i < 30; ++i, ++c) {
    if (c) {
      os << ',';
    }
    os << '['  << c << ',' << this->data_.Day((i + day_begin) % 30) << ']';
  }

  for (int i = 0; i < 24; ++i, ++c) {
    if (c) {
      os << ',';
    }
    os << '['  << c << ',' << this->data_.Hour((i + hour_begin) % 24) << ']';
  }

  for (int i = 0; i < 60; ++i, ++c) {
    if (c) {
      os << ',';
    }
    os << '['  << c << ',' << this->data_.Minute((i + minute_begin) % 60) << ']';
  }

  for (int i = 0; i < 60; ++i, ++c) {
    if (c) {
      os << ',';
    }
    os << '['  << c << ',' << this->data_.Second((i + second_begin) % 60) << ']';
  }
  os << "]}";
}

} // namespace detail
} // namespace tvar
} // namespace tesla


#endif // TESLA_TVAR_DETAIL_SERIES_H_
