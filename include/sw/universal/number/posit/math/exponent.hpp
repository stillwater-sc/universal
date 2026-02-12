#pragma once
// exponent.hpp: exponent functions for posits
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// the current shims are NON-COMPLIANT with the posit standard, which says that every function must be
// correctly rounded for every input value. Anything less sacrifices bitwise reproducibility of results.

// Base-e exponential function
template<unsigned nbits, unsigned es, typename bt>
posit<nbits,es,bt> exp(posit<nbits,es,bt> x) {
	if (isnar(x)) return x;
	posit<nbits, es, bt> p;
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
template<unsigned nbits, unsigned es, typename bt>
posit<nbits,es,bt> exp2(posit<nbits,es,bt> x) {
	if (isnar(x)) return x;
	posit<nbits, es, bt> p;
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
template<unsigned nbits, unsigned es, typename bt>
posit<nbits, es, bt> exp10(posit<nbits, es, bt> x) {
	return posit<nbits, es, bt>(std::pow(10.0, double(x)));
}
		
// Base-e exponential function exp(x)-1
template<unsigned nbits, unsigned es, typename bt>
posit<nbits,es,bt> expm1(posit<nbits,es,bt> x) {
	return posit<nbits,es,bt>(std::expm1(double(x)));
}


}} // namespace sw::universal
