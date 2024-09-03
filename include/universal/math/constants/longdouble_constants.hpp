#pragma once
// longdouble_constants.hpp: definition of math constants in long double precision
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// best practice for C++11
// inject mathematical constants in our namespace

// a_b reads a over b, as in 1_pi being 1 over pi

// long double values
constexpr long double ld_pi_4     = 0.78539816339744830961566084581988l; // pi/4
constexpr long double ld_pi_3     = 1.0471975511965977461542144610932l;  // pi/3
constexpr long double ld_pi_2     = 1.5707963267948966192313216916398l;  // pi/2
constexpr long double ld_pi       = 3.1415926535897932384626433832795l;  // pi
constexpr long double ld_3pi_4    = 4.7123889803846898576939650749193l;  // 3*pi/4
constexpr long double ld_2pi      = 6.283185307179586476925286766559l;   // 2*pi

// TODO: generate long double constants for the following constants
constexpr long double ld_1_pi     = 0.318309886183790671538l; // 1/pi
constexpr long double ld_2_pi     = 0.636619772367581343076l; // 2/pi
constexpr long double ld_2_sqrtpi = 1.12837916709551257390l;  // 2/sqrt(pi)
constexpr long double ld_sqrt2    = 1.41421356237309504880l;  // sqrt(2)
constexpr long double ld_1_sqrt2  = 0.707106781186547524401l; // 1/sqrt(2)

constexpr long double ld_e        = 2.71828182845904523536l;  // e
constexpr long double ld_log2e    = 1.44269504088896340736l;  // log2(e)
constexpr long double ld_log10e   = 0.434294481903251827651l; // log10(e)

constexpr long double ld_ln2      = 0.693147180559945309417l; // ln(2)
constexpr long double ld_ln10     = 2.30258509299404568402l;  // ln(10)

}} // namespace sw::universal
