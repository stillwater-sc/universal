#pragma once
// hypot.hpp: hypot support for quad-double (qd) floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	inline qd hypot(const qd& x, const qd& y) {
		return qd(std::hypot(double(x), double(y)));
	}

}} // namespace sw::universal
