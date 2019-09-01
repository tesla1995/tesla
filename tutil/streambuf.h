#ifndef TESLA_TUTIL_STREAMBUF_H_
#define TESLA_TUTIL_STREAMBUF_H_

#include <streambuf>
#include "tutil/string_view.h"

namespace tutil {

class CharArrayStreamBuf : public std::streambuf {
 public:
  explicit CharArrayStreamBuf() : data_(nullptr), size_(0) {} 
  ~CharArrayStreamBuf();

  int overflow(int ch = traits_type::eof()) override;
  int sync() override;
  
  void reset() { setp(data_, data_ + size_); }
  StringView data() { return StringView(pbase(), pptr() - pbase()); }

 private:
  char* data_;
  size_t size_;
};

} // namespace tutil

#endif // TESLA_TUTIL_STREAMBUF_H_
