#pragma once
// ddc_constants.hpp: definition of math constants in double-double cascade precision
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// Mathematical constants in double-double precision using dd_cascade
// These constants are extracted from the quad-double (qd) oracle constants
// computed by Scibuilders (Jack Poulson) and Stillwater Supercomputing (Theodore Omtzigt)
//
// Each constant uses the first 2 components from the qd oracle for maximum accuracy

// Pi multiples and fractions
constexpr dd_cascade ddc_pi_4     (0.785398163397448279,  3.061616997868383018e-17);  // pi/4
constexpr dd_cascade ddc_pi_3     (1.047197551196597900, -1.072081766451091000e-16);  // pi/3
constexpr dd_cascade ddc_pi_2     (1.570796326794896558,  6.123233995736766036e-17);  // pi/2
constexpr dd_cascade ddc_pi       (3.141592653589793116,  1.224646799147353207e-16);  // pi
constexpr dd_cascade ddc_3pi_4    (2.356194490192344837,  9.1848509936051484375e-17); // 3*pi/4
constexpr dd_cascade ddc_2pi      (6.283185307179586232,  2.449293598294706414e-16);  // 2*pi

// Golden ratio PHI
constexpr dd_cascade ddc_phi      (1.618033988749894900, -5.43211520368250610e-17);   // phi == golden ratio

// Euler's number e
constexpr dd_cascade ddc_e        (2.718281828459045091,  1.445646891729250158e-16);  // e

// Natural logarithm (base = e)
constexpr dd_cascade ddc_ln2      (0.693147180559945290,  2.3190468138462996e-17); // ln(2)
constexpr dd_cascade ddc_ln10     (2.302585092994045900, -2.1707562233822494e-16); // ln(10)

// Binary logarithm (base = 2)
constexpr dd_cascade ddc_lge      (1.442695040888963400,  2.0355273740931033e-17);  // log2(e)
constexpr dd_cascade ddc_lg10     (3.321928094887362200,  1.6616175169735920e-16);  // log2(10)

// Common logarithm (base = 10)
constexpr dd_cascade ddc_log2     (0.301029995663981200, -2.8037281277851704e-18); // log10(2)
constexpr dd_cascade ddc_loge     (0.434294481903251820,  1.0983196502167651e-17); // log10(e)

// Square roots
constexpr dd_cascade ddc_sqrt2    (1.414213562373095100, -9.6672933134529135e-17);  // sqrt(2)
constexpr dd_cascade ddc_sqrt3    (1.732050807568877200,  1.0035084221806903e-16);  // sqrt(3)
constexpr dd_cascade ddc_sqrt5    (2.236067977499789800, -1.0864230407365012e-16);  // sqrt(5)

// Reciprocals
constexpr dd_cascade ddc_1_phi    (0.618033988749894900, -5.4321152036825061e-17);  // 1/phi
constexpr dd_cascade ddc_1_e      (0.367879441171442330, -1.2428753672788363e-17);  // 1/e
constexpr dd_cascade ddc_1_pi     (0.318309886183790690, -1.9678676675182486e-17);  // 1/pi
constexpr dd_cascade ddc_2_pi     (0.636619772367581380, -3.9357353350364972e-17);  // 2/pi

constexpr dd_cascade ddc_1_sqrt2  (0.707106781186547570, -4.8336466567264567e-17);  // 1/sqrt(2)
constexpr dd_cascade ddc_2_sqrtpi (1.128379167095512600,  1.5335459613165881e-17);  // 2/sqrt(pi)

}} // namespace sw::universal
