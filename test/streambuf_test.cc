#include "tutil/streambuf.h"

#include <string.h>
#include <iostream>
#include <gtest/gtest.h>

using namespace std;

namespace {

// The fixture for testing StreamBuf.
class StreamBufTest : public ::testing::Test {
 protected:
  // You can remove any or all of the following functions if its body
  // is empty.

  StreamBufTest() {
     // You can do set-up work for each test here.
  }

  ~StreamBufTest() override {
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
  
}; // namespace StreamBufTest

// Test constructor.
TEST_F(StreamBufTest, CharArrayStreamBufTest) {

  cout << "===== test CharArrayStreamBuf =====" << endl;

  tutil::CharArrayStreamBuf streambuf;
  ostream os(&streambuf); 
  os << 'a';
  os << 'b';
  os << 'c';
  cout << streambuf.data() << endl;
  //ASSERT_STREQ(streambuf.data().data(), "abc");
  ASSERT_TRUE(os.good());
}

}  // namespace

int main(int argc, char **argv) {

  // Parses the command line for googletest flags, and removes all recognized flags.
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
