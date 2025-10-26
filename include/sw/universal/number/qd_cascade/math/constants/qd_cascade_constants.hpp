#pragma once
// qd_cascade_constants.hpp: definition of math constants in quad-double cascade precision
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// Mathematical constants in quad-double precision using qd_cascade
// These constants are from the quad-double (qd) oracle constants
// computed by Scibuilders (Jack Poulson) and Stillwater Supercomputing (Theodore Omtzigt)
//
// Each constant uses all 4 components for maximum 212-bit precision

// Pi multiples and fractions
constexpr qd_cascade qd_cascade_pi_4     (0.78539816339744828, 3.061616997868383e-17, -7.4869245242958492e-34, 2.7811355521584142e-50); // pi/4
constexpr qd_cascade qd_cascade_pi_2     (1.5707963267948966, 6.123233995736766e-17, -1.4973849048591698e-33, 5.5622711043168283e-50); // pi/2
constexpr qd_cascade qd_cascade_pi       (3.1415926535897931, 1.2246467991473532e-16, -2.9947698097183397e-33, 1.1124542208633657e-49); // pi
constexpr qd_cascade qd_cascade_3pi_4    (2.3561944901923448, 9.1848509936051484e-17, 3.9168984647504003e-33, -2.586798163270486e-49); // 3*pi/4
constexpr qd_cascade qd_cascade_2pi      (6.2831853071795862, 2.4492935982947064e-16, -5.9895396194366793e-33, 2.2249084417267313e-49); // 2*pi

// Golden ratio PHI
constexpr qd_cascade qd_cascade_phi      (1.6180339887498949, -5.4321152036825061e-17, 2.6543252083815655e-33, -3.3049919975020988e-50); // phi == golden ratio

// Euler's number e
constexpr qd_cascade qd_cascade_e        (2.7182818284590451, 1.4456468917292502e-16, -2.1277171080381768e-33, 1.515630159841219e-49); // e

// Natural logarithm (base = e)
constexpr qd_cascade qd_cascade_ln2      (0.69314718055994529, 2.3190468138462996e-17, 5.7077084384162121e-34, -3.5824322106018105e-50); // ln(2)
constexpr qd_cascade qd_cascade_ln10     (2.3025850929940459, -2.1707562233822494e-16, -9.9842624544657766e-33, -4.0233574544502071e-49); // ln(10)

// Binary logarithm (base = 2)
constexpr qd_cascade qd_cascade_lge      (1.4426950408889634, 2.0355273740931033e-17, -1.0614659956117258e-33, -1.3836716780181395e-50);  // log2(e)
constexpr qd_cascade qd_cascade_lg10     (3.3219280948873622, 1.6616175169735920e-16, +1.2215512178458181e-32, +5.9551189702782481e-49);  // log2(10)

// Common logarithm (base = 10)
constexpr qd_cascade qd_cascade_log2     (0.3010299956639812, -2.8037281277851704e-18, 5.4719484023146385e-35, 5.1051389831070996e-51); // log10(2)
constexpr qd_cascade qd_cascade_loge     (0.43429448190325182, 1.0983196502167651e-17, 3.7171812331109590e-34, 7.7344843465042927e-51); // log10(e)

// Square roots
constexpr qd_cascade qd_cascade_sqrt2    (1.4142135623730951, -9.6672933134529135e-17, 4.1386753086994136e-33, 4.9355469914683509e-50); // sqrt(2)
constexpr qd_cascade qd_cascade_sqrt3    (1.7320508075688772, 1.0035084221806903e-16, -1.4959542475733896e-33, 5.3061475632961675e-50); // sqrt(3)
constexpr qd_cascade qd_cascade_sqrt5    (2.2360679774997898, -1.0864230407365012e-16, 5.3086504167631309e-33, -6.6099839950042175e-50); // sqrt(5)

// Reciprocals
constexpr qd_cascade qd_cascade_1_phi    (0.6180339887498949, -5.4321152036825061e-17, 2.6543252083815655e-33, -3.3049919975021111e-50);  // 1/phi
constexpr qd_cascade qd_cascade_1_e      (0.36787944117144233, -1.2428753672788363e-17, -5.830044851072742e-34, -2.8267977849017436e-50); // 1/e
constexpr qd_cascade qd_cascade_1_pi     (0.31830988618379069, -1.9678676675182486e-17, -1.0721436282893004e-33, 8.053563926594112e-50); // 1/pi
constexpr qd_cascade qd_cascade_2_pi     (0.63661977236758138, -3.9357353350364972e-17, -2.1442872565786008e-33, 1.6107127853188224e-49); // 2/pi

constexpr qd_cascade qd_cascade_1_sqrt2  (0.70710678118654757, -4.8336466567264567e-17, +2.0693376543497068e-33, +2.4677734957341755e-50); // 1/sqrt(2)
constexpr qd_cascade qd_cascade_2_sqrtpi (1.1283791670955126, 1.5335459613165881e-17, -4.7656845966936863e-34, -2.0077946616552625e-50);  // 2/sqrt(pi)

}} // namespace sw::universal
