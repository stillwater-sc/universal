#pragma once
// truncate.hpp: truncate support for double-double floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// Truncate value by rounding toward zero, returning the nearest integral value that is not larger in magnitude than x
	inline dd_cascade trunc(const dd_cascade& x) {
		return dd_cascade(std::trunc(double(x)));
	}

	// Round to nearest: returns the integral value that is nearest to x, with halfway cases rounded away from zero
    inline dd_cascade round(const dd_cascade& x) {
	    return dd_cascade(std::round(double(x)));
	}

	// floor returns the largest integer value not greater than x
    inline dd_cascade floor(dd_cascade x) {
	    dd_cascade result;
	    result.high() = std::floor(x.high());
	    result.low()  = 0.0;
	    if (result.high() == x.high()) {
		    // high component is already an integer, check low component
		    result.low() = std::floor(x.low());
	    }
	    return result;
    }

    // ceil returns the smallest integer value not less than x
    inline dd_cascade ceil(dd_cascade x) {
	    dd_cascade result;
	    result.high() = std::ceil(x.high());
	    result.low()  = 0.0;
	    if (result.high() == x.high()) {
		    // high component is already an integer, check low component
		    result.low() = std::ceil(x.low());
	    }
	    return result;
    }

}} // namespace sw::universal
