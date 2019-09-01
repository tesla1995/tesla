#include "tutil/string_splitter.h"

#include <string.h>
#include <iostream>
#include <gtest/gtest.h>

using namespace std;

namespace {

// The fixture for testing StringSplitter.
class StringSplitterTest : public ::testing::Test {
 protected:
  // You can remove any or all of the following functions if its body
  // is empty.

  StringSplitterTest() {
     // You can do set-up work for each test here.
  }

  ~StringSplitterTest() override {
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
  
}; // namespace StringSplitterTest

// Test constructor.
TEST_F(StringSplitterTest, Iterator) {
  cout << "===== test StringSplitter =====" << endl;

  const char* the_string_to_split = ",I,will,,be,a,great,man,,,";
  tutil::StringSplitter s(the_string_to_split, ',');
  for (int i = 1; s; ++s, ++i) {
    string str(s.field(), s.length());

    switch (i) {
      case 1:
        ASSERT_EQ(str, string("I")); 
      break;

      case 2:
        ASSERT_EQ(str, string("will")); 
      break;

      case 3:
        ASSERT_EQ(str, string("be")); 
      break;

      case 4:
        ASSERT_EQ(str, string("a")); 
      break;

      case 5:
        ASSERT_EQ(str, string("great")); 
      break;

      case 6:
        ASSERT_EQ(str, string("man")); 
      break;

      default:
        ASSERT_TRUE(false) << str << "the_string_to_split is modified";
    }
  }

  // bug
  const char* string_split = ",I,will,,be,a,great,man,";
  tutil::StringSplitter s1(string_split, ',',
                           tutil::EmptyFieldAction::ALLOW_EMPTY_FIELD);
  for (int i = 1; s1; ++s1, ++i) {
    cout << "the " << i <<" field: " << string(s1.field(), s1.length()) << endl;
  }
}

}  // namespace

int main(int argc, char **argv) {

  // Parses the command line for googletest flags, and removes all recognized flags.
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
