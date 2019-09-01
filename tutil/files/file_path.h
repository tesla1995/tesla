// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// FilePath is a container for pathnames stored in a platform's native string
// type, providing containers for manipulation in according with the
// platform's conventions for pathnames.
// types:
//
//                   POSIX            
//                   --------------- 
// Fundamental type  char[]         
// Encoding          unspecified*  
// Separator         /               
// Drive letters     no             
// Alternate root    // (surprise!)
//
// * The encoding need not be specified on POSIX systems, although some
//   POSIX-compliant systems do specify an encoding.  Mac OS X uses UTF-8.
//   Chrome OS also uses UTF-8.
//   Linux does not specify an encoding, but in practice, the locale's
//   character set may be used.
//
// Several methods are available to perform common operations on a FilePath
// object, such as determining the parent directory (DirName), isolating the
// final path component (BaseName), and appending a relative pathname string
// to an existing FilePath object (Append).  These methods are highly
// recommended over attempting to split and concatenate strings directly.
// These methods are based purely on string manipulation and knowledge of
// platform-specific pathname conventions, and do not consult the filesystem
// at all, making them safe to use without fear of blocking on I/O operations.
// These methods do not function as mutators but instead return distinct
// instances of FilePath objects, and are therefore safe to use on const
// objects.  The objects themselves are safe to share between threads.
//
// To aid in initialization of FilePath objects from string literals, a
// FILE_PATH_LITERAL macro is provided, which accounts for the difference
// between char[]-based pathnames on POSIX systems and wchar_t[]-based
// pathnames on Windows.
//
// Paths can't contain NULs as a precaution agaist premature truncation.
//
// Because a FilePath object should not be instantiated at the global scope,
// instead, use a FilePath::CharType[] and initialize it with
// FILE_PATH_LITERAL.  At runtime, a FilePath object can be created from the
// character array.  Example:
//
// | const FilePath::CharType kLogFileName[] = FILE_PATH_LITERAL("log.txt");
// |
// | void Function() {
// |   FilePath log_file_path(kLogFileName);
// |   [...]
// | }
//
// ARCANE BITS OF PATH TRIVIA
//
//  - A double leading slash is actually part of the POSIX standard.  Systems
//    are allowed to treat // as an alternate root, as Windows does for UNC
//    (network share) paths.  Most POSIX systems don't do anything special
//    with two leading slashes, but FilePath handles this case properly
//    in case it ever comes across such a system.  FilePath needs this support
//    for Windows UNC paths, anyway.
//    References:
//    The Open Group Base Specifications Issue 7, sections 3.266 ("Pathname")
//    and 4.12 ("Pathname Resolution"), available at:
//    http://www.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap03.html#tag_03_266
//    http://www.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap04.html#tag_04_12

// Note: The simplified FilePath class can ONLY be used in POSIX platform. The code is not 
// compatible with windows platform.

#ifndef TESLA_TUTIL_FILES_FILE_PATH_H_
#define TESLA_TUTIL_FILES_FILE_PATH_H_

#include <string>
#include <vector>

#include "tutil/macros.h"
#include "tutil/compiler_specific.h"
#include "tutil/string_view.h"

namespace tutil {

// An abstraction to isolate users from the difference between native 
// pathnames on different platforms.
class FilePath {
 public:
  using StringType = std::string;
  using CharType = StringType::value_type;

  // Null-terminated array of separators used to separate components in
  // hierarchical paths. Each character in this array is a valid separator,
  // but kSeparators[0] is treated as the canonical separator and will be
  // used when composing pathnames.
  static constexpr  CharType kSeparators[] = "/";

  // arraysize(kSeparators).
  static constexpr size_t kSeparatorsLength = ARRAY_SIZE(kSeparators);

  // A special path component meaning "this directory."
  static constexpr CharType kCurrentDirectory[] = ".";

  // A special path component meaning "the parent diretory."
  static constexpr CharType kParentDirectory[] = "..";

  // The character used to identify a file extension.
  static constexpr CharType kExtensionSeparator = '.';

  FilePath();
  FilePath(const FilePath& that);
  FilePath(FilePath&& that);
  explicit FilePath(const StringType& path);
  
  ~FilePath();

  FilePath& operator=(const FilePath& that);
  FilePath& operator=(FilePath&& that);

