// A string-like object that points to a sized piece of memory.
//
// You can use StringView as a function or method parameter.  A StringView
// parameter can receive a double-quoted string literal argument, a "const
// char*" argument, a string argument, or a StringView argument with no data
// copying.  Systematic use of StringView for arguments reduces data
// copies and strlen() calls.
//
// Prefer passing StringViews by value:
//   void MyFunction(StringView arg);
// If circumstances require, you may also pass by const reference:
//   void MyFunction(const StringView& arg);  // not preferred
// Both of these have the same lifetime semantics.  Passing by value
// generates slightly smaller code.
//
// Caveats (again):
// (1) The lifetime of the pointed-to string (or piece of a string)
//     must be longer than the lifetime of the StringView.
// (2) There may or may not be a '\0' character after the end of
//     StringView data.
// (3) A null StringView is empty.
//     An empty StringView may or may not be a null StringView.

#ifndef TESLA_TUTIL_STRING_VIEW_H_
#define TESLA_TUTIL_STRING_VIEW_H_

#include <string>
#include <iosfwd>
//#include <glog/logging.h>
#include "log/logging.h"

namespace tutil {

template<typename CHAR_TYPE, typename TRAITS = std::char_traits<CHAR_TYPE>>
class BasicStringView {
 public:
  
  // types
  using traits_type = TRAITS; 
  using value_type = CHAR_TYPE;
  using pointer = const CHAR_TYPE*;
  using const_pointer = const CHAR_TYPE*;
  using reference = const CHAR_TYPE&;
  using const_reference = const CHAR_TYPE&;
  using const_iterator = const CHAR_TYPE*;
  using iterator = const_iterator;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using reverse_iterator = const_reverse_iterator;
  using size_type = size_t;
  using difference_type = ptrdiff_t;

  static constexpr size_type npos = size_type(-1); // hack.

  // construct/copy
  constexpr BasicStringView() noexcept
      : ptr_(nullptr), length_(0) {}

  constexpr BasicStringView(const char* str) noexcept
      : ptr_(str), length_((str == nullptr) ? 0 : traits_type::length(str)) { }

  constexpr BasicStringView(const char* str, size_type len) noexcept
      : ptr_(str), length_(len) { }

  constexpr BasicStringView(const BasicStringView&) noexcept = default;

  constexpr BasicStringView& operator=(const BasicStringView&) noexcept = default;

  // iterators
  // begin() end() cbegin() cend() rbegin() rend() crbegin() crend()
  constexpr const_iterator begin() const noexcept { return ptr_; }

  constexpr const_iterator end() const noexcept { return ptr_ + length_; }

  constexpr const_iterator cbegin() const noexcept { return ptr_; }

  constexpr const_iterator cend() const noexcept { return ptr_ + length_; }

  constexpr const_reverse_iterator rbegin() const noexcept {
    return const_reverse_iterator(end());
  }

  constexpr const_reverse_iterator rend() const noexcept {
    return const_reverse_iterator(begin());
  }

  constexpr const_reverse_iterator crbegin() const noexcept {
    return const_reverse_iterator(end());
  }

  constexpr const_reverse_iterator crend() const noexcept {
    return const_reverse_iterator(begin());
  }
  
  // capacity
  constexpr size_type size() const noexcept { return length_; }
 
  constexpr size_type length() const noexcept { return length_; }

  // TODO: Add max_size()
  
  constexpr bool empty() const noexcept { return length_ == 0; }

  // element access
  constexpr const CHAR_TYPE& operator[](size_type pos) const {
    //CHECK(pos < length_);
    return ptr_[pos];
  }

  constexpr const CHAR_TYPE& at(size_type pos) const {
    //CHECK(pos < length_) << "BasicStringView::operator[]: pos (which is "
    //  << pos << ") >= size() (which is " << length_ << ")";
    return ptr_[pos];
  }

  constexpr const CHAR_TYPE& front() const {
    //CHECK(0UL != length_) << "BasicStringView::front: 0 == length_";
    return ptr_[0];
  }

  constexpr const CHAR_TYPE& back() const {
    //CHECK(0UL != length_) << "BasicStringView::back: 0 == length_";
    return ptr_[length_ - 1];
  }

  constexpr const CHAR_TYPE* data() const noexcept {
    return ptr_;
  }

