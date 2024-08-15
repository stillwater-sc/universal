#pragma once
// fractional.hpp: fractional support for quad-double (qd) floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// fmod retuns x - n*y where n = x/y with the fractional part truncated
	qd fmod(const qd& x, const qd& y) {
		return qd(std::fmod(double(x), double(y)));
	}

	// shim to stdlib
	qd remainder(const qd& x, const qd& y) {
		return qd(std::remainder(double(x), double(y)));
	}

}} // namespace sw::universal
