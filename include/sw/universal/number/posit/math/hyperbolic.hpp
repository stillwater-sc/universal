#pragma once
// hyperbolic.hpp: hyperbolic functions (sinh, cosh, tanh, asinh, acosh, atanh) for posits
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Correctly rounded at every nbits (#1855): computed by efloat's correctly rounded math library and
// rounded once into the posit by posit_limbs::evaluate, which applies Ziv's test on the posit side.
// No native floating-point type is involved, so the posit's whole precision and range hold.
// sinh and cosh saturate past the posit's range; acosh of x < 1 and atanh of |x| > 1 are NaR, and
// atanh(+-1), a pole, is NaR.
#include <universal/number/posit/math/evaluate.hpp>

namespace sw { namespace universal {

	// the hyperbolic sine
	template<unsigned nbits, unsigned es, typename bt>
	posit<nbits, es, bt> sinh(posit<nbits, es, bt> x) {
		using Posit = posit<nbits, es, bt>;
		if (x.isnar()) return x;
		return posit_limbs::evaluate<Posit>([&](unsigned W) { return sinh(posit_limbs::argument(x, W)); }, posit_limbs::Saturation::overflow);
	}

	// the hyperbolic cosine
	template<unsigned nbits, unsigned es, typename bt>
	posit<nbits, es, bt> cosh(posit<nbits, es, bt> x) {
		using Posit = posit<nbits, es, bt>;
		if (x.isnar()) return x;
		return posit_limbs::evaluate<Posit>([&](unsigned W) { return cosh(posit_limbs::argument(x, W)); }, posit_limbs::Saturation::overflow);
	}

	// the hyperbolic tangent
	template<unsigned nbits, unsigned es, typename bt>
	posit<nbits, es, bt> tanh(posit<nbits, es, bt> x) {
		using Posit = posit<nbits, es, bt>;
		if (x.isnar()) return x;
		return posit_limbs::evaluate<Posit>([&](unsigned W) { return tanh(posit_limbs::argument(x, W)); });
	}

	// the inverse hyperbolic sine
	template<unsigned nbits, unsigned es, typename bt>
	posit<nbits, es, bt> asinh(posit<nbits, es, bt> x) {
		using Posit = posit<nbits, es, bt>;
		if (x.isnar()) return x;
		return posit_limbs::evaluate<Posit>([&](unsigned W) { return asinh(posit_limbs::argument(x, W)); });
	}

	// the inverse hyperbolic cosine
	template<unsigned nbits, unsigned es, typename bt>
	posit<nbits, es, bt> acosh(posit<nbits, es, bt> x) {
		using Posit = posit<nbits, es, bt>;
		if (x.isnar()) return x;
		return posit_limbs::evaluate<Posit>([&](unsigned W) { return acosh(posit_limbs::argument(x, W)); });
	}

	// the inverse hyperbolic tangent
	template<unsigned nbits, unsigned es, typename bt>
	posit<nbits, es, bt> atanh(posit<nbits, es, bt> x) {
		using Posit = posit<nbits, es, bt>;
		if (x.isnar()) return x;
		return posit_limbs::evaluate<Posit>([&](unsigned W) { return atanh(posit_limbs::argument(x, W)); });
	}

}} // namespace sw::universal
