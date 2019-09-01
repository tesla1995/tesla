#ifndef TESLA_TUTIL_MACRO_H_
#define TESLA_TUTIL_MACRO_H_

#define DISALLOW_COPY_AND_ASSIGN(T) \
  T(const T& ) = delete; \
  T& operator=(const T& ) = delete

// array size
#define ARRAY_SIZE(array) \
  sizeof(array) / sizeof(array[0])

#endif // TESLA_TUTIL_MACRO_H_
