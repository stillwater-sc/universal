#pragma once
// error_and_gamma.hpp: error/gamma function support for quad-double floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// Compute the error function erf(x) = 2 over sqrt(PI) times Integral from 0 to x of e ^ (-t)^2 dt
	inline qd_cascade erf(const qd_cascade& x) {
		return qd_cascade(std::erf(double(x[0])));
	}

	// Compute the complementary error function: 1 - erf(x)
    inline qd_cascade erfc(const qd_cascade& x) {
	    return qd_cascade(std::erfc(double(x[0])));
	}

}} // namespace sw::universal
