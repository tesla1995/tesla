#include "allocator/object_pool.h"

#include <string.h>
#include <iostream>
#include <vector>
#include <gtest/gtest.h>

#include "tutil/compiler_specific.h"
#include "tutil/time.h"

using namespace std;
using namespace tesla::allocator;
using namespace tesla::tutil;

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

struct YellObj {
  char dummy_[96];
};

struct SilentObj {
  char buf[sizeof(YellObj)];
};

struct NonDestructObj {
  int a{0};
  int b{0};

  NonDestructObj()
    : a(1),
      b(2) {

  }

  ~NonDestructObj() {
    a = 0;
    b = 0;
  }
};

TEST_F(ObjectPoolTest, NonDestructObj) {
  auto obj = ObjectPool<NonDestructObj>::Singleton()->New();
  ASSERT_EQ(obj->a, 1);
  ASSERT_EQ(obj->b, 2);

  ObjectPool<NonDestructObj>::Singleton()->Delete(obj);
  // ONLY for test, normally need to clear object before use.
  obj = ObjectPool<NonDestructObj>::Singleton()->New();
  ASSERT_EQ(obj->a, 1);
  ASSERT_EQ(obj->b, 2);
}

//TEST_F(ObjectPoolTest, Sanity) {
//  std::vector<SilentObj*> list;
//
//  size_t num_items = ObjectPool<SilentObj>::kNumItemsInBlock;
//  cout << "SilentObj:: num_items=" << num_items << endl;
//
//  list.reserve(2 * num_items + 5);
//  for (size_t i = 0; i < list.capacity(); i++) {
//    list.push_back(ObjectPool<SilentObj>::Singleton()->New()); 
//    ASSERT_EQ(ObjectPool<SilentObj>::Singleton()->GetLocalPoolNumItems()%num_items, (i+1)%num_items);
//  }
//  ASSERT_EQ(ObjectPool<SilentObj>::Singleton()->GetLocalPoolNumItems(), 5);
//
//  for (size_t i = 0; i < list.capacity(); i++) {
//    ObjectPool<SilentObj>::Singleton()->Delete(list[i]);
//    ASSERT_EQ(ObjectPool<SilentObj>::Singleton()->GetLocalPoolNumFreeItems()%num_items, (i+1)%num_items);
//  }
//  ASSERT_EQ(ObjectPool<SilentObj>::Singleton()->GetLocalPoolNumFreeItems(), 5);
//
//  list.clear();
//}

TEST_F(ObjectPoolTest, NewPerformance) {
  size_t N = 10000;
  std::vector<SilentObj*> list1;
  std::vector<SilentObj*> list2;
  list1.reserve(N);
  list2.reserve(N);

  // warm up
  ObjectPool<SilentObj>::Singleton()->Delete(ObjectPool<SilentObj>::Singleton()->New());
  delete (new SilentObj);

  for (size_t j = 0; j < 2; j++) {
    Timer timer;
    Timer timer1, timer2;
    int64_t max_latency = 0;
    int index = 0;

    timer1.start();
    for (size_t i = 0; i < N; i++) {
      list1.push_back(ObjectPool<SilentObj>::Singleton()->New());
    }
    timer1.stop();
    printf("New a SilentObj from ObjectPool takes %luns\n", timer1.n_elapsed()/N);

    for (size_t i = 0; i < N; i++) {
      ObjectPool<SilentObj>::Singleton()->Delete(list1[i]);
      if (timer.n_elapsed() > max_latency) { max_latency = timer.n_elapsed(); index = i; }
    }
    list1.clear();

    timer2.start();
    for (size_t i = 0; i < N; i++) {
      list2.push_back(new SilentObj);
    }
    timer2.stop();
    printf("New a SilentObj from system takes %luns\n", timer2.n_elapsed()/N);

    for (size_t i = 0; i < N; i++) {
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
