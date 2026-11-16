#pragma once
// pow.hpp: pow functions for classic floating-point cfloats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <cmath>         // the <cmath> functions used below
#include <universal/number/cfloat/math/efloat_bridge.hpp>

namespace sw { namespace universal {

template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> pow(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x, cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> y) {
	if constexpr (!cfloat_math::fits_double<cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>>()) {
		if (x.isnan()) return x.quieted();
		if (y.isnan()) return y.quieted();
		return cfloat_math::evaluate<cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>>([&](unsigned W) { return pow(cfloat_math::argument(x, W), cfloat_math::argument(y, W)); });
	}
	return cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>(std::pow(double(x), double(y)));
}
		
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> pow(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x, int y) {
	if constexpr (!cfloat_math::fits_double<cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>>()) {
		if (x.isnan()) return x.quieted();
		return cfloat_math::evaluate<cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>>([&](unsigned W) { return pow(cfloat_math::argument(x, W), cfloat_math::argument_from<cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>>(double(y), W)); });
	}
	return cfloat<nbits,es,bt, hasSubnormals, hasMaxExpValues, isSaturating>(std::pow(double(x), double(y)));
}
		
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> pow(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x, double y) {
	if constexpr (!cfloat_math::fits_double<cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>>()) {
		if (x.isnan()) return x.quieted();
		return cfloat_math::evaluate<cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>>([&](unsigned W) { return pow(cfloat_math::argument(x, W), cfloat_math::argument_from<cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>>(y, W)); });
	}
	return cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>(std::pow(double(x), y));
}

}} // namespace sw::universal
