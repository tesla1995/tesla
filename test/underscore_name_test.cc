#include <iostream>
#include <string>
#include <gtest/gtest.h>
#include "tutil/string_view.h"

using namespace std;

void to_underscored_name(std::string& name, const tutil::StringView& src) {
  name.reserve(name.size() + src.size() + 8 /*just guess*/);
  for (auto p = src.begin(); p != src.end(); ++p) {
    if (isalpha(*p)) {
      if (*p < 'a') { // upper cases
        if (p != src.data() && !isupper(p[-1]) && name.back() != '_') {
          name.push_back('_');
        }
        name.push_back(*p - 'A' + 'a');
      } else {
        name.push_back(*p);
      }
    } else if (isdigit(*p)) {
      // push digits directly.
      name.push_back(*p);
    } else if (name.empty() || name.back() != '_') {
      // charaters which are not alpha and digit would be converted
      // into a underscope.
      name.push_back('_');
    }
  }
}

TEST(UnderscoreNameTest, HandleAnyInput) {
  {
    string name; 
    tutil::StringView src("foo-inl.h");
    to_underscored_name(name, src);
    ASSERT_STREQ(name.c_str(), "foo_inl_h");
  }

  {
    string name; 
    tutil::StringView src("foo::bar::Apple");
    to_underscored_name(name, src);
    ASSERT_STREQ(name.c_str(), "foo_bar_apple");
  }

  {
    string name; 
    tutil::StringView src("Car_Rot");
    to_underscored_name(name, src);
    ASSERT_STREQ(name.c_str(), "car_rot");
  }

  {
    string name; 
    tutil::StringView src("FooBar");
    to_underscored_name(name, src);
    ASSERT_STREQ(name.c_str(), "foo_bar");
  }

  {
    string name; 
    tutil::StringView src("RPCTest");
    to_underscored_name(name, src);
    ASSERT_STREQ(name.c_str(), "rpctest");
  }

  {
    string name; 
    tutil::StringView src("HELLO");
    to_underscored_name(name, src);
    ASSERT_STREQ(name.c_str(), "hello");
  }
}
