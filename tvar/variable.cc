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

#include "tvar/variable.h"

#include <stdio.h>

#include <algorithm>      // std::sort
#include <memory>         // std::unique_ptr
#include <mutex>          // std::mutex
#include <set>            // std::set
#include <sstream>        // std::ostringstream
#include <thread>         // std::once_flag
#include <unordered_map>  // std::unorder_map

//#include <glog/logging.h>
#include <gflags/gflags.h>
#include "log/logging.h"

#include "tutil/streambuf.h"
#include "tutil/wildcard_matcher.h"

namespace tesla {
namespace tvar {

DEFINE_bool(tvar_abort_on_same_name, false,
            "Abort when names of tvar are same");
// Remember abort request before tvar_abort_on_same_name is initialized.
static bool kTvarMayAbort = false;
static bool validate_tvar_abort_on_same_name(const char*, bool v) {
  if (v && kTvarMayAbort) {
    // Name conflict happends before handling args of main(), this is
    // generally caused by global tvar.
    LOG_FATAL << "Abort due to name conflict.";
    abort();
  }
  return true;
}
DEFINE_validator(tvar_abort_on_same_name, &validate_tvar_abort_on_same_name);

DEFINE_bool(tvar_log_dumpped, false,
            "[For debugging] print dumpped info"
            " into logstream before call Dumpper");

// -------------------------------------------------------------------------

constexpr static int kSubMapCount = 32;  // must be power of 2
static_assert(!(kSubMapCount & (kSubMapCount - 1)),
              "number of submap must be power of 2");

struct VarMapWithLock {
  struct VarEntry {
    Variable* var{nullptr};
    DisplayFilter display_filter{DISPLAY_ON_ALL};
  };

  // The expose_name used as key is not need to be ordered, so
  // we select unordered_map rather than map. In addition, the
  // performance of unordered_map is higher than map typically.
  using VarMap = std::unordered_map<std::string, VarEntry>;

