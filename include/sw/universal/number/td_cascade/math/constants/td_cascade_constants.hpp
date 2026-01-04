#pragma once
// tdc_constants.hpp: definition of math constants in triple-double cascade precision
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// Mathematical constants in triple-double precision using td_cascade
// These constants are extracted from the quad-double (qd) oracle constants
// computed by Scibuilders (Jack Poulson) and Stillwater Supercomputing (Theodore Omtzigt)
//
// Each constant uses the first 3 components from the qd oracle for maximum accuracy

// Pi multiples and fractions
constexpr td_cascade tdc_pi_4     (0.78539816339744828,  3.0616169978683830e-17, -7.4869245242958492e-34); // pi/4
constexpr td_cascade tdc_pi_3     (1.04719755119659760,  1.9948904294294560e-17, 0);                             // pi/3
constexpr td_cascade tdc_pi_2     (1.57079632679489660,  6.1232339957367660e-17, -1.4973849048591698e-33); // pi/2
constexpr td_cascade tdc_pi       (3.14159265358979310,  1.2246467991473532e-16, -2.9947698097183397e-33); // pi
constexpr td_cascade tdc_3pi_4    (2.35619449019234480,  9.1848509936051484e-17,  3.9168984647504003e-33); // 3*pi/4
constexpr td_cascade tdc_2pi      (6.28318530717958620,  2.4492935982947064e-16, -5.9895396194366793e-33); // 2*pi

// Golden ratio PHI
constexpr td_cascade tdc_phi      (1.61803398874989490, -5.4321152036825061e-17,  2.6543252083815655e-33); // phi == golden ratio

// Euler's number e
constexpr td_cascade tdc_e        (2.71828182845904510,  1.4456468917292502e-16, -2.1277171080381768e-33); // e

// Natural logarithm (base = e)
constexpr td_cascade tdc_ln2      (0.69314718055994529,  2.3190468138462996e-17,  5.7077084384162121e-34); // ln(2)
constexpr td_cascade tdc_ln10     (2.30258509299404590, -2.1707562233822494e-16, -9.9842624544657766e-33); // ln(10)

// Binary logarithm (base = 2)
constexpr td_cascade tdc_lge      (1.44269504088896340,  2.0355273740931033e-17, -1.0614659956117258e-33); // log2(e)
constexpr td_cascade tdc_lg10     (3.32192809488736220,  1.6616175169735920e-16,  1.2215512178458181e-32); // log2(10)

// Common logarithm (base = 10)
constexpr td_cascade tdc_log2     (0.30102999566398120, -2.8037281277851704e-18,  5.4719484023146385e-35); // log10(2)
constexpr td_cascade tdc_loge     (0.43429448190325182,  1.0983196502167651e-17,  3.7171812331109590e-34); // log10(e)

// Square roots
constexpr td_cascade tdc_sqrt2    (1.41421356237309510, -9.6672933134529135e-17,  4.1386753086994136e-33); // sqrt(2)
constexpr td_cascade tdc_sqrt3    (1.73205080756887720,  1.0035084221806903e-16, -1.4959542475733896e-33); // sqrt(3)
constexpr td_cascade tdc_sqrt5    (2.23606797749978980, -1.0864230407365012e-16,  5.3086504167631309e-33); // sqrt(5)

// Reciprocals
constexpr td_cascade tdc_1_phi    (0.61803398874989490, -5.4321152036825061e-17,  2.6543252083815655e-33); // 1/phi
constexpr td_cascade tdc_1_e      (0.36787944117144233, -1.2428753672788363e-17, -5.8300448510727420e-34); // 1/e
constexpr td_cascade tdc_1_pi     (0.31830988618379069, -1.9678676675182486e-17, -1.0721436282893004e-33); // 1/pi
constexpr td_cascade tdc_2_pi     (0.63661977236758138, -3.9357353350364972e-17, -2.1442872565786008e-33); // 2/pi

constexpr td_cascade tdc_1_sqrt2  (0.70710678118654757, -4.8336466567264567e-17, +2.0693376543497068e-33); // 1/sqrt(2)
constexpr td_cascade tdc_2_sqrtpi (1.12837916709551260,  1.5335459613165881e-17, -4.7656845966936863e-34); // 2/sqrt(pi)

}} // namespace sw::universal
