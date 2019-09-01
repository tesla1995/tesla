#include "tutil/strings/string_split.h"

#include <string.h>
#include <iostream>

#include <glog/logging.h>
#include <gtest/gtest.h>

using namespace std;
using namespace tutil;

namespace {

// The fixture for testing StringSplit.
class StringSplitTest : public ::testing::Test {
 protected:
  // You can remove any or all of the following functions if its body
  // is empty.

  StringSplitTest() {
     // You can do set-up work for each test here.
  }

  ~StringSplitTest() override {
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
  
}; // namespace StringSplitTest

TEST_F(StringSplitTest, SplitString) {
  // an empty source string.
  {
    string line;
    std::vector<std::string> pairs;
    SplitString(line, ';', true, pairs);

    ASSERT_EQ(pairs.size(), 0U);
  }

  // key-value pairs.
  {
    string line("latency=*_latency*"
                "; qps=*_qps*"
                "; error=*_error*"
                "; system=*process_*,*malloc_*,*kernel_*");

    std::vector<std::string> pairs;
    SplitString(line, ';', true, pairs);

    cout << "index " << 0 << ": " << pairs[0] << endl;
    cout << "index " << 1 << ": " << pairs[1] << endl;  
    cout << "index " << 2 << ": " << pairs[2] << endl;  
    cout << "index " << 3 << ": " << pairs[3] << endl;  

    ASSERT_EQ(pairs.size(), 4U);
    ASSERT_EQ(pairs[0], std::string("latency=*_latency*"));
    ASSERT_EQ(pairs[1], std::string("qps=*_qps*"));
    ASSERT_EQ(pairs[2], std::string("error=*_error*"));
    ASSERT_EQ(pairs[3], std::string("system=*process_*,*malloc_*,*kernel_*"));
  } 
}

TEST_F(StringSplitTest, SplitStringIntoKeyValue) {
  // no delimiter.
  {
    std::string pair("latency");
    std::string key;
    std::string value;
    
    bool ret = SplitStringIntoKeyValue(pair, '=', key, value);

    ASSERT_FALSE(ret);
  }

  // no value.
  {
    std::string pair("latency=");
    std::string key;
    std::string value;
    
    bool ret = SplitStringIntoKeyValue(pair, '=', key, value);

    ASSERT_FALSE(ret);
  }

  // no value.
  {
    std::string pair("latency==");
    std::string key;
    std::string value;
    
    bool ret = SplitStringIntoKeyValue(pair, '=', key, value);

    ASSERT_FALSE(ret);
  }

  // key-value pairs.
  {
    std::string pair("latency=*_latency*");
    std::string key;
    std::string value;
    
    bool ret = SplitStringIntoKeyValue(pair, '=', key, value);

    ASSERT_TRUE(ret);
    ASSERT_EQ(key, std::string("latency"));
    ASSERT_EQ(value, std::string("*_latency*"));
  }

  // key-value pairs.
  {
    std::string pair("latency===*_latency*");
    std::string key;
    std::string value;
    
    bool ret = SplitStringIntoKeyValue(pair, '=', key, value);

    ASSERT_TRUE(ret);
    ASSERT_EQ(key, std::string("latency"));
    ASSERT_EQ(value, std::string("*_latency*"));
  }
}

TEST_F(StringSplitTest, SplitStringIntoKeyValuePairs) {
  // empty
  {
    string line;
    StringPairs pairs;
    bool ret = SplitStringIntoKeyValuePairs(line, '=', ';', pairs);

    ASSERT_FALSE(ret);
  }

  {
    string line("latency=*_latency*"
        "; qps="
        "; error=*_error*"
        "; system=*process_*,*malloc_*,*kernel_*");
    StringPairs pairs;
    bool ret = SplitStringIntoKeyValuePairs(line, '=', ';', pairs);

    ASSERT_FALSE(ret);
  }

  // normal
  {
    string line("latency=*_latency*"
        "; qps=*_qps*"
        "; error=*_error*"
        "; system=*process_*,*malloc_*,*kernel_*");

    StringPairs pairs;
    bool ret = SplitStringIntoKeyValuePairs(line, '=', ';', pairs);

    ASSERT_TRUE(ret);
    ASSERT_EQ(pairs.size(), 4);

    auto& pair0 = pairs[0];
    ASSERT_EQ(pair0.first, std::string("latency"));
    ASSERT_EQ(pair0.second, std::string("*_latency*"));

    auto& pair1 = pairs[1];
    ASSERT_EQ(pair1.first, std::string("qps"));
    ASSERT_EQ(pair1.second, std::string("*_qps*"));

    auto& pair2 = pairs[2];
    ASSERT_EQ(pair2.first, std::string("error"));
    ASSERT_EQ(pair2.second, std::string("*_error*"));

    auto& pair3 = pairs[3];
    ASSERT_EQ(pair3.first, std::string("system"));
    ASSERT_EQ(pair3.second, std::string("*process_*,*malloc_*,*kernel_*"));
  }  
}

}  // namespace

int main(int argc, char **argv) {

  // Initialize Google's logging library.
  google::InitGoogleLogging(argv[0]);

  // Parses the command line for googletest flags, and removes all recognized flags.
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
