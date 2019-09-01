#ifndef TESLALOG_LOGFILE_H_
#define TESLALOG_LOGFILE_H_

#include <mutex>
#include <memory>

#include "noncopyable.h"
#include "fileutil.h"

namespace tesla {
namespace log {

class LogFile : Noncopyable {
 public:
  LogFile(const std::string& basename,
          off_t roll_size,
          bool thread_safe = true,
          int flush_interval = 3,
          int check_interval = 1024);

  ~LogFile();

  void Append(const char* logline, int len);

  void Flush();

  bool RollFile();

 private:
  void AppendUnlocked(const char* logline, int len);

  static std::string GetLogFilename(const std::string& basename, time_t* now);

  const std::string basename_;
  const off_t roll_size_;
  const int flush_interval_;
  const int check_interval_;

  int count_;
  
  std::unique_ptr<std::mutex> mutex_;
  time_t start_period_;
  time_t last_roll_;
  time_t last_flush_;
  std::unique_ptr<AppendFile> file_;

  const static int kRollPerSeconds_ = 60*60*24;
};

} // namespace log
} // namespace tesla

#endif // TESLALOG_LOGFILE_H_
