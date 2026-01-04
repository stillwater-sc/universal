#pragma once
// hypot.hpp: hypot support for double-double floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	inline dd_cascade hypot(const dd_cascade& x, const dd_cascade& y) {
		return dd_cascade(std::hypot(double(x), double(y)));
	}

}} // namespace sw::universal
