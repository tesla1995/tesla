#ifndef TESLALOG_FILEUTIL_H_
#define TESLALOG_FILEUTIL_H_

#include <cstdio>

#include <string>

#include "noncopyable.h"

namespace tesla {
namespace log {

class AppendFile : Noncopyable {
 public:
  explicit AppendFile(std::string filename);

  ~AppendFile();

  void Append(const char* logline, const size_t len);

  void Flush();

  off_t WrittenBytes() const { return written_bytes_; }

 private:

  size_t write(const char* logline, size_t len);

  FILE* fp_;
  char buffer_[64*1024];
  off_t written_bytes_;
};

} // namespace log
} // namespace tesla

#endif // TESLALOG_FILEUTIL_H_
