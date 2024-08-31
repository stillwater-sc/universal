#pragma once
// ieee754_constants.hpp: definition of math constants in different ieee-754 floating-piont precisions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// best practice for C++11
// inject mathematical constants in our namespace

// a_b reads a over b, as in 1_pi being 1 over pi

	///////////////////////////////////////////////////////////////////////////////////////////////////////
/// float values
constexpr double f_pi_4     = 0.785398163f; // pi/4
constexpr double f_pi_3     = 1.047197551f; // pi/3
constexpr double f_pi_2     = 1.570796327f; // pi/2
constexpr double f_pi       = 3.141592653f; // pi
constexpr double f_3pi_4    = 4.712388980f; // 3*pi/4
constexpr double f_2pi      = 6.283185307f; // 2*pi

constexpr double f_phi      = 1.618033989f;  // phi == golden ratio == most irrational number

constexpr double f_e        = 2.718281828f; // e
constexpr double f_log2e    = 1.442695041f; // log2(e)
constexpr double f_log10e   = 0.434294482f; // log10(e)

constexpr double f_ln2      = 0.693147181f; // ln(2)
constexpr double f_ln10     = 2.302585093f; // ln(10)

constexpr double f_sqrt2    = 1.4142135624f; // sqrt(2)
constexpr double f_sqrt3    = 1.7320508076f; // sqrt(3)
constexpr double f_sqrt5    = 2.2360679775f; // sqrt(5)

constexpr double f_1_phi    = 0.6180339887f; // 1/phi

constexpr double f_1_e      = 0.3678794411f; // 1/e

constexpr double f_1_pi     = 0.318309886f; // 1/pi
constexpr double f_2_pi     = 0.636619772f; // 2/pi

constexpr double f_1_sqrt2  = 0.707106781f; // 1/sqrt(2)
constexpr double f_2_sqrtpi = 1.128379167f; // 2/sqrt(pi)


///////////////////////////////////////////////////////////////////////////////////////////////////////
/// double values
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


///////////////////////////////////////////////////////////////////////////////////////////////////////
/// long double values
constexpr long double ld_pi_4     = 0.78539816339744830961566084581988l; // pi/4
constexpr long double ld_pi_3     = 1.0471975511965977461542144610932l;  // pi/3
constexpr long double ld_pi_2     = 1.5707963267948966192313216916398l;  // pi/2
constexpr long double ld_pi       = 3.1415926535897932384626433832795l;  // pi
constexpr long double ld_3pi_4    = 4.7123889803846898576939650749193l;  // 3*pi/4
constexpr long double ld_2pi      = 6.283185307179586476925286766559l;   // 2*pi

// TODO: generate long double constants for the following constants
constexpr long double ld_phi = 1.61803398874989484820;  // phi == golden ratio == most irrational number

constexpr long double ld_e        = 2.71828182845904523536l;  // e
constexpr long double ld_log2e    = 1.44269504088896340736l;  // log2(e)
constexpr long double ld_log10e   = 0.434294481903251827651l; // log10(e)

constexpr long double ld_ln2      = 0.693147180559945309417l; // ln(2)
constexpr long double ld_ln10     = 2.30258509299404568402l;  // ln(10)

constexpr long double ld_sqrt2    = 1.41421356237309504880l;  // sqrt(2)
constexpr long double ld_sqrt3    = 1.732050807568877293527l;  // sqrt(2)
constexpr long double ld_sqrt5    = 2.236067977499789696409l;  // sqrt(2)

constexpr long double ld_1_phi    = 0.618033988749894791503l; // 1/phi

constexpr long double ld_1_e      = 0.367879441171442321596l; // 1/e

constexpr long double ld_1_pi     = 0.318309886183790671538l; // 1/pi
constexpr long double ld_2_pi     = 0.636619772367581343076l; // 2/pi

constexpr long double ld_1_sqrt2  = 0.707106781186547524401l; // 1/sqrt(2)
constexpr long double ld_2_sqrtpi = 1.12837916709551257390l;  // 2/sqrt(pi) == 1 / sqrt(pi / 4)

}} // namespace sw::universal
