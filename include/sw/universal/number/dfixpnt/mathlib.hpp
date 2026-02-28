#pragma once
// mathlib.hpp: definition of mathematical functions specialized for dfixpnt types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// absolute value
template<unsigned ndigits, unsigned radix, DecimalEncoding encoding, bool arithmetic, typename bt>
dfixpnt<ndigits, radix, encoding, arithmetic, bt>
abs(const dfixpnt<ndigits, radix, encoding, arithmetic, bt>& v) {
	dfixpnt<ndigits, radix, encoding, arithmetic, bt> result(v);
	result.setsign(false);
	return result;
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
		if (hasFraction) {
			dfixpnt<ndigits, radix, encoding, arithmetic, bt> one;
			one.setdigit(radix, 1);
			result -= one;
		}
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
		if (hasFraction) {
			dfixpnt<ndigits, radix, encoding, arithmetic, bt> one;
			one.setdigit(radix, 1);
			result += one;
		}
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
