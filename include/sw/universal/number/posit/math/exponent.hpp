#pragma once
// exponent.hpp: exponential functions (exp, exp2, exp10, expm1) for posits
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

	// Past the posit's range the result saturates at maxpos, and below it at minpos: a posit rounds no
	// nonzero value to 0. Far enough out efloat's own range ends too (2^(2^62)): its inf and 0 are
	// then an overflow and an underflow, which saturate as well (Saturation::both).

	// e^x
	template<unsigned nbits, unsigned es, typename bt>
	posit<nbits, es, bt> exp(posit<nbits, es, bt> x) {
		using Posit = posit<nbits, es, bt>;
		if (x.isnar()) return x;
		return posit_limbs::evaluate<Posit>([&](unsigned W) { return exp(posit_limbs::argument(x, W)); }, posit_limbs::Saturation::both);
	}

	// 2^x
	template<unsigned nbits, unsigned es, typename bt>
	posit<nbits, es, bt> exp2(posit<nbits, es, bt> x) {
		using Posit = posit<nbits, es, bt>;
		if (x.isnar()) return x;
		return posit_limbs::evaluate<Posit>([&](unsigned W) { return exp2(posit_limbs::argument(x, W)); }, posit_limbs::Saturation::both);
	}

	// 10^x
	template<unsigned nbits, unsigned es, typename bt>
	posit<nbits, es, bt> exp10(posit<nbits, es, bt> x) {
		using Posit = posit<nbits, es, bt>;
		if (x.isnar()) return x;
		return posit_limbs::evaluate<Posit>([&](unsigned W) { return exp10(posit_limbs::argument(x, W)); }, posit_limbs::Saturation::both);
	}

	// e^x - 1, accurate for x near 0
	template<unsigned nbits, unsigned es, typename bt>
	posit<nbits, es, bt> expm1(posit<nbits, es, bt> x) {
		using Posit = posit<nbits, es, bt>;
		if (x.isnar()) return x;
		// expm1 > -1: only an overflow leaves the range
		return posit_limbs::evaluate<Posit>([&](unsigned W) { return expm1(posit_limbs::argument(x, W)); }, posit_limbs::Saturation::overflow);
	}

}} // namespace sw::universal
