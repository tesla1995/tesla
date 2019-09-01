/***************************************************************************
 * FixedBuffer是一个类模板，实例化时需要指定缓存大小，用以实例化一个具体的缓
 * 存类。用法：
 *
 * char message[32];
 * // 填入内容到 message 中
 *
 * FixedBuffer<8000> small_buffer;
 *
 * // 写入内容到缓存中
 * if (!small_buffer.append(message, strlen(message))) {
 *   // 缓存可用空间不足
 * }
 *
 * // 获取缓存内容
 * std::cout << small_buffer->data();
 *
 * 另外，还提供了3个容量固定的缓存类，
 * SmallFixedBuffer  容量为 10*8000 字节
 * MiddleFixedBuffer 容量为 100*8000 字节
 * LargeFixedBuffer  容量为 1000*8000 字节
 *
 * 一般直接使用这三个类就足够了，需要特殊的大小，再使用 FixedBuffer 类模板
 ***************************************************************************/


#ifndef TESLALOG_FIXEDBUFFER_H_
#define TESLALOG_FIXEDBUFFER_H_

#include <cstdio>
#include <cstring>

#include "noncopyable.h"

namespace tesla {
namespace log {

template<int SIZE>
class FixedBuffer : Noncopyable {
 public:
  FixedBuffer()
    : current_(data_),
      end_(data_ + SIZE) { 
    *current_ = 0; 
    set_cookie(CookieStart);
  }

  ~FixedBuffer() {
    set_cookie(CookieEnd);
  }

  // 获取缓存容量
  int capacity() const { return sizeof data_; }
  
  // 获取当前缓存内容的大小
  int length() const { return static_cast<int>(current_ - data_); }

  // 获取缓存可用空间的大小
  int avail() const { return static_cast<int>(end_ - current_); }

  // 获取当前位置
  char* current() { return current_; } 

  // 当前位置向后移动
  void add(int len) { current_ += len; }

  // 将指定内容添加到缓存中
  bool append(const char* buf, size_t len) {
    if (static_cast<size_t>(avail()) > len) {
      memcpy(current_, buf, len);
      current_ += len;
      return true;
    } else {
      return false;
    }
  }

  // 获取缓存的内容
  const char* data() const { 
    *current_ = 0;
    return data_; 
  }

  // 重置缓存
  void reset() {
    current_ = data_;
    *current_ = 0;
  }

  // 清零
  void bzero() {
    ::bzero(data_, sizeof data_);
  }

  void set_cookie(void (*cookie)()) {
    cookie_ = cookie;
  }

  void* cookie() {
    return cookie_; 
  }
  
 private:
  // Must be outline function for cookies.
  static void CookieStart();
  static void CookieEnd();

  void (*cookie_)() = NULL;
  char data_[SIZE];
  char* current_ = NULL;
  char* end_ = NULL;
};

const int kSmallBuffer = 10 * 8000;
const int kMiddleBuffer = 100 * 8000;
const int kLargeBuffer = 1000 * 8000;

typedef FixedBuffer<kSmallBuffer> SmallFixedBuffer;
typedef FixedBuffer<kMiddleBuffer> MiddleFixedBuffer;
typedef FixedBuffer<kLargeBuffer> LargeFixedBuffer;

template<int SIZE>
void FixedBuffer<SIZE>::CookieStart() {

}

template<int SIZE>
void FixedBuffer<SIZE>::CookieEnd() {

}

} // namespace log
} // namespace tesla

#endif // TESLALOG_FIXEDBUFFER_H_
