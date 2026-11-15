#pragma once
// exponent.hpp: exponent functions for cfloats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <cmath>         // the <cmath> functions used below
namespace sw { namespace universal {

// the current shims are NON-COMPLIANT with the Universal standard, which says that every function must be
// correctly rounded for every input value. Anything less sacrifices bitwise reproducibility of results.

// true when the layout's largest finite binade is no higher than double's, and its smallest value is
// no smaller than double's smallest: such a layout's range ends inside double's.  MAX_EXP counts the
// top exponent binade, which holds finite values only with hasMaxExpValues.
template<typename Cfloat>
constexpr bool cfloat_range_fits_double() {
	constexpr int maxFiniteExp = Cfloat::hasMaxExpValues ? Cfloat::MAX_EXP : Cfloat::MAX_EXP - 1;
	return maxFiniteExp <= 1023 && (Cfloat::hasSubnormals ? Cfloat::MIN_EXP_SUBNORMAL : Cfloat::MIN_EXP_NORMAL) >= -1074;
}

// Base-e exponential function
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> exp(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x) {
	if (isnan(x)) return x.quieted();
	cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> p;
	if (x.isinf() && x.sign()) { p.setzero(); return p; }   // exp(-inf) is +0, not minpos (#2088)
	double d = std::exp(double(x));
	if (d == 0.0) {
		// a double underflow is the layout's underflow when its range ends inside double's: the
		// result rounds to 0.  Wider layouts keep minpos until they evaluate past double.
		if constexpr (cfloat_range_fits_double<cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>>()) p.setzero();
		else p.minpos();
	}
	else {
		p = d;
	}
	return p;
}

// Base-2 exponential function
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> exp2(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x) {
	if (isnan(x)) return x.quieted();
	cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> p;
	if (x.isinf() && x.sign()) { p.setzero(); return p; }   // exp2(-inf) is +0, not minpos (#2088)
	double d = std::exp2(double(x));
	if (d == 0.0) {
		// a double underflow is the layout's underflow when its range ends inside double's: the
		// result rounds to 0.  Wider layouts keep minpos until they evaluate past double.
		if constexpr (cfloat_range_fits_double<cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>>()) p.setzero();
		else p.minpos();
	}
	else {
		p = d;
	}
	return p;
}

// Base-10 exponential function
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> exp10(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x) {
	return cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>(std::pow(10.0, double(x)));
}
		
// Base-e exponential function exp(x)-1
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> expm1(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x) {
	return cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>(std::expm1(double(x)));
}


}} // namespace sw::universal
