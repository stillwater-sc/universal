#pragma once
// next.hpp: nextafter/nexttoward functions for ereal adaptive-precision floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// nextafter: return next representable value after x in direction of y

	//   Note: For adaptive precision, "next" may involve adding a small limb
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> nextafter(const ereal<maxlimbs>& x, const ereal<maxlimbs>& y) {
		if (x == y) return y;
	
		if (x.isnan() || y.isnan()) {
			// if either is NaN, return NaN
			return ereal<maxlimbs>(std::numeric_limits<double>::quiet_NaN());
	    }
		
		// find the smallest limb, and move in the direction of y
	    ereal<maxlimbs> n{x};
	    assert(n.limbs().size() > 0);
	    size_t          last = n.limbs().size() - 1;
	    if (x < y) {
		    // move up
		    n[last] = std::nextafter(x.limbs().back(), +INFINITY);
	    } else {
			// move down
		    n[last] = std::nextafter(x.limbs().back(), -INFINITY);
	    }
		return n;
	}

	// nexttoward: return next representable value after x in direction of y (long double)
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> nexttoward(const ereal<maxlimbs>& x, long double y) {
#ifdef LONG_DOUBLE_SUPPORT
	    ereal<maxlimbs> target(y);
#else
	    ereal<maxlimbs> target(static_cast<double>(y));
#endif
		return nextafter(x, target);
	}

}} // namespace sw::universal
