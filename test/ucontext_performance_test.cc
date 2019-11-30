#include <ucontext.h>

#include <cassert>
#include <cstdint>
#include <cstring>

#include <iostream>
#include <vector>

#include "tutil/time.h"

using namespace tesla::tutil;
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

     return limit;
   }

   void deallocate(void* limit) const
   {
     if (limit != NULL) {
       free(limit);
     }
   }
};

using stack_allocator = simple_stack_allocator<
    8 * 1024 * 1024,
    64 * 1024,
    8 * 1024>;

static ucontext_t uctx_main, uctx_foo;

void foo() {
  while (1) {
    swapcontext(&uctx_foo, &uctx_main);
  }
}

int main(int argc, const char *argv[])
{
  size_t jobs = 10000000;  
  Timer timer;
  stack_allocator stack; 

  if (argc > 1) {
    jobs = atoi(argv[1]);
    if (jobs < 1000000) {
      jobs = 1000000;
    }
  }

  void* sp = stack.allocate(stack_allocator::default_stacksize());
  assert(sp != NULL);

  getcontext(&uctx_foo);
  uctx_foo.uc_stack.ss_sp = sp;
  uctx_foo.uc_stack.ss_size = stack_allocator::default_stacksize();
  uctx_foo.uc_stack.ss_flags = 0;
  uctx_foo.uc_link = &uctx_main;
  makecontext(&uctx_foo, foo, 0);

  // cache warm up
  swapcontext(&uctx_main, &uctx_foo);

  timer.start();
  for (size_t i = 0; i < jobs; i++) {
    swapcontext(&uctx_main, &uctx_foo);
  }
  timer.stop();
  double fq = jobs * 2 / timer.s_elapsed(0.0);

  cout << "fcontext switch: [" << fq << "]times/second" << endl;

  stack.deallocate(sp);
  return 0;
}
