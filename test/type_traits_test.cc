#include "tutil/type_traits.h"

#include <string.h>
#include <iostream>
#include <type_traits>

#include <glog/logging.h>
#include <gtest/gtest.h>

using namespace std;
using namespace tesla::tutil;

namespace {

// The fixture for testing TypeTraits.
class TypeTraitsTest : public ::testing::Test {
 protected:
  // You can remove any or all of the following functions if its body
  // is empty.

  TypeTraitsTest() {
     // You can do set-up work for each test here.
  }

  ~TypeTraitsTest() override {
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
  
}; // namespace TypeTraitsTest

TEST_F(TypeTraitsTest, IntegralConstant) {
  {
    using one_type = integral_constant<int, 1>; 
    ASSERT_EQ(one_type::value, 1);
    std::cout << "the value of one_type is " << one_type::value << std::endl;
  }

  {
    ASSERT_EQ(true_type::value, true);
    ASSERT_NE(true_type::value, false);

    ASSERT_EQ(false_type::value, false);
    ASSERT_NE(false_type::value, true);
  }
}

TEST_F(TypeTraitsTest, Conditional) { 
  using A = std::conditional<true, int, float>::type;
  ASSERT_TRUE((std::is_same<A, int>::value));
  ASSERT_FALSE((std::is_same<A, float>::value));

  using B = std::conditional<false, int, float>::type;
  ASSERT_TRUE((std::is_same<B, float>::value));
  ASSERT_FALSE((std::is_same<B, int>::value));
}

TEST_F(TypeTraitsTest, Decay) {
  {
    using A = std::decay<int>::type;
    ASSERT_TRUE((std::is_same<A, int>::value));
  }

  {
    using A = std::decay<int&>::type;
    ASSERT_TRUE((std::is_same<A, int>::value));
    ASSERT_FALSE((std::is_same<A, int&>::value));
  }

  {
    using A = std::decay<int&&>::type;
    ASSERT_TRUE((std::is_same<A, int>::value));
    ASSERT_FALSE((std::is_same<A, int&&>::value));
  }

  {
    using A = std::decay<const int>::type;
    ASSERT_TRUE((std::is_same<A, int>::value));
    ASSERT_FALSE((std::is_same<A, const int>::value));
  }

  {
    using A = std::decay<const int&>::type;
    ASSERT_TRUE((std::is_same<A, int>::value));
    ASSERT_FALSE((std::is_same<A, const int&>::value));
  }

  {
    using A = std::decay<const int&&>::type;
    ASSERT_TRUE((std::is_same<A, int>::value));
    ASSERT_FALSE((std::is_same<A, const int&&>::value));
  }

  {
    using A = std::decay<int[3]>::type;
    ASSERT_TRUE((std::is_same<A, int*>::value));
    ASSERT_FALSE((std::is_same<A, int>::value));
    ASSERT_FALSE((std::is_same<A, int[3]>::value));
  }

  {
    using A = std::decay<int(int)>::type;
    ASSERT_TRUE((std::is_same<A, int(*)(int)>::value));
    ASSERT_FALSE((std::is_same<A, int(int)>::value));
  }
  
}

template <typename T>
typename std::enable_if<std::is_arithmetic<T>::value, bool>::type
IsArith(T t) {
  return true;
}

template <typename T>
typename std::enable_if<!std::is_arithmetic<T>::value, bool>::type
IsArith(T t) {
  return false;
}

TEST_F(TypeTraitsTest, EnableIf) {
  {
    auto r = IsArith(1);
    ASSERT_TRUE(r);
  }

  {
    auto r = IsArith(1.2);
    ASSERT_TRUE(r);
  }

  {
    auto r = IsArith("test");
    ASSERT_FALSE(r);
  }

}

TEST_F(TypeTraitsTest, Power) {
  {
    int value = Power<10, 0>::value;
    ASSERT_EQ(value, 1);
  }

  {
    int value = Power<10, 1>::value;
    ASSERT_EQ(value, 10);
  }

  {
    int value = Power<10, 2>::value;
    ASSERT_EQ(value, 100);
  }

  {
    int value = Power<10, 3>::value;
    ASSERT_EQ(value, 1000);
  }

  {
    int value = Power<10, 4>::value;
    ASSERT_EQ(value, 10000);
  }

  {
    int value = Power<10, 5>::value;
    ASSERT_EQ(value, 100000);
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
