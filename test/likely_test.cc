#include "tutil/compiler_specific.h"

int test(int x) {
  if (TESLA_LIKELY(x)) {
    return 3;    
  } else {
    return 4;
  }
}

int main(void)
{
  test(3);
  return 0;
}
