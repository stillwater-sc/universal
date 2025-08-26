#pragma once
// td_constants.hpp: definition of math constants in triple-double precision
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw::universal {

// best practice for C++11
// inject mathematical constants in our namespace

// a_b reads a over b, as in 1_pi being 1 over pi

// precomputed triple-double constants courtesy Scibuilders, Jack Poulson
constexpr td td_pi_4     (7.853981633974482790e-01, 3.061616997868383018e-17); // pi/4
constexpr td td_pi_3     (1.047197551196597600e+00, 1.994890429429456000e-17); // pi/3
constexpr td td_pi_2     (1.570796326794896558e+00, 6.123233995736766036e-17); // pi/2
constexpr td td_pi       (3.141592653589793116e+00, 1.224646799147353207e-16); // pi
constexpr td td_3pi_4    (2.356194490192344837e+00, 9.1848509936051484375e-17);// 3*pi/4
constexpr td td_2pi      (6.283185307179586232e+00, 2.449293598294706414e-16); // 2*pi

constexpr td td_phi      (1.618033988749894848e+00, 0.0);  // phi == golden ratio == most irrational number: td constant TBD

constexpr td td_e        (2.718281828459045091e+00, 1.445646891729250158e-16); // e
constexpr td td_log2     (6.931471805599452862e-01, 2.319046813846299558e-17); // ln(2)
constexpr td td_log10    (2.302585092994045901e+00, -2.170756223382249351e-16);// ln(10)

constexpr td td_ln2      (0.69314718055994529e+00,  2.3190468138462996e-17); // ln(2)
constexpr td td_ln10     (2.30258509299404590e+00, -2.1707562233822494e-16); // ln(10)

constexpr td td_lge      (1.44269504088896340e+00,  2.0355273740931027e-17);
constexpr td td_lg10     (3.32192809488736220e+00,  1.6616175169735918e-16);
constexpr td td_loge     (0.43429448190325182e+00,  1.0983196502167652e-17);

constexpr td td_sqrt2    (1.41421356237309510e+00, -9.6672933134529122e-17); // sqrt(2)
constexpr td td_sqrt3    (1.73205080756887720e+00,  1.0035084221806903e-16); // sqrt(3)
constexpr td td_sqrt5    (2.23606797749979000e+00, -1.0864230407365012e-16); // sqrt(5)

constexpr td td_1_phi    (0.61803398874989479e+00, 0.0); // 1/phi: td constant TBD

constexpr td td_1_e      (0.36787944117144233e+00, -1.2428753672788364e-17); // 1/e

constexpr td td_1_pi     (0.31830988618379069e+00, -1.9678676675182486e-17); // 1/pi
constexpr td td_2_pi     (0.63661977236758138e+00, -3.9357353350364972e-17); // 2/pi == 1 / (pi/2)

constexpr td td_1_sqrt2  (0.70710678118654757e+00, -4.8336466567264561e-17); // 1 / sqrt(2)
constexpr td td_2_sqrtpi (1.12837916709551257e+00, 1.5335459652770156e-17);  // 2 / sqrt(pi) = 1 / sqrt(pi/4)


}} // namespace sw::universal
