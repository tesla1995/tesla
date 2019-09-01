#include "tutil/wildcard_matcher.h"

#include <string.h>
#include <iostream>
#include <gtest/gtest.h>

using namespace std;

namespace {

// The fixture for testing WildcardMatcher.
class WildcardMatcherTest : public ::testing::Test {
 protected:
  // You can remove any or all of the following functions if its body
  // is empty.

  WildcardMatcherTest() {
     // You can do set-up work for each test here.
  }

  ~WildcardMatcherTest() override {
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
  
}; // namespace WildcardMatcherTest

TEST_F(WildcardMatcherTest, Match) {

  std::string str("qiuy*,tesla,ju?u");

  tutil::WildcardMatcher matcher(str, '?', false);

  ASSERT_TRUE(matcher.Match("qiuy"));
  ASSERT_TRUE(matcher.Match("qiuyan"));

  ASSERT_TRUE(matcher.Match("tesla"));
  ASSERT_FALSE(matcher.Match("zhuzhu"));
  
  ASSERT_FALSE(matcher.Match("juu"));
  ASSERT_TRUE(matcher.Match("juju"));
  ASSERT_FALSE(matcher.Match("jujju"));

}

}  // namespace

int main(int argc, char **argv) {

  // Parses the command line for googletest flags, and removes all recognized flags.
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
