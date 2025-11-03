#pragma once
// fractional.hpp: fractional support for quad-double floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// fmod retuns x - n*y where n = x/y with the fractional part truncated
	inline qd_cascade fmod(const qd_cascade& x, const qd_cascade& y) {
		return qd_cascade(std::fmod(double(x), double(y)));
	}

	// shim to stdlib
    inline qd_cascade remainder(const qd_cascade& x, const qd_cascade& y) {
	    return qd_cascade(std::remainder(double(x), double(y)));
	}

}} // namespace sw::universal
