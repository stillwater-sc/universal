#pragma once
// truncate.hpp: truncate support for double-double cascade floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/shared/multidouble_exact.hpp>

namespace sw { namespace universal {

	// The integer-valued functions are exact, computed on the limbs (#1667): they used to round
	// the argument to double, which above 2^53 gives a different integer and below it ignores a
	// fraction carried in the low limbs.

	// Truncate value by rounding toward zero, returning the nearest integral value that is not larger in magnitude than x
	inline dd_cascade trunc(const dd_cascade& x) { return multidouble::trunc(x); }

	// Round to nearest: returns the integral value that is nearest to x, with halfway cases rounded away from zero
	inline dd_cascade round(const dd_cascade& x) { return multidouble::round(x); }

	// floor returns the largest integer value not greater than x
	inline dd_cascade floor(const dd_cascade& x) { return multidouble::floor(x); }

	// ceil returns the smallest integer value not less than x
	inline dd_cascade ceil(const dd_cascade& x) { return multidouble::ceil(x); }

}} // namespace sw::universal
