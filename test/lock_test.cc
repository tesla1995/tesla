#include <mutex>
#include <iostream>
#include <thread>
#include <atomic>

#include <gtest/gtest.h>

#include "log/logging.h"
#include "tutil/count_down_latch.h"
#include "tutil/current_thread.h"
#include "tutil/time.h"
#include "wait_free/spin_lock.h"

using namespace std;
using namespace tesla::tutil;
using namespace tesla::wait_free;

namespace {

static size_t kThreadNum = 4;

CountDownLatch start_execute_lock(1);
CountDownLatch end_execute_lock(kThreadNum);

CountDownLatch start_execute_atomic(1);
CountDownLatch end_execute_atomic(kThreadNum);

CountDownLatch start_execute_spinlock(1);
CountDownLatch end_execute_spinlock(kThreadNum);

int kAddTimes = 1000000;

int kAddWithLock = 0;
std::mutex kAddMutex;

std::atomic<int> kAddAtomic(0);

int kAddSpinLock = 0;
SpinLock kSpinLock;

////////////////////////////////////////////////////////////////////////

void AddWithMutex(size_t times) {
  start_execute_lock.Wait();
  while (times--) {
    kAddMutex.lock();
    ++kAddWithLock; 
    //++kAddWithLock; 
    //++kAddWithLock; 
    //++kAddWithLock; 
    //++kAddWithLock; 
    kAddMutex.unlock();
  }
  end_execute_lock.CountDown();
}

void AddWithSpinLock(size_t times) {
  start_execute_spinlock.Wait();
  while (times--) {
    kSpinLock.Lock();
    ++kAddSpinLock; 
    //++kAddSpinLock; 
    //++kAddSpinLock; 
    //++kAddSpinLock; 
    //++kAddSpinLock; 
    kSpinLock.Unlock();
  }
  end_execute_spinlock.CountDown();
}

void AddAtomic(size_t times) {
  start_execute_atomic.Wait();
  while (times--) {
    kAddAtomic.fetch_add(1);
    //kAddAtomic.fetch_add(1);
    //kAddAtomic.fetch_add(1);
    //kAddAtomic.fetch_add(1);
    //kAddAtomic.fetch_add(1);
  }
  end_execute_atomic.CountDown();
}

////////////////////////////////////////////////////////////////////////

// The fixture for testing Mutex.
class LockTest : public ::testing::Test {
 protected:
  // You can remove any or all of the following functions if its body
  // is empty.

  LockTest() {
     // You can do set-up work for each test here.
  }

  ~LockTest() override {
     // You can do clean-up work that doesn't throw exceptions here.
  }

  // If the constructor and destructor are not enough for setting up
  // and cleaning up each test, you can define the following methods:

  void SetUp() override {
     // Code here will be called immediately after the constructor (right
     // before each test).
     //cout << "===== Set Up =====" << endl;
  }

  void TearDown() override {
     // Code here will be called immediately after each test (right
     // before the destructor).
     //cout << "===== Tear Down =====" << endl;
  }

  // Objects declared here can be used by all tests in the test case for Flags.
}; // namespace LockTest

TEST_F(LockTest, AddWithLock) {
  Timer timer;
  std::vector<std::thread> v;

  for (size_t i = 0; i < kThreadNum; ++i) {
    v.push_back(std::thread(AddWithMutex, kAddTimes));
  }

  start_execute_lock.CountDown();
  timer.start();
  end_execute_lock.Wait();
  timer.stop();
  LOG_INFO << "In AddWithLock, elapsed time is " << timer.m_elapsed(0.0) << "ms";

  for (auto& t : v) {
    if (t.joinable()) {
      t.join();
    }
  }
}

TEST_F(LockTest, AddAtomic) {
  Timer timer;
  std::vector<std::thread> v;

  for (size_t i = 0; i < kThreadNum; ++i) {
    v.push_back(std::thread(AddAtomic, kAddTimes));
  }

  start_execute_atomic.CountDown();
  timer.start();
  end_execute_atomic.Wait();
  timer.stop();
  LOG_INFO << "In AddAtomic, elapsed time is " << timer.m_elapsed(0.0) << "ms";

  for (auto& t : v) {
    if (t.joinable()) {
      t.join();
    }
  }
}

TEST_F(LockTest, AddWithSpinLock) {
  Timer timer;
  std::vector<std::thread> v;

  for (size_t i = 0; i < kThreadNum; ++i) {
    v.push_back(std::thread(AddWithSpinLock, kAddTimes));
  }

  start_execute_spinlock.CountDown();
  timer.start();
  end_execute_spinlock.Wait();
  timer.stop();
  LOG_INFO << "In AddWithSpinLock, elapsed time is " << timer.m_elapsed(0.0) << "ms";

  for (auto& t : v) {
    if (t.joinable()) {
      t.join();
    }
  }

  LOG_INFO << "In AddWithSpinLock: kAddSpinLock is " << kAddSpinLock;
}

}  // namespace

int main(int argc, char **argv) {

  if (argc > 1) {
    kThreadNum = atoi(argv[1]);
  }

  // Parses the command line for googletest flags, and removes all recognized flags.
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
