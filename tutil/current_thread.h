#ifndef TESLA_TUTIL_CURRENTTHREAD_H_
#define TESLA_TUTIL_CURRENTTHREAD_H_

#include <cstdint>

namespace tesla {
namespace tutil {

// internal
extern __thread int kCachedThreadId;
extern __thread char kThreadIdString[32];
extern __thread int kThreadIdStringLength;

class CurrentThread {
 public:

  static void CacheThreadId();

  inline static int ThreadId()
  {
    if (kCachedThreadId == 0)
    {
      CacheThreadId();
    }
    return kCachedThreadId;
  }

  inline static const char* ThreadIdString() // for logging
  {
    return kThreadIdString;
  }

  inline static int ThreadIdStringLength() // for logging
  {
    return kThreadIdStringLength;
  }  
};

} // namespace tutil
} // namespace tesla

#endif // TESLA_TUTIL_CURRENTTHREAD_H_
