#include "tutil/string_view.h"

#include <string.h>
#include <iostream>
#include <sstream>
#include <gtest/gtest.h>

using namespace std;

namespace {

// The fixture for testing StringView.
class StringViewTest : public ::testing::Test {
 protected:
  // You can remove any or all of the following functions if its body
  // is empty.

  StringViewTest() {
     // You can do set-up work for each test here.
  }

  ~StringViewTest() override {
     // You can do clean-up work that doesn't throw exceptions here.
  }

  // If the constructor and destructor are not enough for setting up
  // and cleaning up each test, you can define the following methods:

  void SetUp() override {
     // Code here will be called immediately after the constructor (right
     // before each test).
     snprintf(str, sizeof(str), "hello, world!");
     //cout << "===== Set Up =====" << endl;
  }

  void TearDown() override {
     // Code here will be called immediately after each test (right
     // before the destructor).
     //cout << "===== Tear Down =====" << endl;
  }

  // Objects declared here can be used by all tests in the test case for Flags.
  char str[64];
  
}; // namespace StringViewTest

// Test constructor.
TEST_F(StringViewTest, Constructor) {
  cout << "===== Constructor/Copy =====" << endl;

  tutil::StringView string_view1;
  cout << "output string_view1: " << string_view1 << endl;
  ASSERT_EQ(string_view1.data(), nullptr);
  ASSERT_EQ(string_view1.length(), 0);
  ASSERT_EQ(string_view1.size(), 0);
  ASSERT_TRUE(string_view1.empty());

  tutil::StringView string_view2(str);
  cout << "output string_view2: " << string_view2 << endl;
  ASSERT_STREQ(string_view2.data(), str);
  ASSERT_EQ(string_view2.length(), strlen(str));
  ASSERT_EQ(string_view2.size(), strlen(str));
  ASSERT_FALSE(string_view2.empty());

  tutil::StringView string_view3(str, 5);
  cout << "output string_view3: " << string_view3 << endl;
  ASSERT_STREQ(string_view3.data(), str);
  ASSERT_EQ(string_view3.length(), 5);
  ASSERT_EQ(string_view3.size(), 5);
  ASSERT_FALSE(string_view3.empty());

  tutil::StringView string_view4(string_view2);
  cout << "output string_view4: " << string_view4 << endl;
  ASSERT_STREQ(string_view4.data(), string_view2.data());
  ASSERT_EQ(string_view4.length(), string_view2.length());
  ASSERT_EQ(string_view4.size(), string_view2.size());
  ASSERT_EQ(string_view4.empty(), string_view2.empty());

  tutil::StringView string_view5 = string_view2;
  cout << "output string_view5: " << string_view5 << endl;
  ASSERT_STREQ(string_view5.data(), string_view2.data());
  ASSERT_EQ(string_view5.length(), string_view2.length());
  ASSERT_EQ(string_view5.size(), string_view2.size());
  ASSERT_EQ(string_view5.empty(), string_view2.empty());
}

// Test iterators.
TEST_F(StringViewTest, Iterators) {
  cout << "===== Iterators =====" << endl;

  {
    cout << "test begin(), end()" << endl;
    tutil::StringView string_view(str);
    ostringstream oss;

    for (auto begin = string_view.begin();
        begin != string_view.end(); ++begin) {
      oss << *begin;
    }
    ASSERT_STREQ(oss.str().c_str(), str);
  }

  {
    cout << "test cbegin(), cend()" << endl;
    tutil::StringView string_view(str);
    ostringstream oss;

    for (auto begin = string_view.cbegin();
        begin != string_view.cend(); ++begin) {
      oss << *begin;
    }
    ASSERT_STREQ(oss.str().c_str(), str);
  }

  {
    cout << "test rbegin(), rend()" << endl;
    tutil::StringView string_view(str);
    ostringstream oss;

    for (auto begin = string_view.rbegin();
        begin != string_view.rend(); ++begin) {
      oss << *begin;
    }
    std::string str_back = oss.str();
    std::reverse(str_back.begin(), str_back.end());
    ASSERT_STREQ(str_back.c_str(), str);
  }

  {
    cout << "test crbegin(), crend()" << endl;
    tutil::StringView string_view(str);
    ostringstream oss;

    for (auto begin = string_view.crbegin();
        begin != string_view.crend(); ++begin) {
      oss << *begin;
    }
    std::string str_back = oss.str();
    std::reverse(str_back.begin(), str_back.end());
    ASSERT_STREQ(str_back.c_str(), str);
  }
}

// Test element access.
TEST_F(StringViewTest, ElementAccess) {
  cout << "===== Element Access =====" << endl;

  {
    cout << "test operator[]" << endl;
    ostringstream oss;
    tutil::StringView string_view(str);

    auto length = string_view.length();
    for (decltype(length) i = 0; i != length; i++) {
      oss << string_view[i];
    }
    ASSERT_STREQ(oss.str().c_str(), str);
  }

  {
    cout << "test at()" << endl;

    tutil::StringView string_view(str);
    auto length = string_view.length();

    ASSERT_TRUE('h' == string_view.at(0));
    ASSERT_TRUE('!' == string_view.at(length - 1));

    cout << "test front()" << endl;
    ASSERT_TRUE('h' == string_view.front());

    cout << "test front()" << endl;
    ASSERT_TRUE('h' == string_view.front());

    cout << "test back()" << endl;
    ASSERT_TRUE('!' == string_view.back());

    cout << "test data()" << endl;
    ASSERT_TRUE(str == string_view.data());
  }

  {
    cout << "test remove_prefix()" << endl;

    tutil::StringView string_view(str);
    auto length = string_view.length();
    string_view.remove_prefix(3);
    ASSERT_EQ(string_view.data(), str + 3);
    ASSERT_EQ(string_view.length(), length - 3);
  }

  {
    cout << "test remove_suffix()" << endl;

    tutil::StringView string_view(str);
    auto length = string_view.length();
    string_view.remove_suffix(3);
    ASSERT_EQ(string_view.data(), str);
    ASSERT_EQ(string_view.length(), length - 3);
  }
}

// Test Comparison function.
TEST_F(StringViewTest, ComparisonFunction) {
  cout << "===== Comparison function =====" << endl;

  {
    cout << "test operator==" << endl;

    tutil::StringView view1("123456");
    tutil::StringView view2("123456");
    tutil::StringView view3("123");

    char str1[] = "123456";
    char str2[] = "123";

    ASSERT_TRUE(view1 == view2);
    ASSERT_FALSE(view1 == view3);

    ASSERT_TRUE(view1 == str1);
    ASSERT_FALSE(view1 == str2);

    ASSERT_TRUE(str1 == view1);
    ASSERT_FALSE(str2 == view1);
  }

  {
    cout << "test operator!=" << endl;

    tutil::StringView view1("123456");
    tutil::StringView view2("123456");
    tutil::StringView view3("123");

    char str1[] = "123456";
    char str2[] = "123";

    ASSERT_TRUE(view1 != view3);
    ASSERT_FALSE(view1 != view2);

    ASSERT_FALSE(view1 != str1);
    ASSERT_TRUE(view1 != str2);

    ASSERT_FALSE(str1 != view1);
    ASSERT_TRUE(str2 != view1);
  }
}

// Test Access violation.
TEST_F(StringViewTest, AccessViolation) {
  cout << "===== Access violation =====" << endl;

  {
    cout << "test operator[]" << endl;
    tutil::StringView string_view(str);
    auto length = string_view.length();

    ASSERT_DEATH(string_view[length], "");
    ASSERT_DEATH(string_view[length + 3], "");
  }

  {
    cout << "test at" << endl;
    tutil::StringView string_view(str);
    auto length = string_view.length();

    ASSERT_DEATH(string_view.at(length), "");
    ASSERT_DEATH(string_view.at(length + 3), "");
  }

  {
    cout << "test front" << endl;
    tutil::StringView string_view;

    ASSERT_DEATH(string_view.front(), "");
  }

  {
    cout << "test back" << endl;
    tutil::StringView string_view;

    ASSERT_DEATH(string_view.back(), "");
  }

  {
    cout << "test remove_prefix" << endl;
    tutil::StringView string_view(str);
    auto length = string_view.length();

    ASSERT_DEATH(string_view.remove_prefix(length + 1), "");
  }

  {
    cout << "test remove_suffix" << endl;
    tutil::StringView string_view(str);
    auto length = string_view.length();

    ASSERT_DEATH(string_view.remove_suffix(length + 1), "");
  }
}

}  // namespace

int main(int argc, char **argv) {

  // Parses the command line for googletest flags, and removes all recognized flags.
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
