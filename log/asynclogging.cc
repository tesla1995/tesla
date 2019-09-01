#include "asynclogging.h"
#include "logfile.h"
#include "timestamp.h"

#include <cassert>

using namespace std;

namespace tesla {
namespace log {

static unique_ptr<AsyncLogging> kAsyncLog;

void AsyncLogging::Init(char* path, Logger::LogLevel level,
                        int roll_size, int flush_interval) {
  char *ptr = NULL;
  if ((ptr = strrchr(path, '/')) != NULL) {
    ++ptr;
  }

  kAsyncLog.reset(new AsyncLogging(ptr, roll_size, flush_interval)) ;
  Logger::set_output([](const char* message, int len) { kAsyncLog->Append(message, len); });
  Logger::set_loglevel(level);
  kAsyncLog->Start();
}


AsyncLogging::AsyncLogging(const string& basename,
                           off_t roll_size,
                           int flush_interval)
  : basename_(basename),
    roll_size_(roll_size),
    flush_interval_(flush_interval),
    running_(false),
    latch_(1),
    thread_(&AsyncLogging::ThreadFunction, this),
    current_buffer_(new Buffer),
    next_buffer_(new Buffer) {

  current_buffer_->bzero();
  next_buffer_->bzero();
  buffers_.reserve(16);
}


AsyncLogging::~AsyncLogging() {
  if (running_) {
    Stop();
  }
}

void AsyncLogging::Append(const char* logline, int len) {
  std::lock_guard<std::mutex> lock(mutex_); 
  if (current_buffer_->avail() > len) {
    current_buffer_->append(logline, len);
  } else {
    buffers_.push_back(std::move(current_buffer_));

    if (next_buffer_) {
      current_buffer_ = std::move(next_buffer_);
    } else {
      current_buffer_.reset(new Buffer); // Rarely happens     
    }
    current_buffer_->append(logline, len);

    condition_.notify_one(); // Wake up the log backend thread.
  }
}

void AsyncLogging::ThreadFunction() {

  LogFile output(basename_, roll_size_, false);

  BufferPtr new_buffer1_(new Buffer);
  BufferPtr new_buffer2_(new Buffer);
  new_buffer1_->bzero();
  new_buffer2_->bzero();
  BufferVectorPtr buffers_to_write_;
  buffers_to_write_.reserve(16);
  
  // Wait the call of Start().
  latch_.Wait();

  while (running_) {
    {
      std::unique_lock<std::mutex> lock(mutex_); 

      // wait for the notification or flush_interval_ seconds.
      if (buffers_.empty()) {
        condition_.wait_for(lock, std::chrono::seconds(flush_interval_));
      }

      buffers_.push_back(std::move(current_buffer_));
      buffers_to_write_.swap(buffers_);

      current_buffer_ = std::move(new_buffer1_);
      if (!next_buffer_) {
        next_buffer_ = std::move(new_buffer2_);
      }
    }

    assert(!buffers_to_write_.empty());

    // discard log
    if (buffers_to_write_.size() > 25) {
      char buf[256];
      snprintf(buf, sizeof buf, "Dropped log message at %s, %zd larger buffers\n",
               Timestamp::now().toFormattedString().c_str(),
               buffers_to_write_.size() -2);
      fputs(buf, stderr);
      output.Append(buf, static_cast<int>(strlen(buf)));
      buffers_to_write_.erase(buffers_to_write_.begin()+2, buffers_to_write_.end());
    }

    for (size_t i = 0; i < buffers_to_write_.size(); ++i) {
      output.Append(buffers_to_write_[i]->data(), 
                    buffers_to_write_[i]->length());
    }

    if (!new_buffer1_) {
      assert(!buffers_to_write_.empty());
      new_buffer1_ = std::move(buffers_to_write_[0]);
      new_buffer1_->reset();
    }

    if (!new_buffer2_) {
      assert(!buffers_to_write_.empty());
      new_buffer2_ = std::move(buffers_to_write_[1]);
      new_buffer2_->reset();
    }

    buffers_to_write_.clear();
    output.Flush();
  }
  output.Flush();
}

void AsyncLogging::Start() {
  running_ = true;
  latch_.CountDown();
}

void AsyncLogging::Stop() {
  running_ = false;
  condition_.notify_one();
  thread_.join();
}

} // namespace log
} // namespace tesla
