#include "tutil/packed_cache.h"

#include <gtest/gtest.h>

using namespace std;
using namespace tesla::tutil;

namespace {

// The fixture for testing PackedCache.
class PackedCacheTest : public ::testing::Test {
 protected:
  // You can remove any or all of the following functions if its body
  // is empty.

  PackedCacheTest() {
     // You can do set-up work for each test here.
  }

  ~PackedCacheTest() override {
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
  
}; // namespace PackedCacheTest

TEST_F(PackedCacheTest, smallestbits) {
  uint32_t value = -1;
  PackedCache<16> cache; 

  ASSERT_FALSE(cache.TryGet(1, &value));

  cache.Put(1, 0);
  ASSERT_TRUE(cache.TryGet(1, &value));
  ASSERT_EQ(value, 0);

  cache.Put(1, 25);
  ASSERT_TRUE(cache.TryGet(1, &value));
  ASSERT_EQ(value, 25);

  cache.Put(1, 127);
  ASSERT_TRUE(cache.TryGet(1, &value));
  ASSERT_EQ(value, 127);

  cache.Invalidate(1);
  ASSERT_FALSE(cache.TryGet(1, &value));
}

TEST_F(PackedCacheTest, 32bits) {
  uint32_t value = -1;
  PackedCache<32> cache; 

  ASSERT_FALSE(cache.TryGet(1, &value));

  cache.Put(1, 0);
  ASSERT_TRUE(cache.TryGet(1, &value));
  ASSERT_EQ(value, 0);

  cache.Put(1, 25);
  ASSERT_TRUE(cache.TryGet(1, &value));
  ASSERT_EQ(value, 25);

  cache.Put(1, 127);
  ASSERT_TRUE(cache.TryGet(1, &value));
  ASSERT_EQ(value, 127);

  cache.Invalidate(1);
  ASSERT_FALSE(cache.TryGet(1, &value));
}

}  // namespace

int main(int argc, char **argv) {

  // Parses the command line for googletest flags, and removes all recognized flags.
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
