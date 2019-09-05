#include "tutil/explicitly_constructed.h"

#include <string>
#include <gtest/gtest.h>

using namespace std;
using namespace tesla::tutil;

class TestClass {
 public:
  TestClass() = default;
  TestClass(int a) : a_(a) {}
  TestClass(int a, int b) : a_(a), b_(b) {}
  ~TestClass() = default;
 public:
  int get_a() const { return a_; }
  int get_b() const { return b_; }
  void set_a(int a) { a_ = a; }
  void set_b(int b) { b_ = b; }
 private:
  int a_{1};
  int b_{2};
  std::string name_{"tesla"};
};

namespace {

// The fixture for testing ExplicitlyConstructed.
class ExplicitlyConstructedTest : public ::testing::Test {
 protected:
  // You can remove any or all of the following functions if its body
  // is empty.

  ExplicitlyConstructedTest() {
     // You can do set-up work for each test here.
  }

  ~ExplicitlyConstructedTest() override {
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
  
}; // namespace ExplicitlyConstructedTest

TEST_F(ExplicitlyConstructedTest, AlignmentValue) {
  ASSERT_EQ(alignof(ExplicitlyConstructed<short>), alignof(short));
  ASSERT_EQ(alignof(ExplicitlyConstructed<int>), alignof(int));
  ASSERT_EQ(alignof(ExplicitlyConstructed<long>), alignof(long));
  ASSERT_EQ(alignof(ExplicitlyConstructed<float>), alignof(float));
  ASSERT_EQ(alignof(ExplicitlyConstructed<double>), alignof(double));

  ASSERT_EQ(alignof(std::string), alignof(std::string));
  ASSERT_EQ(alignof(ExplicitlyConstructed<TestClass>), alignof(TestClass));
}

TEST_F(ExplicitlyConstructedTest, DefaultConstruct) {
  ExplicitlyConstructed<TestClass> wrapper;
  wrapper.DefaultConstruct();

  ASSERT_EQ(wrapper.get().get_a(), 1);
  ASSERT_EQ(wrapper.get().get_b(), 2);

  wrapper.Destruct();
}

TEST_F(ExplicitlyConstructedTest, Construct) {
  ExplicitlyConstructed<TestClass> wrapper;

  {
    wrapper.Construct(3);
    ASSERT_EQ(wrapper.get().get_a(), 3);
    ASSERT_EQ(wrapper.get().get_b(), 2);
    wrapper.Destruct();
  }

  {
    wrapper.Construct(4, 5);
    ASSERT_EQ(wrapper.get().get_a(), 4);
    ASSERT_EQ(wrapper.get().get_b(), 5);
    wrapper.Destruct();
  }
}

TEST_F(ExplicitlyConstructedTest, get_mutable) {
  ExplicitlyConstructed<TestClass> wrapper;
  wrapper.Construct(4, 5);
  ASSERT_EQ(wrapper.get().get_a(), 4);
  ASSERT_EQ(wrapper.get().get_b(), 5);

  wrapper.get_mutable()->set_a(6);
  wrapper.get_mutable()->set_b(7);
  ASSERT_EQ(wrapper.get().get_a(), 6);
  ASSERT_EQ(wrapper.get().get_b(), 7);

  wrapper.Destruct();
}

}  // namespace

int main(int argc, char **argv) {

  // Parses the command line for googletest flags, and removes all recognized flags.
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
