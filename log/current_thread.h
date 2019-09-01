#ifndef TESLALOG_CURRENTTHREAD_H_
#define TESLALOG_CURRENTTHREAD_H_

#include <cstdint>

namespace tesla {
namespace log {

// internal
extern __thread int t_cachedTid;
extern __thread char t_tidString[32];
extern __thread int t_tidStringLength;

class CurrentThread {
 public:

  static void cacheTid();

  inline static int tid()
  {
    if (t_cachedTid == 0)
    {
      cacheTid();
    }
    return t_cachedTid;
  }

  inline static const char* tidString() // for logging
  {
    return t_tidString;
  }

  inline static int tidStringLength() // for logging
  {
    return t_tidStringLength;
  }  
};

} // namespace log
} // namespace tesla

#endif // TESLALOG_CURRENTTHREAD_H_
