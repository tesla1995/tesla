#ifndef TESLA_TUTIL_WILDCARD_MATCHER_H_
#define TESLA_TUTIL_WILDCARD_MATCHER_H_

// Author: Michael,Tesla(michaeltesla1995@gmail.com)
// Date: Mon Jan 21 23:00:30 CST 2019

#include <string>
#include <vector>
#include <set>

namespace tutil {

class WildcardMatcher {
 public:
  WildcardMatcher(const std::string& wildcards,
                  char question_mark,
                  bool on_both_empty);


  bool Match(const std::string& name) const ; 

  const std::vector<std::string>& wildcards() const { return wildcards_; }
  const std::set<std::string>& exact_names() const { return exact_names_; }

 private:
  char question_mark_;
  bool on_both_empty_;
  std::vector<std::string> wildcards_;
  std::set<std::string> exact_names_;  // find efficiently
};

} // namespace tutil

#endif // TESLA_TUTIL_WILDCARD_MATCHER_H_
