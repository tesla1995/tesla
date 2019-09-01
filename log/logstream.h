#ifndef TESLALOG_LOGSTREAM_H_
#define TESLALOG_LOGSTREAM_H_

#include <string>

#include "noncopyable.h"
#include "fixedbuffer.h"

namespace tesla {
namespace log {

class LogStream : Noncopyable {
 public:
  const static int kSizeOfLogBuffer = 4000;
  typedef FixedBuffer<kSizeOfLogBuffer> Buffer;

  LogStream& operator<<(short v);
  LogStream& operator<<(unsigned short v);
  
  LogStream& operator<<(int);
  LogStream& operator<<(unsigned int v);

  LogStream& operator<<(long);
  LogStream& operator<<(unsigned long v);

  LogStream& operator<<(long long);
  LogStream& operator<<(unsigned long long v);

  LogStream& operator<<(const void*);

  LogStream& operator<<(char v) {
    buffer_.append(&v, 1);
    return *this;
  }

  LogStream& operator<<(const char* str) {
    if (str) {
      buffer_.append(str, strlen(str));
    } else {
      buffer_.append("(null)", 6);
    }
    return *this;
  }

  LogStream& operator<<(const std::string& v) {
    buffer_.append(v.c_str(), v.size());
    return *this;
  }

  LogStream& operator<<(float v);
  LogStream& operator<<(double v);
  
  const Buffer& buffer() const { return buffer_; }

  void Append(const char* buf, int len) { buffer_.append(buf, static_cast<size_t>(len)); }

 private:
  template<typename T>
  void formatInteger(T);

  Buffer buffer_; 
  
  static const int kMaxNumericSize = 32;
};

class Fmt
{
 public:
  template<typename T>
  Fmt(const char* fmt, T val);

  const char* data() const { return buf_; }
  int length() const { return length_; }

 private:
  char buf_[32];
  int length_;
};

inline LogStream& operator<<(LogStream& s, const Fmt& fmt)
{
  s.Append(fmt.data(), fmt.length());
  return s;
}

} // namespace log
} // namespace tesla

#endif
