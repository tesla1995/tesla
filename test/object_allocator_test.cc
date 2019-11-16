#include "allocator/object_allocator.h"

#include <string.h>
#include <iostream>
#include <gtest/gtest.h>

#include "tutil/compiler_specific.h"

using namespace std;
using namespace tesla::allocator;

namespace {

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

// The fixture for testing ObjectAllocator.
class ObjectAllocatorTest : public ::testing::Test {
 protected:
  // You can remove any or all of the following functions if its body
  // is empty.

  ObjectAllocatorTest() {
     // You can do set-up work for each test here.
  }

  ~ObjectAllocatorTest() override {
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
  ObjectAllocator<TestClass> test_allocator_;
  
}; // namespace ObjectAllocatorTest

TEST_F(ObjectAllocatorTest, inuse) {
  test_allocator_.Init();
  ASSERT_TRUE(test_allocator_.inuse() == 0);

  TestClass* p = test_allocator_.New();
  ASSERT_TRUE(p != nullptr);
  ASSERT_TRUE(test_allocator_.inuse() == 1);

  p->set_name("tesla");
  p->set_value("happy");

  test_allocator_.Delete(p);
  ASSERT_TRUE(test_allocator_.inuse() == 0);

  // ONLY for test free list.
  p = test_allocator_.New();
  ASSERT_TRUE(p != nullptr);
  ASSERT_TRUE(test_allocator_.inuse() == 1);
  ASSERT_EQ(p->name(), "tesla");
  ASSERT_EQ(p->value(), "happy");
}

}  // namespace

int main(int argc, char **argv) {

  // Parses the command line for googletest flags, and removes all recognized flags.
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
