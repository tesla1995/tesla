// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESLA_TUTIL_COMPILER_SPECIFIC_H_
#define TESLA_TUTIL_COMPILER_SPECIFIC_H_

#include "tutil/build_config.h"

// Mark a branch likely or unlikely to be true.
// We can't remove the TESLA_ prefix because the name is likely to conflict
// namely kylin already has the macro.
#if defined(COMPILER_GCC)
#  if defined(__cplusplus)
#    define TESLA_LIKELY(expr) (__builtin_expect((bool)(expr), true))
#    define TESLA_UNLIKELY(expr) (__builtin_expect((bool)(expr), false))
#  else
#    define TESLA_LIKELY(expr) (__builtin_expect(!!(expr), 1))
#    define TESLA_UNLIKELY(expr) (__builtin_expect(!!(expr), 0))
#  endif
#else
#    define TESLA_LIKELY(expr) (expr)
#    define TESLA_UNLIKELY(expr) (expr)
#endif

// Annotate a function indicating the caller must examine the return value.
// Use like:
//   int foo() WARN_UNUSED_RESULT;
#if defined(COMPILER_GCC) && __cplusplus >= 201103 && \
      (__GNUC__ * 10000 + __GNUC_MINOR__ * 100) >= 40700
#define WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#else
#define WARN_UNUSED_RESULT
#endif

// Cacheline related ----------------------------------------------------
#define TESLA_CACHELINE_SIZE 64

#ifdef __GNUC__
#define TESLA_CACHELINE_ALIGNMENT __attribute__((aligned(TESLA_CACHELINE_SIZE)))
#endif /* __GNUC__ */

#ifdef _MSC_VER
#define TESLA_CACHELINE_ALIGNMENT __declspec(aligned(TESLA_CACHELINE_SIZE))
#endif /* _MSC_VER */

#endif // TESLA_TUTIL_COMPILER_SPECIFIC_H_
