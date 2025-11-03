#pragma once
// minmax.hpp: minmax support for quad-double floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	inline qd_cascade min(const qd_cascade& x, const qd_cascade& y) {
	return qd_cascade(std::min(double(x), double(y)));
	}

	inline qd_cascade max(const qd_cascade& x, const qd_cascade& y) {
	    return qd_cascade(std::max(double(x), double(y)));
	}

}} // namespace sw::universal
