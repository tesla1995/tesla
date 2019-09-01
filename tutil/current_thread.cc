#include "tutil/current_thread.h"

#include <unistd.h>
#include <syscall.h>
#include <cstdio>

namespace tesla {
namespace tutil {

__thread int kCachedThreadId = 0;
__thread char kThreadIdString[32];
__thread int kThreadIdStringLength = 6;

static pid_t gettid()
{
  return static_cast<pid_t>(::syscall(SYS_gettid));
}

void CurrentThread::CacheThreadId()
{
  if (kCachedThreadId == 0)
  {
    kCachedThreadId = gettid();
    kThreadIdStringLength = snprintf(kThreadIdString, sizeof(kThreadIdString), 
                                     "%5d ", kCachedThreadId);
  }
}

} // namespace tutil
} // namespace tesla

