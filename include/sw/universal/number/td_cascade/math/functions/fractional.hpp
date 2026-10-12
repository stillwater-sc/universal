#pragma once
// fractional.hpp: fractional support for triple-double cascade floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/shared/multidouble_exact.hpp>

namespace sw { namespace universal {

	// Both are exact (#1667): the remainder of x and y themselves, not of their double roundings,
	// reduced in an exact expansion whatever the size of x / y, and rounded once.

	// fmod returns x - n*y where n = x/y with the fractional part truncated
	inline td_cascade fmod(const td_cascade& x, const td_cascade& y) { return multidouble::fmod(x, y); }

	// remainder returns x - n*y where n = x/y rounded to the nearest integer, halfway cases to even
	inline td_cascade remainder(const td_cascade& x, const td_cascade& y) { return multidouble::remainder(x, y); }

}} // namespace sw::universal
