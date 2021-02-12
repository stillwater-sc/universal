#pragma once
// math_functions.hpp: definition of integer mathematical functions
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/integer/exceptions.hpp>

#if defined(__clang__)
/* Clang/LLVM. ---------------------------------------------- */


#elif defined(__ICC) || defined(__INTEL_COMPILER)
/* Intel ICC/ICPC. ------------------------------------------ */


#elif defined(__GNUC__) || defined(__GNUG__)
/* GNU GCC/G++. --------------------------------------------- */


#elif defined(__HP_cc) || defined(__HP_aCC)
/* Hewlett-Packard C/aC++. ---------------------------------- */

#elif defined(__IBMC__) || defined(__IBMCPP__)
/* IBM XL C/C++. -------------------------------------------- */

#elif defined(_MSC_VER)
/* Microsoft Visual Studio. --------------------------------- */


#elif defined(__PGI)
/* Portland Group PGCC/PGCPP. ------------------------------- */

#elif defined(__SUNPRO_C) || defined(__SUNPRO_CC)
/* Oracle Solaris Studio. ----------------------------------- */

#endif

namespace sw::universal {

// square root of an arbitrary integer
template<size_t nbits, typename BlockType>
integer<nbits, BlockType> sqrt(const integer<nbits, BlockType>& a) {
	if (a.iszero() || a.isone()) return a;
	if (a < 0) throw "negative argument to sqrt";

	return floor_sqrt(a);
}

// square root of an arbitrary integer does a binary search for the floor(sqrt(a))
template<size_t nbits, typename BlockType>
integer<nbits, BlockType> floor_sqrt(const integer<nbits, BlockType>& a) {
	if (a.iszero() || a.isone()) return a;
	if (a < 0) throw "negative argument to floor_sqrt";

	using Integer = integer<nbits, BlockType>;
	Integer start(1), end(a), v(a), root(0);
	while (start <= end) {
		Integer midpoint = start + (end - start) / 2;
//		std::cout << start << " : " << midpoint << " : " << end << " = " << root << std::endl;
		if (midpoint == v / midpoint) return midpoint;
		if (midpoint  < v / midpoint) {   // midpoint * midpoint can overflow badly hence the use of divide to stay in the numerical range of Integer
			start = midpoint + 1;
			root  = midpoint;
		}
		else {
			end = midpoint - 1;
		}
//		std::cout << start << " : " << midpoint << " : " << end << " = " << root << " <--- update" << std::endl;
	}
	return root; 
}

// square root of an arbitrary integer does a binary search for the ceil(sqrt(a))
template<size_t nbits, typename BlockType>
integer<nbits, BlockType> ceil_sqrt(const integer<nbits, BlockType>& a) {
	if (a.iszero() || a.isone()) return a;
	if (a < 0) throw "negative argument to ceil_sqrt";

	using Integer = integer<2*nbits, BlockType>;
	Integer start(1), end(a), v(a);
	integer<nbits, BlockType> root(0);
	while (start <= end) {
		Integer midpoint = start + (end - start) / 2;
//		std::cout << start << " : " << midpoint << " : " << end << " = " << root << std::endl;
		if (midpoint*midpoint == v) return midpoint;
		if (midpoint*midpoint < v) {   // midpoint * midpoint can overflow badly hence the use of 
			start = midpoint + 1;
			root.bitcopy(midpoint);
		}
		else {
			end = midpoint - 1;
		}
//		std::cout << start << " : " << midpoint << " : " << end << " = " << root << " <--- update" << std::endl;
	}
	++root;
	return root;
}

// test if the argument is a perfect square
template<size_t nbits, typename BlockType>
bool perfect_square(const integer<nbits, BlockType>& a) {
	using Integer = integer<nbits, BlockType>;
	Integer square = sqrt(a);
	return (a == square * square) ? true : false;
}

} // namespace sw::universal
