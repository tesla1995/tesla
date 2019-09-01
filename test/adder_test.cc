#include "tvar/reducer.h"

#include <string.h>
#include <iostream>
#include <thread>
#include <gtest/gtest.h>

#include "tutil/count_down_latch.h"

using namespace tesla::tutil;
using namespace tesla::tvar;
using namespace std;

static size_t kThreadNum = 4;
CountDownLatch latch(kThreadNum);

void AddNum(Adder<int>* p) {
  *p << 1 << 2 << 3;
  latch.CountDown();
}

namespace {

// The fixture for testing Adder.
class AdderTest : public ::testing::Test {
 protected:
  // You can remove any or all of the following functions if its body
  // is empty.

  AdderTest() {
     // You can do set-up work for each test here.
  }

  ~AdderTest() override {
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
  Adder<int> adder;
  
}; // namespace AdderTest

// Test constructor.
TEST_F(AdderTest, SingleThread) {
  adder << 1 << 2 << 3;
  ASSERT_EQ(adder.GetValue(), 6);
}

TEST_F(AdderTest, MultiThread) {
  std::vector<std::thread> v;

  for (size_t i = 0; i < kThreadNum; ++i) {
    v.push_back(std::thread(AddNum, &adder));
  }
  latch.Wait();
  ASSERT_EQ(adder.GetValue(), 6 * kThreadNum);

  for (auto& t : v) {
    if (t.joinable()) {
      t.join();
    }
  }
}

}  // namespace

int main(int argc, char **argv) {

  // Parses the command line for googletest flags, and removes all recognized flags.
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
