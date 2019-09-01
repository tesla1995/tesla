#include "tutil/timestamp.h"

#include <string.h>
#include <iostream>
#include <gtest/gtest.h>

using namespace std;
using namespace tesla::tutil;

namespace {

// The fixture for testing Timestamp.
class TimestampTest : public ::testing::Test {
 protected:
  // You can remove any or all of the following functions if its body
  // is empty.

  TimestampTest() {
     // You can do set-up work for each test here.
  }

  ~TimestampTest() override {
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
  
}; // namespace TimestampTest

TEST_F(TimestampTest, Valid) {
  {
    struct timeval now;
    gettimeofday(&now, NULL); 
    Timestamp t(now);
    ASSERT_TRUE(t.IsValid());
  }

  {
    Timestamp t;
    ASSERT_FALSE(t.IsValid());
  }
  
}

TEST_F(TimestampTest, TimeVal) {
  struct timeval now;
  gettimeofday(&now, NULL); 
  Timestamp t(now);
  
  struct timeval from_t = t.TimeVal();
  ASSERT_EQ(from_t.tv_sec, now.tv_sec);
  ASSERT_EQ(from_t.tv_usec, now.tv_usec);
}

TEST_F(TimestampTest, TimeSpec) {
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now); 
  Timestamp t(now);
  
  struct timespec from_t = t.TimeSpec();
  ASSERT_EQ(from_t.tv_sec, now.tv_sec);
  ASSERT_EQ(from_t.tv_nsec, now.tv_nsec);
}

TEST_F(TimestampTest, UnixTime) {
  struct timeval now;
  gettimeofday(&now, NULL); 
  Timestamp t(now);
  
  int64_t n_s = now.tv_sec * 1000000000L + now.tv_usec * 1000L;
  ASSERT_EQ(t.UnixNanoseconds(), n_s);

  int64_t u_s = now.tv_sec * 1000000L + now.tv_usec;
  ASSERT_EQ(t.UnixMicroseconds(), u_s);

  int64_t m_s = now.tv_sec * 1000L + now.tv_usec / 1000L;
  ASSERT_EQ(t.UnixMilliseconds(), m_s);

  ASSERT_EQ(t.UnixSeconds(), now.tv_sec);
}

TEST_F(TimestampTest, RelationalOperator) {
  struct timeval now;
  gettimeofday(&now, NULL); 
  Timestamp t1(now);

  // operator ==
  Timestamp t2(now);
  ASSERT_TRUE(t1 == t2);
  
  now.tv_sec += 3;
  Timestamp t3(now);
  // operator < 
  ASSERT_TRUE(t1 < t3);
  ASSERT_TRUE(t2 < t3);

  // operator >
  ASSERT_TRUE(t3 > t2);
  ASSERT_TRUE(t3 > t1);
}

TEST_F(TimestampTest, Operator) {
  // Add Duration
  {
    struct timeval now;
    gettimeofday(&now, NULL); 
    Timestamp t1(now);

    Duration d(3.0);

    int64_t s = t1.UnixSeconds();
    t1.Add(d); 
    ASSERT_EQ(t1.UnixSeconds(), s + 3);
  }

  // operator+= Duration
  {
    struct timeval now;
    gettimeofday(&now, NULL); 
    Timestamp t1(now);

    Duration d(3.0);
    
    t1 += d;
    ASSERT_EQ(t1.UnixSeconds(), now.tv_sec + 3);
  }

  // operator+ Duration
  {
    struct timeval now;
    gettimeofday(&now, NULL); 
    Timestamp t1(now);

    Duration d(3.0);
    
    Timestamp t2 = t1 + d;
    ASSERT_EQ(t2.UnixSeconds(), now.tv_sec + 3);
  }

  // operator-= Duration
  {
    struct timeval now;
    gettimeofday(&now, NULL); 
    Timestamp t1(now);
    ASSERT_EQ(t1.UnixSeconds(), now.tv_sec);

    Duration d(3.0);
    
    t1 -= d;
    ASSERT_EQ(t1.UnixSeconds(), now.tv_sec - 3);
  }

  // operator- Duration
  {
    struct timeval now;
    gettimeofday(&now, NULL); 
    Timestamp t1(now);

    Duration d(3.0);
    
    Timestamp t2 = t1 - d;
    ASSERT_EQ(t2.UnixSeconds(), now.tv_sec - 3);
  }

  // operator- Timestamp
  {
    struct timeval now;
    gettimeofday(&now, NULL); 
    Timestamp t1(now);

    now.tv_sec += 3;
    Timestamp t2(now);

    Duration d = t2 - t1;

    ASSERT_EQ(d, Duration(3.0));
  }
}

}  // namespace

int main(int argc, char **argv) {

  // Parses the command line for googletest flags, and removes all recognized flags.
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
