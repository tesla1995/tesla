// Copyright (c) 2019 Tesla, Inc.
//
// Licensed under the Apache License, Version 2.0 (the License);
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an AS IS BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Author: Michael Tesla (michaeltesla1995@gmail.com)
// Date: Sat Nov 16 17:23:13 CST 2019

// The file is copied from https://github.com/google/glog.git

//////////////////////////////////////////////////////////////////////////

#include <unistd.h>  // for writes()
#include <assert.h>

#define WRITE_TO_STDERR(buf, len) write(STDERR_FILENO, buf, len)

// CHECK dies with a fatal error if condition is not true.  It is *not*
// controlled by NDEBUG, so the check will be executed regardless of
// compilation mode.  Therefore, it is safe to do things like:
//    CHECK(fp->Write(x) == 4)
// Note we use write instead of printf/puts to avoid the risk we'll
// call malloc().
#define TESLA_CHECK(condition)                                          \
  do {                                                                  \
    if (!(condition)) {                                                 \
      WRITE_TO_STDERR("Check failed: " #condition "\n",                 \
                      sizeof("Check failed: " #condition "\n")-1);      \
      abort();                                                          \
    }                                                                   \
  } while (0)

// Helper macro for binary operators; prints the two values on error
// Don't use this macro directly in your code, use CHECK_EQ et al below

// WARNING: These don't compile correctly if one of the arguments is a pointer
// and the other is NULL. To work around this, simply static_cast NULL to the
// type of the desired pointer.

// TODO(jandrews): Also print the values in case of failure.  Requires some
// sort of type-sensitive ToString() function.
#define TESLA_CHECK_OP(op, val1, val2)                                  \
  do {                                                                  \
    if (!((val1) op (val2))) {                                          \
      fprintf(stderr, "[%s:%d] Check failed: %s %s %s\n", __FILE__,      \
                       __LINE__, #val1, #op, #val2);                    \
      abort();                                                          \
    }                                                                   \
  } while (0)

#define TESLA_CHECK_EQ(val1, val2) TESLA_CHECK_OP(==, val1, val2)
#define TESLA_CHECK_NE(val1, val2) TESLA_CHECK_OP(!=, val1, val2)
#define TESLA_CHECK_LE(val1, val2) TESLA_CHECK_OP(<=, val1, val2)
#define TESLA_CHECK_LT(val1, val2) TESLA_CHECK_OP(< , val1, val2)
#define TESLA_CHECK_GE(val1, val2) TESLA_CHECK_OP(>=, val1, val2)
#define TESLA_CHECK_GT(val1, val2) TESLA_CHECK_OP(> , val1, val2)

// A few more checks that only happen in debug mode
#ifdef NDEBUG
#define TESLA_DCHECK_EQ(val1, val2)
#define TESLA_DCHECK_NE(val1, val2)
#define TESLA_DCHECK_LE(val1, val2)
#define TESLA_DCHECK_LT(val1, val2)
#define TESLA_DCHECK_GE(val1, val2)
#define TESLA_DCHECK_GT(val1, val2)
#else
#define TESLA_DCHECK_EQ(val1, val2)  TESLA_CHECK_EQ(val1, val2)
#define TESLA_DCHECK_NE(val1, val2)  TESLA_CHECK_NE(val1, val2)
#define TESLA_DCHECK_LE(val1, val2)  TESLA_CHECK_LE(val1, val2)
#define TESLA_DCHECK_LT(val1, val2)  TESLA_CHECK_LT(val1, val2)
#define TESLA_DCHECK_GE(val1, val2)  TESLA_CHECK_GE(val1, val2)
#define TESLA_DCHECK_GT(val1, val2)  TESLA_CHECK_GT(val1, val2)
#endif

#define TESLA_CHECK_TRUE(cond)     TESLA_CHECK(cond)
#define TESLA_CHECK_FALSE(cond)    TESLA_CHECK(!(cond))
#define TESLA_CHECK_STREQ(a, b)    TESLA_CHECK(strcmp(a, b) == 0)
