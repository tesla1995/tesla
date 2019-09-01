#include "tutil/files/file_path.h"
#include "tutil/strings/string_util.h"

namespace tutil {

using StringType = FilePath::StringType;

// internal ---------------------------------------------------------
namespace internal {

const FilePath::CharType kStringTerminator = FILE_PATH_LITERAL('\0');

const char* kCommonDoubleExtensionSuffixes[] = { "gz", "z", "bz2" };
const char* kCommonDoubleExtensions[] = { "user.js" };

// Find the position of the '.' that separates the extension from the rest
// of the file name. The position is relative to BaseName(), not value().
// Returns npos if it can't find an extension.
//
// Example:
//   "."         -> StringType::npos
//   ".."        -> StringType::npos
//   "name"      -> StringType::npos
//   "name.jpg"  -> 4
//   ".jpg"      -> 0
StringType::size_type FinalExtensionSeparatorPosition(const StringType& path) {
  // Special case "." and ".."
  if (path == FilePath::kCurrentDirectory || path == FilePath::kParentDirectory) {
    return StringType::npos;
  }

  return path.rfind(FilePath::kExtensionSeparator);
}

// Same as above, but allow a second extension component of up to 4
// characters when the rightmost extension component is a common double
// extension(gz, bz2, Z). For example, foo.tar.gz or foo.tar.Z would have
// extension components of '.tar.gz' and '.tar.Z' respectively.
StringType::size_type ExtensionSeparatorPosition(const StringType& path) {
  const StringType::size_type last_dot = FinalExtensionSeparatorPosition(path);

  // No extension, or the extension is the whole filename.
  if (last_dot == StringType::npos || last_dot == 0U) {
    return last_dot;
  }

  const StringType::size_type penultimate_dot =
      path.rfind(FilePath::kExtensionSeparator, last_dot - 1);

  const StringType::size_type last_separator =
      path.find_last_of(FilePath::kSeparators, last_dot - 1,
                        FilePath::kSeparatorsLength - 1);

  // No second extension.
  if (penultimate_dot == StringType::npos ||
      (last_separator != StringType::npos &&
       penultimate_dot < last_separator)) {
    return last_dot;
  }

  for (size_t i = 0; i < ARRAY_SIZE(kCommonDoubleExtensions); ++i) {
    StringType extension(path, penultimate_dot + 1);
    if (LowerCaseEqualsASCII(extension, kCommonDoubleExtensions[i])) {
      return penultimate_dot;
    }
  }

  StringType extension(path, last_dot + 1);
  for (size_t i = 0; i < ARRAY_SIZE(kCommonDoubleExtensionSuffixes); ++i) {
    if (LowerCaseEqualsASCII(extension, kCommonDoubleExtensionSuffixes[i])) {
      if ((last_dot - penultimate_dot) <= 5U &&
          (last_dot - penultimate_dot) > 1U) {
        return penultimate_dot;
      }
    } 
  }

  return last_dot;
}

// Returns true if path is "", ".", or "..".
bool IsEmptyOrSpecialCase(const StringType& path) {
  if (path.empty() || path == FilePath::kCurrentDirectory ||
      path == FilePath::kParentDirectory) {
    return true;
  }

  return false;
}

} // namespace internal

// construtor ------------------------------------------------
FilePath::FilePath() {

}

FilePath::FilePath(const FilePath& that) : path_(that.path_) {

}

FilePath::FilePath(FilePath&& that) : path_(std::move(that.path_)) {

}

FilePath::FilePath(const StringType& path) : path_(path) {
  StringType::size_type nul_pos = path_.find(internal::kStringTerminator);  
  if (nul_pos != StringType::npos) {
    path_.erase(nul_pos, StringType::npos);
  }
}

FilePath::~FilePath() {

}

// overloaded operator ------------------------------------------
FilePath& FilePath::operator=(const FilePath& that) {
  path_ = that.path_;
  return *this;
}

FilePath& FilePath::operator=(FilePath&& that) {
  path_ = std::move(that.path_);
  return *this;
}

bool FilePath::operator==(const FilePath& that) const {
  return path_ == that.path_;
}

bool FilePath::operator!=(const FilePath& that) const {
  return path_ != that.path_;
}

// static -------------------------------------------------------
bool FilePath::IsSeparator(CharType character) {
  for (size_t i = 0; i < kSeparatorsLength - 1; ++i) {
    if (character == kSeparators[i]) {
      return true;
    }
  }

  return false;
}

// libgen's dirname and basename aren't guaranteed to be thread-safe and aren't
// guaranteed to not modify their input strings, and in fact are implemented 
// differently in this regrad on different platforms. Don't use them, but adhere
// to their behavior.
//
// Example:
//   FilePath("test").DirName() = FilePath(".")
//   FilePath("/test").DirName() = FilePath("/")
//   FilePath("//test").DirName() = FilePath("//")
//   FilePath("/home/test").DirName() = FilePath("/home")
//   FilePath("/home//test").DirName() = FilePath("/home")
FilePath FilePath::DirName() const {
  FilePath new_path(path_);
  new_path.StripTrailingSeparatorsInternal();

  // Finds the last character equal to the one of the characters in the given character
  // sequences `kSeparators'.
  StringType::size_type last_separator =
    new_path.path_.find_last_of(kSeparators, StringType::npos, kSeparatorsLength - 1);

  if (last_separator == StringType::npos) {
    // path_ is in the current directory.
    new_path.path_.resize(0);
  } else if (last_separator == 0) {
    // path_ is in the root directory.
    new_path.path_.resize(1);
  } else if (last_separator == 1 &&
             IsSeparator(new_path.path_[0])) {
    // path_ is in "//"; leave the double separator intact indicating alternate root.
    new_path.path_.resize(2); 
  } else if (last_separator != 0) {
    // path_ is somewhere else, trim the basename.
    new_path.path_.resize(last_separator);
  }

  new_path.StripTrailingSeparatorsInternal();
  if (!new_path.path_.length()) {
    new_path.path_ = kCurrentDirectory;
  }

  return new_path;
}

FilePath FilePath::BaseName() const {
  FilePath new_path(path_);
  new_path.StripTrailingSeparatorsInternal();

  // Keep everything after the final separator, but if the pathname is only
  // one character and it's a separator, leave it alone.
  StringType::size_type last_separator =
    new_path.path_.find_last_of(kSeparators, StringType::npos,
                                kSeparatorsLength - 1);

  if (last_separator != StringType::npos &&
      last_separator < new_path.path_.length() - 1) {
    new_path.path_.erase(0, last_separator + 1); 
  }

  return new_path;
}


StringType FilePath::Extension() const {
  // FIXME: test whether it use move constructor.
  FilePath base(BaseName()); 
  const StringType::size_type dot = internal::ExtensionSeparatorPosition(base.path_);  
  if (dot == StringType::npos) {
    return StringType();
  }

  return base.path_.substr(dot, StringType::npos);
}

StringType FilePath::FinalExtension() const {
  FilePath base(BaseName());  
  const StringType::size_type dot = internal::FinalExtensionSeparatorPosition(base.path_);
  if (dot == StringType::npos) {
    return StringType();
  }

  return base.path_.substr(dot, StringType::npos);
}

FilePath FilePath::RemoveExtension() const {
  if (Extension().empty()) {
    return *this;
  }

  const StringType::size_type dot = internal::ExtensionSeparatorPosition(path_);
  if (dot == StringType::npos) {
    return *this;
  }

  return FilePath(path_.substr(0, dot));
}

FilePath FilePath::RemoveFinalExtension() const {
  if (FinalExtension().empty()) {
    return *this;
  }

  const StringType::size_type dot = internal::FinalExtensionSeparatorPosition(path_);
  if (dot == StringType::npos) {
    return *this;
  }

  return FilePath(path_.substr(0, dot));
}

FilePath FilePath::InsertBeforeExtension(const StringType& suffix) const {
  if (suffix.empty()) {
    return FilePath(path_);
  }

  if (internal::IsEmptyOrSpecialCase(BaseName().value())) {
    return FilePath();
  }

  StringType ext = Extension();
  StringType ret = RemoveExtension().value();
  ret.append(suffix);
  ret.append(ext);
  return FilePath(ret);
}

FilePath FilePath::AddExtension(const StringType& extension) const {
  if (internal::IsEmptyOrSpecialCase(BaseName().value())) {
    return FilePath();
  }

  // If the new extension is "" or ".", then just return the current FilePath.
  if (extension.empty() || extension == StringType(1, kExtensionSeparator)) {
    return *this;
  }

  StringType str = path_;
  if (extension[0] != kExtensionSeparator &&
      *(str.end() - 1) != kExtensionSeparator) {
    str.append(1, kExtensionSeparator);
  }
  str.append(extension);
  return FilePath(str);
}

FilePath FilePath::ReplaceExtension(const StringType& extension) const {
  if (internal::IsEmptyOrSpecialCase(BaseName().value())) {
    return FilePath();
  }

  FilePath no_ext = RemoveExtension();
  // If the new extension is "" or ".", then just remove the current extension.
  if (extension.empty() || extension == StringType(1, kExtensionSeparator)) {
    return no_ext;
  }

  StringType str = no_ext.value();
  if (extension[0] != kExtensionSeparator) {
    str.append(1, kExtensionSeparator);
  }
  str.append(extension);
  return FilePath(str);
}

bool FilePath::MatchesExtension(const StringType& extension) const {
  //CHECK(extension.empty() || extension[0] == kExtensionSeparator);

  StringType current_extension = Extension();

  if (current_extension.length() != extension.length()) {
    return false;
  }

  return FilePath::CompareEqualIgnoreCase(current_extension, extension);
}

int FilePath::CompareIgnoreCase(const StringType& string1,
                                const StringType& string2) {
  int comparison = strcasecmp(string1.c_str(), string2.c_str());
  if (comparison < 0) {
    return -1;
  }

  if (comparison > 0) {
    return 1;
  }

  return 0;
}

// Examples:
//   "/"       ->     "/"
//   "//"      ->     "//"
//   "///"     ->     "/"
//   "./"      ->     "."
//   "../"     ->     ".."
//   "aaaa"    ->     "aaaa"
//   "aaaa/"   ->     "aaaa"
//   "aaaa//"  ->     "aaaa"
//   "aaaa///" ->     "aaaa"
void FilePath::StripTrailingSeparatorsInternal() {
  // start will be 1, which prevent stripping the leading separator if there is
  // only one separator.
  StringType::size_type start = 1;

  StringType::size_type last_stripped = StringType::npos;
  for (StringType::size_type pos = path_.length();
       pos > start && IsSeparator(path_[pos - 1]);
       --pos) {
    // If the string only has two separators and they're at beginning,
    // don't strip them, unless the string began with more than two separators.
    if (pos != start + 1 || last_stripped == start + 2 ||
        !IsSeparator(path_[start - 1])) {
      path_.resize(pos - 1);
      last_stripped = pos;
    }
  }
}

} // namespace tutil
