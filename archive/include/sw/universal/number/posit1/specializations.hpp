#pragma once
// specializations.hpp: header to include and configure any posit specializations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
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
#define POSIT_FAST_POSIT_8_2   1
#define POSIT_FAST_POSIT_16_1  1
#define POSIT_FAST_POSIT_16_2  1
#define POSIT_FAST_POSIT_32_2  1
#define POSIT_FAST_POSIT_48_2  0
#define POSIT_FAST_POSIT_64_2  0
#define POSIT_FAST_POSIT_64_3  0
#define POSIT_FAST_POSIT_128_2 0
#define POSIT_FAST_POSIT_128_4 0
#define POSIT_FAST_POSIT_256_2 0
#define POSIT_FAST_POSIT_256_5 0
#endif

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4242 ) // warning C4242: 'argument': conversion from 'int32_t' to 'const int8_t', possible loss of data
#pragma warning( disable : 4244 ) // warning C4244: '=': conversion from 'uint32_t' to 'uint16_t', possible loss of data
#pragma warning( disable : 4365 ) // warning C4365: 'initializing': conversion from 'long' to 'uint32_t', signed/unsigned mismatch
#endif

// fast specializations for special posit configurations
#ifndef POSIT_FAST_POSIT_2_0
#define POSIT_FAST_POSIT_2_0 0
#endif
#if POSIT_FAST_POSIT_2_0
#include <universal/number/posit1/specialized/posit_2_0.hpp>
#endif
#ifndef POSIT_FAST_POSIT_3_0
#define POSIT_FAST_POSIT_3_0 0
#endif
#if POSIT_FAST_POSIT_3_0
#include <universal/number/posit1/specialized/posit_3_0.hpp>
#endif
#ifndef POSIT_FAST_POSIT_3_1
#define POSIT_FAST_POSIT_3_1 0
#endif
#if POSIT_FAST_POSIT_3_1
#include <universal/number/posit1/specialized/posit_3_1.hpp>
#endif
#ifndef POSIT_FAST_POSIT_4_0
#define POSIT_FAST_POSIT_4_0 0
#endif
#if POSIT_FAST_POSIT_4_0
#include <universal/number/posit1/specialized/posit_4_0.hpp>
#endif
#ifndef POSIT_FAST_POSIT_8_0
#define POSIT_FAST_POSIT_8_0 0
#endif
#if POSIT_FAST_POSIT_8_0
#include <universal/number/posit1/specialized/posit_8_0.hpp>
#endif
#ifndef POSIT_FAST_POSIT_8_1
#define POSIT_FAST_POSIT_8_1 0
#endif
#if POSIT_FAST_POSIT_8_1
#include <universal/number/posit1/specialized/posit_8_1.hpp>
#endif
#ifndef POSIT_FAST_POSIT_8_2
#define POSIT_FAST_POSIT_8_2 0
#endif
#if POSIT_FAST_POSIT_8_2
#include <universal/number/posit1/specialized/posit_8_2.hpp>
#endif
#ifndef POSIT_FAST_POSIT_16_1
#define POSIT_FAST_POSIT_16_1 0
#endif
#if POSIT_FAST_POSIT_16_1
#include <universal/number/posit1/specialized/posit_16_1.hpp>
#endif
#ifndef POSIT_FAST_POSIT_16_2
#define POSIT_FAST_POSIT_16_2 0
#endif
#if POSIT_FAST_POSIT_16_2
#include <universal/number/posit1/specialized/posit_16_2.hpp>
#endif
#ifndef POSIT_FAST_POSIT_32_2
#define POSIT_FAST_POSIT_32_2 0
#endif
#if POSIT_FAST_POSIT_32_2
#include <universal/number/posit1/specialized/posit_32_2.hpp>
#endif
#ifndef POSIT_FAST_POSIT_48_2
#define POSIT_FAST_POSIT_48_2 0
#endif
#if POSIT_FAST_POSIT_48_2
#include <universal/number/posit1/specialized/posit_48_2.hpp>
#endif
#ifndef POSIT_FAST_POSIT_64_2
#define POSIT_FAST_POSIT_64_2 0
#endif
#if POSIT_FAST_POSIT_64_2
#include <universal/number/posit1/specialized/posit_64_2.hpp>
#endif
#ifndef POSIT_FAST_POSIT_64_3
#define POSIT_FAST_POSIT_64_3 0
#endif
#if POSIT_FAST_POSIT_64_3
#include <universal/number/posit1/specialized/posit_64_3.hpp>
#endif
#ifndef POSIT_FAST_POSIT_128_2
#define POSIT_FAST_POSIT_128_2 0
#endif
#if POSIT_FAST_POSIT_128_2
#include <universal/number/posit1/specialized/posit_128_2.hpp>
#endif
#ifndef POSIT_FAST_POSIT_128_4
#define POSIT_FAST_POSIT_128_4 0
#endif
#if POSIT_FAST_POSIT_128_4
#include <universal/number/posit1/specialized/posit_128_4.hpp>
#endif
#ifndef POSIT_FAST_POSIT_256_2
#define POSIT_FAST_POSIT_256_2 0
#endif
#if POSIT_FAST_POSIT_256_2
#include <universal/number/posit1/specialized/posit_256_2.hpp>
#endif
#ifndef POSIT_FAST_POSIT_256_5
#define POSIT_FAST_POSIT_256_5 0
#endif
#if POSIT_FAST_POSIT_256_5
#include <universal/number/posit1/specialized/posit_256_5.hpp>
#endif

#ifdef _MSC_VER
#pragma warning( pop )
#endif