  constexpr void remove_prefix(size_type n) {
    //CHECK(n <= length_) << "BasicStringView::remove_prefix: n (which is "
    //  << n << ") > length_ (which is " << length_ << ")";
    ptr_ += n;
    length_ -= n;
  }

  constexpr void remove_suffix(size_type n) {
    //CHECK(n <= length_) << "BasicStringView::remove_suffix: n (which is "
    // << n << ") > length_ (which is " << length_ << ")";
    length_ -= n;
  }

  // string operations
  constexpr BasicStringView substr(size_type pos, size_type n = npos) const {
    //CHECK(pos <= length_);
    const size_type rlen = std::min(n, length_ - pos);
    return BasicStringView{ptr_ + pos, rlen};
  }

  constexpr int compare(BasicStringView str) const noexcept {
    const size_type rlen = std::min(length_, str.length_);
    int ret = traits_type::compare(ptr_, str.ptr_, rlen);
    if (ret == 0) {
      if (length_ < str.length_) ret = -1;
      if (length_ > str.length_) ret = +1;
    }
    return ret;
  }

  constexpr int compare(size_type pos1, size_type n1, BasicStringView str) const {
    return substr(pos1, n1).compare(str);
  }

  constexpr int compare(size_type pos1, size_type n1,
    BasicStringView str, size_type pos2, size_type n2) const {
    return substr(pos1, n1).compare(str.substr(pos2, n2)); 
  }

  constexpr int compare(const CHAR_TYPE* str) const {
    return compare(BasicStringView(str));
  }

  constexpr int compare(size_type pos1, size_type n1, const CHAR_TYPE* str) const {
    return substr(pos1, n1).compare(BasicStringView(str));
  }

  int compare(size_type pos1, size_type n1,
      const CHAR_TYPE* str, size_type n2) const {
    return substr(pos1, n1).compare(BasicStringView(str, n2));
  }

