#pragma once
// sqrt.hpp: sqrt functions for integers
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/integer/exceptions.hpp>

namespace sw { namespace universal {

	// square root of an arbitrary integer
	template<unsigned nbits, typename BlockType, IntegerNumberType NumberType>
	integer<nbits, BlockType, NumberType> sqrt(const integer<nbits, BlockType, NumberType>& a) {
		// if (a < 0) is very inefficient as it will do a diff between a and 0
		// instead, use the isneg() function which simply checks the sign bit
#if INTEGER_THROW_ARITHMETIC_EXCEPTION
		if (a.isneg()) {
			throw integer_negative_sqrt_arg();
		}
#else 
		if (a.isneg()) std::cerr << "integer_negative_sqrt_arg\n";
#endif
		if (a.iszero() || a.isone()) return a;
		return floor_sqrt(a);
	}

	///////////////////////////////////////////////////////////////////
	// specialized sqrt configurations

	// square root of an arbitrary integer does a binary search for the floor(sqrt(a))
	template<unsigned nbits, typename BlockType, IntegerNumberType NumberType>
	integer<nbits, BlockType, NumberType> floor_sqrt(const integer<nbits, BlockType, NumberType>& a) {
		// if (a < 0) is very inefficient as it will do a diff between a and 0
		// instead, use the isneg() function which simply checks the sign bit
#if INTEGER_THROW_ARITHMETIC_EXCEPTION
		if (a.isneg()) {
			throw integer_negative_sqrt_arg();
		}
#else 
		if (a.isneg()) std::cerr << "integer_negative_sqrt_arg\n";
#endif
		if (a.iszero() || a.isone()) return a;

		using Integer = integer<nbits, BlockType, NumberType>;
		Integer start(1), end(a), v(a), root(0);
		while (start <= end) {
			Integer midpoint = start + (end - start) / 2;
			//		std::cout << start << " : " << midpoint << " : " << end << " = " << root << '\n';
			if (midpoint == v / midpoint) return midpoint;
			if (midpoint < v / midpoint) {   // midpoint * midpoint can overflow badly hence the use of divide to stay in the numerical range of Integer
				start = midpoint + 1;
				root = midpoint;
			}
			else {
				end = midpoint - 1;
			}
			//		std::cout << start << " : " << midpoint << " : " << end << " = " << root << " <--- update" << '\n';
		}
		return root;
	}

	// square root of an arbitrary integer does a binary search for the ceil(sqrt(a))
	template<unsigned nbits, typename BlockType, IntegerNumberType NumberType>
	integer<nbits, BlockType, NumberType> ceil_sqrt(const integer<nbits, BlockType, NumberType>& a) {
		integer<nbits, BlockType, NumberType> c = floor_sqrt(a);
		if (c * c != a) ++c;
		return c;
	}

	// test if the argument is a perfect square
	template<unsigned nbits, typename BlockType, IntegerNumberType NumberType>
	bool perfect_square(const integer<nbits, BlockType, NumberType>& a) {
		using Integer = integer<nbits, BlockType, NumberType>;
		Integer square = sqrt(a);
		return (a == square * square) ? true : false;
	}

}} // namespace sw::universal
