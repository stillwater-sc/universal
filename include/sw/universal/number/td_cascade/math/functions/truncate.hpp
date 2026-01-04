#pragma once
// truncate.hpp: truncate support for triple-double floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// Truncate value by rounding toward zero, returning the nearest integral value that is not larger in magnitude than x
	inline td_cascade trunc(const td_cascade& x) {
		return td_cascade(std::trunc(double(x)));
	}

	// Round to nearest: returns the integral value that is nearest to x, with halfway cases rounded away from zero
    inline td_cascade round(const td_cascade& x) {
	    return td_cascade(std::round(double(x)));
	}

	// floor returns the largest integer value not greater than x
    inline td_cascade floor(td_cascade x) {
	    td_cascade result;
	    result[0] = std::floor(x[0]);
	    result[1] = 0.0;
	    result[2] = 0.0;
	    if (result[0] == x[0]) {
		    // high component is already an integer, check other components
		    result[1] = std::floor(x[1]);
		    if (result[1] == x[1]) {
			    result[2] = std::floor(x[2]);
		    }
	    }
	    return result;
    }

    // ceil returns the smallest integer value not less than x
    inline td_cascade ceil(td_cascade x) {
	    td_cascade result;
	    result[0] = std::ceil(x[0]);
	    result[1] = 0.0;
	    result[2] = 0.0;
	    if (result[0] == x[0]) {
		    // high component is already an integer, check other components
		    result[1] = std::ceil(x[1]);
		    if (result[1] == x[1]) {
			    result[2] = std::ceil(x[2]);
		    }
	    }
	    return result;
    }

}} // namespace sw::universal
