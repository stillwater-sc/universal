#pragma once
// logarithm.hpp: logarithm functions (log, log2, log10, log1p) for posits
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

	// log of 0 or of a negative value, and log1p of -1 or below, are NaR, which takes the place of
	// IEEE-754's -inf and NaN.

	// the natural logarithm
	template<unsigned nbits, unsigned es, typename bt>
	posit<nbits, es, bt> log(posit<nbits, es, bt> x) {
		using Posit = posit<nbits, es, bt>;
		if (x.isnar()) return x;
		return posit_limbs::evaluate<Posit>([&](unsigned W) { return log(posit_limbs::argument(x, W)); });
	}

	// the binary logarithm
	template<unsigned nbits, unsigned es, typename bt>
	posit<nbits, es, bt> log2(posit<nbits, es, bt> x) {
		using Posit = posit<nbits, es, bt>;
		if (x.isnar()) return x;
		return posit_limbs::evaluate<Posit>([&](unsigned W) { return log2(posit_limbs::argument(x, W)); });
	}

	// the common logarithm
	template<unsigned nbits, unsigned es, typename bt>
	posit<nbits, es, bt> log10(posit<nbits, es, bt> x) {
		using Posit = posit<nbits, es, bt>;
		if (x.isnar()) return x;
		return posit_limbs::evaluate<Posit>([&](unsigned W) { return log10(posit_limbs::argument(x, W)); });
	}

	// log(1 + x), accurate for x near 0
	template<unsigned nbits, unsigned es, typename bt>
	posit<nbits, es, bt> log1p(posit<nbits, es, bt> x) {
		using Posit = posit<nbits, es, bt>;
		if (x.isnar()) return x;
		return posit_limbs::evaluate<Posit>([&](unsigned W) { return log1p(posit_limbs::argument(x, W)); });
	}

}} // namespace sw::universal
