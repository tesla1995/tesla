#include "fileutil.h"
#include "logging.h"

#include <cerrno>

namespace tesla {
namespace log {

AppendFile::AppendFile(std::string filename)
  : fp_(::fopen(filename.c_str(), "ae")),
    written_bytes_(0) {
  ::setbuffer(fp_, buffer_, sizeof buffer_);
}

AppendFile::~AppendFile() {
  ::fclose(fp_);
}

void AppendFile::Append(const char* logline, const size_t len) {
  size_t remain = len;
  ssize_t written = 0;
  size_t current = 0;

  while (remain > 0) {
    written = write(logline+current, remain);
    if (written == 0) {
      int err = ferror(fp_);
      if (err) {
        fprintf(stderr, "AppendFile::Append failed %s\n", strerror_tl(errno));
      }
      break;
    }

    remain -= written;
    current += written; 
  }

  written_bytes_ += len;
}

size_t AppendFile::write(const char* logline, size_t len) {
  return ::fwrite_unlocked(logline, 1, len, fp_);
}

void AppendFile::Flush() {
  ::fflush(fp_);
}

} // namespace log
} // namespace tesla
