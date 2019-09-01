#include "tutil/timestamp.h"

#include <string.h>
#include <iostream>
#include <gtest/gtest.h>

using namespace std;
using namespace tesla::tutil;

namespace {

// The fixture for testing Duration.
class DurationTest : public ::testing::Test {
 protected:
  // You can remove any or all of the following functions if its body
  // is empty.

  DurationTest() {
     // You can do set-up work for each test here.
  }

  ~DurationTest() override {
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
  
}; // namespace DurationTest


TEST_F(DurationTest, ComplexAssignmentOperator) {
  // operator +=
  {
    Duration d(3.0);
    auto s = d.Seconds();
    d += Duration(2.0);
    ASSERT_EQ(d.Seconds(), s + 2);
  }

  // operator +=
  {
    Duration d(3.0);
    auto s = d.Seconds();
    d -= Duration(2.0);
    ASSERT_EQ(d.Seconds(), s - 2);
  }

  // operator * 
  {
    Duration d(3.0);
    auto s = d.Seconds();
    d *= 2;
    ASSERT_EQ(d.Seconds(), s * 2);
  }

  // operator / 
  {
    Duration d(8.0);
    auto s = d.Seconds();
    d /= 2;
    ASSERT_EQ(d.Seconds(), s / 2);
  }
}

}  // namespace

int main(int argc, char **argv) {

  // Parses the command line for googletest flags, and removes all recognized flags.
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
