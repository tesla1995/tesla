#include "allocator/system_alloc.h"

#include <string.h>
#include <iostream>
#include <gtest/gtest.h>

using namespace std;
using namespace tesla::allocator;

namespace {

// The fixture for testing SystemAlloc.
class SystemAllocTest : public ::testing::Test {
 protected:
  // You can remove any or all of the following functions if its body
  // is empty.

  SystemAllocTest() {
     // You can do set-up work for each test here.
  }

  ~SystemAllocTest() override {
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
  
}; // namespace SystemAllocTest

static size_t kOnePage =   1 << 12;
static size_t kTwoPage =   1 << 13;
static size_t kThreePage = 1 << 14;
static size_t kFourPage =  1 << 15;
static size_t kFivePage =  1 << 16;

static size_t times = 1000;

TEST_F(SystemAllocTest, AlignedAddress) {
  for (size_t i = 0; i < times; i++)
  {
    {
      // size = 3K, aligned on 4K.
      size_t actual_size = 0;
      size_t bytes = kOnePage - 1024;
      void* result = TeslaMalloc_SystemAlloc(bytes, &actual_size, 0);
      ASSERT_TRUE(result != NULL);
      uintptr_t ptr = reinterpret_cast<uintptr_t>(result);
      ASSERT_TRUE((ptr & (kOnePage - 1)) == 0);
      ASSERT_TRUE(actual_size == kOnePage);
      ASSERT_TRUE(TeslaMalloc_SystemRelease(result, kOnePage));
    }

    {
      // size = 4K
      size_t actual_size = 0;
      void* result = TeslaMalloc_SystemAlloc(kOnePage, &actual_size, kOnePage);
      ASSERT_TRUE(result != NULL);
      uintptr_t ptr = reinterpret_cast<uintptr_t>(result);
      ASSERT_TRUE((ptr & (kOnePage - 1)) == 0);
      ASSERT_TRUE(actual_size == kOnePage);
      ASSERT_TRUE(TeslaMalloc_SystemRelease(result, kOnePage));
    }

    {
      // size = 8K
      size_t actual_size = 0;
      void* result = TeslaMalloc_SystemAlloc(kTwoPage, &actual_size, kTwoPage);
      ASSERT_TRUE(result != NULL);
      uintptr_t ptr = reinterpret_cast<uintptr_t>(result);
      ASSERT_TRUE((ptr & (kTwoPage - 1)) == 0) << "ptr[" << ptr 
        << "], kTwoPage[" << kTwoPage << "]";
      ASSERT_TRUE(actual_size == kTwoPage);
      ASSERT_TRUE(TeslaMalloc_SystemRelease(result, kTwoPage));
    }

    {
      // size = 16K
      size_t actual_size = 0;
      void* result = TeslaMalloc_SystemAlloc(kThreePage, &actual_size, kThreePage);
      ASSERT_TRUE(result != NULL);
      uintptr_t ptr = reinterpret_cast<uintptr_t>(result);
      ASSERT_TRUE((ptr & (kThreePage - 1)) == 0);
      ASSERT_TRUE(actual_size == kThreePage);
      ASSERT_TRUE(TeslaMalloc_SystemRelease(result, kThreePage));
    }

    {
      // size = 32K
      size_t actual_size = 0;
      void* result = TeslaMalloc_SystemAlloc(kFourPage, &actual_size, kFourPage);
      ASSERT_TRUE(result != NULL);
      uintptr_t ptr = reinterpret_cast<uintptr_t>(result);
      ASSERT_TRUE((ptr & (kFourPage - 1)) == 0);
      ASSERT_TRUE(actual_size == kFourPage);
      ASSERT_TRUE(TeslaMalloc_SystemRelease(result, kFourPage));
    }

    {
      // size = 61K
      size_t actual_size = 0;
      size_t bytes = kFivePage - 3*1024;
      void* result = TeslaMalloc_SystemAlloc(bytes, &actual_size, kFivePage);
      ASSERT_TRUE(result != NULL);
      uintptr_t ptr = reinterpret_cast<uintptr_t>(result);
      ASSERT_TRUE((ptr & (kFivePage - 1)) == 0);
      ASSERT_TRUE(actual_size == kFivePage);
      ASSERT_TRUE(TeslaMalloc_SystemRelease(result, kFivePage));
    }

    {
      // size = 64K
      size_t actual_size = 0;
      void* result = TeslaMalloc_SystemAlloc(kFivePage, &actual_size, kFivePage);
      ASSERT_TRUE(result != NULL);
      uintptr_t ptr = reinterpret_cast<uintptr_t>(result);
      ASSERT_TRUE((ptr & (kFivePage - 1)) == 0);
      ASSERT_TRUE(actual_size == kFivePage);
      ASSERT_TRUE(TeslaMalloc_SystemRelease(result, kFivePage));
    }
  }
}

}  // namespace

int main(int argc, char **argv) {

  // Parses the command line for googletest flags, and removes all recognized flags.
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
