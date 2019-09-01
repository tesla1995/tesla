#include "tutil/timer_heap.h"

#include <string.h>
#include <iostream>
#include <gtest/gtest.h>

using namespace std;
using namespace tesla::tutil;

namespace {

// The fixture for testing TimerHeap.
class TimerHeapTest : public ::testing::Test {
 protected:
  // You can remove any or all of the following functions if its body
  // is empty.

  TimerHeapTest() {
     // You can do set-up work for each test here.
  }

  ~TimerHeapTest() override {
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
  TimerHeap timer;
  
}; // namespace TimerHeapTest

TEST_F(TimerHeapTest, Empty) {
  timer.RemoveTimer(3);
  ASSERT_EQ(-1, timer.GetNextTimeout());
  ASSERT_FALSE(timer.HasNextTimeout());
}

TEST_F(TimerHeapTest, Single) {
  {
    TimerId id = timer.AddTimer(Timestamp::Now() + Duration(2.0), []{ cout << "timeout" << endl; });
    ASSERT_EQ(2, timer.GetNextTimeout());
    timer.RemoveTimer(id);
    ASSERT_EQ(-1, timer.GetNextTimeout());
    ASSERT_FALSE(timer.HasNextTimeout());
  }

  {
    timer.AddTimer(Timestamp::Now() + Duration(2.0), []{ cout << "timeout" << endl; });
    ASSERT_EQ(2, timer.GetNextTimeout());

    sleep(3);

    ASSERT_EQ(0, timer.GetNextTimeout());
    ASSERT_TRUE(timer.HasNextTimeout());
    timer.ExecuteNextTimeout();
    ASSERT_EQ(-1, timer.GetNextTimeout());
    ASSERT_FALSE(timer.HasNextTimeout());
  }
}

TEST_F(TimerHeapTest, Multi) {
  for (int i = 1; i < 6; i++) {
    timer.AddTimer(Timestamp::Now() + Duration((double)i), [i]{ cout << i << " timeout" << endl; });
  }
  timer.Traversal();
  sleep(11);
  ASSERT_EQ(0, timer.GetNextTimeout());

  while (timer.HasNextTimeout()) {
    timer.Traversal();
    timer.ExecuteNextTimeout();
  }
}

}  // namespace

int main(int argc, char **argv) {

  // Parses the command line for googletest flags, and removes all recognized flags.
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
