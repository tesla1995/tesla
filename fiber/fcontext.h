//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef TESLA_FIBER_FCONTEXT_H_
#define TESLA_FIBER_FCONTEXT_H_

#include <cstddef>

typedef void*   fcontext_t;

struct transfer_t {
    fcontext_t  fctx;
    void*       data;
};

#ifdef __cplusplus
extern "C" {
#endif

transfer_t jump_fcontext( fcontext_t const to, void * vp);
fcontext_t make_fcontext( void * sp, size_t size, void (* fn)( transfer_t) );
// based on an idea of Giovanni Derreta
transfer_t ontop_fcontext( fcontext_t const to, void * vp, transfer_t (* fn)( transfer_t) );

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // TESLA_FIBER_FCONTEXT_H_
