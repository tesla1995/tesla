#include "tutil/containers/bounded_queue.h"

#include <iostream>
#include <gtest/gtest.h>

#include "tutil/timestamp.h"

using namespace std;
using namespace tesla::tutil;

struct Sampler {
  int64_t data{0};
  Timestamp time;

  Sampler() = default;
  Sampler(int64_t data2, Timestamp time2)
    : data(data2),
      time(time2) {}
  ~Sampler() = default;
};


////////////////////////////////////////////////////////////////////////
namespace {

// The fixture for testing BoundedQueue.
class BoundedQueueTest : public ::testing::Test {
 protected:
  // You can remove any or all of the following functions if its body
  // is empty.

  BoundedQueueTest() {
     // You can do set-up work for each test here.
  }

  ~BoundedQueueTest() override {
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
}; // namespace BoundedQueueTest

TEST_F(BoundedQueueTest, DefaultConstructor) {
  BoundedQueue<int> queue;
  int tmp{0};

  ASSERT_FALSE(queue.initialized());
  ASSERT_FALSE(queue.ownership());
  ASSERT_EQ(queue.size(), 0);
  ASSERT_EQ(queue.capacity(), 0);
  ASSERT_TRUE(queue.empty());
  ASSERT_TRUE(queue.full());
  ASSERT_FALSE(queue.front());
  ASSERT_FALSE(queue.front(0));
  ASSERT_FALSE(queue.front(1));
  ASSERT_FALSE(queue.front(2));
  ASSERT_FALSE(queue.back());
  ASSERT_FALSE(queue.back(0));
  ASSERT_FALSE(queue.back(1));
  ASSERT_FALSE(queue.back(2));
  ASSERT_FALSE(queue.push_back());
  ASSERT_FALSE(queue.push_back(1));
  ASSERT_FALSE(queue.push_front());
  ASSERT_FALSE(queue.push_front(1));
  ASSERT_FALSE(queue.pop_back());
  ASSERT_FALSE(queue.pop_front());
  ASSERT_FALSE(queue.pop_back(tmp));
  ASSERT_FALSE(queue.pop_front(tmp));
}

TEST_F(BoundedQueueTest, DefaultItem) {
  size_t capacity = sizeof(int) * 5;
  char* p = (char*)malloc(capacity);  
  ASSERT_TRUE(p != NULL);

  // init
  BoundedQueue<int> queue(p, capacity, StorageOwnership::kOwnsStorage);
  queue.push_back();
  queue.push_front();
}

TEST_F(BoundedQueueTest, ScalarType) {
  size_t size = 0;
  size_t capacity = sizeof(int) * 5;
  char* p = (char*)malloc(capacity);  
  ASSERT_TRUE(p != NULL);

  // init
  BoundedQueue<int> queue(p, capacity, StorageOwnership::kOwnsStorage);
  ASSERT_TRUE(queue.initialized());
  ASSERT_TRUE(queue.ownership());
  ASSERT_EQ(queue.size(), 0);
  ASSERT_EQ(queue.capacity(), capacity / sizeof(int));
  ASSERT_TRUE(queue.empty());
  ASSERT_FALSE(queue.full());

  // push_back 1 - true
  ASSERT_TRUE(queue.push_back(1));
  ASSERT_EQ(queue.size(), ++size);
  ASSERT_FALSE(queue.empty());
  ASSERT_FALSE(queue.full());
  ASSERT_EQ(*queue.front(), 1);
  ASSERT_EQ(*queue.back(), 1);

  // push_back 2 - true
  ASSERT_TRUE(queue.push_back(2));
  ASSERT_EQ(queue.size(), ++size);
  ASSERT_FALSE(queue.full());
  ASSERT_FALSE(queue.empty());
  ASSERT_EQ(*queue.front(), 1);
  ASSERT_EQ(*queue.back(), 2);

  // push_back 3 - true
  ASSERT_TRUE(queue.push_back(3));
  ASSERT_EQ(queue.size(), ++size);
  ASSERT_FALSE(queue.full());
  ASSERT_FALSE(queue.empty());
  ASSERT_EQ(*queue.front(), 1);
  ASSERT_EQ(*queue.back(), 3);

  // push_back 4 - true
  ASSERT_TRUE(queue.push_back(4));
  ASSERT_EQ(queue.size(), ++size);
  ASSERT_FALSE(queue.full());
  ASSERT_FALSE(queue.empty());
  ASSERT_EQ(*queue.front(), 1);
  ASSERT_EQ(*queue.back(), 4);

  // push_back 5 - true
  ASSERT_TRUE(queue.push_back(5));
  ASSERT_EQ(queue.size(), ++size);
  ASSERT_TRUE(queue.full());
  ASSERT_FALSE(queue.empty());
  ASSERT_EQ(*queue.front(), 1);
  ASSERT_EQ(*queue.back(), 5);

  // push_back 6 - false
  ASSERT_FALSE(queue.push_back(6));
  ASSERT_EQ(queue.size(), size);
  ASSERT_TRUE(queue.full());
  ASSERT_FALSE(queue.empty());
  ASSERT_EQ(*queue.front(), 1);
  ASSERT_EQ(*queue.back(), 5);

  // push_back 7 - false
  ASSERT_FALSE(queue.push_back(7));
  ASSERT_EQ(queue.size(), size);
  ASSERT_TRUE(queue.full());
  ASSERT_FALSE(queue.empty());
  ASSERT_EQ(*queue.front(), 1);
  ASSERT_EQ(*queue.back(), 5);

  // pop_front 1 - true
  int tmp = -1;
  ASSERT_TRUE(queue.pop_front(tmp));
  ASSERT_EQ(tmp, 1);
  ASSERT_EQ(queue.size(), --size);
  ASSERT_FALSE(queue.full());
  ASSERT_FALSE(queue.empty());
  ASSERT_EQ(*queue.front(), 2);
  ASSERT_EQ(*queue.back(), 5);

  // pop_back 5 - true
  // X 2 3 4 X
  ASSERT_TRUE(queue.pop_back(tmp));
  ASSERT_EQ(tmp, 5);
  ASSERT_EQ(queue.size(), --size);
  ASSERT_FALSE(queue.full());
  ASSERT_FALSE(queue.empty());
  ASSERT_EQ(*queue.front(), 2);
  ASSERT_EQ(*queue.back(), 4);


  // push_back 6 - true
  // X 2 3 4 6
  ASSERT_TRUE(queue.push_back(6));
  ASSERT_EQ(queue.size(), ++size);
  ASSERT_EQ(*queue.front(), 2);
  ASSERT_EQ(*queue.back(), 6);
  ASSERT_FALSE(queue.full());
  ASSERT_FALSE(queue.empty());


  // push_back 7 - true
  // 7 2 3 4 6
  ASSERT_TRUE(queue.push_back(7));
  ASSERT_EQ(queue.size(), ++size);
  ASSERT_EQ(*queue.front(), 2);
  ASSERT_EQ(*queue.back(), 7);
  ASSERT_TRUE(queue.full());
  ASSERT_FALSE(queue.empty());

  // pop_back 7 - true
  // X 2 3 4 6
  ASSERT_TRUE(queue.pop_back(tmp));
  ASSERT_EQ(tmp, 7);
  ASSERT_EQ(queue.size(), --size);
  ASSERT_FALSE(queue.full());
  ASSERT_FALSE(queue.empty());
  ASSERT_EQ(*queue.front(), 2);
  ASSERT_EQ(*queue.back(), 6);
  
  // pop_back 6 - true
  // X 2 3 4 X
  ASSERT_TRUE(queue.pop_back(tmp));
  ASSERT_EQ(tmp, 6);
  ASSERT_EQ(queue.size(), --size);
  ASSERT_FALSE(queue.full());
  ASSERT_FALSE(queue.empty());
  ASSERT_EQ(*queue.front(), 2);
  ASSERT_EQ(*queue.back(), 4);

  // push_front 1 - true
  // 1 2 3 4 X
  ASSERT_TRUE(queue.push_front(1));
  ASSERT_EQ(queue.size(), ++size);
  ASSERT_FALSE(queue.full());
  ASSERT_FALSE(queue.empty());
  ASSERT_EQ(*queue.front(), 1);
  ASSERT_EQ(*queue.back(), 4);

  // push_front 5 - true
  // 1 2 3 4 5
  ASSERT_TRUE(queue.push_front(5));
  ASSERT_EQ(queue.size(), ++size);
  ASSERT_TRUE(queue.full());
  ASSERT_FALSE(queue.empty());
  ASSERT_EQ(*queue.front(), 5);
  ASSERT_EQ(*queue.back(), 4);

  // eliminate_push 6
  // 1 2 3 4 6
  queue.eliminate_push(6);
  ASSERT_EQ(queue.size(), size);
  ASSERT_TRUE(queue.full());
  ASSERT_FALSE(queue.empty());
  ASSERT_EQ(*queue.front(), 1);
  ASSERT_EQ(*queue.back(), 6);
}

TEST_F(BoundedQueueTest, RandomlyAccess) {

  size_t size = 0;
  size_t capacity = sizeof(Sampler) * 5;
  char* p = (char*)malloc(capacity);  
  ASSERT_TRUE(p != NULL);

  // init
  BoundedQueue<Sampler> queue(p, capacity, StorageOwnership::kOwnsStorage);
  ASSERT_TRUE(queue.initialized());
  ASSERT_TRUE(queue.ownership());
  ASSERT_EQ(queue.size(), 0);
  ASSERT_EQ(queue.capacity(), capacity / sizeof(Sampler));
  ASSERT_TRUE(queue.empty());
  ASSERT_FALSE(queue.full());

  // push_back 1 - true
  ASSERT_TRUE(queue.push_back(1, Timestamp::Now()));
  ASSERT_EQ(queue.size(), ++size);
  ASSERT_FALSE(queue.empty());
  ASSERT_FALSE(queue.full());
  ASSERT_EQ(queue.front()->data, 1);
  ASSERT_EQ(queue.back()->data, 1);

  // push_front 0 - true
  ASSERT_TRUE(queue.push_front(0, Timestamp::Now()));
  ASSERT_EQ(queue.size(), ++size);
  ASSERT_FALSE(queue.empty());
  ASSERT_FALSE(queue.full());
  ASSERT_EQ(queue.front()->data, 0);
  ASSERT_EQ(queue.back()->data, 1);

  // push_back 2 - true
  ASSERT_TRUE(queue.push_back(2, Timestamp::Now()));
  ASSERT_EQ(queue.size(), ++size);
  ASSERT_FALSE(queue.empty());
  ASSERT_FALSE(queue.full());
  ASSERT_EQ(queue.front()->data, 0);
  ASSERT_EQ(queue.back()->data, 2);

  // push_back 3 - true
  ASSERT_TRUE(queue.push_back(3, Timestamp::Now()));
  ASSERT_EQ(queue.size(), ++size);
  ASSERT_FALSE(queue.empty());
  ASSERT_FALSE(queue.full());
  ASSERT_EQ(queue.front()->data, 0);
  ASSERT_EQ(queue.back()->data, 3);

  // push_back 4 - true
  ASSERT_TRUE(queue.push_back(4, Timestamp::Now()));
  ASSERT_EQ(queue.size(), ++size);
  ASSERT_FALSE(queue.empty());
  ASSERT_TRUE(queue.full());
  ASSERT_EQ(queue.front()->data, 0);
  ASSERT_EQ(queue.back()->data, 4);

  // front(index), back(index)
  for (size_t i = 0; i < queue.size(); i++) {
    ASSERT_EQ(queue.front(i)->data, i);
    ASSERT_EQ(queue.back(i)->data, queue.size() - i - 1); 
  }
}

TEST_F(BoundedQueueTest, ClassMember) {
  class Foo {
   public:
    explicit Foo(size_t capacity) : capacity_(capacity) {}
    ~Foo() = default;
  
    bool Init() {
      BoundedQueue<int> tmp(capacity_);
      if (!tmp.initialized()) {
        return false;
      }
      tmp.swap(queue_);
      return true;
    }

    size_t queue_capacity() { return queue_.capacity(); }

   private: 
    size_t capacity_{0};
    BoundedQueue<int> queue_;
  };

  Foo test(3);
  ASSERT_TRUE(test.Init());
  ASSERT_EQ(3, test.queue_capacity());
}

}  // namespace

int main(int argc, char **argv) {

  // Parses the command line for googletest flags, and removes all recognized flags.
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
