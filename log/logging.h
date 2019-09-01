#ifndef TESLALOG_LOGGING_H_
#define TESLALOG_LOGGING_H_

#include <cstring>

#include "logstream.h"
#include "timestamp.h"

namespace tesla {
namespace log {

const char* strerror_tl(int savedErrno);

class Logger {
 public:
  enum LogLevel {
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL,
    NUM_LOG_LEVELS,
  };

  // compile time calculation of basename of source file(why ?)
  class SourceFile {
   public:
    template<int N>
    inline SourceFile(const char (&arr)[N])
    : data_(arr),
      size_(N-1) {
      const char* slash = strrchr(data_, '/');
      if (slash) {
        data_ = slash + 1;
        size_ -= static_cast<int>(data_ - arr);
      }
    }

    explicit SourceFile(const char* filename)
      : data_(filename) {
      const char* slash = strrchr(data_, '/');
      if (slash) {
        data_ = slash + 1;
      }
      size_ = static_cast<int>(strlen(data_));
    }

    const char* data_;
    int size_;
  };

  Logger(SourceFile file, int line);
  Logger(SourceFile file, int line, LogLevel level);
  Logger(SourceFile file, int line, LogLevel level, const char* func);
  Logger(SourceFile file, int line, bool toAbort);

  ~Logger();
  
  LogStream& stream() { return impl_.stream_; }

  static LogLevel loglevel();
  static void set_loglevel(LogLevel level);

  typedef void (*OutputFunc)(const char* message, int len);
  typedef void (*FlushFunc)();

  static void set_output(OutputFunc);
  static void set_flush(FlushFunc);

 private:
  class Impl {
   public:
    typedef Logger::LogLevel LogLevel;
    Impl(LogLevel level, int old_errno, const SourceFile& file, int line);
    void formatTime();
    void finish();

    Timestamp time_;
    LogStream stream_;
    LogLevel level_;
    int line_;
    SourceFile basename_;
  };
 
  Impl impl_; 
};

extern Logger::LogLevel kLogLevel;
inline Logger::LogLevel Logger::loglevel() { return kLogLevel; }

#define LOG_TRACE if (tesla::log::Logger::loglevel() <= tesla::log::Logger::TRACE) \
  tesla::log::Logger(__FILE__, __LINE__, tesla::log::Logger::TRACE, __func__).stream()

#define LOG_DEBUG if (tesla::log::Logger::loglevel() <= tesla::log::Logger::DEBUG) \
  tesla::log::Logger(__FILE__, __LINE__, tesla::log::Logger::DEBUG, __func__).stream()

#define LOG_INFO if (tesla::log::Logger::loglevel() <= tesla::log::Logger::INFO) \
  tesla::log::Logger(__FILE__, __LINE__).stream()

#define LOG_WARN tesla::log::Logger(__FILE__, __LINE__, tesla::log::Logger::WARN).stream()

#define LOG_ERROR tesla::log::Logger(__FILE__, __LINE__, tesla::log::Logger::ERROR).stream()

#define LOG_FATAL tesla::log::Logger(__FILE__, __LINE__, tesla::log::Logger::FATAL).stream()

#define LOG_SYSERR tesla::log::Logger(__FILE__, __LINE__, false).stream()

#define LOG_SYSFATAL tesla::log::Logger(__FILE__, __LINE__, true).stream()

// Taken from glog/logging.h
//
// Check that the input is non NULL.  This very useful in constructor
// initializer lists.

#define CHECK_NOTNULL(val) \
  tesla::log::CheckNotNull(__FILE__, __LINE__, "'" #val "' Must be non NULL", (val))

// A small helper for CHECK_NOTNULL().
template <typename T>
T* CheckNotNull(Logger::SourceFile file, int line, const char *names, T* ptr)
{
  if (ptr == NULL)
  {
   Logger(file, line, Logger::FATAL).stream() << names;
  }
  return ptr;
}


// This class is used to explicitly ignore values in the conditional
// logging macros.  This avoids compiler warnings like "value computed
// is not used" and "statement has no effect".

// Youcompleteme: left operand to ? is void, but right operand is of type 'tesla::log::LogStream'

class LogMessageVoidify {
 public:
  LogMessageVoidify() = default;
  ~LogMessageVoidify() = default;
  // This has to be an operator with a precedence lower than << but
  // higher than ?:
  void operator&(tesla::log::LogStream&) { }
};

/*
#define LOG_IF(severity, condition) \
  !(condition) ? (void)0 : tesla::log::LogMessageVoidify() & tesla::log::Logger(__FILE__, __LINE__, severity).stream()

#define CHECK(condition) \
  LOG_IF(tesla::log::Logger::FATAL, !(condition)) << "Check failed: `" #condition "` "
*/

} // namespace log
} // namespace tesla

#endif // TESLALOG_LOGGING_H_
