#pragma once
// truncate.hpp: truncation functions (trunc, round, floor, and ceil) for posits
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// the current shims are NON-COMPLIANT with the posit standard, which says that every function must be
// correctly rounded for every input value. Anything less sacrifices bitwise reproducibility of results.

// Truncate value by rounding toward zero, returning the nearest integral value that is not larger in magnitude than x
template<unsigned nbits, unsigned es, typename bt>
posit<nbits,es,bt> trunc(posit<nbits,es,bt> x) {
	return posit<nbits,es,bt>(std::trunc(double(x)));
}

// Round to nearest: returns the integral value that is nearest to x, with halfway cases rounded away from zero
template<unsigned nbits, unsigned es, typename bt>
posit<nbits,es,bt> round(posit<nbits,es,bt> x) {
	return posit<nbits,es,bt>(std::round(double(x)));
}

// Round x downward, returning the largest integral value that is not greater than x
template<unsigned nbits, unsigned es, typename bt>
posit<nbits,es,bt> floor(posit<nbits,es,bt> x) {
	return posit<nbits,es,bt>(std::floor(double(x)));
}

// Round x upward, returning the smallest integral value that is greater than x
template<unsigned nbits, unsigned es, typename bt>
posit<nbits,es,bt> ceil(posit<nbits,es,bt> x) {
	return posit<nbits,es,bt>(std::ceil(double(x)));
}

#ifdef NOW
by design, non compilable text;
this is the libc algorithm for floor(double)

static double huge = 1.0e300;

// TBD, defined to compile, not functionally correct
#define __HI(x) x
#define __LO(x) x
// algorithm for floor(double)
template<unsigned nbits, unsigned es, typename bt>
posit<nbits, es, bt> floor(posit<nbits, es, bt> x) {
	int i0, i1, j0;
	unsigned i, j;
	i0 = __HI(x);
	i1 = __LO(x);
	j0 = ((i0 >> 20) & 0x7ff) - 0x3ff;
	if (j0 < 20) {
		if (j0 < 0) { 	/* raise inexact if x != 0 */
			if (huge + x > 0.0) {/* return 0*sign(x) if |x|<1 */
				if (i0 >= 0) { i0 = i1 = 0; }
				else if (((i0 & 0x7fffffff) | i1) != 0)
				{
					i0 = 0xbff00000; i1 = 0;
				}
			}
		}
		else {
			i = (0x000fffff) >> j0;
			if (((i0&i) | i1) == 0) return x; /* x is integral */
			if (huge + x > 0.0) {	/* raise inexact flag */
				if (i0 < 0) i0 += (0x00100000) >> j0;
				i0 &= (~i); i1 = 0;
			}
		}
	}
	else if (j0 > 51) {
		if (j0 == 0x400) return x + x;	/* inf or NaN */
		else return x;		/* x is integral */
	}
	else {
		i = ((unsigned)(0xffffffff)) >> (j0 - 20);
		if ((i1&i) == 0) return x;	/* x is integral */
		if (huge + x > 0.0) { 		/* raise inexact flag */
			if (i0 < 0) {
				if (j0 == 20) i0 += 1;
				else {
					j = i1 + (1 << (52 - j0));
					if (j < i1) i0 += 1; 	/* got a carry */
					i1 = j;
				}
			}
			i1 &= (~i);
		}
	}
	__HI(x) = i0;
	__LO(x) = i1;
	return x;
}
#endif //NOW

}} // namespace sw::universal
