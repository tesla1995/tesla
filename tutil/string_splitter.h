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

// Iteratively split a string by one or multiple separators.

#ifndef TESLA_TUTIL_STRING_SPILTER_H
#define TESLA_TUTIL_STRING_SPILTER_H

#include <stdlib.h>
#include <stdint.h>

// It's common to encode data into strings separated by special characters
// and decode them back, but functions such as `split_string' has to modify
// the input string, which is bad. If we parse the string from scratch, the
// code will be filled with pointer operations and obscure to understand.
//
// What we want is:
// - Scan the string once: just do simple things efficiently.
// - Do not modify input string: Changing input is bad, it may bring hidden
//   bugs, concurrency issues and non-const propagations.
// - Split the string in-place without additional buffer/array.
//
// StringSplitter does meet these requirements.
// Usage:
//     const char* the_string_to_split = ...;
//     for (StringSplitter s(the_string_to_split, '\t'); s; ++s) {
//         printf("%*s\n", s.length(), s.field());
//     }
//
// "s" behaves as an iterator and evaluates to true before ending.
// "s.field()" and "s.length()" are address and length of current field
// respectively. Notice that "s.field()" may not end with '\0' because
// we don't modify input. You can copy the field to a dedicated buffer
// or apply a function supporting length.

namespace tutil {

enum class EmptyFieldAction {
  SKIP_EMPTY_FIELD,
  ALLOW_EMPTY_FIELD,
};

// Split a string with one charater.
class StringSplitter {
 public:
  // Split `input' with `separator'. If `action' is SKIP_EMPTY_FIELD, zero-
  // length() field() will be skipped.
  StringSplitter(const char* input, char separator,
                 EmptyFieldAction action = EmptyFieldAction::SKIP_EMPTY_FIELD);

  // Allow containing embedded '\0' characters and separator can be '\0',
  // if str_end is not NULLPTR.
  StringSplitter(const char* str_begin, const char* str_end, char separator,
                 EmptyFieldAction action = EmptyFieldAction::ALLOW_EMPTY_FIELD);

  // Move splitter forward.
  StringSplitter& operator++();
  StringSplitter operator++(int);

  // True if field() is valid.
  // operator const void*() const; // C++98-style
  
  // modify by tesla at Sun Jan 20 23:43:23 CST 2019.
  explicit operator bool() const; // C++11-style

  // Begining address and length of the field. *(field() + length()) may not be
  // '\0' because we don't modify `input'.
  const char* field() const;
  size_t length() const;

  // Cast field to specific type, and write the value into `pv'.
  // Return 0 on success, -1 otherwise.
  // NOTE: If separator is a digit, casting functions always return -1.
  int to_int8(int8_t* pv) const;
  int to_uint8(uint8_t* pv) const;
  int to_int(int* pv) const;
  int to_uint(unsigned int* pv) const;
  int to_long(long* pv) const;
  int to_ulong(unsigned long* pv) const;
  int to_longlong(long long* pv) const;
  int to_ulonglong(unsigned long long* pv) const;
  int to_float(float* pv) const;
  int to_double(double* pv) const;

 private:
  bool not_end(const char* p) const;
  void init();

  const char* head_;     // the beginning of the current field.
  const char* tail_;     // one past the ending of the current field.
  const char* str_tail_; // the ending of input.
  char separator_;
  const EmptyFieldAction empty_field_action_;
};

// ------------------------------------------------------------------

// Split a string with one of the separators.
class StringMultiSplitter {
 public:
  // Split `input' with one character of `separators'. If `action' is 
  // SKIP_EMPTY_FIELD, zero-length() field() will be skipped.
  // NOTE: This utility stores pointer of `separators' directly rather than
  //       copying the content because this utility is intended to be used
  //       in ad-hoc manner where lifetime of `separators' is generally
  //       longer than this utility.
  StringMultiSplitter(const char* input, const char* separators,
                      EmptyFieldAction action = EmptyFieldAction::SKIP_EMPTY_FIELD);

  // Allows containing embedded '\0' characters if str_end is not NULL.
  // NOTE: `Separators' cannot constain embedded '\0' character.
  StringMultiSplitter(const char* str_begin, const char* str_end,
                      const char* separators,
                      EmptyFieldAction action = EmptyFieldAction::SKIP_EMPTY_FIELD);

  // Move splitter forward.
  StringMultiSplitter& operator++();
  StringMultiSplitter operator++(int);

  // True if field() is valid.
  explicit operator bool() const;
 
  // Beginning address and length of the current field. *(field() + length())
  // may not be '\0' because we don't modify `input'.
  const  char* field() const;
  size_t length() const;

  // Cast field to specific type, and write the value into `pv'.
  // Returns 0 on success, -1 otherwise.
  // NOTE: If separators contains digit, casting functions always return -1.
  int to_int8(int8_t* pv) const;
  int to_uint8(uint8_t* pv) const;
  int to_int(int* pv) const;
  int to_uint(unsigned int* pv) const;
  int to_long(long* pv) const;
  int to_ulong(unsigned long* pv) const;
  int to_longlong(long long* pv) const;
  int to_ulonglong(unsigned long long* pv) const;
  int to_float(float* pv) const;
  int to_double(double* pv) const;

 private:
  bool is_separator(char c) const;
  bool not_end(const char* p) const;
  void init();

  const char* head_;
  const char* tail_;
  const char* str_tail_;
  const char* const separators_;
  const EmptyFieldAction empty_field_action_;
};

} // namespace tutil

#include "tutil/string_splitter_inl.h"

#endif // TESLA_TUTIL_STRING_SPILTER_H
