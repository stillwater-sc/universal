#pragma once
// next.hpp: nextafter/nexttoward functions for cfloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <cerrno>

namespace sw { namespace universal {

/*
Parameters
	x  Base value.
	t  Value toward which the return value is approximated.
If both parameters compare equal, the function returns t.
If either parameter is a NaN, the function returns a quiet NaN.

Return Value
	The next representable value after x in the direction of t.

	If x is the largest finite value representable in the type,
	and the result is infinite or not representable, an overflow range error occurs.

	If an overflow range error occurs:
	- And math_errhandling has MATH_ERRNO set: the global variable errno is set to ERANGE.
	- And math_errhandling has MATH_ERREXCEPT set: FE_OVERFLOW is raised.
	cfloat sets errno; it has no floating-point status flags, so nothing is raised.
	*/
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits,es, bt, hasSubnormals, hasMaxExpValues, isSaturating> nextafter(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x, cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> target) {
	using Cfloat = cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>;
	if (x.isnan() || target.isnan()) { // a NaN operand gives a quiet NaN (#1901)
		Cfloat qnan;
		qnan.setnan(NAN_TYPE_QUIET);
		return qnan;
	}
	if (x == target) return target;
	Cfloat next(x);
	if (x > target) --next; else ++next;
	if (next.isinf() && !x.isinf()) errno = ERANGE;  // overflow range error
	return next;
}

// nexttoward steps x by one ulp of x's own type; the target only sets the direction (#1901: it
// used to step a 128-bit copy of x and round back, which never moved a narrower x). x and the
// target are compared in a format that holds both exactly: cfloat<128,15> for any x up to that
// size, wider in exponent and/or fraction for a wider x.
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits,es, bt, hasSubnormals, hasMaxExpValues, isSaturating> nexttoward(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x, cfloat<128, 15, bt, hasSubnormals, hasMaxExpValues, isSaturating> target) {
	using Cfloat = cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>;
	using Wide   = cfloat<128, 15, bt, hasSubnormals, hasMaxExpValues, isSaturating>;
	constexpr unsigned cmpEs    = (es > Wide::es ? es : Wide::es);
	constexpr unsigned cmpFbits = (Cfloat::fbits > Wide::fbits ? Cfloat::fbits : Wide::fbits);
	using Compare = cfloat<1u + cmpEs + cmpFbits, cmpEs, bt, hasSubnormals, hasMaxExpValues, isSaturating>;
	if (x.isnan() || target.isnan()) { // a NaN operand gives a quiet NaN
		Cfloat qnan;
		qnan.setnan(NAN_TYPE_QUIET);
		return qnan;
	}
	Compare cx(x), ct(target);  // both exact
	if (cx == ct) return Cfloat(target);
	Cfloat next(x);
	if (cx > ct) --next; else ++next;
	if (next.isinf() && !x.isinf()) errno = ERANGE;  // overflow range error
	return next;
}

}} // namespace sw::universal
