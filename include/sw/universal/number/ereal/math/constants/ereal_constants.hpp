#pragma once
// ereal_constants.hpp: high-precision math constants for ereal adaptive precision
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <string>

namespace sw { namespace universal {

// Mathematical constants for ereal adaptive-precision arithmetic.
//
// These are genuine high-precision constants, NOT double-precision placeholders:
// a `double` literal can carry only ~16 digits, so initializing an ereal from
// one (e.g. ereal<m>(3.14159...long...)) silently truncates to double precision.
// That capped every transcendental at ~16 digits (issue #1002). Instead:
//   - pi, ln2, ln10 are parsed from verified high-precision decimal strings
//     (parse() builds a true multi-component expansion), and
//   - the remaining constants are DERIVED from those via exact operations
//     (multiply/divide by powers of two, small integers) or extended-precision
//     algorithms (the ereal sqrt/exp/divide are themselves extended-precision),
//     so their correctness follows from the algorithm rather than from
//     hand-transcribed tail digits.
// Each accessor caches its value in a function-local static (computed once per
// ereal<maxlimbs> instantiation).

namespace ereal_detail {
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> parse_constant(const char* digits) {
		ereal<maxlimbs> v;
		v.parse(std::string(digits));
		return v;
	}
}

// ---- base constants, parsed from verified high-precision strings ----

template<unsigned maxlimbs = 8>
inline ereal<maxlimbs> ereal_pi() {  // pi to 100 digits (OEIS A000796)
	static const ereal<maxlimbs> v = ereal_detail::parse_constant<maxlimbs>(
		"3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679");
	return v;
}

template<unsigned maxlimbs = 8>
inline ereal<maxlimbs> ereal_ln2() {  // ln(2) to 125 digits (OEIS A002162)
	static const ereal<maxlimbs> v = ereal_detail::parse_constant<maxlimbs>(
		"0.69314718055994530941723212145817656807550013436025525412068000949339362196969471560586332699641868754200148102057068573");
	return v;
}

template<unsigned maxlimbs = 8>
inline ereal<maxlimbs> ereal_ln10() {  // ln(10) to ~230 digits (OEIS A002392)
	static const ereal<maxlimbs> v = ereal_detail::parse_constant<maxlimbs>(
		"2.3025850929940456840179914546843642076011014886287729760333279009675726096773524802359972050895982983419677840422862486334095254650828067566662873690987816894829072083255546808437998948262331985283935053089653777326288461633662222876982198");
	return v;
}

// ---- pi multiples / fractions: exact scaling by powers of two and small ints ----

template<unsigned maxlimbs = 8>
inline ereal<maxlimbs> ereal_pi_2() {  // pi/2  (exact: pi * 2^-1)
	static const ereal<maxlimbs> v = ereal_pi<maxlimbs>() * ereal<maxlimbs>(0.5);
	return v;
}

template<unsigned maxlimbs = 8>
inline ereal<maxlimbs> ereal_pi_4() {  // pi/4  (exact)
	static const ereal<maxlimbs> v = ereal_pi<maxlimbs>() * ereal<maxlimbs>(0.25);
	return v;
}

template<unsigned maxlimbs = 8>
inline ereal<maxlimbs> ereal_3pi_4() {  // 3*pi/4  (exact: 3/4 is exact in binary)
	static const ereal<maxlimbs> v = ereal_pi<maxlimbs>() * ereal<maxlimbs>(0.75);
	return v;
}

template<unsigned maxlimbs = 8>
inline ereal<maxlimbs> ereal_2pi() {  // 2*pi  (exact)
	static const ereal<maxlimbs> v = ereal_pi<maxlimbs>() * ereal<maxlimbs>(2.0);
	return v;
}

template<unsigned maxlimbs = 8>
inline ereal<maxlimbs> ereal_pi_3() {  // pi/3  (extended-precision divide)
	static const ereal<maxlimbs> v = ereal_pi<maxlimbs>() / ereal<maxlimbs>(3.0);
	return v;
}

// ---- square roots: extended-precision ereal Newton sqrt ----

template<unsigned maxlimbs = 8>
inline ereal<maxlimbs> ereal_sqrt2() {
	static const ereal<maxlimbs> v = sqrt(ereal<maxlimbs>(2.0));
	return v;
}

template<unsigned maxlimbs = 8>
inline ereal<maxlimbs> ereal_sqrt3() {
	static const ereal<maxlimbs> v = sqrt(ereal<maxlimbs>(3.0));
	return v;
}

template<unsigned maxlimbs = 8>
inline ereal<maxlimbs> ereal_sqrt5() {
	static const ereal<maxlimbs> v = sqrt(ereal<maxlimbs>(5.0));
	return v;
}

// ---- e and golden ratio: derived ----

template<unsigned maxlimbs = 8>
inline ereal<maxlimbs> ereal_e() {  // e = exp(1)  (extended-precision Taylor)
	static const ereal<maxlimbs> v = exp(ereal<maxlimbs>(1.0));
	return v;
}

template<unsigned maxlimbs = 8>
inline ereal<maxlimbs> ereal_phi() {  // phi = (1 + sqrt(5)) / 2
	static const ereal<maxlimbs> v = (ereal<maxlimbs>(1.0) + ereal_sqrt5<maxlimbs>()) * ereal<maxlimbs>(0.5);
	return v;
}

// ---- logarithm bases: derived from ln2 / ln10 (extended-precision divide) ----

template<unsigned maxlimbs = 8>
inline ereal<maxlimbs> ereal_lge() {  // log2(e) = 1/ln(2)
	static const ereal<maxlimbs> v = ereal<maxlimbs>(1.0) / ereal_ln2<maxlimbs>();
	return v;
}

template<unsigned maxlimbs = 8>
inline ereal<maxlimbs> ereal_lg10() {  // log2(10) = ln(10)/ln(2)
	static const ereal<maxlimbs> v = ereal_ln10<maxlimbs>() / ereal_ln2<maxlimbs>();
	return v;
}

template<unsigned maxlimbs = 8>
inline ereal<maxlimbs> ereal_log2() {  // log10(2) = ln(2)/ln(10)
	static const ereal<maxlimbs> v = ereal_ln2<maxlimbs>() / ereal_ln10<maxlimbs>();
	return v;
}

template<unsigned maxlimbs = 8>
inline ereal<maxlimbs> ereal_loge() {  // log10(e) = 1/ln(10)
	static const ereal<maxlimbs> v = ereal<maxlimbs>(1.0) / ereal_ln10<maxlimbs>();
	return v;
}

// ---- reciprocals / other derived constants ----

template<unsigned maxlimbs = 8>
inline ereal<maxlimbs> ereal_1_phi() {  // 1/phi = phi - 1  (exact identity)
	static const ereal<maxlimbs> v = ereal_phi<maxlimbs>() - ereal<maxlimbs>(1.0);
	return v;
}

template<unsigned maxlimbs = 8>
inline ereal<maxlimbs> ereal_1_e() {  // 1/e
	static const ereal<maxlimbs> v = ereal<maxlimbs>(1.0) / ereal_e<maxlimbs>();
	return v;
}

template<unsigned maxlimbs = 8>
inline ereal<maxlimbs> ereal_1_pi() {  // 1/pi
	static const ereal<maxlimbs> v = ereal<maxlimbs>(1.0) / ereal_pi<maxlimbs>();
	return v;
}

template<unsigned maxlimbs = 8>
inline ereal<maxlimbs> ereal_2_pi() {  // 2/pi
	static const ereal<maxlimbs> v = ereal<maxlimbs>(2.0) / ereal_pi<maxlimbs>();
	return v;
}

template<unsigned maxlimbs = 8>
inline ereal<maxlimbs> ereal_1_sqrt2() {  // 1/sqrt(2) = sqrt(2)/2  (exact scaling)
	static const ereal<maxlimbs> v = ereal_sqrt2<maxlimbs>() * ereal<maxlimbs>(0.5);
	return v;
}

template<unsigned maxlimbs = 8>
inline ereal<maxlimbs> ereal_2_sqrtpi() {  // 2/sqrt(pi)
	static const ereal<maxlimbs> v = ereal<maxlimbs>(2.0) / sqrt(ereal_pi<maxlimbs>());
	return v;
}

}} // namespace sw::universal
