#pragma once
// next.hpp: nextafter/nexttoward functions for fixed-points
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
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
// fixpnt has no NaN, so the direction is the comparison alone (#2111: nexttoward called an isnar()
// fixpnt lacks). The step is one ulp of the encoding; past maxpos or maxneg it follows ++ and --.
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
fixpnt<nbits, rbits, arithmetic, bt> nextafter(fixpnt<nbits, rbits, arithmetic, bt> x, fixpnt<nbits, rbits, arithmetic, bt> target) {
	if (x == target) return target;
	if (x > target) --x; else ++x;
	return x;
}

template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
fixpnt<nbits, rbits, arithmetic, bt> nexttoward(fixpnt<nbits, rbits, arithmetic, bt> x, fixpnt<nbits, rbits, arithmetic, bt> target) {
	return nextafter(x, target);
}

}} // namespace sw::universal
