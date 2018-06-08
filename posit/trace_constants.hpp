#pragma once
// trace_constants.hpp: definition of constants that direct intermediate result reporting
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.


namespace sw {
namespace unum {


# ifndef POSIT_VERBOSE_OUTPUT
// posit decode and conversion
constexpr bool _trace_decode      = false;
constexpr bool _trace_conversion  = false;
constexpr bool _trace_rounding    = false;

// arithmetic operator tracing
constexpr bool _trace_add         = false;
constexpr bool _trace_sub         = false;
constexpr bool _trace_mul         = false;
constexpr bool _trace_div         = false;
constexpr bool _trace_reciprocate = false;
constexpr bool _trace_sqrt        = false;

// quire update tracing
constexpr bool _trace_quire_add   = false;

# else // !POSIT_VERBOSE_OUTPUT

#ifdef POSIT_TRACE_ALL
#define POSIT_TRACE_DECODE
#define POSIT_TRACE_CONVERSION
#define POSIT_TRACE_ROUNDING
#define POSIT_TRACE_ADD
#define POSIT_TRACE_SUB
#define POSIT_TRACE_MUL
#define POSIT_TRACE_DIV
#define POSIT_TRACE_RECIPROCATE
#define POSIT_TRACE_SQRT
#endif

#ifdef QUIRE_TRACE_ALL
#define QUIRE_TRACE_ADD
#endif

// posit decode and conversion

#ifndef POSIT_TRACE_DECODE
constexpr bool _trace_decode = false;
#else
constexpr bool _trace_decode = true;
#endif

#ifndef POSIT_TRACE_CONVERSION
constexpr bool _trace_conversion = false;
#else
constexpr bool _trace_conversion = true;
#endif

#ifndef POSIT_TRACE_ROUNDING
constexpr bool _trace_rounding = false;
#else
constexpr bool _trace_rounding = true;
#endif

// arithmetic operator tracing
#ifndef POSIT_TRACE_ADD
constexpr bool _trace_add = false;
#else
constexpr bool _trace_add = true;
#endif

#ifndef POSIT_TRACE_SUB
constexpr bool _trace_sub = false;
#else
constexpr bool _trace_sub = true;
#endif

#ifndef POSIT_TRACE_MUL
constexpr bool _trace_mul = false;
#else
constexpr bool _trace_mul = true;
#endif

#ifndef POSIT_TRACE_DIV
constexpr bool _trace_div = false;
#else
constexpr bool _trace_div = true;
#endif

#ifndef POSIT_TRACE_RECIPROCATE
constexpr bool _trace_reciprocate = false;
#else
constexpr bool _trace_reciprocate = true;
#endif

#ifndef POSIT_TRACE_SQRT
constexpr bool _trace_sqrt = false;
#else
constexpr bool _trace_sqrt = true;
#endif

// QUIRE tracing
#ifndef QUIRE_TRACE_ADD
constexpr bool _trace_quire_add = false;
#else
constexpr bool _trace_quire_add = true;
#endif

# endif


}  // namespace unum

}  // namespace sw

