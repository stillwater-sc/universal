#pragma once
// logarithm.hpp: logarithm functions for classic floating-point cfloats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <cmath>         // the <cmath> functions used below
#include <universal/number/cfloat/math/efloat_bridge.hpp>

namespace sw { namespace universal {

// Natural logarithm of x
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> log(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x) {
	if constexpr (!cfloat_math::fits_double<cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>>()) return cfloat_math::unary(x, [](const auto& e) { return log(e); }, false);
	return cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>(std::log(double(x)));
}

// Binary logarithm of x
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> log2(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x) {
	if constexpr (!cfloat_math::fits_double<cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>>()) return cfloat_math::unary(x, [](const auto& e) { return log2(e); }, false);
	return cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>(std::log2(double(x)));
}

// Decimal logarithm of x
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> log10(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x) {
	if constexpr (!cfloat_math::fits_double<cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>>()) return cfloat_math::unary(x, [](const auto& e) { return log10(e); }, false);
	return cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>(std::log10(double(x)));
}
		
// Natural logarithm of 1+x
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> log1p(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x) {
	if constexpr (!cfloat_math::fits_double<cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>>()) return cfloat_math::unary(x, [](const auto& e) { return log1p(e); }, true);
	return cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>(std::log1p(double(x)));
}

}} // namespace sw::universal
