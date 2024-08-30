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

// long double values
constexpr long double ld_pi_4     = 0.78539816339744830961566084581988l; // pi/4
constexpr long double ld_pi_3     = 1.0471975511965977461542144610932l;  // pi/3
constexpr long double ld_pi_2     = 1.5707963267948966192313216916398l;  // pi/2
constexpr long double ld_pi       = 3.1415926535897932384626433832795l;  // pi
constexpr long double ld_3pi_4    = 4.7123889803846898576939650749193l;  // 3*pi/4
constexpr long double ld_2pi      = 6.283185307179586476925286766559l;   // 2*pi

}} // namespace sw::universal
