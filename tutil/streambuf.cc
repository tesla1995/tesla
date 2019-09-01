#include "tutil/streambuf.h"

namespace tutil {

CharArrayStreamBuf::~CharArrayStreamBuf() {
  free(data_);
}

int CharArrayStreamBuf::sync() {
  // we don't need to synchronized put area with underlying device,
  // so always return 0 on success.
  return 0;
}

int CharArrayStreamBuf::overflow(int ch) {
  if (ch == std::streambuf::traits_type::eof()) {
    return ch;
  }

  size_t new_size = std::max(size_ * 3 / 2, (size_t)64);
  char* new_data = (char*)malloc(new_size);
  if (new_data == nullptr) {
    setp(nullptr, nullptr);
    return std::streambuf::traits_type::eof();
  }
  memcpy(new_data, data_, size_);
  free(data_);
  data_ = new_data;
  const size_t old_size = size_;
  size_ = new_size;
  setp(data_, data_ + size_);
  pbump(old_size);
  // if size == 1, this function will call overflow again.
  return sputc(ch);
}

} // namespace tutil
