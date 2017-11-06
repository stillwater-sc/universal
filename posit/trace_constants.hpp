#pragma once
// trace_constants.hpp: definition of constants that direct intermediate result reporting
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

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

# else // !POSIT_VERBOSE_OUTPUT

// posit decode and conversion
constexpr bool _trace_decode      = true;
constexpr bool _trace_conversion  = true;
constexpr bool _trace_rounding    = true;

// arithmetic operator tracing
constexpr bool _trace_add         = true;
constexpr bool _trace_sub         = true;
constexpr bool _trace_mul         = true;
constexpr bool _trace_div         = true;
constexpr bool _trace_reciprocate = true;

# endif