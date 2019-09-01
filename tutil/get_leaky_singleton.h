#ifndef TESLA_TUTIL_GET_LEAKY_SINGLETON_H_
#define TESLA_TUTIL_GET_LEAKY_SINGLETON_H_

#include <string.h>
#include <pthread.h>
#include <atomic>

namespace tesla {
namespace tutil {

// Thread-safe
template <typename T>
inline T& GetLeakySingleton()
{
  static T object;
  return object;
}

}  // tutil
}  // tesla

#endif  // TESLA_TUTIL_GET_LEAKY_SINGLETON_H_
