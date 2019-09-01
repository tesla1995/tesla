#include "tutil/wildcard_matcher.h"
#include "tutil/string_splitter.h"

// Author: Michael,Tesla(michaeltesla1995@gmail.com)
// Date: Mon Jan 21 23:00:30 CST 2019

namespace tutil {

namespace internal {

// Written by Jack Handy
// <A href="mailto:jakkhandy@hotmail.com">jakkhandy@hotmail.com</A>
//
// TODO: understand how does it works.
inline bool wildcmp(const char* wild, const char* str, char question_mark) {
  const char* cp = NULL;
  const char* mp = NULL;

  while (*str && *wild != '*') {
    if (*wild != *str && *wild != question_mark) {
      return false;
    }
    ++wild;
    ++str;
  }

  while (*str) {
    if (*wild == '*') {
      if (!*++wild) {
        return true;
      }
      mp = wild;
      cp = str+1;
    } else if (*wild == *str || *wild == question_mark) {
      ++wild;
      ++str;
    } else {
      wild = mp;
      str = cp++;
    }
  }

  while (*wild == '*') {
    ++wild;
  }
  return !*wild;
}

} // namespace internal	

WildcardMatcher::WildcardMatcher(const std::string& wildcards,
                                 char question_mark,
                                 bool on_both_empty)
    : question_mark_(question_mark),
      on_both_empty_(on_both_empty) {

  if (wildcards.empty()) {
    return;     
  }

  std::string name;
  const char wc_pattern[3] = { '*', question_mark, '\0' };
  const char separators[16] = ",;";
  for (tutil::StringMultiSplitter sp(wildcards.c_str(), separators);
       sp; ++sp) {
    name.assign(sp.field(), sp.length()); 
    if (name.find_first_of(wc_pattern) != std::string::npos) {
      // Estimate space size, reduce memory allocation and copy times.
      if (wildcards_.empty()) {
        wildcards_.reserve(8);
      }
      wildcards_.push_back(name);
    } else {
      exact_names_.insert(name);
    }
  }
}

bool WildcardMatcher::Match(const std::string& name) const {
  if (!exact_names_.empty()) {
    if (exact_names_.find(name) != exact_names_.end()) {
      return true;
    }
  } else if (wildcards_.empty()) {
    return on_both_empty_;
  }
  for (size_t i = 0; i < wildcards_.size(); ++i) {
    if (internal::wildcmp(wildcards_[i].c_str(), name.c_str(), question_mark_)) {
      return true;
    }
  }
  return false;
} 

} // namespace tutil
