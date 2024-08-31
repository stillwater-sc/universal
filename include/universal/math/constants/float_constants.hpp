﻿#pragma once
// float_constants.hpp: definition of math constants in double precision
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// best practice for C++11
// inject mathematical constants in our namespace

// a_b reads a over b, as in 1_pi being 1 over pi

// float values
constexpr double f_pi_4     = 0.785398163f; // pi/4
constexpr double f_pi_3     = 1.047197551f; // pi/3
constexpr double f_pi_2     = 1.570796327f; // pi/2
constexpr double f_pi       = 3.141592653f; // pi
constexpr double f_3pi_4    = 4.712388980f; // 3*pi/4
constexpr double f_2pi      = 6.283185307f; // 2*pi

constexpr double f_1_pi     = 0.318309886f; // 1/pi
constexpr double f_2_pi     = 0.636619772f; // 2/pi
constexpr double f_2_sqrtpi = 1.128379167f; // 2/sqrt(pi)
constexpr double f_sqrt2    = 1.414213562f; // sqrt(2)
constexpr double f_1_sqrt2  = 0.707106781f; // 1/sqrt(2)

constexpr double f_e        = 2.718281828f; // e
constexpr double f_log2e    = 1.442695041f; // log2(e)
constexpr double f_log10e   = 0.434294482f; // log10(e)

constexpr double f_ln2      = 0.693147181f; // ln(2)
constexpr double f_ln10     = 2.302585093f; // ln(10)

}} // namespace sw::universal
