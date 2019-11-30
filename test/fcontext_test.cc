#include "fiber/fcontext.h"

#include <cassert>
#include <cstdint>
#include <cstring>

#include <iostream>
#include <vector>

#include <gtest/gtest.h>

using namespace std;

template< std::size_t Max, std::size_t Default, std::size_t Min >
class simple_stack_allocator
{
 public:
   static std::size_t maximum_stacksize()
   { return Max; }

   static std::size_t default_stacksize()
   { return Default; }

   static std::size_t minimum_stacksize()
   { return Min; }

   void* allocate(std::size_t size) const
   {
     assert(minimum_stacksize() <= size);
     assert(maximum_stacksize() >= size);

     void* limit = malloc( size);
     if (!limit) throw std::bad_alloc();

     return static_cast<char*>(limit) + size;
   }

   void deallocate(void* vp, std::size_t size) const
   {
     assert(vp);
     assert(minimum_stacksize() <= size);
     assert(maximum_stacksize() >= size);

     void* limit = static_cast< char * >(vp) - size;
     free(limit);
   }
};

using stack_allocator = simple_stack_allocator<
    8 * 1024 * 1024,
    64 * 1024,
    8 * 1024>;

int value1 = 0;
int value2 = 0;
int value3 = 0;

void f1(transfer_t t)
{
  jump_fcontext(t.fctx, 0);
}

void f2(transfer_t t)
{
  transfer_t t1;
  value1 = 1;
  t1 = jump_fcontext(t.fctx, 0);
  value1 = 2;
  jump_fcontext(t1.fctx, 0);
}

void f3(transfer_t t)
{
  transfer_t t1;
  *static_cast<int *>(t.data) += 1;
  t1 = jump_fcontext(t.fctx, t.data);
  *static_cast<int *>(t1.data) += 1;
  jump_fcontext(t1.fctx, t1.data);
}

transfer_t f4(transfer_t t)
{
  *static_cast<int *>(t.data) += 1;
  return t;
}

void f5(transfer_t t)
{
  t = jump_fcontext(t.fctx, t.data);
  *static_cast<int *>(t.data) += 1;
  jump_fcontext(t.fctx, t.data);
}

namespace {

// The fixture for testing Fcontext.
class FcontextTest : public ::testing::Test {
  protected:
    // You can remove any or all of the following functions if its body
    // is empty.

    FcontextTest() {
      // You can do set-up work for each test here.
    }

    ~FcontextTest() override {
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

}; // namespace FcontextTest

TEST_F(FcontextTest, alloc) {
  stack_allocator alloc;
  void* sp = alloc.allocate(stack_allocator::default_stacksize());
  ASSERT_TRUE(sp != NULL);
  alloc.deallocate(sp, stack_allocator::default_stacksize());
}

TEST_F(FcontextTest, make) {
  stack_allocator alloc;
  void* sp = alloc.allocate(stack_allocator::default_stacksize());
  ASSERT_TRUE(sp != NULL);

  fcontext_t ctx = make_fcontext(sp, stack_allocator::default_stacksize(), f1);
  ASSERT_TRUE(ctx != NULL);

  alloc.deallocate(sp, stack_allocator::default_stacksize());
}

TEST_F(FcontextTest, jump) {
  stack_allocator alloc;
  void* sp = alloc.allocate(stack_allocator::default_stacksize());
  ASSERT_TRUE(sp != NULL);

  fcontext_t fctx = make_fcontext(sp, stack_allocator::default_stacksize(), f2);
  ASSERT_TRUE(fctx != NULL);

  value1 = 0;
  transfer_t t = jump_fcontext(fctx, 0);
  ASSERT_EQ(value1, 1);
  t = jump_fcontext(t.fctx, 0);
  ASSERT_EQ(value1, 2);

  alloc.deallocate(sp, stack_allocator::default_stacksize());
}

TEST_F(FcontextTest, transfer_data) {
  stack_allocator alloc;
  void* sp = alloc.allocate(stack_allocator::default_stacksize());
  ASSERT_TRUE(sp != NULL);

  fcontext_t fctx = make_fcontext(sp, stack_allocator::default_stacksize(), f3);
  ASSERT_TRUE(fctx != NULL);

  value2 = 1;
  transfer_t t = jump_fcontext(fctx, &value2);
  ASSERT_EQ(value2, 2);
  ASSERT_EQ(*static_cast<int *>(t.data), 2);

  *static_cast<int *>(t.data) += 1;
  ASSERT_EQ(value2, 3);
  ASSERT_EQ(*static_cast<int *>(t.data), 3);

  t = jump_fcontext(t.fctx, t.data);
  ASSERT_EQ(value2, 4);
  ASSERT_EQ(*static_cast<int *>(t.data), 4);

  alloc.deallocate(sp, stack_allocator::default_stacksize());
}

TEST_F(FcontextTest, ontop) {
  stack_allocator alloc;
  void* sp = alloc.allocate(stack_allocator::default_stacksize());
  ASSERT_TRUE(sp != NULL);

  fcontext_t fctx = make_fcontext(sp, stack_allocator::default_stacksize(), f5);
  ASSERT_TRUE(fctx != NULL);

  value3 = 1;
  transfer_t t = jump_fcontext(fctx, &value3);
  ASSERT_EQ(value3, 1);
  ASSERT_EQ(*static_cast<int *>(t.data), 1);

  t = ontop_fcontext(t.fctx, t.data, f4);
  ASSERT_EQ(value3, 3);
  ASSERT_EQ(*static_cast<int *>(t.data), 3);

  alloc.deallocate(sp, stack_allocator::default_stacksize());
}

}  // namespace

int main(int argc, char **argv) {
  // Parses the command line for googletest flags, and removes all recognized flags.
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
