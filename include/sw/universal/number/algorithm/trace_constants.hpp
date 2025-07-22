#pragma once
// trace_constants.hpp: definition of constants that direct intermediate result reporting
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

# ifndef ALGORITHM_VERBOSE_OUTPUT
// number value decode and conversion
constexpr bool _trace_decode      = false;
constexpr bool _trace_conversion  = false;
constexpr bool _trace_rounding    = false;

// arithmetic operator tracing
constexpr bool _trace_add         = false;
constexpr bool _trace_sub         = false;
constexpr bool _trace_mul         = false;
constexpr bool _trace_div         = false;
constexpr bool _trace_reciprocal  = false;
constexpr bool _trace_sqrt        = false;

// quire update tracing
constexpr bool _trace_quire_add   = false;

# else // !ALGORITHM_VERBOSE_OUTPUT

#ifdef ALGORITHM_TRACE_ALL
#define ALGORITHM_TRACE_DECODE
#define ALGORITHM_TRACE_CONVERSION
#define ALGORITHM_TRACE_ROUNDING
#define ALGORITHM_TRACE_ADD
#define ALGORITHM_TRACE_SUB
#define ALGORITHM_TRACE_MUL
#define ALGORITHM_TRACE_DIV
#define ALGORITHM_TRACE_RECIPROCAL
#define ALGORITHM_TRACE_SQRT

#define VALUE_TRACE_CONVERSION
#define VALUE_TRACE_ADD
#define VALUE_TRACE_SUB
#define VALUE_TRACE_MUL
#define VALUE_TRACE_DIV
#endif

#ifdef QUIRE_TRACE_ALL
#define QUIRE_TRACE_ADD
#endif

// number value decode and conversion

#ifndef ALGORITHM_TRACE_DECODE
constexpr bool _trace_decode = false;
#else
constexpr bool _trace_decode = true;
#endif

#ifndef ALGORITHM_TRACE_CONVERSION
constexpr bool _trace_conversion = false;
#else
#define VALUE_TRACE_CONVERSION
constexpr bool _trace_conversion = true;
#endif

#ifndef ALGORITHM_TRACE_ROUNDING
constexpr bool _trace_rounding = false;
#else
constexpr bool _trace_rounding = true;
#endif

// arithmetic operator tracing
#ifndef ALGORITHM_TRACE_ADD
constexpr bool _trace_add = false;
#else
#define VALUE_TRACE_ADD
constexpr bool _trace_add = true;
#endif

#ifndef ALGORITHM_TRACE_SUB
constexpr bool _trace_sub = false;
#else
#define VALUE_TRACE_SUB
constexpr bool _trace_sub = true;
#endif

#ifndef ALGORITHM_TRACE_MUL
constexpr bool _trace_mul = false;
#else
#define VALUE_TRACE_MUL
constexpr bool _trace_mul = true;
#endif

#ifndef ALGORITHM_TRACE_DIV
constexpr bool _trace_div = false;
#else
#define VALUE_TRACE_DIV
constexpr bool _trace_div = true;
#endif

#ifndef ALGORITHM_TRACE_RECIPROCAL
constexpr bool _trace_reciprocal = false;
#else
constexpr bool _trace_reciprocal = true;
#endif

#ifndef ALGORITHM_TRACE_SQRT
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

}} // namespace sw::universal