  VarMap map;
  std::mutex mutex;
};

static std::unique_ptr<VarMapWithLock[]> kVarMaps = nullptr;
static std::once_flag kVarMapsInitOnce;

// Allocate a fixed-size VarMapWithLock array.
static void InitVarMaps() {
  kVarMaps = std::make_unique<VarMapWithLock[]>(kSubMapCount);
}

// Initialize ONCE.
// TODO(tesla): Check the type of return value whether is
// std::unique_ptr<VarMapWithLock[]>&.
inline decltype(auto) GetVarMaps() {
  std::call_once(kVarMapsInitOnce, InitVarMaps);
  return (kVarMaps);
}

// Calculate the sequence number of VarMapWithLock by `str'.
inline size_t SubMapIndex(const std::string& str) {
  if (str.empty()) {
    return 0;
  }

  size_t h = 0;
  for (const char* p = str.c_str(); *p; ++p) {
    h = h * 5 + *p;
  }
  return h & (kSubMapCount - 1);
}

// Get VarMapWithLock by `name'.
// TODO(tesla): Check the type of return value whether is VarMapWithLock&.
inline decltype(auto) GetVarMap(const std::string& name) {
  auto& m = GetVarMaps()[SubMapIndex(name)];
  return m;
}

// -------------------------------------------------------------------------

Variable::~Variable() {
  //CHECK(!hide())
  //    << "Subclass of Variable MUST call hide() manually in their"
  //       " dtors to avoid displaying a variable that is just destructing";
}

int Variable::expose_impl(const tutil::StringView& prefix,
                          const tutil::StringView& name,
                          DisplayFilter display_filter) {
  if (name.empty()) {
    LOG_ERROR << "Parameter[name] is empty";
    return -1;
  }

  // Remove previous pointer from the map if needed.
  hide();

  // Build the name.
  name_.clear();
  name_.reserve((prefix.size() + name.size()) * 5 / 4);
  if (!prefix.empty()) {
    to_underscored_name(name_, prefix);
    if (!name_.empty() && name_.back() != '_') {
      name_.push_back('_');
    }
  }
  to_underscored_name(name_, name);

  auto& m = GetVarMap(name_);
  {
    std::lock_guard<std::mutex> lock(m.mutex);
    auto entry = m.map.find(name_);
    if (entry == m.map.end()) {
      // `name_` does not exist, so we insert it.
      auto& new_entry = m.map[name_];
      new_entry.var = this;
      new_entry.display_filter = display_filter;
      return 0;
    }
  }

  if (FLAGS_tvar_abort_on_same_name) {
    LOG_FATAL << "Abort due to name conflict.";
    abort();
  } else if (!kTvarMayAbort) {
    // NOTE:
    // Mark name conflict occurs, If this conflict happens before
    // Initialization of tvar_abort_on_same_name, the validator will
    // abort the program if needed.
    kTvarMayAbort = true;
  }

  LOG_ERROR << "Already exposed `" << name_ << "' whose value is `"
            << describe_exposed(name_) << '\'';
  name_.clear();
  return -1;
}

bool Variable::hide() {
  if (name_.empty()) {
    return false;
  }

  auto& m = GetVarMap(name_);
  std::lock_guard<std::mutex> lock(m.mutex);
  auto it = m.map.find(name_);
  if (it == m.map.end()) {
    //CHECK(false) << "`" << name_ << "' must exist";
  } else {
    //CHECK(1UL == m.map.erase(name_));
  }
  name_.clear();
  return true;
}

void Variable::list_exposed(std::vector<std::string>& names,
                            DisplayFilter display_filter) {
  names.clear();
  if (names.size() < 32) {
    names.reserve(count_exposed());
  }

  auto& var_maps = GetVarMaps();
  for (size_t i = 0; i < kSubMapCount; ++i) {
    auto& m = var_maps[i];
    std::lock_guard<std::mutex> lock(m.mutex);
    for (auto it = m.map.begin(); it != m.map.end(); ++it) {
      // TODO(tesla): limit max iterated one pass.
      if (it->second.display_filter & display_filter) {
        names.push_back(it->first);
      }
    }
  }
}

size_t Variable::count_exposed() {
  size_t n = 0;
  auto& var_maps = GetVarMaps();
  for (size_t i = 0; i < kSubMapCount; ++i) {
    auto& m = var_maps[i];
    // TODO(tesla): lock may be required here if reading a size_t
    // variable is not a atomic operation.
    n += m.map.size();
  }
  return n;
}

int Variable::describe_exposed(const std::string& name, std::ostream& os,
                               bool quote_string,
                               DisplayFilter display_filter) {
  auto& m = GetVarMap(name);
  std::lock_guard<std::mutex> lock(m.mutex);
  auto it = m.map.find(name);
  if (it == m.map.end()) {
    return -1;
  }

  if (!(it->second.display_filter & display_filter)) {
    return -1;
  }

  it->second.var->describe(os, quote_string);
  return 0;
}

std::string Variable::describe_exposed(const std::string& name,
                                       bool quote_string,
                                       DisplayFilter display_filter) {
  std::ostringstream oss;
  if (describe_exposed(name, oss, quote_string, display_filter) == 0) {
    return oss.str();
  }
  return std::string();
}

std::string Variable::get_description() const {
  std::ostringstream os;
  describe(os, false);
  return os.str();
}

void to_underscored_name(std::string& name, const tutil::StringView& src) {
  name.reserve(name.size() + src.size() + 8 /*just guess*/);
  for (auto p = src.begin(); p != src.end(); ++p) {
    if (isalpha(*p)) {
      if (*p < 'a') {  // upper cases
        if (p != src.data() && !isupper(p[-1]) && name.back() != '_') {
          name.push_back('_');
        }
        name.push_back(*p - 'A' + 'a');
      } else {
        name.push_back(*p);
      }
    } else if (isdigit(*p)) {
      // push digits directly.
      name.push_back(*p);
    } else if (name.empty() || name.back() != '_') {
      // charaters which are not alpha and digit would be converted
      // into a underscope.
      name.push_back('_');
    }
  }
}

DumpOptions::DumpOptions()
    : quote_string(true),
      question_mark('?'),
      display_filter(DISPLAY_ON_PLAIN_TEXT) {}

int Variable::dump_exposed(Dumper& dumper, const DumpOptions* options) {
  DumpOptions opt;
  if (options) {
    opt = *options;
  }

  tutil::CharArrayStreamBuf streambuf;
  std::ostream os(&streambuf);
  int count = 0;
  tutil::WildcardMatcher black_matcher(opt.black_wildcards, opt.question_mark,
                                       false);

  tutil::WildcardMatcher white_matcher(opt.white_wildcards, opt.question_mark,
                                       true);

  std::ostringstream dumpped_info;
  const bool log_dumpped = FLAGS_tvar_log_dumpped;

  if (white_matcher.wildcards().empty() &&
      !white_matcher.exact_names().empty()) {
    for (auto it = white_matcher.exact_names().cbegin();
         it != white_matcher.exact_names().cend(); ++it) {
      auto& name = *it;
      if (!black_matcher.Match(name)) {
        if (tvar::Variable::describe_exposed(name, os, opt.quote_string,
                                             opt.display_filter) != 0) {
          continue;
        }

        if (log_dumpped) {
          dumpped_info << '\n' << name << ": " << streambuf.data();
        }

        if (!dumper.dump(name, streambuf.data())) {
          return -1;
        }
        streambuf.reset();
        ++count;
      }
    }
  } else {
    // Have to iterate all variable.
    std::vector<std::string> variable_names;
    tvar::Variable::list_exposed(variable_names, opt.display_filter);

    // Sort the names to make them more readable.
    std::sort(variable_names.begin(), variable_names.end());
    for (auto it = variable_names.begin(); it != variable_names.end(); ++it) {
      const std::string& name = *it;
      if (white_matcher.Match(name) && !black_matcher.Match(name)) {
        if (tvar::Variable::describe_exposed(name, os, opt.quote_string,
                                             opt.display_filter)) {
          continue;
        }

        if (log_dumpped) {
          dumpped_info << '\n' << name << ": " << streambuf.data();
        }
        if (!dumper.dump(name, streambuf.data())) {
          return -1;
        }
        streambuf.reset();
        ++count;
      }
    }
  }

  if (log_dumpped) {
    LOG_INFO << "Dumpped variables:" << dumpped_info.str();
  }
  return count;
}

// ---------------------- export to files --------------------------

class FileDumper : public Dumper {
 public:
  FileDumper(const std::string& filename, tutil::StringView prefix)
      : filename_(filename), fp_(nullptr, FileDumper::FileClose) {
    // setting prefix
    // remove trailing spaces.
    const char* p = prefix.data() + prefix.size();
    for (; p != prefix.data() && isspace(p[-1]); --p) {
    }
    prefix.remove_suffix(prefix.data() + prefix.size() - p);
    // normalize it.
    if (!prefix.empty()) {
      to_underscored_name(prefix_, prefix);
      if (prefix_.back() != '_') {
        prefix_.push_back('_');
      }
    }
  }

  ~FileDumper() {}

  bool dump(const std::string& name, const tutil::StringView& description) {
    if (fp_ == nullptr) {
      // TODO(tesla): call mkdir first?
      fp_.reset(fopen(filename_.c_str(), "w"));
      if (fp_ == nullptr) {
        LOG_ERROR << "Fail to open " << filename_;
        return false;
      }
    }
    if (fprintf(fp_.get(), "%.*s%.*s : %.*s\r\n", (int)prefix_.size(),
                prefix_.data(), (int)name.size(), name.data(),
                (int)description.size(), description.data()) < 0) {
      LOG_ERROR << "Fail to write into " << filename_;
      return false;
    }
    return true;
  }

 private:
  static void FileClose(FILE* fp) {
    if (fp) {
      fclose(fp);
    }
  }

  std::string filename_;
  std::string prefix_;
  std::unique_ptr<FILE, decltype(FileDumper::FileClose)*> fp_;
};

class FileDumperGroup : public Dumper {
 public:
 private:
};

}  // namespace tvar
}  // namespace tesla
