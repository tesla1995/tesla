#include "tutil/strings/string_util.h"

namespace tutil {

constexpr char kWhitespaceASCII[] = {
  0x09,  // CHARACTER TABULATION
  0x0A,  // LINE FEED (LF) 
  0x0B,  // LINE TABULATION
  0x0C,  // FORM FEED (FF)
  0x0D,  // CARRIAGE RETURN (CR)
  0x20,  // SPACE
  0
};

template <typename Iter>
static inline bool DoLowerCaseEqualsASCII(Iter a_begin,
                                          Iter a_end,
                                          const char* b) {
  for (Iter it = a_begin; it != a_end; ++it, ++b) {
    if (!*b || ToLowerASCII(*it) != *b) {
      return false;
    }
  }

  return *b == 0;
}

// Front-ends for LowerCaseEqualsASCII.
bool LowerCaseEqualsASCII(const std::string& a, const char* b) {
  return DoLowerCaseEqualsASCII(a.begin(), a.end(), b);
}

bool LowerCaseEqualsASCII(std::string::const_iterator a_begin,
                          std::string::const_iterator a_end,
                          const char* b) {
  return DoLowerCaseEqualsASCII(a_begin, a_end, b);
}

bool LowerCaseEqualsASCII(const char* a_begin,
                          const char* a_end,
                          const char* b) {
  return DoLowerCaseEqualsASCII(a_begin, a_end, b);
}

TrimPositions TrimString(const std::string& input,
                         const std::string& trim_chars,
                         TrimPositions positions,
                         std::string& output) {

  // Find the edges of leading/trailing whitespace as desired.
  const auto last_char = input.length() - 1;
  const auto first_good_char = (positions & TRIM_LEADING) ?
      input.find_first_not_of(trim_chars) : 0;
  const auto last_good_char = (positions & TRIM_TRAILING) ?
      input.find_last_not_of(trim_chars) : last_char;

  // When the string was all whitespace, report that we stripped off whitespace
  // from whichever position the caller was interested in. For emtpy input, we
  // stripped no whitespace, but we still need to clear |output|.
  if (input.empty() ||
      (first_good_char == std::string::npos) ||
      (last_good_char == std::string::npos)) {
    bool input_was_empty = input.empty();
    output.clear();
    return input_was_empty ? TRIM_NONE : positions;
  }

  // Trim the whitespace.
  output =
    input.substr(first_good_char, last_good_char - first_good_char + 1);

  // Return where we trimmed from.
  return static_cast<TrimPositions>(
    ((first_good_char == 0) ? TRIM_NONE : TRIM_LEADING) |
     ((last_good_char == last_char) ? TRIM_NONE : TRIM_TRAILING));
}

TrimPositions TrimWhitespaceASCII(const std::string& input,
                                  TrimPositions positions,
                                  std::string& output) {
  return TrimString(input, std::string(kWhitespaceASCII), positions, output);
}


} // namespace tutil
