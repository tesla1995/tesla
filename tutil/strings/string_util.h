#ifndef TESLA_TUTIL_STRINGS_STRING_UTIL_H_
#define TESLA_TUTIL_STRINGS_STRING_UTIL_H_

#include <string>

namespace tutil {

// ASCII-specific tolower. The standard library's lower is locale sensitive,
// so we don't want to use it here.
template <typename Char> inline Char ToLowerASCII(Char c) {
  return (c >= 'A' && c <= 'Z') ? (c + ('a' - 'A')) : c;
}

// ASCII-specific toupper. The standard library's toupper is locale sensitive,
// so we don't want to use it here.
template <typename Char> inline Char ToUpperASCII(Char c) {
  return (c >= 'a' && c <= 'z') ? (c + ('A' - 'a')) : c;
}

// Compare the lower-case form of the given string against the given ASCII
// string. This is useful for doing check if an input string matches some
// token, and it is optimized to avoid intermediate string copies. This API
// is borrowed from the equivalent APIs in Mozilla.
bool LowerCaseEqualsASCII(const std::string& a, const char* b);

bool LowerCaseEqualsASCII(std::string::const_iterator a_begin,
                          std::string::const_iterator a_end,
                          const char* b);

bool LowerCaseEqualsASCII(const char* a_begin,
                          const char* a_end,
                          const char* b);


// Trims all whitespace from either end of the input string. Returns where
// whitespace was found.
// NOTE: safe to use the same variable for both input and output.
enum TrimPositions {
  TRIM_NONE      = 0,
  TRIM_LEADING   = 1 << 0,
  TRIM_TRAILING  = 1 << 1,
  TRIM_ALL       = TRIM_LEADING | TRIM_TRAILING,
};

TrimPositions TrimWhitespaceASCII(const std::string& input,
                                  TrimPositions positions,
                                  std::string& output);

} // namespace tutil

#endif // TESLA_TUTIL_STRINGS_STRING_UTIL_H_
