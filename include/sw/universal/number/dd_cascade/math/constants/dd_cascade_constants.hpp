#pragma once
// dd_cascade_constants.hpp: definition of math constants in double-double cascade precision
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
constexpr dd_cascade dd_cascade_pi_4     (0.78539816339744828, 3.061616997868383e-17); // pi/4
constexpr dd_cascade dd_cascade_pi_2     (1.5707963267948966, 6.123233995736766e-17); // pi/2
constexpr dd_cascade dd_cascade_pi       (3.1415926535897931, 1.2246467991473532e-16); // pi
constexpr dd_cascade dd_cascade_3pi_4    (2.3561944901923448, 9.1848509936051484e-17); // 3*pi/4
constexpr dd_cascade dd_cascade_2pi      (6.2831853071795862, 2.4492935982947064e-16); // 2*pi

// Golden ratio PHI
constexpr dd_cascade dd_cascade_phi      (1.6180339887498949, -5.4321152036825061e-17); // phi == golden ratio

// Euler's number e
constexpr dd_cascade dd_cascade_e        (2.7182818284590451, 1.4456468917292502e-16); // e

// Natural logarithm (base = e)
constexpr dd_cascade dd_cascade_ln2      (0.69314718055994529, 2.3190468138462996e-17); // ln(2)
constexpr dd_cascade dd_cascade_ln10     (2.3025850929940459, -2.1707562233822494e-16); // ln(10)

// Binary logarithm (base = 2)
constexpr dd_cascade dd_cascade_lge      (1.4426950408889634, 2.0355273740931033e-17);  // log2(e)
constexpr dd_cascade dd_cascade_lg10     (3.3219280948873622, 1.6616175169735920e-16);  // log2(10)

// Common logarithm (base = 10)
constexpr dd_cascade dd_cascade_log2     (0.3010299956639812, -2.8037281277851704e-18); // log10(2)
constexpr dd_cascade dd_cascade_loge     (0.43429448190325182, 1.0983196502167651e-17); // log10(e)

// Square roots
constexpr dd_cascade dd_cascade_sqrt2    (1.4142135623730951, -9.6672933134529135e-17); // sqrt(2)
constexpr dd_cascade dd_cascade_sqrt3    (1.7320508075688772, 1.0035084221806903e-16); // sqrt(3)
constexpr dd_cascade dd_cascade_sqrt5    (2.2360679774997898, -1.0864230407365012e-16); // sqrt(5)

// Reciprocals
constexpr dd_cascade dd_cascade_1_phi    (0.6180339887498949, -5.4321152036825061e-17);  // 1/phi
constexpr dd_cascade dd_cascade_1_e      (0.36787944117144233, -1.2428753672788363e-17); // 1/e
constexpr dd_cascade dd_cascade_1_pi     (0.31830988618379069, -1.9678676675182486e-17); // 1/pi
constexpr dd_cascade dd_cascade_2_pi     (0.63661977236758138, -3.9357353350364972e-17); // 2/pi

constexpr dd_cascade dd_cascade_1_sqrt2  (0.70710678118654757, -4.8336466567264567e-17); // 1/sqrt(2)
constexpr dd_cascade dd_cascade_2_sqrtpi (1.1283791670955126, 1.5335459613165881e-17);   // 2/sqrt(pi)

}} // namespace sw::universal
