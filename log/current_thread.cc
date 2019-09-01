#include "current_thread.h"

#include <unistd.h>
#include <syscall.h>
#include <cstdio>

namespace tesla {
namespace log {

__thread int t_cachedTid = 0;
__thread char t_tidString[32];
__thread int t_tidStringLength = 6;

static pid_t gettid()
{
  return static_cast<pid_t>(::syscall(SYS_gettid));
}

void CurrentThread::cacheTid()
{
  if (t_cachedTid == 0)
  {
    t_cachedTid =gettid();
    t_tidStringLength = snprintf(t_tidString, sizeof t_tidString, "%5d ", t_cachedTid);
  }
}

} // namespace log
} // namespace tesla

