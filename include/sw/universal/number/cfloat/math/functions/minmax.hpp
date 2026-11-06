#pragma once
// minmax.hpp: min/max and fmin/fmax functions for classic floating-point cfloats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// All four compare the cfloat operands directly, so the result is one of the operands, every
// bit of it, for any configuration (#1901: they used to round through double, which lost the
// precision of quad and of anything wider than double).
//
// min/max are IEEE-754-2019 minimum/maximum (9.6): a NaN operand, quiet or signalling, gives a
// quiet NaN, and -0 orders below +0.
// fmin/fmax are C fmin/fmax, as C23 fminimum_num/fmaximum_num and IEEE-754-2019
// minimumNumber/maximumNumber: a NaN operand, quiet or signalling, is ignored and the other
// operand is returned, two NaN operands give a quiet NaN, and -0 orders below +0.
// None of them returns a signalling NaN (#1899).

namespace sw { namespace universal {

template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>
min(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x, cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> y) {
	if (x.isnan() || y.isnan()) {
		cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> qnan;
		qnan.setnan(NAN_TYPE_QUIET);
		return qnan;
	}
	if (x.iszero() && y.iszero()) return x.sign() ? x : y;  // -0 < +0
	return (y < x) ? y : x;
}

template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>
max(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x, cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> y) {
	if (x.isnan() || y.isnan()) {
		cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> qnan;
		qnan.setnan(NAN_TYPE_QUIET);
		return qnan;
	}
	if (x.iszero() && y.iszero()) return x.sign() ? y : x;  // +0 > -0
	return (x < y) ? y : x;
}

template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>
fmin(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x, cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> y) {
	if (x.isnan()) return y.quieted();  // y, or a quiet NaN when both are NaN
	if (y.isnan()) return x;
	return min(x, y);
}

template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>
fmax(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x, cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> y) {
	if (x.isnan()) return y.quieted();  // y, or a quiet NaN when both are NaN
	if (y.isnan()) return x;
	return max(x, y);
}

}} // namespace sw::universal
