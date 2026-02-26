#pragma once
// fractional.hpp: fractional functions for classic floating-point cfloats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> cfloatmod(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x, cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> y) {
	using Real = cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>;
	if (y.iszero() || x.isinf() || x.isnan() || y.isnan()) {
		Real nan;
		nan.setnan(false);  // quiet NaN
		return nan;
	}
	if (y.isinf() || x.iszero()) {
		return x;
	}

	y.setsign(false); // equivalent but faster than y = abs(y);
	int yexp;
	frexp(y, &yexp);  // ignore the fraction that comes back
	Real r = x;
	if (x < 0) r = -x;
	Real d = r / y;
	if (d.isinf()) return x;
	Real n = trunc(d);
	r = r - n * y;
	if (x < 0) r = -r;

	return r;
}

// fmod retuns x - n*y where n = x/y with the fractional part truncated
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits,es, bt, hasSubnormals, hasMaxExpValues, isSaturating> fmod(cfloat<nbits,es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x, cfloat<nbits,es, bt, hasSubnormals, hasMaxExpValues, isSaturating> y) {
	return cfloatmod(x, y);
}

// shim to stdlib
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits,es, bt, hasSubnormals, hasMaxExpValues, isSaturating> remainder(cfloat<nbits,es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x, cfloat<nbits,es, bt, hasSubnormals, hasMaxExpValues, isSaturating> y) {
	return cfloat<nbits,es, bt, hasSubnormals, hasMaxExpValues, isSaturating>(std::remainder(double(x), double(y)));
}

// TODO: validate the rounding of these conversion, versus a method that manipulates the fraction bits directly

// frac returns the fraction of a cfloat value that is > 1
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits,es, bt, hasSubnormals, hasMaxExpValues, isSaturating> frac(cfloat<nbits,es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x) {
	using Real = cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>;
	long long intValue = (long long)(x);
	return abs(x-Real(intValue));  // with the logic that fractions are unsigned quantities
}


}} // namespace sw::universal
