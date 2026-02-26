#pragma once
// exponent.hpp: exponent functions for cfloats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// the current shims are NON-COMPLIANT with the Universal standard, which says that every function must be
// correctly rounded for every input value. Anything less sacrifices bitwise reproducibility of results.

// Base-e exponential function
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> exp(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x) {
	if (isnan(x)) return x;
	cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> p;
	double d = std::exp(double(x));
	if (d == 0.0) {
		p.minpos();
	}
	else {
		p = d;
	}
	return p;
}

// Base-2 exponential function
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> exp2(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x) {
	if (isnan(x)) return x;
	cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> p;
	double d = std::exp2(double(x));
	if (d == 0.0) {
		p.minpos();
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
