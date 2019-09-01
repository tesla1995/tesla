// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file adds defines about the platform we're currently building on.
//  Operating System:
//    OS_WIN / OS_MACOSX / OS_LINUX / OS_POSIX (MACOSX or LINUX) /
//    OS_NACL (NACL_SFI or NACL_NONSFI) / OS_NACL_SFI / OS_NACL_NONSFI
//  Compiler:
//    COMPILER_MSVC / COMPILER_GCC
//  Processor:
//    ARCH_CPU_X86 / ARCH_CPU_X86_64 / ARCH_CPU_X86_FAMILY (X86 or X86_64)
//    ARCH_CPU_32_BITS / ARCH_CPU_64_BITS

#ifndef TESLA_TUTIL_BUILD_CONFIG_H_
#define TESLA_TUTIL_BUILD_CONFIG_H_

// platform detection.
#if defined(__linux__)
#define OS_LINUX 1
// include a system header to pull in features.h for glibc/uclibc macros.
#include <unistd.h>
#if defined(__GLIBC__) && !defined(__UCLIBC__)
// we are really using glibc, not uClibc pretending to be glibc.
#define LIBC_GLIBC 1
#endif
#elif defined(_WIN32)
#define OS_WIN 1
#define TOOLKIT_VIEWS 1
#else
#error Please add support for your platform in tuil/build_config.h
#endif

// Compiler detection
#if defined(__GNUC__)
#define COMPILER_GCC 1
#elif defined(_MSC_VER)
#define COMPILER_MSVC 1
#else
#error Please add support for your compiler in tutil/build_config.h
#endif

// For access to standard POSIXish features, use OS_POSIX instead of a
// more specific macro.
#if defined(OS_LINUX)
#define OS_POSIX 1
#endif

#endif // TESLA_TUTIL_BUILD_CONFIG_H_
