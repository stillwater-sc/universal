#pragma once
// math_constants.hpp: definition of math constants
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// default is double constants
#undef PI_IN_LONG_DOUBLES

namespace sw { namespace universal {

// best practice for C++11
// inject mathematical constants in our namespace

#ifndef PI_IN_LONG_DOUBLES
// double values
constexpr double m_pi_4     = 0.785398163397448309616; // pi/4
constexpr double m_pi_3     = 1.04719755119659774615;  // pi/3
constexpr double m_pi_2     = 1.57079632679489661923;  // pi/2
constexpr double m_pi       = 3.14159265358979323846;  // pi
constexpr double m_3pi_4    = 4.71238898038468985769;  // 3*pi/4
constexpr double m_2pi      = 6.28318530717958647693;  // 2*pi
#else
// long double values
constexpr long double m_pi_4     = 0.78539816339744830961566084581988l; // pi/4
constexpr long double m_pi_3     = 1.0471975511965977461542144610932l;  // pi/3
constexpr long double m_pi_2     = 1.5707963267948966192313216916398l;  // pi/2
constexpr long double m_pi       = 3.1415926535897932384626433832795l;  // pi
constexpr long double m_3pi_4    = 4.7123889803846898576939650749193l;  // 3*pi/4
constexpr long double m_2pi      = 6.283185307179586476925286766559l;   // 2*pi
#endif

constexpr double m_1_pi     = 0.318309886183790671538; // 1/pi
constexpr double m_2_pi     = 0.636619772367581343076; // 2/pi
constexpr double m_2_sqrtpi = 1.12837916709551257390;  // 2/sqrt(pi)
constexpr double m_sqrt2    = 1.41421356237309504880;  // sqrt(2)
constexpr double m_sqrt1_2  = 0.707106781186547524401; // 1/sqrt(2)
constexpr double m_e        = 2.71828182845904523536;  // e
constexpr double m_log2e    = 1.44269504088896340736;  // log2(e)
constexpr double m_log10e   = 0.434294481903251827651; // log10(e)
constexpr double m_ln2      = 0.693147180559945309417; // ln(2)
constexpr double m_ln10     = 2.30258509299404568402;  // ln(10)

}} // namespace sw::universal
