#include "timestamp.h"

#include <sys/time.h>
#include <cinttypes>
#include <ctime>

using namespace std;

namespace tesla {
namespace log {

string Timestamp::toString() const {
  char buf[32] = {0};
  int64_t seconds = microseconds_since_epoch_ / kMicroSecondsPerSecond;
  int64_t microseconds = microseconds_since_epoch_ % kMicroSecondsPerSecond;
  snprintf(buf, sizeof(buf) - 1, "%" PRId64 ".%06" PRId64 "", seconds, microseconds);
  return buf;
}

string Timestamp::toFormattedString(bool showMicroseconds) const {
  char buf[64] = {0};
  time_t seconds = static_cast<int>(microseconds_since_epoch_ / kMicroSecondsPerSecond);
  struct tm tm_time;
  gmtime_r(&seconds, &tm_time);
  //localtime_r(&seconds, &tm_time);

  if (showMicroseconds) {
    int microseconds = static_cast<int>(microseconds_since_epoch_ % kMicroSecondsPerSecond);
    snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d.%06d",
             tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
             tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec,
             microseconds);
  } else {
    snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d",
             tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
             tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
  }

  return buf;
}

Timestamp Timestamp::now() {
  struct timeval tv;
  gettimeofday(&tv, NULL);

  return Timestamp(tv.tv_sec * kMicroSecondsPerSecond + tv.tv_usec);
}

} // namespace log
} // namespace tesla
