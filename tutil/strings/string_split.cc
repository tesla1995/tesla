#include "tutil/strings/string_split.h"
#include "tutil/strings/string_util.h"

//#include <glog/logging.h>
#include "log/logging.h"

namespace tutil {

void SplitString(const std::string& str,
                 const char separator,
                 bool trim_whitespace,
                 std::vector<std::string>& r) {
  r.clear();

  size_t last = 0;
  size_t size = str.size();

  for (size_t pos = 0; pos <= size; ++pos) {
    if (str[pos] == separator || pos == size) {
      std::string tmp(str, last, pos - last);
      if (trim_whitespace) {
        TrimWhitespaceASCII(tmp, TRIM_ALL, tmp);
      }
      // Avoid converting an empty or all-whitespace source string into a vector
      // of one empty string.
      if (!(pos == size && r.empty() && tmp.empty())) {
        r.push_back(std::move(tmp));
      }
      last = pos + 1;
    }
  }
}

void SplitString(const std::string& str,
                 const char separator,
                 std::vector<std::string>& r) {
  SplitString(str, separator, true, r);
}

bool SplitStringIntoKeyValue(const std::string& line,
                             char key_value_delimiter,
                             std::string& key,
                             std::string& value) {

  key.clear();
  value.clear();

  // Find the position of delimiter.
  size_t delimiter_position = line.find_first_of(key_value_delimiter);
  if (delimiter_position == std::string::npos) {
    LOG_ERROR << "cannot find delimiter in: " << line;
    return false;
  }
  key.assign(line, 0, delimiter_position);

  // Find the value string.
  size_t value_begin_position =
    line.find_first_not_of(key_value_delimiter, delimiter_position + 1);
  if (value_begin_position == std::string::npos) {
    LOG_ERROR << "cannot parse value from line: " << line;
    return false;
  }
  value.assign(line, value_begin_position, line.size() - value_begin_position);
  return true;
}

bool SplitStringIntoKeyValuePairs(const std::string& line,
                                  char key_value_delimiter,
                                  char key_value_pair_delimiter,
                                  StringPairs& key_value_pairs) {
  key_value_pairs.clear();

  if (line.empty()) {
    return false;
  }

  std::vector<std::string> pairs;
  SplitString(line, key_value_pair_delimiter, pairs);

  bool success = true;
  for (auto it = pairs.cbegin(); it != pairs.cend(); ++it) {
    // Don't add empty pairs into the result.
    if (it->empty()) {
      continue;
    }

    std::string key;
    std::string value;
    if (!SplitStringIntoKeyValue(*it, key_value_delimiter, key, value)) {
      success = false;
    }
    key_value_pairs.push_back(std::make_pair(std::move(key), std::move(value)));
  }
  return success;
}

} // namespace tutil
