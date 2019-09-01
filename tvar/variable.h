// Copyright (c) 2014 Baidu, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Author: Michael,Tesla(michaeltesla1995@gmail.com)
// Date: 2019/01/17 23:10:00

#ifndef TESLA_TVAR_VARIABLE_H_
#define TESLA_TVAR_VARIABLE_H_

#include <ostream>             // std::ostream
#include <vector>              // std::vector
#include "tutil/macros.h"      // DISALLOW_COPY_AND_ASSIGN 
#include "tutil/string_view.h" // tutil::StringView

namespace tesla {
namespace tvar {

// Bitwise masks of displayable targets.
enum DisplayFilter {
  DISPLAY_ON_HTML = 1,
  DISPLAY_ON_PLAIN_TEXT = 2,
  DISPLAY_ON_ALL = 3,
};

// Implement this class to write variables into different places.
// If dump() return false, Variable::dump_exposed() stops and return -1.
class Dumper {
 public:
  Dumper() = default;
  virtual ~Dumper() = default;
  virtual bool dump(const std::string& name,
                    const tutil::StringView& description) = 0;
};

// Options for Variable::dump_exposed().
struct DumpOptions {
  // Constructed with default options.
  DumpOptions();

  // If this is true, string-type values will be quoted.
  bool quote_string;

  // The ? in wildcards. Wildcards in URL need to use another character
  // because ? is reserved.
  char question_mark;

  // Dump variables with matched display_filter
  DisplayFilter display_filter;

  // Name matched by these wildcards are kept.
  std::string white_wildcards;

  // Name matched by these wildcards are skipped.
  std::string black_wildcards;
};

// Base class of all tvar.
// tvar is NOT thread-safe:
//   You should not operate one tvar from different threads simultaneously.
//   If you need to, protect the ops with locks. Similarly with ordinary
//   variables, const methods are thread-safe, namely you can call describe()/
//   get_description()/get_value() etc from different threads safely.
class Variable {
 public:
  Variable() = default;
  virtual ~Variable();

  // tvar uses TLS, thus copying/assignment need to copy TLS stuff as well,
  // which is heavy. We disable copying/assignment now.
  DISALLOW_COPY_AND_ASSIGN(Variable);

  // Implement this method to print the variable into ostream.
  virtual void describe(std::ostream&, bool quote_string) const = 0;

  // string form of describe().
  std::string get_description() const;

  // Expose this variable globally so that it's counted in follwing
  // functions:
  //   list_exposed
  //   count_exposed
  //   describe_exposed
  //   find_exposed
  // Return 0 on success, -1 otherwise.
  int expose(const tutil::StringView& name,
             DisplayFilter display_filter = DISPLAY_ON_ALL) {
    return expose_impl(tutil::StringView(), name, display_filter);
  }

  // Expose this variable globally with a prefix.
  int expose_as(const tutil::StringView& prefix,
                const tutil::StringView& name,
                DisplayFilter display_filter = DISPLAY_ON_ALL) {
    return expose_impl(prefix, name, display_filter);
  }

  // Hide this variable so that it's not counted in *_exposed functions.
  // Returns false if this variable is already hidden.
  // CAUTION!!! Subclasses must call hide() manually to avoid displaying
  // a variable that is just destructing.
  bool hide();

  // Get exposed name.
  // If this variable is not exposed, the name is empty.
  const std::string& name() const { return name_; }

  // ==================================================================
  
  // Get names of all exposed variables into `names'.
  // If you want to print all variables, you have to go through `names'
  // and call `describe_exposed' on each name. This prevents an iteration
  // from taking the lock too long.
  // Return empty vector when not exposed variables.
  static void list_exposed(std::vector<std::string>& names, DisplayFilter = DISPLAY_ON_ALL);

  // Get number of exposed variables.
  static size_t count_exposed();

  // Find an exposed variable by `name' and put its description into `os'.
  // Return 0 on found, -1 otherwise.
  static int describe_exposed(const std::string& name,
                              std::ostream& os,
                              bool quote_string = false,
                              DisplayFilter = DISPLAY_ON_ALL);

  // String form. Return empty string when not found.
  static std::string describe_exposed(const std::string& name,
                                      bool quote_string = false,
                                      DisplayFilter = DISPLAY_ON_ALL);

  // Find all exposed variables matching `white_wildcards' but `black_wildcards'
  // and send them to `dumper'.
  // Use default options when `options' is empty.
  // Return number of dumped variables, -1 on error.
  static int dump_exposed(Dumper& dumper, const DumpOptions* options);

 protected:
  virtual int expose_impl(const tutil::StringView& prefix,
                          const tutil::StringView& name,
                          DisplayFilter display_filter);
 private:
  std::string name_;
};

// Make name only use lowercased alphabets / digits / underscores, and append
// the result to `out'.
// Examples:
//   foo-inl.h       -> foo_inl_h
//   foo::bar::Apple -> foo_bar_apple
//   Car_Rot         -> car_rot
//   FooBar          -> foo_bar
//   RPCTest         -> rpctest
//   HELLO           -> hello
void to_underscored_name(std::string& out, const tutil::StringView& name);

} // namespace tvar
} // namespace tesla

// Make variables printable.
namespace std {

inline ostream& operator<<(ostream& os, const ::tesla::tvar::Variable& var) {
  var.describe(os, false);
  return os;
}

} // namespace std

#endif // TESLA_TVAR_VARIABLE_H_
