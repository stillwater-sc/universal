#pragma once
// exponent.hpp: exponent functions for lns
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// the current shims are NON-COMPLIANT with the Universal standard, which says that every function must be
// correctly rounded for every input value. Anything less sacrifices bitwise reproducibility of results.

// Base-e exponential function
template<size_t nbits, size_t rbits, ArithmeticBehavior behavior, typename bt>
lns<nbits, rbits, behavior, bt> exp(lns<nbits, rbits, behavior, bt> x) {
	if (isnan(x)) return x;
	lns<nbits, rbits, behavior, bt> p;
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
template<size_t nbits, size_t rbits, ArithmeticBehavior behavior, typename bt>
lns<nbits, rbits, behavior, bt> exp2(lns<nbits, rbits, behavior, bt> x) {
	if (isnan(x)) return x;
	lns<nbits, rbits, behavior, bt> p;
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
template<size_t nbits, size_t rbits, ArithmeticBehavior behavior, typename bt>
lns<nbits, rbits, behavior, bt> exp10(lns<nbits, rbits, behavior, bt> x) {
	return lns<nbits, rbits, behavior, bt>(std::pow(10.0, double(x)));
}
		
// Base-e exponential function exp(x)-1
template<size_t nbits, size_t rbits, ArithmeticBehavior behavior, typename bt>
lns<nbits, rbits, behavior, bt> expm1(lns<nbits, rbits, behavior, bt> x) {
	return lns<nbits, rbits, behavior, bt>(std::expm1(double(x)));
}


}} // namespace sw::universal
