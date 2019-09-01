#include "logging.h"

#include <cassert>
#include <ctime>
#include <cerrno>

#include "current_thread.h"

namespace tesla {
namespace log {

__thread char t_errnobuf[512];
__thread char t_time[64];
__thread time_t t_lastSecond;

// It is safe to get strerror in multiple threads.
const char* strerror_tl(int savedErrno)
{
  return strerror_r(savedErrno, t_errnobuf, sizeof t_errnobuf);
}

Logger::LogLevel InitLogLevel() {
  if (::getenv("DREAM_LOG_TRACE")) {
    return Logger::TRACE;
  } else if (::getenv("DREAM_LOG_DEBUG")) {
    return Logger::DEBUG;
  } else {
    return Logger::INFO;
  }
}

// global log level
Logger::LogLevel kLogLevel = InitLogLevel();

const char* LogLevelName[Logger::NUM_LOG_LEVELS] = {
  "TRACE ",
  "DEBUG ",
  "INFO  ",
  "WARN  ",
  "ERROR ",
  "FATAL ",
};

void DefaultOutput(const char* message, int len) {
  fwrite(message, len, 1, stdout);
}

void DefaultFlush() {
  fflush(stdout);
}

// function that output the log message
Logger::OutputFunc kOutputFunc = DefaultOutput;
// function that flush log buffer
Logger::FlushFunc kFlushFunc = DefaultFlush;

// helper class for known string length at compile time
class T {
 public:
  T(const char* string, unsigned len) 
    : string_(string),
    len_(len) {
    assert(strlen(string) == len_);
  }

  const char* string_;
  const unsigned len_;
};

inline LogStream& operator<<(LogStream& s, T v) {
  s.Append(v.string_, v.len_);
  return s;
}

inline LogStream& operator<<(LogStream& s, const Logger::SourceFile& v) {
  s.Append(v.data_, v.size_);
  return s;
}

Logger::Impl::Impl(LogLevel level, int savedErrno, const SourceFile& file, int line)
  : time_(Timestamp::now()),
    stream_(),
    level_(level),
    line_(line),
    basename_(file) {
  // output datetime
  formatTime(); 
  // output thread id
  CurrentThread::tid();
  stream_ << T(CurrentThread::tidString(), CurrentThread::tidStringLength());
  // output loglevel
  stream_ << T(LogLevelName[level], 6); 
  if (savedErrno != 0) {
    stream_ << strerror_tl(savedErrno) << " (errno=" << savedErrno << ") ";
  }
}

void Logger::Impl::formatTime() {
  int64_t microseconds_since_epoch = time_.microseconds_since_epoch();
  time_t seconds = static_cast<int>(microseconds_since_epoch / Timestamp::kMicroSecondsPerSecond);
  int microseconds = static_cast<int>(microseconds_since_epoch % Timestamp::kMicroSecondsPerSecond);

  if (seconds != t_lastSecond) {
    t_lastSecond = seconds;

    struct tm tm_time;
    gmtime_r(&seconds, &tm_time);
    //localtime_r(&seconds, &tm_time);

    snprintf(t_time, sizeof(t_time), "%4d%02d%02d %02d:%02d:%02d",
      tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
      tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
  }

  Fmt us(".%06dZ ", microseconds);
  assert(us.length() == 9);
  stream_ << T(t_time, 17) << T(us.data(), 9);
}

Logger::Logger(SourceFile file, int line)
  : impl_(INFO, 0, file, line) {

}

Logger::Logger(SourceFile file, int line, LogLevel level, const char* func)
  : impl_(level, 0, file, line) {
  impl_.stream_ << func << ' ';
}

Logger::Logger(SourceFile file, int line, LogLevel level)
  : impl_(level, 0, file, line) {

}

Logger::Logger(SourceFile file, int line, bool toAbort)
    : impl_(toAbort?FATAL:ERROR, errno, file, line) {
}

void Logger::Impl::finish() {
  stream_ << " - " << basename_ << ':' << line_ << '\n';
}

Logger::~Logger() {
  // output filename and line number
  impl_.finish(); 
  const LogStream::Buffer& buf(stream().buffer());  
  kOutputFunc(buf.data(), buf.length());
  if (impl_.level_ == FATAL) {
    kFlushFunc();
    abort();
  }
}

void Logger::set_loglevel(Logger::LogLevel level) {
  kLogLevel = level;
}

void Logger::set_output(Logger::OutputFunc out) {
  kOutputFunc = out;
}

void Logger::set_flush(Logger::FlushFunc flush) {
  kFlushFunc = flush;
}

} // namespace log
} // namespace tesla
