#include "allocator/object_pool.h"

#include <string.h>
#include <iostream>
#include <gtest/gtest.h>

#include "tutil/compiler_specific.h"

using namespace std;
using namespace tesla::allocator;

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

TEST_F(ObjectPoolTest, SingleTest) {
  auto pool = ObjectPool<TestClass>::Singleton();

  for (size_t i = 0; i < 10; i++) {
    ASSERT_TRUE(ObjectPool<TestClass>::Singleton() == pool);
  }
}

}  // namespace

int main(int argc, char **argv) {

  // Parses the command line for googletest flags, and removes all recognized flags.
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
