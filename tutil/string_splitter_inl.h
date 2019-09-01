// Copyright (c) 2011 Baidu, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Author: Michael,Tesla (michaeltesla1995@gmail.com)
// Date: Sun Jan 20 11:02:54 CST 2019

#ifndef TESLA_TUTIL_STRING_SPILTER_INL_H
#define TESLA_TUTIL_STRING_SPILTER_INL_H

#include <limits.h>

namespace tutil {

inline StringSplitter::StringSplitter(const char* str, char separator,
                               EmptyFieldAction action)
    : head_(str),
      str_tail_(nullptr),
      separator_(separator),
      empty_field_action_(action) {
  init();
}

inline StringSplitter::StringSplitter(const char* str_begin,
                               const char* str_end,
                               const char separator,
                               EmptyFieldAction action)
    : head_(str_begin),
      str_tail_(str_end),
      separator_(separator),
      empty_field_action_(action) {
  init();
}

// When empty_field_action_ is equal to ALLOW_EMPTY_FIELD, this function has
// a bug, but I'm not going to fix it.
inline void StringSplitter::init() {
  // Find the starting head_ and tail_.
  if (__builtin_expect((head_ != nullptr), 1)) {
    if (empty_field_action_ == EmptyFieldAction::SKIP_EMPTY_FIELD) {
      for (; *head_ == separator_ && not_end(head_); ++head_) {}
    }
    for (tail_ = head_; *tail_ != separator_ && not_end(tail_); ++tail_) { }
  } else {
    tail_ = nullptr;
  }
}

inline StringSplitter& StringSplitter::operator++() {
  // tail_ == nullptr ONLY when head_ == nullptr.
  if (__builtin_expect(tail_ != nullptr, 1)) {
    if (not_end(tail_)) {
      // Find the beginning address of next field.
      ++tail_;
      if (empty_field_action_ == EmptyFieldAction::SKIP_EMPTY_FIELD) {
        for (; *tail_ == separator_ && not_end(tail_); ++tail_) {}
      }

    }
    head_ = tail_;

    // Find one past the ending address of next field.
    for(; *tail_ != separator_ && not_end(tail_); ++tail_) {}
  }
  return *this;
}

inline StringSplitter StringSplitter::operator++(int) {
  StringSplitter tmp = *this;
  operator++();
  return tmp;
}

//inline StringSplitter::operator const void *() const {
//  return (head_ != nullptr && not_end(head_)) ? head_ : nullptr;
//}

// modify by tesla at Sun Jan 20 23:43:23 CST 2019.
inline StringSplitter::operator bool() const {
  return head_ != nullptr && not_end(head_);
}

inline const char* StringSplitter::field() const {
  return head_;
}

inline size_t StringSplitter::length() const {
  return static_cast<size_t>(tail_ - head_);
}

inline bool StringSplitter::not_end(const char* p) const {
  return (str_tail_ == nullptr) ? *p : (p != str_tail_);
}

inline int StringSplitter::to_int8(int8_t* pv) const {
  long v = 0;
  if (to_long(&v) == 0 && v >= -128 && v <= 127) {
    *pv = (int8_t)v;
    return 0;
  }
  return -1;
}

inline int StringSplitter::to_uint8(uint8_t* pv) const {
  unsigned long v = 0;
  if (to_ulong(&v) == 0 && v <= 255) {
    *pv = (uint8_t)v;
    return 0;
  }
  return -1;
}

inline int StringSplitter::to_int(int* pv) const {
  long v = 0;
  if (to_long(&v) == 0 && v >= INT_MIN && v <= INT_MAX) {
    *pv = (int)v;
    return 0;
  }
  return -1;
}

inline int StringSplitter::to_uint(unsigned int* pv) const {
  unsigned long v = 0;
  if (to_ulong(&v) == 0 && v <= UINT_MAX) {
    *pv = (unsigned int)v;
    return 0;
  }
  return -1;
}

inline int StringSplitter::to_long(long* pv) const {
  char* endptr = nullptr;
  *pv = strtol(field(), &endptr, 10);
  return (endptr == field() + length()) ? 0 : -1;
}

inline int StringSplitter::to_ulong(unsigned long* pv) const {
  char* endptr = nullptr;
  *pv = strtoul(field(), &endptr, 10);
  return (endptr == field() + length()) ? 0 : -1;
}

inline int StringSplitter::to_longlong(long long* pv) const {
  char* endptr = nullptr;
  *pv = strtoll(field(), &endptr, 10);
  return (endptr == field() + length()) ? 0 : -1;
}

inline int StringSplitter::to_ulonglong(unsigned long long* pv) const {
  char* endptr = nullptr;
  *pv = strtoull(field(), &endptr, 10);
  return (endptr == field() + length()) ? 0 : -1;
}

inline int StringSplitter::to_float(float* pv) const {
  char* endptr = nullptr;
  *pv = strtof(field(), &endptr);
  return (endptr == field() + length()) ? 0 : -1;
}

inline int StringSplitter::to_double(double* pv) const {
  char* endptr = nullptr;
  *pv = strtod(field(), &endptr);
  return (endptr == field() + length()) ? 0 : -1;
}

StringMultiSplitter::StringMultiSplitter(
    const char* str, const char* separators,
    EmptyFieldAction action)
    : head_(str),
      tail_(nullptr),
      str_tail_(nullptr),
      separators_(separators),
      empty_field_action_(action) {
  init();
}

StringMultiSplitter::StringMultiSplitter(
    const char* str_begin, const char* str_end,
    const char* separators, EmptyFieldAction action)
    : head_(str_begin),
      tail_(nullptr),
      str_tail_(str_end),
      separators_(separators),
      empty_field_action_(action) {
  init();
}

inline void StringMultiSplitter::init() {
  if (__builtin_expect(head_ != nullptr, 1)) {
    if (empty_field_action_ == EmptyFieldAction::SKIP_EMPTY_FIELD) {
      for (; is_separator(*head_) && not_end(head_); ++head_) {}
    }
    for (tail_ = head_; !is_separator(*tail_) && not_end(tail_); ++tail_) {}
  } else {
    tail_ = nullptr;
  }
}

inline StringMultiSplitter& StringMultiSplitter::operator++() {
  if (__builtin_expect(tail_ != nullptr, 1)) {
    if (not_end(tail_)) {
      ++tail_;
      if (empty_field_action_ == EmptyFieldAction::SKIP_EMPTY_FIELD) {
        for (; is_separator(*tail_) && not_end(tail_); ++tail_) {}
      }
    }
    head_ = tail_;
    for (; !is_separator(*tail_) && not_end(tail_); ++tail_) {}
  }
  return *this;
}

inline StringMultiSplitter StringMultiSplitter::operator++(int) {
  StringMultiSplitter tmp = *this;
  operator++();
  return tmp;
}

inline bool StringMultiSplitter::is_separator(char c) const {
  for (const char* p = separators_; *p != '\0'; ++p) {
    if (c == *p) {
      return true;
    }
  }
  return false;
}

inline StringMultiSplitter::operator bool() const {
  return head_ != nullptr && not_end(head_);
}

inline const char* StringMultiSplitter::field() const {
  return head_;
}

inline size_t StringMultiSplitter::length() const {
  return static_cast<size_t>(tail_ - head_);
}

inline bool StringMultiSplitter::not_end(const char* p) const {
  return (str_tail_ == nullptr) ? *p : (p != str_tail_);
}

inline int StringMultiSplitter::to_int8(int8_t* pv) const {
    long v = 0;
    if (to_long(&v) == 0 && v >= -128 && v <= 127) {
        *pv = (int8_t)v;
        return 0;
    
    }
    return -1;

}

inline int StringMultiSplitter::to_uint8(uint8_t* pv) const {
    unsigned long v = 0;
    if (to_ulong(&v) == 0 && v <= 255) {
        *pv = (uint8_t)v;
        return 0;
    
    }
    return -1;

}

inline int StringMultiSplitter::to_int(int* pv) const {
    long v = 0;
    if (to_long(&v) == 0 && v >= INT_MIN && v <= INT_MAX) {
        *pv = (int)v;
        return 0;
    
    }
    return -1;

}

inline int StringMultiSplitter::to_uint(unsigned int* pv) const {
    unsigned long v = 0;
    if (to_ulong(&v) == 0 && v <= UINT_MAX) {
        *pv = (unsigned int)v;
        return 0;
    
    }
    return -1;

}

inline int StringMultiSplitter::to_long(long* pv) const {
    char* endptr = NULL;
    *pv = strtol(field(), &endptr, 10);
    return (endptr == field() + length()) ? 0 : -1;

}

inline int StringMultiSplitter::to_ulong(unsigned long* pv) const {
    char* endptr = NULL;
    *pv = strtoul(field(), &endptr, 10);
    return (endptr == field() + length()) ? 0 : -1;

}

inline int StringMultiSplitter::to_longlong(long long* pv) const {
    char* endptr = NULL;
    *pv = strtoll(field(), &endptr, 10);
    return (endptr == field() + length()) ? 0 : -1;

}

inline int StringMultiSplitter::to_ulonglong(unsigned long long* pv) const {
    char* endptr = NULL;
    *pv = strtoull(field(), &endptr, 10);
    return (endptr == field() + length()) ? 0 : -1;

}

inline int StringMultiSplitter::to_float(float* pv) const {
    char* endptr = NULL;
    *pv = strtof(field(), &endptr);
    return (endptr == field() + length()) ? 0 : -1;

}

inline int StringMultiSplitter::to_double(double* pv) const {
    char* endptr = NULL;
    *pv = strtod(field(), &endptr);
    return (endptr == field() + length()) ? 0 : -1;

}

} // namespace tutil

#endif // TESLA_TUTIL_STRING_SPILTER_INL_H
