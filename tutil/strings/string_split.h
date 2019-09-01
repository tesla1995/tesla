#ifndef TESLA_TUTIL_STRINGS_STRING_SPLIT_H_
#define TESLA_TUTIL_STRINGS_STRING_SPLIT_H_

#include <vector>
#include <string>

namespace tutil {

void SplitString(const std::string& str,
                 const char separator,
                 bool trim_whitespace,
                 std::vector<std::string>& r);

void SplitString(const std::string& str,
                 const char separator,
                 std::vector<std::string>& r);

bool SplitStringIntoKeyValue(const std::string& line,
                             char key_value_delimiter,
                             std::string& key,
                             std::string& value);

using StringPairs = std::vector<std::pair<std::string, std::string>>;

// Splits |line| into key value pairs according to the given delimiters and
// removes whitespace leading each key and trailing each value. Returns true
// only if each pair has a non-empty key and value. |key_value_pairs| will 
// include ("", "") pairs for entries without |key_value_delimiter|.
bool SplitStringIntoKeyValuePairs(const std::string& line,
                                  char key_value_delimiter,
                                  char key_value_pair_delimiter,
                                  StringPairs& key_value_pairs);

}; // namespace tutil

#endif // TESLA_TUTIL_STRINGS_STRING_SPLIT_H_
