#pragma once
// exponent.hpp: exponent functions for adaptive precision decimal rationals
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// the current shims are NON-COMPLIANT with the Universal standard, which says that every function must be
// correctly rounded for every input value. Anything less sacrifices bitwise reproducibility of results.

// Base-e exponential function
erational exp(erational x) {
	if (isnan(x)) return x;
	erational p;
	p = std::exp(double(x));
	return p;
}

// Base-2 exponential function
erational exp2(erational x) {
	if (isnan(x)) return x;
	erational p;
	p = std::exp2(double(x));
	return p;
}

// Base-10 exponential function
erational exp10(erational x) {
	return erational(std::pow(10.0, double(x)));
}
		
// Base-e exponential function exp(x)-1
erational expm1(erational x) {
	return erational(std::expm1(double(x)));
}


}} // namespace sw::universal
