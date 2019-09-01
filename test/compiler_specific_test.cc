#include "tutil/compiler_specific.h"

#include <string.h>
#include <iostream>

#include <glog/logging.h>
#include <gtest/gtest.h>

using namespace std;

namespace {

// The fixture for testing CompilerSpecific.
class CompilerSpecificTest : public ::testing::Test {
 protected:
  // You can remove any or all of the following functions if its body
  // is empty.

  CompilerSpecificTest() {
     // You can do set-up work for each test here.
  }

  ~CompilerSpecificTest() override {
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
  
}; // namespace CompilerSpecificTest

struct X1 {
  char a[64];
  int b;
};

struct TESLA_CACHELINE_ALIGNMENT X2 {
  char a[64];
  int b;
};


TEST_F(CompilerSpecificTest, cacheline) {
  ASSERT_EQ(sizeof(X1), 68); 
  ASSERT_EQ(sizeof(X2), 128); 
}

}  // namespace

int main(int argc, char **argv) {

  // Initialize Google's logging library.
  google::InitGoogleLogging(argv[0]);

  // Parses the command line for googletest flags, and removes all recognized flags.
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
