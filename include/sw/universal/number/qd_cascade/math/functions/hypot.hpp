#pragma once
// hypot.hpp: hypot support for quad-double floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	inline qd_cascade hypot(const qd_cascade& x, const qd_cascade& y) {
		return qd_cascade(std::hypot(double(x), double(y)));
	}

}} // namespace sw::universal
