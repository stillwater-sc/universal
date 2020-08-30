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
#define POSIT_FAST_POSIT_64_3  0
#define POSIT_FAST_POSIT_128_4 0
#define POSIT_FAST_POSIT_256_5 0
#endif

// fast specializations for special posit configurations
#include "specialized/posit_2_0.hpp"
#include "specialized/posit_3_0.hpp"
#include "specialized/posit_3_1.hpp"
#include "specialized/posit_4_0.hpp"
#include "specialized/posit_8_0.hpp"
#include "specialized/posit_8_1.hpp"
#include "specialized/posit_16_1.hpp"
#include "specialized/posit_32_2.hpp"
#include "specialized/posit_64_3.hpp"
#include "specialized/posit_128_4.hpp"
#include "specialized/posit_256_5.hpp"
