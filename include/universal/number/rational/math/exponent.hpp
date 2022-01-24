#pragma once
// exponent.hpp: exponent functions for rationals
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// the current shims are NON-COMPLIANT with the Universal standard, which says that every function must be
// correctly rounded for every input value. Anything less sacrifices bitwise reproducibility of results.

// Base-e exponential function
rational exp(rational x) {
	if (isnan(x)) return x;
	rational p;
	p = std::exp(double(x));
	return p;
}

// Base-2 exponential function
rational exp2(rational x) {
	if (isnan(x)) return x;
	rational p;
	p = std::exp2(double(x));
	return p;
}

// Base-10 exponential function
rational exp10(rational x) {
	return rational(std::pow(10.0, double(x)));
}
		
// Base-e exponential function exp(x)-1
rational expm1(rational x) {
	return rational(std::expm1(double(x)));
}


}} // namespace sw::universal