  bool operator==(const FilePath& that) const;
  bool operator!=(const FilePath& that) const;

  // Required for some STL containers and operations.
  bool operator<(const FilePath& that) const {
    return path_ < that.path_;
  }

  const StringType& value() const { return path_; }

  bool empty() const { return path_.empty(); }

  void clear() { path_.clear(); }

  // Returns true if |character| is in kSeparators.
  static bool IsSeparator(CharType character);
  
  // Returns a vector of all of the components of the provided path. It is
  // equivalent to calling DirName().value() on the path's root component,
  // and BaseName().value() on each child component.
  //
  // To make sure this is lossless so we can differentiate absolute and
  // relative paths, the root slash will be included even though no other
  // slashes will be. The precise behavior is:
  //
  // Posix:  "/foo/bar"  ->  [ "/", "foo", "bar"  ]
  // Windows:  "C:\foo\bar"  ->  [ "C:", "\\", "foo", "bar"  ]
  void GetComponent(std::vector<FilePath::StringType>* components) const;

  // Returns true if this FilePath is a strict parent of the |child|. Absolute
  // and relative paths are accepted i.e. is /foo parent to /foo/bar and
  // is foo parent to foo/bar. Does not convert paths to absolute, follow
  // symlinks or directory navigation (e.g. ".."). A path is *NOT* its own
  // parent.
  bool IsParent(const FilePath& child) const;

  // If IsParent(child) holds, appends to path (if non-NULL) the
  // relative path to child and returns true.  For example, if parent
  // holds "/Users/johndoe/Library/Application Support", child holds
  // "/Users/johndoe/Library/Application Support/Google/Chrome/Default", and
  // *path holds "/Users/johndoe/Library/Caches", then after
  // parent.AppendRelativePath(child, path) is called *path will hold
  // "/Users/johndoe/Library/Caches/Google/Chrome/Default".  Otherwise,
  // returns false.
  bool AppendRelativePath(const FilePath& child, FilePath* path) const;

  // Returns a FilePath corresponding to the directory containing the path
  // named by this object, stripping away the file component.  If this object
  // only contains one component, returns a FilePath identifying
  // kCurrentDirectory.  If this object already refers to the root directory,
  // returns a FilePath identifying the root directory.
  FilePath DirName() const WARN_UNUSED_RESULT;

  // Returns a FilePath corresponding to the last path component of this
  // object, either a file or a directory.  If this object already refers to
  // the root directory, returns a FilePath identifying the root directory;
  // this is the only situation in which BaseName will return an absolute path.
  FilePath BaseName() const WARN_UNUSED_RESULT;

  // Returns ".jpg" for path "C:\pics\jojo.jpg", or an empty string if
  // the file has no extension.  If non-empty, Extension() will always start
  // with precisely one ".".  The following code should always work regardless
  // of the value of path.  For common double-extensions like .tar.gz and
  // .user.js, this method returns the combined extension.  For a single
  // component, use FinalExtension().
  // new_path = path.RemoveExtension().value().append(path.Extension());
  // ASSERT(new_path == path.value());
  // NOTE: this is different from the original file_util implementation which
  // returned the extension without a leading "." ("jpg" instead of ".jpg")
  StringType Extension() const;

  // Returns the path's file extension, as in Extension(), but will
  // never return a double extension.
  //
  // TODO(davidben): Check all our extension-sensitive code to see if
  // we can rename this to Extension() and the other to something like
  // LongExtension(), defaulting to short extensions and leaving the
  // long "extensions" to logic like butil::GetUniquePathNumber().
  StringType FinalExtension() const;

  // Returns "C:\pics\jojo" for path "C:\pics\jojo.jpg"
  // NOTE: this is slightly different from the similar file_util implementation
  // which returned simply 'jojo'.
  FilePath RemoveExtension() const WARN_UNUSED_RESULT;

  // Removes the path's file extension, as in RemoveExtension(), but
  // ignores double extensions.
  FilePath RemoveFinalExtension() const WARN_UNUSED_RESULT;

