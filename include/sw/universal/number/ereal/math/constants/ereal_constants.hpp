#pragma once
// ereal_constants.hpp: high-precision math constants for ereal adaptive precision
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <string>
#include <cstddef>

namespace sw { namespace universal {

// Mathematical constants for ereal adaptive-precision arithmetic.
//
// These are genuine high-precision constants, NOT double-precision placeholders:
// a `double` literal can carry only ~16 digits, so initializing an ereal from
// one (e.g. ereal<m>(3.14159...long...)) silently truncates to double precision.
// That capped every transcendental at ~16 digits (issue #1002).
//
//   - pi, ln2, ln10 are stored as precomputed non-overlapping double-component
//     expansions (the dd/qd approach), generated offline with MPFR and verified
//     value-accurate to ~313 decimal digits. They are reconstructed by summing
//     the components into an ereal<maxlimbs> (exact: the components are a valid
//     Priest expansion, so the running sum loses no precision). This bypasses
//     parse() entirely -- parse() degrades past ~130 digits and cannot represent
//     a 300-digit constant (it mangled the 230-digit ln10 string down to ~86
//     correct digits), which left ln10/pi/ln2-dependent transcendentals capped
//     well below full precision (issue #1002, deeper root cause).
//   - the remaining constants are DERIVED from those via exact operations
//     (multiply/divide by powers of two, small integers) or extended-precision
//     algorithms (the ereal sqrt/exp/divide are themselves extended-precision),
//     so their correctness follows from the algorithm rather than from
//     hand-transcribed tail digits.
//
// Each accessor caches its value in a function-local static (computed once per
// ereal<maxlimbs> instantiation).

namespace ereal_detail {
	// parse a decimal string into an expansion. Retained for short ad-hoc
	// constants (e.g. trigonometry's atan(1/2) seed); NOT used for the base
	// constants below, which are stored as limb expansions to bypass parse's
	// long-string precision loss.
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> parse_constant(const char* digits) {
		ereal<maxlimbs> v;
		v.parse(std::string(digits));
		return v;
	}

	// Reconstruct an ereal from a stored non-overlapping double expansion.
	// Summing a valid Priest expansion (decreasing magnitude, non-overlapping)
	// via repeated two_sum growth is exact, so the result carries the full
	// precision of the stored components, truncated only by ereal<maxlimbs>.
	template<unsigned maxlimbs, std::size_t N>
	inline ereal<maxlimbs> from_limbs(const double (&components)[N]) {
		ereal<maxlimbs> v(0.0);
		for (std::size_t i = 0; i < N; ++i) v += components[i];
		return v;
	}

	// pi   (Archimedes' constant)
	// 19 limbs, value-accurate to ~315 decimal digits (verified vs MPFR)
	static constexpr double ereal_pi_limbs[] = {
		3.141592653589793, 1.2246467991473532e-16, -2.9947698097183397e-33,
		1.1124542208633653e-49, 5.672231979640316e-66, 1.7449862161352486e-83,
		6.02937273224954e-100, 1.91012354687999e-116, 3.0439781653442933e-133,
		-4.714300030947029e-150, 1.0015491694355389e-166, 6.210404478415895e-183,
		-1.7168132391611603e-199, -5.495774958969099e-216, -1.7490089948024087e-232,
		-3.5915099785785793e-249, 1.768540572980925e-265, 9.569391635737116e-282,
		5.839748741415892e-298
	};

	// ln(2) (natural log of 2)
	// 19 limbs, value-accurate to ~313 decimal digits (verified vs MPFR)
	static constexpr double ereal_ln2_limbs[] = {
		0.6931471805599453, 2.3190468138462996e-17, 5.707708438416212e-34,
		-3.5824322106018114e-50, -1.352169675798863e-66, 6.080638740240814e-83,
		2.8955024332347147e-99, 2.351386712145641e-116, 4.459774417014281e-133,
		-3.069933263232527e-149, -2.0151474461966832e-165, 1.618534348863741e-182,
		-1.3094978047454462e-198, 6.665188278589824e-215, -2.93171211597727e-231,
		7.859799559040711e-248, 5.978862565926012e-264, -3.5958643436716937e-280,
		-1.4081782025014926e-297
	};

