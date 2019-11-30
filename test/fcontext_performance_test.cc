#include "fiber/fcontext.h"

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

void foo(transfer_t t1) {
  transfer_t t = t1;
  while (1) {
    t = jump_fcontext(t.fctx, t.data);  
  }
}

int main(int argc, const char *argv[])
{
  size_t jobs = 10000000;  
  Timer timer;
  stack_allocator stack; 
  transfer_t t{0, 0};

  if (argc > 1) {
    jobs = atoi(argv[1]);
    if (jobs < 1000000) {
      jobs = 1000000;
    }
  }

  void* sp = stack.allocate(stack_allocator::default_stacksize());
  fcontext_t fctx = make_fcontext(sp, 
                        stack_allocator::default_stacksize(), foo);

  // cache warm up
  t = jump_fcontext(fctx, 0);  

  timer.start();
  for (size_t i = 0; i < jobs; i++) {
    t = jump_fcontext(t.fctx, t.data);  
  }
  timer.stop();
  double fq = jobs * 2 / timer.s_elapsed(0.0);

  cout << "fcontext switch: [" << fq << "]times/second" << endl;

  stack.deallocate(sp, stack_allocator::default_stacksize());
  return 0;
}
