#pragma once
// ereal_constants.hpp: definition of math constants for ereal adaptive precision
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// Mathematical constants for ereal adaptive-precision arithmetic
//
// NOTE: Phase 0 - These are placeholder double-precision constants
// TODO: Phase 1 - Implement high-precision constants using expansion arithmetic
//
// Each constant is a template function that returns an ereal<maxlimbs> initialized
// from double precision. Future versions will generate multi-component expansions
// for arbitrary precision.

// Pi multiples and fractions
template<unsigned maxlimbs = 1024>
inline ereal<maxlimbs> ereal_pi_4() {
	return ereal<maxlimbs>(0.78539816339744828);  // pi/4
}

template<unsigned maxlimbs = 1024>
inline ereal<maxlimbs> ereal_pi_3() {
	return ereal<maxlimbs>(1.04719755119659760);  // pi/3
}

template<unsigned maxlimbs = 1024>
inline ereal<maxlimbs> ereal_pi_2() {
	return ereal<maxlimbs>(1.57079632679489660);  // pi/2
}

template<unsigned maxlimbs = 1024>
inline ereal<maxlimbs> ereal_pi() {
	return ereal<maxlimbs>(3.14159265358979310);  // pi
}

template<unsigned maxlimbs = 1024>
inline ereal<maxlimbs> ereal_3pi_4() {
	return ereal<maxlimbs>(2.35619449019234480);  // 3*pi/4
}

template<unsigned maxlimbs = 1024>
inline ereal<maxlimbs> ereal_2pi() {
	return ereal<maxlimbs>(6.28318530717958620);  // 2*pi
}

// Golden ratio PHI
template<unsigned maxlimbs = 1024>
inline ereal<maxlimbs> ereal_phi() {
	return ereal<maxlimbs>(1.6180339887498949);  // phi == golden ratio
}

// Euler's number e
template<unsigned maxlimbs = 1024>
inline ereal<maxlimbs> ereal_e() {
	return ereal<maxlimbs>(2.7182818284590451);  // e
}

// Natural logarithm (base = e)
template<unsigned maxlimbs = 1024>
inline ereal<maxlimbs> ereal_ln2() {
	return ereal<maxlimbs>(0.69314718055994529);  // ln(2)
}

template<unsigned maxlimbs = 1024>
inline ereal<maxlimbs> ereal_ln10() {
	return ereal<maxlimbs>(2.3025850929940459);  // ln(10)
}

// Binary logarithm (base = 2)
template<unsigned maxlimbs = 1024>
inline ereal<maxlimbs> ereal_lge() {
	return ereal<maxlimbs>(1.4426950408889634);  // log2(e)
}

template<unsigned maxlimbs = 1024>
inline ereal<maxlimbs> ereal_lg10() {
	return ereal<maxlimbs>(3.3219280948873622);  // log2(10)
}

// Common logarithm (base = 10)
template<unsigned maxlimbs = 1024>
inline ereal<maxlimbs> ereal_log2() {
	return ereal<maxlimbs>(0.3010299956639812);  // log10(2)
}

template<unsigned maxlimbs = 1024>
inline ereal<maxlimbs> ereal_loge() {
	return ereal<maxlimbs>(0.43429448190325182);  // log10(e)
}

// Square roots
template<unsigned maxlimbs = 1024>
inline ereal<maxlimbs> ereal_sqrt2() {
	return ereal<maxlimbs>(1.4142135623730951);  // sqrt(2)
}

template<unsigned maxlimbs = 1024>
inline ereal<maxlimbs> ereal_sqrt3() {
	return ereal<maxlimbs>(1.7320508075688772);  // sqrt(3)
}

template<unsigned maxlimbs = 1024>
inline ereal<maxlimbs> ereal_sqrt5() {
	return ereal<maxlimbs>(2.2360679774997898);  // sqrt(5)
}

// Reciprocals
template<unsigned maxlimbs = 1024>
inline ereal<maxlimbs> ereal_1_phi() {
	return ereal<maxlimbs>(0.6180339887498949);  // 1/phi
}

template<unsigned maxlimbs = 1024>
inline ereal<maxlimbs> ereal_1_e() {
	return ereal<maxlimbs>(0.36787944117144233);  // 1/e
}

template<unsigned maxlimbs = 1024>
inline ereal<maxlimbs> ereal_1_pi() {
	return ereal<maxlimbs>(0.31830988618379069);  // 1/pi
}

template<unsigned maxlimbs = 1024>
inline ereal<maxlimbs> ereal_2_pi() {
	return ereal<maxlimbs>(0.63661977236758138);  // 2/pi
}

template<unsigned maxlimbs = 1024>
inline ereal<maxlimbs> ereal_1_sqrt2() {
	return ereal<maxlimbs>(0.70710678118654757);  // 1/sqrt(2)
}

template<unsigned maxlimbs = 1024>
inline ereal<maxlimbs> ereal_2_sqrtpi() {
	return ereal<maxlimbs>(1.1283791670955126);  // 2/sqrt(pi)
}

}} // namespace sw::universal