 private:
  const CHAR_TYPE* ptr_;
  size_t length_;
}; // class BasicStringView


// non-member BasicStringView comparison function ----------------------------------

namespace detail {
  // Identity transform to create a non-deduced context, so that only one
  // argument participates in template argument deduction and the other
  // argument gets implicitly converted to the deduced type. See n3766.html.
  template<typename T>
  using idt = std::common_type_t<T>;
}; // namespace detail

// operator==
template<typename CHAR_TYPE, typename TRAITS>
constexpr bool operator==(BasicStringView<CHAR_TYPE, TRAITS> x,
                          BasicStringView<CHAR_TYPE, TRAITS> y) noexcept {
  return x.size() == y.size() && x.compare(y) == 0;
}

template<typename CHAR_TYPE, typename TRAITS>
constexpr bool operator==(BasicStringView<CHAR_TYPE, TRAITS> x,
                          detail::idt<BasicStringView<CHAR_TYPE, TRAITS>> y) noexcept {
  return x.size() == y.size() && x.compare(y) == 0;
}

template<typename CHAR_TYPE, typename TRAITS>
constexpr bool operator==(detail::idt<BasicStringView<CHAR_TYPE, TRAITS>> x,
                          BasicStringView<CHAR_TYPE, TRAITS> y) noexcept {
  return x.size() == y.size() && x.compare(y) == 0;
}

// operator!=
template<typename CHAR_TYPE, typename TRAITS>
constexpr bool operator!=(BasicStringView<CHAR_TYPE, TRAITS> x,
                          BasicStringView<CHAR_TYPE, TRAITS> y) noexcept {
  return !(x == y);
}

template<typename CHAR_TYPE, typename TRAITS>
constexpr bool operator!=(BasicStringView<CHAR_TYPE, TRAITS> x,
                          detail::idt<BasicStringView<CHAR_TYPE, TRAITS>> y) noexcept {
  return !(x == y);
}

template<typename CHAR_TYPE, typename TRAITS>
constexpr bool operator!=(detail::idt<BasicStringView<CHAR_TYPE, TRAITS>> x,
                          BasicStringView<CHAR_TYPE, TRAITS> y) noexcept {
  return !(x == y);
}

// operator<
template<typename CHAR_TYPE, typename TRAITS>
constexpr bool operator< (BasicStringView<CHAR_TYPE, TRAITS> x,
                          BasicStringView<CHAR_TYPE, TRAITS> y) noexcept {
  return x.compare(y) < 0;
}

template<typename CHAR_TYPE, typename TRAITS>
constexpr bool operator< (BasicStringView<CHAR_TYPE, TRAITS> x,
                          detail::idt<BasicStringView<CHAR_TYPE, TRAITS>> y) noexcept {
  return x.compare(y) < 0;
}

template<typename CHAR_TYPE, typename TRAITS>
constexpr bool operator< (detail::idt<BasicStringView<CHAR_TYPE, TRAITS>> x,
                          BasicStringView<CHAR_TYPE, TRAITS> y) noexcept {
  return x.compare(y) < 0;
}

// operator>
template<typename CHAR_TYPE, typename TRAITS>
constexpr bool operator> (BasicStringView<CHAR_TYPE, TRAITS> x,
                          BasicStringView<CHAR_TYPE, TRAITS> y) noexcept {
  return x.compare(y) > 0;
}

template<typename CHAR_TYPE, typename TRAITS>
constexpr bool operator> (BasicStringView<CHAR_TYPE, TRAITS> x,
                          detail::idt<BasicStringView<CHAR_TYPE, TRAITS>> y) noexcept {
  return x.compare(y) > 0;
}

template<typename CHAR_TYPE, typename TRAITS>
constexpr bool operator> (detail::idt<BasicStringView<CHAR_TYPE, TRAITS>> x,
                          BasicStringView<CHAR_TYPE, TRAITS> y) noexcept {
  return x.compare(y) > 0;
}

// operator<=
template<typename CHAR_TYPE, typename TRAITS>
constexpr bool operator<=(BasicStringView<CHAR_TYPE, TRAITS> x,
                          BasicStringView<CHAR_TYPE, TRAITS> y) noexcept {
  return x.compare(y) <= 0;
}

template<typename CHAR_TYPE, typename TRAITS>
constexpr bool operator<=(BasicStringView<CHAR_TYPE, TRAITS> x,
                          detail::idt<BasicStringView<CHAR_TYPE, TRAITS>> y) noexcept {
  return x.compare(y) <= 0;
}

template<typename CHAR_TYPE, typename TRAITS>
constexpr bool operator<=(detail::idt<BasicStringView<CHAR_TYPE, TRAITS>> x,
                          BasicStringView<CHAR_TYPE, TRAITS> y) noexcept {
  return x.compare(y) <= 0;
}

// operator>=
template<typename CHAR_TYPE, typename TRAITS>
constexpr bool operator>=(BasicStringView<CHAR_TYPE, TRAITS> x,
                          BasicStringView<CHAR_TYPE, TRAITS> y) noexcept {
  return x.compare(y) >= 0;
}

template<typename CHAR_TYPE, typename TRAITS>
constexpr bool operator>=(BasicStringView<CHAR_TYPE, TRAITS> x,
                          detail::idt<BasicStringView<CHAR_TYPE, TRAITS>> y) noexcept {
  return x.compare(y) >= 0;
}

template<typename CHAR_TYPE, typename TRAITS>
constexpr bool operator>=(detail::idt<BasicStringView<CHAR_TYPE, TRAITS>> x,
                          BasicStringView<CHAR_TYPE, TRAITS> y) noexcept {
  return x.compare(y) >= 0;
}

// IO ---------------------------------------------------------------------------

template<typename CHAR_TYPE, typename TRAITS>
inline std::basic_ostream<CHAR_TYPE, TRAITS>&
operator<<(std::basic_ostream<CHAR_TYPE, TRAITS>& os,
    BasicStringView<CHAR_TYPE, TRAITS> str) {
  return __ostream_insert(os, str.data(), str.size());
}

// BasicStringView typedef names.
using StringView = BasicStringView<char>;

// Hashing ---------------------------------------------------------------------

// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// Copied from strings/stringpiece.h with modifications

#define HASH_STRING_PIECE(StringViewType, stirng_view)         \
  std::size_t result = 0;                                      \
  for (StringViewType::const_iterator i = stirng_view.begin(); \
       i != stirng_view.end(); ++i)                            \
    result = (result * 131) + *i;                              \
  return result;

struct StringViewHash {
  std::size_t operator()(const StringView& sp) const {
    HASH_STRING_PIECE(StringView, sp);
  }
};
#undef HASH_STRING_PIECE

}; // namespace tutil

#endif // TESLA_TUTIL_STRING_VIEW_H_
