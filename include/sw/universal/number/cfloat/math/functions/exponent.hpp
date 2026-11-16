#pragma once
// exponent.hpp: exponent functions for cfloats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <cmath>         // the <cmath> functions used below
#include <universal/number/cfloat/math/efloat_bridge.hpp>

namespace sw { namespace universal {

// A layout that fits a double evaluates through the host's libm; a wider one evaluates in efloat and is
// correctly rounded (cfloat_math, #2088).

// Base-e exponential function
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> exp(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x) {
	if constexpr (!cfloat_math::fits_double<cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>>()) return cfloat_math::unary(x, [](const auto& e) { return exp(e); }, false);
	if (isnan(x)) return x.quieted();
	cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> p;
	if (x.isinf() && x.sign()) { p.setzero(); return p; }   // exp(-inf) is +0, not minpos (#2088)
	double d = std::exp(double(x));
	if (d == 0.0) {
		// the layout's range ends inside double's on this path: a double underflow is its underflow
		p.setzero();
	}
	else {
		p = d;
	}
	return p;
}

// Base-2 exponential function
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> exp2(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x) {
	if constexpr (!cfloat_math::fits_double<cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>>()) return cfloat_math::unary(x, [](const auto& e) { return exp2(e); }, false);
	if (isnan(x)) return x.quieted();
	cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> p;
	if (x.isinf() && x.sign()) { p.setzero(); return p; }   // exp2(-inf) is +0, not minpos (#2088)
	double d = std::exp2(double(x));
	if (d == 0.0) {
		// the layout's range ends inside double's on this path: a double underflow is its underflow
		p.setzero();
	}
	else {
		p = d;
	}
	return p;
}

// Base-10 exponential function
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> exp10(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x) {
	if constexpr (!cfloat_math::fits_double<cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>>()) return cfloat_math::unary(x, [](const auto& e) { return exp10(e); }, false);
	return cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>(std::pow(10.0, double(x)));
}
		
// Base-e exponential function exp(x)-1
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> expm1(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x) {
	if constexpr (!cfloat_math::fits_double<cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>>()) return cfloat_math::unary(x, [](const auto& e) { return expm1(e); }, true);
	return cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>(std::expm1(double(x)));
}


}} // namespace sw::universal
