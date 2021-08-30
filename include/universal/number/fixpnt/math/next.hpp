#pragma once
// next.hpp: nextafter/nexttoward functions for fixed-points
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw::universal {

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
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
fixpnt<nbits, es, arithmetic, bt> nextafter(fixpnt<nbits, rbits, arithmetic, bt> x, fixpnt<nbits, rbits, arithmetic, bt> target) {
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
		
template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
fixpnt<nbits, es, arithmetic, bt> nexttoward(fixpnt<nbits, rbits, arithmetic, bt> x, fixpnt<nbits, rbits, arithmetic, bt> target) {
	if (x == target) return x;
	if (target.isnar()) {
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

}  // namespace sw::universal
