#pragma once
// next.hpp: nextafter/nexttoward functions for logarithmic floating-point
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

/*
Parameters
	x  Base value.
	t  Value toward which the return value is approximated.
If both parameters compare equal, the function returns t.

Return Value
	The next representable value after x in the direction of t.

	If x is the largest finite value representable in the type,
	and the result is infinite or not representable, an overflow range error occurs.

	If an overflow range error occurs:
	- And math_errhandling has MATH_ERRNO set: the global variable errno is set to ERANGE.
	- And math_errhandling has MATH_ERREXCEPT set: FE_OVERFLOW is raised.
	*/
template<size_t nbits, size_t rbits, typename bt, auto... xtra>
lns<nbits, rbits, bt, xtra...> nextafter(lns<nbits, rbits, bt, xtra...> x, lns<nbits, rbits, bt, xtra...> target) {
	if (x == target) return target;
	if (target.isnan()) {
		if (x.isneg()) {
			--x;
		}
		else {
			++x;
		}
	}
	else {
		if (x > target) {
			--x;
		}
		else {
			++x;
		}
	}
	return x;
}
		
template<size_t nbits, size_t rbits, typename bt, auto... xtra>
lns<nbits, rbits, bt, xtra...> nexttoward(lns<nbits, rbits, bt, xtra...> x, lns<128, 15, bt, xtra...> target) {
	lns<128, 15, bt, xtra...> _x(x);
	if (_x == target) return x;
	if (target.isnan()) {
		if (_x.isneg()) {
			--_x;
		}
		else {
			++_x;
		}
	}
	else {
		if (_x > target) {
			--_x;
		}
		else {
			++_x;
		}
	}
	x = _x;
	return x;
}

}} // namespace sw::universal
