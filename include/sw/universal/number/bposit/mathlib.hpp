#pragma once
// mathlib.hpp: the elementary functions of the bounded posit (#1653)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Every function is found by ADL on bposit<nbits, rs, es, bt>, so generic code that
// calls sin(x) or sqrt(x) on its value type works for b-posits.  Phase 1 evaluates the
// functions in double and rounds once: exact for classification, nextafter, min/max,
// fabs and copysign, and for the truncations and remainders whenever the argument
// converts exactly to double, but NOT correctly rounded for the transcendental
// functions.  math/shim.hpp states the guarantee and what it covers.
#include <universal/number/bposit/math/shim.hpp>
#include <universal/number/bposit/math/classify.hpp>
#include <universal/number/bposit/math/complex.hpp>
#include <universal/number/bposit/math/error_and_gamma.hpp>
#include <universal/number/bposit/math/exponent.hpp>
#include <universal/number/bposit/math/fractional.hpp>
#include <universal/number/bposit/math/hyperbolic.hpp>
#include <universal/number/bposit/math/hypot.hpp>
#include <universal/number/bposit/math/logarithm.hpp>
#include <universal/number/bposit/math/minmax.hpp>
#include <universal/number/bposit/math/next.hpp>
#include <universal/number/bposit/math/pow.hpp>
#include <universal/number/bposit/math/sqrt.hpp>
#include <universal/number/bposit/math/trigonometry.hpp>
#include <universal/number/bposit/math/truncate.hpp>

namespace sw { namespace universal {

// the magnitude of x; NaR stays NaR
template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> fabs(const bposit<nbits, rs, es, bt>& x) { return abs(x); }

// the magnitude of a with the sign of b; NaR in either gives NaR, and zero has no sign
template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> copysign(const bposit<nbits, rs, es, bt>& a, const bposit<nbits, rs, es, bt>& b) {
	if (a.isnar() || b.isnar()) return bposit<nbits, rs, es, bt>(SpecificValue::nar);
	const bposit<nbits, rs, es, bt> m = abs(a);
	return b.isneg() ? -m : m;
}

}} // namespace sw::universal
