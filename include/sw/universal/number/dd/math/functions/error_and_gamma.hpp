#pragma once
// error_and_gamma.hpp: error and gamma functions of double-double floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Evaluated in the type itself (#1667): erf and erfc used to go through double, and tgamma and
// lgamma were missing.  See number/shared/multidouble_special.hpp for the algorithms.
#include <universal/number/shared/multidouble_special.hpp>

namespace sw { namespace universal {

	// erf(x) = 2 / sqrt(pi) times the integral from 0 to x of e^(-t^2) dt
	inline dd erf(const dd& x) { return multidouble::erf(x); }

	// erfc(x) = 1 - erf(x), relative-accurate where it is small
	inline dd erfc(const dd& x) { return multidouble::erfc(x); }

	// the logarithm of |Gamma(x)|; +inf at the poles
	inline dd lgamma(const dd& x) { return multidouble::lgamma(x); }

	// the gamma function; NaN at the poles
	inline dd tgamma(const dd& x) { return multidouble::tgamma(x); }

}} // namespace sw::universal
