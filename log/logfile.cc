#include "logfile.h"

#include <unistd.h>

using namespace std;

namespace tesla {
namespace log {

LogFile::LogFile(const std::string& basename,
                 off_t roll_size,
                 bool thread_safe,
                 int flush_interval,
                 int check_interval) 
  : basename_(basename),
    roll_size_(roll_size),
    flush_interval_(flush_interval),
    check_interval_(check_interval),
    count_(0),
    mutex_(thread_safe ? new std::mutex : NULL),
    start_period_(0),
    last_roll_(0),
    last_flush_(0) {

   RollFile();
}

LogFile::~LogFile() {

}

void LogFile::Append(const char* logline, int len) {
  if (mutex_) {
    lock_guard<mutex> lock(*mutex_);
    AppendUnlocked(logline, len);
  } else {
    AppendUnlocked(logline, len);
  }
}

void LogFile::Flush() {
  if (mutex_) {
    lock_guard<mutex> lock(*mutex_);
    file_->Flush();
  } else {
    file_->Flush();
  }
}

void LogFile::AppendUnlocked(const char* logline, int len) {
  file_->Append(logline, len);

  if (file_->WrittenBytes() > roll_size_) {
    RollFile();
  } else {
    ++count_;
    if (count_ >= check_interval_) {
      count_ = 0;
      time_t now = ::time(NULL);
      time_t this_period = now / kRollPerSeconds_ * kRollPerSeconds_;
      if (this_period != start_period_) {
        RollFile();
      } else if (now - last_flush_ > flush_interval_) {
        last_flush_ = now;
        file_->Flush();
      }
    }
  }
}

bool LogFile::RollFile() {
  time_t now = 0;
  string filename = GetLogFilename(basename_, &now);
  time_t start = now / kRollPerSeconds_ * kRollPerSeconds_;

  if (now > last_roll_) {
    last_roll_ = now;
    last_flush_ = now;
    start_period_ = start;
    file_.reset(new AppendFile(filename));
    return true;
  }

  return false;
}

string LogFile::GetLogFilename(const string& basename, time_t* now) {
  string filename;
  filename.reserve(basename.size() + 64);
  filename = basename;  

  char timebuf[32];
  struct tm tm;
  *now = time(NULL);
  //localtime_r(now, &tm);
  gmtime_r(now, &tm);
  strftime(timebuf, sizeof timebuf, ".%Y%m%d-%H%M%S.", &tm);
  filename += timebuf;

  char namebuf[64];
  gethostname(namebuf, 64);
  filename += namebuf;

  char pidbuf[32];
  snprintf(pidbuf, sizeof pidbuf, ".%d", getpid());
  filename += pidbuf;

  filename += ".log";

  return filename;
}

} // namespace log
} // namespace tesla
