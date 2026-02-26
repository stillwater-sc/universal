#pragma once
// exponent.hpp: exponent functions for IBM System/360 hexadecimal floating-point hfloats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// the current shims are NON-COMPLIANT with the Universal standard, which says that every function must be
// correctly rounded for every input value. Anything less sacrifices bitwise reproducibility of results.

// Base-e exponential function
template<unsigned ndigits, unsigned es, typename bt>
hfloat<ndigits, es, bt> exp(hfloat<ndigits, es, bt> x) {
	return hfloat<ndigits, es, bt>(std::exp(double(x)));
}

// Base-2 exponential function
template<unsigned ndigits, unsigned es, typename bt>
hfloat<ndigits, es, bt> exp2(hfloat<ndigits, es, bt> x) {
	return hfloat<ndigits, es, bt>(std::exp2(double(x)));
}

// Base-10 exponential function
template<unsigned ndigits, unsigned es, typename bt>
hfloat<ndigits, es, bt> exp10(hfloat<ndigits, es, bt> x) {
	return hfloat<ndigits, es, bt>(std::pow(10.0, double(x)));
}

// Base-e exponential function exp(x)-1
template<unsigned ndigits, unsigned es, typename bt>
hfloat<ndigits, es, bt> expm1(hfloat<ndigits, es, bt> x) {
	return hfloat<ndigits, es, bt>(std::expm1(double(x)));
}

}} // namespace sw::universal
