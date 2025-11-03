#pragma once
// minmax.hpp: minmax support for triple-double floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	inline td_cascade min(const td_cascade& x, const td_cascade& y) {
	return td_cascade(std::min(double(x), double(y)));
	}

	inline td_cascade max(const td_cascade& x, const td_cascade& y) {
	    return td_cascade(std::max(double(x), double(y)));
	}

}} // namespace sw::universal
