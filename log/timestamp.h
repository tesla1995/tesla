#ifndef TESLALOG_TIMESTAMP_H_
#define TESLALOG_TIMESTAMP_H_

#include <cstdint>
#include <string>

namespace tesla {
namespace log {

// Time stamp in UTC, in microseconds resolution.
class Timestamp {
 public:
  // Timestamp() = default;

  explicit Timestamp(int64_t microSecondsSinceEpochArg = 0)
    : microseconds_since_epoch_(microSecondsSinceEpochArg) {

  }

  int64_t microseconds_since_epoch() { return microseconds_since_epoch_; }

  bool valid() const { return microseconds_since_epoch_ > 0; }

  void swap(Timestamp& that) {
    std::swap(microseconds_since_epoch_, that.microseconds_since_epoch_);
  }

  // Convert time stamp to string.
  std::string toString() const;

  std::string toFormattedString(bool showMicroseconds = true) const; 

  // one seconds is equal to 1000*1000 microseconds
  const static int kMicroSecondsPerSecond = 1000 * 1000;

  // Get time of now.
  static Timestamp now();

  // Convert unix time  to time stamp. 
  static Timestamp fromUnixTime(time_t seconds) {
    return fromUnixTime(seconds, 0);
  }

  static Timestamp fromUnixTime(time_t seconds, int microseconds) {
    return Timestamp(static_cast<int64_t>(seconds) * kMicroSecondsPerSecond + microseconds);
  }

 private:
  int64_t  microseconds_since_epoch_;
};

} // log
} // tesla

#endif // TESLALOG_TIMESTAMP_H_
