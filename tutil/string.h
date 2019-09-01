#ifndef TESLA_TUTIL_STRING_H_
#define TESLA_TUTIL_STRING_H_

namespace tesla {
namespace tutil {

int StrToLong(const char* nptr, int base, long int* value);
int StrToInt(const char* nptr, int base, int* value);

}  // tutil
}  // tesla

#endif  // TESLA_TUTIL_STRING_H_