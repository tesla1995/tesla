/************************************************************
 * CountDownLatch(倒计时)是一种常用且易用的同步手段，通常用于
 * 1. 用于主线程等待多个子线程完成初始化
 * 2. 用于多个子线程等待主线程发出”起跑“命令
 ***********************************************************/

#ifndef TESLALOG_COUNTDOWNLATCH_H_
#define TESLALOG_COUNTDOWNLATCH_H_

#include <mutex>
#include <condition_variable>
#include "noncopyable.h"

namespace tesla {
namespace log {

class CountDownLatch : Noncopyable {
 public:
  explicit CountDownLatch(int count)    // 倒数几次
    : count_(count)
  {  }

  void Wait()                         // 等待计数值变为0
  {
    std::unique_lock<std::mutex> lock(mutex_); 
    // method 1:
    //while (count_)
    //{
    //  condition_.wait(lock);
    //}

    // method 2:
    condition_.wait(lock, [&]{ return count_ == 0; });
  }

  void CountDown()                    // 计数减一
  {
    std::lock_guard<std::mutex> lock(mutex_);

    --count_;
    if (!count_)
    {
      condition_.notify_all();
    }
  }

 private:
  mutable std::mutex mutex_;
  std::condition_variable condition_; 
  int count_;
};

} // namespace log
} // namespace tesla

#endif // TESLALOG_COUNTDOWNLATCH_H_
