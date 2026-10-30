#pragma once
// hypot.hpp: the hypotenuse function for posits
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Correctly rounded at every nbits (#1855): computed by efloat's correctly rounded math library and
// rounded once into the posit by posit_limbs::evaluate, which applies Ziv's test on the posit side.
// No native floating-point type is involved, so the posit's whole precision and range hold.
#include <universal/number/posit/math/evaluate.hpp>

namespace sw { namespace universal {

	// sqrt(x^2 + y^2), without overflow or underflow in between: hypot(x, y), hypot(y, x) and
	// hypot(x, -y) are the same, and hypot(x, 0) is |x|. NaR in, NaR out.
	template<unsigned nbits, unsigned es, typename bt>
	posit<nbits, es, bt> hypot(posit<nbits, es, bt> x, posit<nbits, es, bt> y) {
		using Posit = posit<nbits, es, bt>;
		if (x.isnar() || y.isnar()) return Posit(SpecificValue::nar);
		return posit_limbs::evaluate<Posit>([&](unsigned W) { return hypot(posit_limbs::argument(x, W), posit_limbs::argument(y, W)); });
	}

	// sqrt(x^2 + y^2 + z^2)
	template<unsigned nbits, unsigned es, typename bt>
	posit<nbits, es, bt> hypot(posit<nbits, es, bt> x, posit<nbits, es, bt> y, posit<nbits, es, bt> z) {
		using Posit = posit<nbits, es, bt>;
		if (x.isnar() || y.isnar() || z.isnar()) return Posit(SpecificValue::nar);
		// The squares are exact, their sum is formed at 2W bits and its root at 2W, then rounded to W:
		// within 1/2 ulp at W plus a few ulps at 2W of the exact value, inside the +-1 ulp at W that
		// evaluate() tests. The sum is exact unless the scales lie far apart, so an exact result, such
		// as hypot(1, 2, 2) = 3, comes out exact; with the scales far apart the result is not exact.
		// hypot(hypot(x, y), z) would round twice and could not form an exact result on a tie point.
		return posit_limbs::evaluate<Posit>([&](unsigned W) {
			const auto a = efloat_detail::exact_product(posit_limbs::to_efloat(x), posit_limbs::to_efloat(x));
			const auto b = efloat_detail::exact_product(posit_limbs::to_efloat(y), posit_limbs::to_efloat(y));
			const auto c = efloat_detail::exact_product(posit_limbs::to_efloat(z), posit_limbs::to_efloat(z));
			auto sum = a;
			sum.set_precision(2u * W);
			sum = sum + b;
			sum = sum + c;
			auto r = sqrt(sum);
			r.set_precision(W);
			return r;
		});
	}

	// the float and long double variants of the C library: one hypot for posits
	template<unsigned nbits, unsigned es, typename bt>
	posit<nbits, es, bt> hypotf(posit<nbits, es, bt> x, posit<nbits, es, bt> y) { return hypot(x, y); }

	template<unsigned nbits, unsigned es, typename bt>
	posit<nbits, es, bt> hypotl(posit<nbits, es, bt> x, posit<nbits, es, bt> y) { return hypot(x, y); }

}} // namespace sw::universal
