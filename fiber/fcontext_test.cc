#include <cstdio>
#include "fiber/fcontext.h"

alignas(64) char stack[1 * 1024 * 1024];

int i = 0;

void fn1(transfer_t transfer)
{
  *(int*)transfer.data += 1;
  //printf("===== first enter fn1 =====\n");
  //printf("===== fn1 work... =====\n");
  //printf("===== fn1 yield =====\n");
  transfer = jump_fcontext(transfer.fctx, transfer.data);
  //printf("===== enter fn1 again =====\n");
  //printf("===== fn1 work... =====\n");
  //printf("===== fn1 return =====\n");
  *(int*)transfer.data += 1;
  jump_fcontext(transfer.fctx, transfer.data);
}

int main(void)
{
  i = 1;
  transfer_t transfer{0, 0};

  printf("===== i[%d] =====\n", i);
  transfer.fctx = make_fcontext(stack, sizeof(stack), fn1);
  printf("===== jump to fn1 in main =====\n");
  transfer = jump_fcontext(transfer.fctx, &i);
  printf("===== return from fn1 in main =====\n");
  printf("===== i[%d] =====\n", i);
  printf("===== jump to fn1 in main again =====\n");
  transfer = jump_fcontext(transfer.fctx, &i);
  printf("===== return from fn1 in main again=====\n");
  printf("===== i[%d] =====\n", i);
  return 0;
}
