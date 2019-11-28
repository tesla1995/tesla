#include "allocator/object_pool_baidu.h"

#include <string.h>
#include <iostream>
#include <vector>
#include <gtest/gtest.h>

#include "tutil/time.h"

using namespace std;
using namespace tesla::tutil;
using namespace baidu;

class TestClass {
 public:
  TestClass() {}
  ~TestClass() {}

  const std::string& name() { return name_; }
  void set_name(const std::string& name) { name_ = name; }

  const std::string& value() { return value_; }
  void set_value(const std::string& value) { value_ = value; }

 private:
  char buf[sizeof(void*)]; // for test
  std::string name_;
  std::string value_;
} TESLA_CACHELINE_ALIGNMENT;

namespace {

// The fixture for testing ObjectPool.
class ObjectPoolTest : public ::testing::Test {
 protected:
  // You can remove any or all of the following functions if its body
  // is empty.

  ObjectPoolTest() {
     // You can do set-up work for each test here.
  }

  ~ObjectPoolTest() override {
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
  
}; // namespace ObjectPoolTest


struct YellObj {
  char dummy_[96];
};

struct SilentObj {
  char buf[sizeof(YellObj)];
};

TEST_F(ObjectPoolTest, NewPerformance) {
  size_t N = 10000;
  std::vector<SilentObj*> list1;
  std::vector<SilentObj*> list2;
  list1.reserve(N);
  list2.reserve(N);

  Timer tm1;
  Timer tm2;

  // warm up
  return_object(get_object<SilentObj>());
  delete (new SilentObj);

  // Run twice, the second time will be must faster.
  for (size_t j = 0; j < 2; ++j) {

    tm1.start();
    for (size_t i = 0; i < N; ++i) {
      list1.push_back(get_object<SilentObj>());
    }
    tm1.stop();
    printf("get a SilentObj takes %luns\n", tm1.n_elapsed()/N);
    //clear_objects<SilentObj>(); // free all blocks
    for (size_t i = 0; i < list1.size(); ++i) {
      return_object<SilentObj>(list1[i]);
    }
    list1.clear();

    tm2.start();
    for (size_t i = 0; i < N; ++i) {
      list2.push_back(new SilentObj);
    }
    tm2.stop();
    printf("new a SilentObj takes %luns\n", tm2.n_elapsed()/N);
    for (size_t i = 0; i < list2.size(); ++i) {
      delete list2[i];
    }
    list2.clear();
  }
}

}  // namespace

int main(int argc, char **argv) {
  // Parses the command line for googletest flags, and removes all recognized flags.
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