  // Inserts |suffix| after the file name portion of |path| but before the
  // extension.  Returns "" if BaseName() == "." or "..".
  // Examples:
  // path == "C:\pics\jojo.jpg" suffix == " (1)", returns "C:\pics\jojo (1).jpg"
  // path == "jojo.jpg"         suffix == " (1)", returns "jojo (1).jpg"
  // path == "C:\pics\jojo"     suffix == " (1)", returns "C:\pics\jojo (1)"
  // path == "C:\pics.old\jojo" suffix == " (1)", returns "C:\pics.old\jojo (1)"
  FilePath InsertBeforeExtension(
      const StringType& suffix) const WARN_UNUSED_RESULT;
  FilePath InsertBeforeExtensionASCII(
      const tutil::StringView& suffix) const WARN_UNUSED_RESULT;

  // Adds |extension| to |file_name|. Returns the current FilePath if
  // |extension| is empty. Returns "" if BaseName() == "." or "..".
  FilePath AddExtension(
      const StringType& extension) const WARN_UNUSED_RESULT;

  // Replaces the extension of |file_name| with |extension|.  If |file_name|
  // does not have an extension, then |extension| is added.  If |extension| is
  // empty, then the extension is removed from |file_name|.
  // Returns "" if BaseName() == "." or "..".
  FilePath ReplaceExtension(
      const StringType& extension) const WARN_UNUSED_RESULT;

  // Returns true if the file path matches the specified extension. The test is
  // case insensitive. Don't forget the leading period if appropriate.
  bool MatchesExtension(const StringType& extension) const;

  // Returns a FilePath by appending a separator and the supplied path
  // component to this object's path.  Append takes care to avoid adding
  // excessive separators if this object's path already ends with a separator.
  // If this object's path is kCurrentDirectory, a new FilePath corresponding
  // only to |component| is returned.  |component| must be a relative path;
  // it is an error to pass an absolute path.
  FilePath Append(const StringType& component) const WARN_UNUSED_RESULT;
  FilePath Append(const FilePath& component) const WARN_UNUSED_RESULT;

  // Returns true if this FilePath contains an absolute path.  On Windows, an
  // absolute path begins with either a drive letter specification followed by
  // a separator character, or with two separator characters.  On POSIX
  // platforms, an absolute path begins with a separator character.
  bool IsAbsolute() const;

  // Returns true if the patch ends with a path separator character.
  bool EndsWithSeparator() const WARN_UNUSED_RESULT;

  // Returns a copy of this FilePath that ends with a trailing separator. If
  // the input path is empty, an empty FilePath will be returned.
  FilePath AsEndingWithSeparator() const WARN_UNUSED_RESULT;

  // Returns a copy of this FilePath that does not end with a trailing
  // separator.
  FilePath StripTrailingSeparators() const WARN_UNUSED_RESULT;

  // Returns true if this FilePath contains an attempt to reference a parent
  // directory (e.g. has a path component that is "..").
  bool ReferencesParent() const;

  // Compare two strings in the same way the file system does.
  // Note that these always ignore case, even on file systems that are case-
  // sensitive. If case-sensitive comparison is ever needed, add corresponding
  // methods here.
  // The methods are written as a static method so that they can also be used
  // on parts of a file path, e.g., just the extension.
  // CompareIgnoreCase() returns -1, 0 or 1 for less-than, equal-to and
  // greater-than respectively.
  static int CompareIgnoreCase(const StringType& string1,
                               const StringType& string2);
  static bool CompareEqualIgnoreCase(const StringType& string1,
                                     const StringType& string2) {
    return CompareIgnoreCase(string1, string2) == 0;
  }
  static bool CompareLessIgnoreCase(const StringType& string1,
                                    const StringType& string2) {
    return CompareIgnoreCase(string1, string2) < 0;
  }

 private:
  // Removing trailing separators from this object. If the path is absolute, it 
  // will never be stripped any more than to refer to the absolute root
  // directory, so "////" will become "/", not "". A leading pair of separators
  // is never stripped, to support alternate roots.
  void StripTrailingSeparatorsInternal();

  StringType path_;
};

} // namespace tutil

// Macros for string literal initialization of FilePath::CharType[], and for
// using a FilePath::CharType[] in a printf-style format string.
#define FILE_PATH_LITERAL(x) x
#define PRFilePath "s"
#define PRFilePathLiteral "%s"

namespace std {

template<>
struct hash<tutil::FilePath> {
  size_t operator()(const tutil::FilePath& f) const {
    return hash<std::string>()(f.value());
  }
};

} // namespace std

#endif // TESLA_TUTIL_FILES_FILE_PATH_H_
