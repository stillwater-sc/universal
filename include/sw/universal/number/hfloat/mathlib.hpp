#pragma once
// mathlib.hpp: definition of mathematical functions for the IBM System/360 hexadecimal floating-point hfloats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <universal/number/hfloat/math/functions/classify.hpp>
#include <universal/number/hfloat/math/functions/error_and_gamma.hpp>
#include <universal/number/hfloat/math/functions/exponent.hpp>
#include <universal/number/hfloat/math/functions/fractional.hpp>
#include <universal/number/hfloat/math/functions/hyperbolic.hpp>
#include <universal/number/hfloat/math/functions/hypot.hpp>
#include <universal/number/hfloat/math/functions/logarithm.hpp>
#include <universal/number/hfloat/math/functions/minmax.hpp>
#include <universal/number/hfloat/math/functions/next.hpp>
#include <universal/number/hfloat/math/functions/pow.hpp>
#include <universal/number/hfloat/math/functions/sqrt.hpp>
#include <universal/number/hfloat/math/functions/trigonometry.hpp>
#include <universal/number/hfloat/math/functions/truncate.hpp>

namespace sw { namespace universal {

	////////////////////////////////////////////////////////////////////////

	// copysign returns a value with the magnitude of a, and the sign of b
	template<unsigned ndigits, unsigned es, typename bt>
	inline hfloat<ndigits, es, bt> copysign(const hfloat<ndigits, es, bt>& a, const hfloat<ndigits, es, bt>& b) {
		hfloat<ndigits, es, bt> c(a);
		if (a.sign() == b.sign()) return c;
		return -c;
	}

}} // namespace sw::universal
