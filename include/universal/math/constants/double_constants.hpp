#pragma once
// double_constants.hpp: definition of math constants in double precision
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// best practice for C++11
// inject mathematical constants in our namespace

// a_b reads a over b, as in 1_pi being 1 over pi

// double values
constexpr double d_pi_4     = 0.785398163397448309616; // pi/4
constexpr double d_pi_3     = 1.04719755119659774615;  // pi/3
constexpr double d_pi_2     = 1.57079632679489661923;  // pi/2
constexpr double d_pi       = 3.14159265358979323846;  // pi
constexpr double d_3pi_4    = 4.71238898038468985769;  // 3*pi/4
constexpr double d_2pi      = 6.28318530717958647693;  // 2*pi

constexpr double d_phi      = 1.61803398874989484820;  // phi == golden ratio == most irrational number

constexpr double d_e        = 2.71828182845904523536;  // e
constexpr double d_log2e    = 1.44269504088896340736;  // log2(e)
constexpr double d_log10e   = 0.434294481903251827651; // log10(e)

constexpr double d_ln2      = 0.693147180559945309417; // ln(2)
constexpr double d_ln10     = 2.30258509299404568402;  // ln(10)

constexpr double d_sqrt2    = 1.414213562373095048801; // sqrt(2)
constexpr double d_sqrt3    = 1.732050807568877293527; // sqrt(3)
constexpr double d_sqrt5    = 2.236067977499789696409; // sqrt(5)

constexpr double d_1_phi    = 0.618033988749894791503; // 1/phi

constexpr double d_1_e      = 0.367879441171442321596; // 1/e

constexpr double d_1_pi     = 0.318309886183790671538; // 1/pi
constexpr double d_2_pi     = 0.636619772367581343076; // 2/pi

constexpr double d_1_sqrt2  = 0.707106781186547524401; // 1/sqrt(2)
constexpr double d_2_sqrtpi = 1.12837916709551257390;  // 2/sqrt(pi) == 1 / sqrt(pi / 4)

}} // namespace sw::universal
