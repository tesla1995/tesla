#ifndef TESLALOG_ASYNCLOGGING_H_
#define TESLALOG_ASYNCLOGGING_H_

#include <string>
#include <atomic>
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "noncopyable.h"
#include "fixedbuffer.h"
#include "count_down_latch.h"
#include "logging.h"

namespace tesla {
namespace log {


class AsyncLogging : Noncopyable {
 public:
  AsyncLogging(const std::string& basename,
               off_t roll_size,
               int flush_interval);

  ~AsyncLogging();

  void Append(const char* logline, int len);

  void Start();
  void Stop();
  
  static void Init(char* path, Logger::LogLevel level = Logger::INFO,
                   int roll_size = 1024 * 1024 * 1024,
                   int flush_interval = 3);

 private:
  typedef LargeFixedBuffer Buffer;
  typedef std::unique_ptr<Buffer> BufferPtr;
  typedef std::vector<BufferPtr> BufferVectorPtr;

  void ThreadFunction();

  std::string basename_;
  off_t roll_size_;
  const int flush_interval_;

  bool running_;

  CountDownLatch latch_;  
  std::mutex mutex_;
  std::condition_variable condition_;
  std::thread thread_;

  BufferPtr current_buffer_;
  BufferPtr next_buffer_;
  BufferVectorPtr buffers_;
};

} // namespace log
} // namespace tesla

#endif // TESLALOG_ASYNCLOGGING_H_
