#pragma once
// trigonometry.hpp: trigonometric functions (sin, cos, tan, asin, acos, atan, atan2, cot, sec, csc) for posits
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Correctly rounded at every nbits (#1855): computed by efloat's correctly rounded math library and
// rounded once into the posit by posit_limbs::evaluate, which applies Ziv's test on the posit side.
// No native floating-point type is involved, so the posit's whole precision and range hold.
// The argument reduction is efloat's, exact at any magnitude (Payne-Hanek), so sin(x) is correct for
// every posit x. asin and acos of |x| > 1 are NaR.
#include <math/constants/double_constants.hpp>  // posit.hpp users get d_pi and friends through here
#include <universal/number/posit/math/evaluate.hpp>

namespace sw { namespace universal {

	// the sine
	template<unsigned nbits, unsigned es, typename bt>
	posit<nbits, es, bt> sin(posit<nbits, es, bt> x) {
		using Posit = posit<nbits, es, bt>;
		if (x.isnar()) return x;
		return posit_limbs::evaluate<Posit>([&](unsigned W) { return sin(posit_limbs::argument(x, W)); });
	}

	// the cosine
	template<unsigned nbits, unsigned es, typename bt>
	posit<nbits, es, bt> cos(posit<nbits, es, bt> x) {
		using Posit = posit<nbits, es, bt>;
		if (x.isnar()) return x;
		return posit_limbs::evaluate<Posit>([&](unsigned W) { return cos(posit_limbs::argument(x, W)); });
	}

	// the tangent
	template<unsigned nbits, unsigned es, typename bt>
	posit<nbits, es, bt> tan(posit<nbits, es, bt> x) {
		using Posit = posit<nbits, es, bt>;
		if (x.isnar()) return x;
		return posit_limbs::evaluate<Posit>([&](unsigned W) { return tan(posit_limbs::argument(x, W)); });
	}

	// the arc sine, in [-pi/2, pi/2]
	template<unsigned nbits, unsigned es, typename bt>
	posit<nbits, es, bt> asin(posit<nbits, es, bt> x) {
		using Posit = posit<nbits, es, bt>;
		if (x.isnar()) return x;
		return posit_limbs::evaluate<Posit>([&](unsigned W) { return asin(posit_limbs::argument(x, W)); });
	}

	// the arc cosine, in [0, pi]
	template<unsigned nbits, unsigned es, typename bt>
	posit<nbits, es, bt> acos(posit<nbits, es, bt> x) {
		using Posit = posit<nbits, es, bt>;
		if (x.isnar()) return x;
		return posit_limbs::evaluate<Posit>([&](unsigned W) { return acos(posit_limbs::argument(x, W)); });
	}

	// the arc tangent, in (-pi/2, pi/2)
	template<unsigned nbits, unsigned es, typename bt>
	posit<nbits, es, bt> atan(posit<nbits, es, bt> x) {
		using Posit = posit<nbits, es, bt>;
		if (x.isnar()) return x;
		return posit_limbs::evaluate<Posit>([&](unsigned W) { return atan(posit_limbs::argument(x, W)); });
	}

	// the angle of the point (x, y), in [-pi, pi]; NaR in, NaR out
	template<unsigned nbits, unsigned es, typename bt>
	posit<nbits, es, bt> atan2(posit<nbits, es, bt> y, posit<nbits, es, bt> x) {
		using Posit = posit<nbits, es, bt>;
		if (y.isnar() || x.isnar()) return Posit(SpecificValue::nar);
		return posit_limbs::evaluate<Posit>([&](unsigned W) { return atan2(posit_limbs::argument(y, W), posit_limbs::argument(x, W)); });
	}

	// the cotangent: 1 / tan(x) of tan at 2W bits, divided at 2W and rounded to W, which stays within the
	// 1 ulp at W that evaluate() tests; a zero of tan is a pole, NaR
	template<unsigned nbits, unsigned es, typename bt>
	posit<nbits, es, bt> cot(posit<nbits, es, bt> x) {
		using Posit = posit<nbits, es, bt>;
		if (x.isnar()) return x;
		return posit_limbs::evaluate<Posit>([&](unsigned W) {
			const auto t = tan(posit_limbs::argument(x, 2u * W));
			auto r = decltype(t)(1) / t;
			r.set_precision(W);
			return r;
		});
	}

	// the secant: 1 / cos(x) of cos at 2W bits, divided at 2W and rounded to W, which stays within the
	// 1 ulp at W that evaluate() tests; a zero of cos is a pole, NaR
	template<unsigned nbits, unsigned es, typename bt>
	posit<nbits, es, bt> sec(posit<nbits, es, bt> x) {
		using Posit = posit<nbits, es, bt>;
		if (x.isnar()) return x;
		return posit_limbs::evaluate<Posit>([&](unsigned W) {
			const auto t = cos(posit_limbs::argument(x, 2u * W));
			auto r = decltype(t)(1) / t;
			r.set_precision(W);
			return r;
		});
	}

	// the cosecant: 1 / sin(x) of sin at 2W bits, divided at 2W and rounded to W, which stays within the
	// 1 ulp at W that evaluate() tests; a zero of sin is a pole, NaR
	template<unsigned nbits, unsigned es, typename bt>
	posit<nbits, es, bt> csc(posit<nbits, es, bt> x) {
		using Posit = posit<nbits, es, bt>;
		if (x.isnar()) return x;
		return posit_limbs::evaluate<Posit>([&](unsigned W) {
			const auto t = sin(posit_limbs::argument(x, 2u * W));
			auto r = decltype(t)(1) / t;
			r.set_precision(W);
			return r;
		});
	}

}} // namespace sw::universal
