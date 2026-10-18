#pragma once
// sqrt.hpp: sqrt function for decimal floating-point dfloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cerrno>

namespace sw { namespace universal {

// sqrt for dfloat, correctly rounded (#1725).
//
// This used to be dfloat(std::sqrt(double(a))) in both branches of a DFLOAT_NATIVE_SQRT switch,
// one of them marked "TBD": the decimal value converted to binary, its binary root, and that
// converted back to decimal.  A double holds about 15.9 decimal digits, so decimal64 and
// decimal128 roots were wrong in their trailing digits, and even decimal32 was rounded twice.
// The root is now the exact integer root of the scaled decimal significand, rounded once to
// ndigits, ties to even (dfloat::square_root).  The switch is gone.
template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename bt>
inline dfloat<ndigits, es, Encoding, bt> sqrt(const dfloat<ndigits, es, Encoding, bt>& a) {
	using Dfloat = dfloat<ndigits, es, Encoding, bt>;
	if (a.isnan()) return a;
	if (a.iszero()) return a;                         // sqrt(+-0) is +-0
	if (a.isneg()) {                                  // -inf included: sqrt(-inf) is NaN
#if DFLOAT_THROW_ARITHMETIC_EXCEPTION
		throw dfloat_negative_sqrt_arg();
#else
		errno = EDOM;                                 // as std::sqrt: EDOM and a quiet NaN, with no message
		Dfloat nan;
		nan.setnan(NAN_TYPE_QUIET);
		return nan;
#endif
	}
	if (a.isinf()) return a;                          // sqrt(+inf) is +inf
	return a.square_root();
}

// reciprocal sqrt
template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename bt>
inline dfloat<ndigits, es, Encoding, bt> rsqrt(const dfloat<ndigits, es, Encoding, bt>& a) {
	dfloat<ndigits, es, Encoding, bt> v = sqrt(a);
	return v.reciprocate();
}

}} // namespace sw::universal
