#pragma once
// fractional.hpp: fractional functions (fmod, remainder, frac) for posits
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// fmod and remainder are correctly rounded at any nbits (#1855). In IEEE-754 arithmetic their results
// are always exact, but a posit's precision tapers away from 1, so x - n y, a multiple of the finer of
// the two operands' last bits, need not fit the posit at its own scale. Both are computed exactly in
// efloat, whose fmod and remainder are exact for any quotient (#1783), and rounded once into the
// posit. frac(x) = x - trunc(x) is exact: its bits are x's bits below the binary point, and a posit
// below 1 has at least as many fraction bits at that scale as x had.
#include <universal/number/posit/math/efloat_bridge.hpp>
#include <universal/number/efloat/math/fractional.hpp>   // efloat's exact fmod and remainder
#include <universal/number/posit/math/truncate.hpp>

namespace sw { namespace universal {

	// x - n y with n = trunc(x / y), the sign of x; NaR when either operand is NaR or y is 0
	template<unsigned nbits, unsigned es, typename bt>
	posit<nbits,es,bt> fmod(posit<nbits,es,bt> x, posit<nbits,es,bt> y) {
		using Posit = posit<nbits, es, bt>;
		if (x.isnar() || y.isnar() || y.iszero()) return Posit(SpecificValue::nar);
		if (x.iszero()) return x;
		Posit r;
		return posit_limbs::from_efloat(r, fmod(posit_limbs::to_efloat(x), posit_limbs::to_efloat(y)));
	}

	// x - n y with n the integer nearest x / y, ties to even; NaR when either operand is NaR or y is 0
	template<unsigned nbits, unsigned es, typename bt>
	posit<nbits,es,bt> remainder(posit<nbits,es,bt> x, posit<nbits,es,bt> y) {
		using Posit = posit<nbits, es, bt>;
		if (x.isnar() || y.isnar() || y.iszero()) return Posit(SpecificValue::nar);
		if (x.iszero()) return x;
		Posit r;
		return posit_limbs::from_efloat(r, remainder(posit_limbs::to_efloat(x), posit_limbs::to_efloat(y)));
	}

	// the fractional part, x - trunc(x), with the sign of x
	template<unsigned nbits, unsigned es, typename bt>
	posit<nbits,es,bt> frac(posit<nbits,es,bt> x) {
		if (x.isnar()) return x;
		return x - trunc(x);
	}

}} // namespace sw::universal