	// ln(10) (natural log of 10)
	// 19 limbs, value-accurate to ~313 decimal digits (verified vs MPFR)
	static constexpr double ereal_ln10_limbs[] = {
		2.302585092994046, -2.1707562233822494e-16, -9.984262454465777e-33,
		-4.023357454450206e-49, 1.928899528969337e-65, -5.212570118151255e-82,
		-2.6037369898693294e-98, 8.297417620821901e-115, -4.1060098162989226e-131,
		-5.261488051675406e-148, -3.1868290930695915e-164, -1.1910622301553096e-180,
		9.106734402820891e-197, 3.226984866273686e-213, -1.757386416107899e-229,
		2.8790985845649945e-246, 5.984794382489271e-263, -2.897106789574831e-280,
		-1.0839661654967099e-296
	};
}

// ---- base constants, reconstructed from stored limb expansions ----

template<unsigned maxlimbs = 19>
inline ereal<maxlimbs> ereal_pi() {
	static const ereal<maxlimbs> v = ereal_detail::from_limbs<maxlimbs>(ereal_detail::ereal_pi_limbs);
	return v;
}

template<unsigned maxlimbs = 19>
inline ereal<maxlimbs> ereal_ln2() {
	static const ereal<maxlimbs> v = ereal_detail::from_limbs<maxlimbs>(ereal_detail::ereal_ln2_limbs);
	return v;
}

template<unsigned maxlimbs = 19>
inline ereal<maxlimbs> ereal_ln10() {
	static const ereal<maxlimbs> v = ereal_detail::from_limbs<maxlimbs>(ereal_detail::ereal_ln10_limbs);
	return v;
}

// ---- pi multiples / fractions: exact scaling by powers of two and small ints ----

template<unsigned maxlimbs = 19>
inline ereal<maxlimbs> ereal_pi_2() {  // pi/2  (exact: pi * 2^-1)
	static const ereal<maxlimbs> v = ereal_pi<maxlimbs>() * ereal<maxlimbs>(0.5);
	return v;
}

template<unsigned maxlimbs = 19>
inline ereal<maxlimbs> ereal_pi_4() {  // pi/4  (exact)
	static const ereal<maxlimbs> v = ereal_pi<maxlimbs>() * ereal<maxlimbs>(0.25);
	return v;
}

template<unsigned maxlimbs = 19>
inline ereal<maxlimbs> ereal_3pi_4() {  // 3*pi/4  (exact: 3/4 is exact in binary)
	static const ereal<maxlimbs> v = ereal_pi<maxlimbs>() * ereal<maxlimbs>(0.75);
	return v;
}

template<unsigned maxlimbs = 19>
inline ereal<maxlimbs> ereal_2pi() {  // 2*pi  (exact)
	static const ereal<maxlimbs> v = ereal_pi<maxlimbs>() * ereal<maxlimbs>(2.0);
	return v;
}

template<unsigned maxlimbs = 19>
inline ereal<maxlimbs> ereal_pi_3() {  // pi/3  (extended-precision divide)
	static const ereal<maxlimbs> v = ereal_pi<maxlimbs>() / ereal<maxlimbs>(3.0);
	return v;
}

// ---- square roots: extended-precision ereal Newton sqrt ----

template<unsigned maxlimbs = 19>
inline ereal<maxlimbs> ereal_sqrt2() {
	static const ereal<maxlimbs> v = sqrt(ereal<maxlimbs>(2.0));
	return v;
}

template<unsigned maxlimbs = 19>
inline ereal<maxlimbs> ereal_sqrt3() {
	static const ereal<maxlimbs> v = sqrt(ereal<maxlimbs>(3.0));
	return v;
}

template<unsigned maxlimbs = 19>
inline ereal<maxlimbs> ereal_sqrt5() {
	static const ereal<maxlimbs> v = sqrt(ereal<maxlimbs>(5.0));
	return v;
}

// ---- e and golden ratio: derived ----

template<unsigned maxlimbs = 19>
inline ereal<maxlimbs> ereal_e() {  // e = exp(1)  (extended-precision Taylor)
	static const ereal<maxlimbs> v = exp(ereal<maxlimbs>(1.0));
	return v;
}

template<unsigned maxlimbs = 19>
inline ereal<maxlimbs> ereal_phi() {  // phi = (1 + sqrt(5)) / 2
	static const ereal<maxlimbs> v = (ereal<maxlimbs>(1.0) + ereal_sqrt5<maxlimbs>()) * ereal<maxlimbs>(0.5);
	return v;
}

// ---- logarithm bases: derived from ln2 / ln10 (extended-precision divide) ----

template<unsigned maxlimbs = 19>
inline ereal<maxlimbs> ereal_lge() {  // log2(e) = 1/ln(2)
	static const ereal<maxlimbs> v = ereal<maxlimbs>(1.0) / ereal_ln2<maxlimbs>();
	return v;
}

template<unsigned maxlimbs = 19>
inline ereal<maxlimbs> ereal_lg10() {  // log2(10) = ln(10)/ln(2)
	static const ereal<maxlimbs> v = ereal_ln10<maxlimbs>() / ereal_ln2<maxlimbs>();
	return v;
}

template<unsigned maxlimbs = 19>
inline ereal<maxlimbs> ereal_log2() {  // log10(2) = ln(2)/ln(10)
	static const ereal<maxlimbs> v = ereal_ln2<maxlimbs>() / ereal_ln10<maxlimbs>();
	return v;
}

template<unsigned maxlimbs = 19>
inline ereal<maxlimbs> ereal_loge() {  // log10(e) = 1/ln(10)
	static const ereal<maxlimbs> v = ereal<maxlimbs>(1.0) / ereal_ln10<maxlimbs>();
	return v;
}

// ---- reciprocals / other derived constants ----

template<unsigned maxlimbs = 19>
inline ereal<maxlimbs> ereal_1_phi() {  // 1/phi = phi - 1  (exact identity)
	static const ereal<maxlimbs> v = ereal_phi<maxlimbs>() - ereal<maxlimbs>(1.0);
	return v;
}

template<unsigned maxlimbs = 19>
inline ereal<maxlimbs> ereal_1_e() {  // 1/e
	static const ereal<maxlimbs> v = ereal<maxlimbs>(1.0) / ereal_e<maxlimbs>();
	return v;
}

template<unsigned maxlimbs = 19>
inline ereal<maxlimbs> ereal_1_pi() {  // 1/pi
	static const ereal<maxlimbs> v = ereal<maxlimbs>(1.0) / ereal_pi<maxlimbs>();
	return v;
}

template<unsigned maxlimbs = 19>
inline ereal<maxlimbs> ereal_2_pi() {  // 2/pi
	static const ereal<maxlimbs> v = ereal<maxlimbs>(2.0) / ereal_pi<maxlimbs>();
	return v;
}

template<unsigned maxlimbs = 19>
inline ereal<maxlimbs> ereal_1_sqrt2() {  // 1/sqrt(2) = sqrt(2)/2  (exact scaling)
	static const ereal<maxlimbs> v = ereal_sqrt2<maxlimbs>() * ereal<maxlimbs>(0.5);
	return v;
}

template<unsigned maxlimbs = 19>
inline ereal<maxlimbs> ereal_2_sqrtpi() {  // 2/sqrt(pi)
	static const ereal<maxlimbs> v = ereal<maxlimbs>(2.0) / sqrt(ereal_pi<maxlimbs>());
	return v;
}

}} // namespace sw::universal
