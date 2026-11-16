#pragma once
// trigonometric.hpp: trigonometric functions for classic floating-point cfloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cmath>         // the <cmath> functions used below
#include <math/constants/double_constants.hpp>

#include <universal/number/cfloat/math/efloat_bridge.hpp>

namespace sw { namespace universal {

// value representing an angle expressed in radians
// One radian is equivalent to 180/PI degrees

// sine of an angle of x radians
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> sin(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x) {
	if constexpr (!cfloat_math::fits_double<cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>>()) return cfloat_math::unary(x, [](const auto& e) { return sin(e); }, true);
	return cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>(std::sin(double(x)));
}

// cosine of an angle of x radians
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> cos(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x) {
	if constexpr (!cfloat_math::fits_double<cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>>()) return cfloat_math::unary(x, [](const auto& e) { return cos(e); }, false);
	return cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>(std::cos(double(x)));
}

// tangent of an angle of x radians
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> tan(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x) {
	if constexpr (!cfloat_math::fits_double<cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>>()) return cfloat_math::unary(x, [](const auto& e) { return tan(e); }, true);
	return cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>(std::tan(double(x)));
}

// cotangent of an angle of x radians
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> atan(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x) {
	if constexpr (!cfloat_math::fits_double<cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>>()) return cfloat_math::unary(x, [](const auto& e) { return atan(e); }, true);
	return cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>(std::atan(double(x)));
}
		
// Arc tangent with two parameters
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> atan2(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> y, cfloat<nbits,es,bt, hasSubnormals, hasMaxExpValues, isSaturating> x) {
	if constexpr (!cfloat_math::fits_double<cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>>()) {
		if (y.isnan()) return y.quieted();
		if (x.isnan()) return x.quieted();
		return cfloat_math::evaluate<cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>>([&](unsigned W) { return atan2(cfloat_math::argument(y, W), cfloat_math::argument(x, W)); });
	}
	return cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>(std::atan2(double(y),double(x)));
}

// cosecant of an angle of x radians
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> acos(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x) {
	if constexpr (!cfloat_math::fits_double<cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>>()) return cfloat_math::unary(x, [](const auto& e) { return acos(e); }, false);
	return cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>(std::acos(double(x)));
}

// secant of an angle of x radians
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> asin(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x) {
	if constexpr (!cfloat_math::fits_double<cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>>()) return cfloat_math::unary(x, [](const auto& e) { return asin(e); }, true);
	return cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>(std::asin(double(x)));
}

// cotangent an angle of x radians
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> cot(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x) {
	if constexpr (!cfloat_math::fits_double<cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>>()) {
		// 1/tan at 2W, rounded to W: well inside the 1 ulp at W that evaluate() tests
		if (x.isnan()) return x.quieted();
		return cfloat_math::evaluate<cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>>([&](unsigned W) {
			const auto t = tan(cfloat_math::argument(x, 2u * W));
			auto r = decltype(t)(1) / t;
			r.set_precision(W);
			return r;
		});
	}
	return cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>(std::tan(sw::universal::d_pi_2-double(x)));
}

// secant of an angle of x radians
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> sec(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x) {
	if constexpr (!cfloat_math::fits_double<cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>>()) {
		// 1/cos at 2W, rounded to W: well inside the 1 ulp at W that evaluate() tests
		if (x.isnan()) return x.quieted();
		return cfloat_math::evaluate<cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>>([&](unsigned W) {
			const auto t = cos(cfloat_math::argument(x, 2u * W));
			auto r = decltype(t)(1) / t;
			r.set_precision(W);
			return r;
		});
	}
	return cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>(1.0/std::cos(double(x)));
}

// cosecant of an angle of x radians
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> csc(cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> x) {
	if constexpr (!cfloat_math::fits_double<cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>>()) {
		// 1/sin at 2W, rounded to W: well inside the 1 ulp at W that evaluate() tests
		if (x.isnan()) return x.quieted();
		return cfloat_math::evaluate<cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>>([&](unsigned W) {
			const auto t = sin(cfloat_math::argument(x, 2u * W));
			auto r = decltype(t)(1) / t;
			r.set_precision(W);
			return r;
		});
	}
	return cfloat<nbits, es, bt,hasSubnormals,hasMaxExpValues,isSaturating>(1.0/std::sin(double(x)));
}

}} // namespace sw::universal
