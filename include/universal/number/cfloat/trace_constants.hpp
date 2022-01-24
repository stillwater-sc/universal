#pragma once
// trace_constants.hpp: definition of constants that direct intermediate result reporting
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

# ifndef CFLOAT_VERBOSE_OUTPUT
// CFLOAT decode and conversion
constexpr bool cfloat_trace_decode      = false;
constexpr bool cfloat_trace_conversion  = false;
constexpr bool cfloat_trace_rounding    = false;

// arithmetic operator tracing
constexpr bool cfloat_trace_add         = false;
constexpr bool cfloat_trace_sub         = false;
constexpr bool cfloat_trace_mul         = false;
constexpr bool cfloat_trace_div         = false;
constexpr bool cfloat_trace_reciprocate = false;
constexpr bool cfloat_trace_sqrt        = false;

// quire update tracing
constexpr bool cfloat_trace_quire_add   = false;

# else // !CFLOAT_VERBOSE_OUTPUT

#ifdef CFLOAT_TRACE_ALL
#define CFLOAT_TRACE_DECODE
#define CFLOAT_TRACE_CONVERSION
#define CFLOAT_TRACE_ROUNDING
#define CFLOAT_TRACE_ADD
#define CFLOAT_TRACE_SUB
#define CFLOAT_TRACE_MUL
#define CFLOAT_TRACE_DIV
#define CFLOAT_TRACE_RECIPROCATE
#define CFLOAT_TRACE_SQRT

// trace into the constituent components?
#define BLOCKTRIPLE_VERBOSE_OUTPUT
#define BLOCKTRIPLE_TRACE_CONVERSION
#define BLOCKTRIPLE_TRACE_ADD
#define BLOCKTRIPLE_TRACE_SUB
#define BLOCKTRIPLE_TRACE_MUL
#define BLOCKTRIPLE_TRACE_DIV
#endif

#ifdef CFLOAT_QUIRE_TRACE_ALL
#define CFLOAT_QUIRE_TRACE_ADD
#endif

// CFLOAT decode and conversion

#ifndef CFLOAT_TRACE_DECODE
constexpr bool cfloat_trace_decode = false;
#else
constexpr bool cfloat_trace_decode = true;
#endif

#ifndef CFLOAT_TRACE_CONVERSION
constexpr bool cfloat_trace_conversion = false;
#else
#define VALUE_TRACE_CONVERSION
constexpr bool cfloat_trace_conversion = true;
#endif

#ifndef CFLOAT_TRACE_ROUNDING
constexpr bool cfloat_trace_rounding = false;
#else
constexpr bool cfloat_trace_rounding = true;
#endif

// arithmetic operator tracing
#ifndef CFLOAT_TRACE_ADD
constexpr bool cfloat_trace_add = false;
#else
#define VALUE_TRACE_ADD
constexpr bool cfloat_trace_add = true;
#endif

#ifndef CFLOAT_TRACE_SUB
constexpr bool cfloat_trace_sub = false;
#else
#define VALUE_TRACE_SUB
constexpr bool cfloat_trace_sub = true;
#endif

#ifndef CFLOAT_TRACE_MUL
constexpr bool cfloat_trace_mul = false;
#else
#define VALUE_TRACE_MUL
constexpr bool cfloat_trace_mul = true;
#endif

#ifndef CFLOAT_TRACE_DIV
constexpr bool cfloat_trace_div = false;
#else
#define VALUE_TRACE_DIV
constexpr bool cfloat_trace_div = true;
#endif

#ifndef CFLOAT_TRACE_RECIPROCATE
constexpr bool cfloat_trace_reciprocate = false;
#else
constexpr bool cfloat_trace_reciprocate = true;
#endif

#ifndef CFLOAT_TRACE_SQRT
constexpr bool cfloat_trace_sqrt = false;
#else
constexpr bool cfloat_trace_sqrt = true;
#endif

// quire tracing
#ifndef CFLOAT_QUIRE_TRACE_ADD
constexpr bool cfloat_trace_quire_add = false;
#else
constexpr bool cfloat_trace_quire_add = true;
#endif

# endif

}} // namespace sw::universal
