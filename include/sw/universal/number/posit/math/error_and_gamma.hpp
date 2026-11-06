#pragma once
// error_and_gamma.hpp: the error and gamma functions (erf, erfc, tgamma, lgamma) for posits
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Correctly rounded at every nbits (#1855): computed by efloat's correctly rounded math library and
// rounded once into the posit by posit_limbs::evaluate, which applies Ziv's test on the posit side.
// No native floating-point type is involved, so the posit's whole precision and range hold.
// tgamma and lgamma have poles at 0 and the negative integers, where they are NaR; tgamma and erfc are
// never 0 and saturate at minpos past the posit's range, tgamma and lgamma at maxpos.
#include <universal/number/posit/math/evaluate.hpp>

namespace sw { namespace universal {

	// the error function
	template<unsigned nbits, unsigned es, typename bt>
	posit<nbits, es, bt> erf(posit<nbits, es, bt> x) {
		using Posit = posit<nbits, es, bt>;
		if (x.isnar()) return x;
		return posit_limbs::evaluate<Posit>([&](unsigned W) { return erf(posit_limbs::argument(x, W)); });
	}

	// the complementary error function, 1 - erf(x)
	template<unsigned nbits, unsigned es, typename bt>
	posit<nbits, es, bt> erfc(posit<nbits, es, bt> x) {
		using Posit = posit<nbits, es, bt>;
		if (x.isnar()) return x;
		return posit_limbs::evaluate<Posit>([&](unsigned W) { return erfc(posit_limbs::argument(x, W)); }, posit_limbs::Saturation::both);
	}

	// the gamma function
	template<unsigned nbits, unsigned es, typename bt>
	posit<nbits, es, bt> tgamma(posit<nbits, es, bt> x) {
		using Posit = posit<nbits, es, bt>;
		if (x.isnar()) return x;
		if (x.isinteger() && (x.iszero() || x.isneg())) return Posit(SpecificValue::nar);   // a pole
		return posit_limbs::evaluate<Posit>([&](unsigned W) { return tgamma(posit_limbs::argument(x, W)); }, posit_limbs::Saturation::both);
	}

	// the logarithm of |gamma(x)|
	template<unsigned nbits, unsigned es, typename bt>
	posit<nbits, es, bt> lgamma(posit<nbits, es, bt> x) {
		using Posit = posit<nbits, es, bt>;
		if (x.isnar()) return x;
		if (x.isinteger() && (x.iszero() || x.isneg())) return Posit(SpecificValue::nar);   // a pole
		return posit_limbs::evaluate<Posit>([&](unsigned W) { return lgamma(posit_limbs::argument(x, W)); }, posit_limbs::Saturation::overflow);
	}

}} // namespace sw::universal
