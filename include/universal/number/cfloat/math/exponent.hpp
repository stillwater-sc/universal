#pragma once
// exponent.hpp: exponent functions for cfloats
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw::universal {

// the current shims are NON-COMPLIANT with the cfloat standard, which says that every function must be
// correctly rounded for every input value. Anything less sacrifices bitwise reproducibility of results.

// Base-e exponential function
template<size_t nbits, size_t es, typename bt>
cfloat<nbits, es, bt> exp(cfloat<nbits, es, bt> x) {
	if (isnan(x)) return x;
	cfloat<nbits, es, bt> p;
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
template<size_t nbits, size_t es, typename bt>
cfloat<nbits, es, bt> exp2(cfloat<nbits, es, bt> x) {
	if (isnan(x)) return x;
	cfloat<nbits, es, bt> p;
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
template<size_t nbits, size_t es, typename bt>
cfloat<nbits, es, bt> exp10(cfloat<nbits, es, bt> x) {
	return cfloat<nbits, es, bt>(std::pow(10.0, double(x)));
}
		
// Base-e exponential function exp(x)-1
template<size_t nbits, size_t es, typename bt>
cfloat<nbits, es, bt> expm1(cfloat<nbits, es, bt> x) {
	return cfloat<nbits, es, bt>(std::expm1(double(x)));
}


}  // namespace sw::universal
