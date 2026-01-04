#pragma once
// fractional.hpp: fractional support for triple-double floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// fmod retuns x - n*y where n = x/y with the fractional part truncated
	inline td_cascade fmod(const td_cascade& x, const td_cascade& y) {
		return td_cascade(std::fmod(double(x), double(y)));
	}

	// shim to stdlib
    inline td_cascade remainder(const td_cascade& x, const td_cascade& y) {
	    return td_cascade(std::remainder(double(x), double(y)));
	}

}} // namespace sw::universal
