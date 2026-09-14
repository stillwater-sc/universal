#pragma once
// mathlib.hpp: definition of mathematical functions specialized for dfixpnt types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/dfixpnt/core.hpp>   // the functions compute; the core is all they need (#1334)

namespace sw { namespace universal {

// absolute value
template<unsigned ndigits, unsigned radix, DecimalEncoding encoding, bool arithmetic, typename bt>
dfixpnt<ndigits, radix, encoding, arithmetic, bt>
abs(const dfixpnt<ndigits, radix, encoding, arithmetic, bt>& v) {
	dfixpnt<ndigits, radix, encoding, arithmetic, bt> result(v);
	result.setsign(false);
	return result;
}

// the value 1 in the type of v. dfixpnt is trivially constructible, so a default-constructed
// value holds indeterminate digits and sign: start from zero, then set the units digit (#1476).
// A type with no integer digits (radix == ndigits) cannot hold 1, and with Modulo arithmetic
// 1 wraps to 0, so there the unit is 0 and floor and ceil return the integer part, 0.
template<unsigned ndigits, unsigned radix, DecimalEncoding encoding, bool arithmetic, typename bt>
dfixpnt<ndigits, radix, encoding, arithmetic, bt> UnitOf(const dfixpnt<ndigits, radix, encoding, arithmetic, bt>&) {
	dfixpnt<ndigits, radix, encoding, arithmetic, bt> one;
	one.setzero();
	if constexpr (radix < ndigits) one.setdigit(radix, 1);
	return one;
}

// floor: largest integer value not greater than v
template<unsigned ndigits, unsigned radix, DecimalEncoding encoding, bool arithmetic, typename bt>
dfixpnt<ndigits, radix, encoding, arithmetic, bt>
floor(const dfixpnt<ndigits, radix, encoding, arithmetic, bt>& v) {
	dfixpnt<ndigits, radix, encoding, arithmetic, bt> result(v);
	// zero out all fractional digits
	for (unsigned i = 0; i < radix; ++i) result.setdigit(i, 0);
	// if negative and had fractional part, subtract 1 from integer part
	if (v.sign()) {
		bool hasFraction = false;
		for (unsigned i = 0; i < radix; ++i) {
			if (v.digit(i) != 0) { hasFraction = true; break; }
		}
		if (hasFraction) result -= UnitOf(v);
	}
	return result;
}

// ceil: smallest integer value not less than v
template<unsigned ndigits, unsigned radix, DecimalEncoding encoding, bool arithmetic, typename bt>
dfixpnt<ndigits, radix, encoding, arithmetic, bt>
ceil(const dfixpnt<ndigits, radix, encoding, arithmetic, bt>& v) {
	dfixpnt<ndigits, radix, encoding, arithmetic, bt> result(v);
	for (unsigned i = 0; i < radix; ++i) result.setdigit(i, 0);
	if (!v.sign()) {
		bool hasFraction = false;
		for (unsigned i = 0; i < radix; ++i) {
			if (v.digit(i) != 0) { hasFraction = true; break; }
		}
		if (hasFraction) result += UnitOf(v);
	}
	return result;
}

// classification functions
template<unsigned ndigits, unsigned radix, DecimalEncoding encoding, bool arithmetic, typename bt>
bool isnan(const dfixpnt<ndigits, radix, encoding, arithmetic, bt>&) { return false; }

template<unsigned ndigits, unsigned radix, DecimalEncoding encoding, bool arithmetic, typename bt>
bool isinf(const dfixpnt<ndigits, radix, encoding, arithmetic, bt>&) { return false; }

template<unsigned ndigits, unsigned radix, DecimalEncoding encoding, bool arithmetic, typename bt>
bool isfinite(const dfixpnt<ndigits, radix, encoding, arithmetic, bt>&) { return true; }

template<unsigned ndigits, unsigned radix, DecimalEncoding encoding, bool arithmetic, typename bt>
bool isnormal(const dfixpnt<ndigits, radix, encoding, arithmetic, bt>& v) { return !v.iszero(); }

}} // namespace sw::universal
