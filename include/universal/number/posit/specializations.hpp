#pragma once
// specializations.hpp: header to include and configure any posit specializations
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// enable fast implementations of the standard posits
// POSIT_FAST_SPECIALIZATION when set will turn on all fast implementations
// Each implementation defines a macros POSIT_FAST_POSIT_`nbits`_`es,
// and includes the fast implementation if set to 1.
// For example, POSIT_FAST_POSIT_8_0, when set to 1, will enable the fast implementation of posit<8,0>.
// The individual POSIT_FAST_### macros enable fine grain control over which configurations
// use fast code.
#ifdef POSIT_FAST_SPECIALIZATION
#define POSIT_FAST_POSIT_2_0   1
#define POSIT_FAST_POSIT_3_0   1
#define POSIT_FAST_POSIT_3_1   1
#define POSIT_FAST_POSIT_4_0   1
#define POSIT_FAST_POSIT_8_0   1
#define POSIT_FAST_POSIT_8_1   1
#define POSIT_FAST_POSIT_16_1  1
#define POSIT_FAST_POSIT_32_2  1
#define POSIT_FAST_POSIT_48_2  0
#define POSIT_FAST_POSIT_64_3  0
#define POSIT_FAST_POSIT_128_4 0
#define POSIT_FAST_POSIT_256_5 0
#endif

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4242 ) // warning C4242: 'argument': conversion from 'int32_t' to 'const int8_t', possible loss of data
#pragma warning( disable : 4244 ) // warning C4244: '=': conversion from 'uint32_t' to 'uint16_t', possible loss of data
#pragma warning( disable : 4365 ) // warning C4365: 'initializing': conversion from 'long' to 'uint32_t', signed/unsigned mismatch
#endif

// fast specializations for special posit configurations
#if POSIT_FAST_POSIT_2_0
#include <universal/number/posit/specialized/posit_2_0.hpp>
#endif
#if POSIT_FAST_POSIT_3_0
#include <universal/number/posit/specialized/posit_3_0.hpp>
#endif
#if POSIT_FAST_POSIT_3_1
#include <universal/number/posit/specialized/posit_3_1.hpp>
#endif
#if POSIT_FAST_POSIT_4_0
#include <universal/number/posit/specialized/posit_4_0.hpp>
#endif
#if POSIT_FAST_POSIT_8_0
#include <universal/number/posit/specialized/posit_8_0.hpp>
#endif
#if POSIT_FAST_POSIT_8_1
#include <universal/number/posit/specialized/posit_8_1.hpp>
#endif
#if POSIT_FAST_POSIT_16_1
#include <universal/number/posit/specialized/posit_16_1.hpp>
#endif
#if POSIT_FAST_POSIT_32_2
#include <universal/number/posit/specialized/posit_32_2.hpp>
#endif
#if POSIT_FAST_POSIT_48_2
#include <universal/number/posit/specialized/posit_48_2.hpp>
#endif
#if POSIT_FAST_POSIT_64_3
#include <universal/number/posit/specialized/posit_64_3.hpp>
#endif
#if POSIT_FAST_POSIT_128_4
#include <universal/number/posit/specialized/posit_128_4.hpp>
#endif
#if POSIT_FAST_POSIT_256_5
#include <universal/number/posit/specialized/posit_256_5.hpp>
#endif

#ifdef _MSC_VER
#pragma warning( pop )
#endif
