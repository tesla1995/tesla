#include "tutil/string.h"
#include <ctype.h>
#include <string>
#include <limits.h>
#include "log/logging.h"

namespace tesla {
namespace tutil {

int StrToLong(const char* nptr, int base, long int* value) {
  if (!nptr || !value) {
    LOG_ERROR << "null pointer!";
    return -1;
  }

  errno = 0;
  char* endptr = NULL;
  *value = strtol(nptr, &endptr, base);
  if (errno > 0) {
    return -1;
  } else if (endptr == nptr) {
    LOG_ERROR << "No digits was found: " << nptr;
    return -1;
  } else if (endptr) {
    for (; isspace(*endptr); endptr++);  // "123  " is not an error.
    if (*endptr) {
      LOG_ERROR << "Further characters after numbers: " << endptr;
      return -1;
    }
  }

  return 0;
}

int StrToInt(const char* nptr, int base, int* value) {
  int ret = 0;
  long tmp;

  ret = StrToLong(nptr, base, &tmp);
  if (ret < 0) {
    LOG_ERROR << "Fail to call StrToLong";
    return ret;
  }

  if (tmp < INT_MIN || tmp > INT_MAX) {
    LOG_ERROR << "out of range";
    errno = ERANGE;
    return -1;
  }
  *value = tmp;
  
  return 0;
}

}  // tutil
}  // tesla