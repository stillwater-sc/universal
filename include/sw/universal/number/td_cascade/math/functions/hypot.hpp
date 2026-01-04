#pragma once
// hypot.hpp: hypot support for triple-double floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	inline td_cascade hypot(const td_cascade& x, const td_cascade& y) {
		return td_cascade(std::hypot(double(x), double(y)));
	}

}} // namespace sw::universal
