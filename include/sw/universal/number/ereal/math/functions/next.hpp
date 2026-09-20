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
	template<unsigned maxlimbs, typename FpType>
	inline ereal<maxlimbs, FpType> nextafter(const ereal<maxlimbs, FpType>& x, const ereal<maxlimbs, FpType>& y) {
		if (x == y) return y;
	
		if (x.isnan() || y.isnan()) {
			// if either is NaN, return NaN
			return ereal<maxlimbs, FpType>(std::numeric_limits<double>::quiet_NaN());
	    }
		
		// find the smallest limb, and move in the direction of y
	    ereal<maxlimbs, FpType> n{x};
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
	template<unsigned maxlimbs, typename FpType>
	inline ereal<maxlimbs, FpType> nexttoward(const ereal<maxlimbs, FpType>& x, long double y) {
#if LONG_DOUBLE_SUPPORT
	    ereal<maxlimbs, FpType> target(y);
#else
	    ereal<maxlimbs, FpType> target(static_cast<double>(y));
#endif
		return nextafter(x, target);
	}

}} // namespace sw::universal
