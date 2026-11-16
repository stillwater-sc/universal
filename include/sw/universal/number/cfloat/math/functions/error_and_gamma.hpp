#pragma once
// error_gamma.hpp: error and gamma functions for cfloats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <universal/number/cfloat/math/efloat_bridge.hpp>

namespace sw { namespace universal {

// Compute the error function erf(x) = 2 over sqrt(PI) times Integral from 0 to x of e ^ (-t)^2 dt
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> erf(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x) {
	if constexpr (!cfloat_math::fits_double<cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>>()) return cfloat_math::unary(x, [](const auto& e) { return erf(e); }, true);
	return cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>(std::erf(double(x)));
}

// Compute the complementary error function: 1 - erf(x)
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> erfc(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x) {
	if constexpr (!cfloat_math::fits_double<cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>>()) return cfloat_math::unary(x, [](const auto& e) { return erfc(e); }, false);
	return cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>(std::erfc(double(x)));
}

}} // namespace sw::universal
