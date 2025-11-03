#pragma once
// minmax.hpp: minmax support for double-double floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	inline dd_cascade min(const dd_cascade& x, const dd_cascade& y) {
	return dd_cascade(std::min(double(x), double(y)));
	}

	inline dd_cascade max(const dd_cascade& x, const dd_cascade& y) {
	    return dd_cascade(std::max(double(x), double(y)));
	}

}} // namespace sw::universal
