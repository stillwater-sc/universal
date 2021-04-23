#pragma once
// integers.hpp: manipulators for native integer types
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <sstream>
#include <string>
#include <type_traits>

namespace sw::universal {

// fast power of 2 for integers
template<typename Integer,
	typename = typename std::enable_if< std::is_integral<Integer>::value, Integer >::type
>
inline Integer two_to_the_power(Integer n) {
	return (Integer(1) << n);
}

/// <summary>
/// efficient and fast integer power function
/// </summary>
/// <param name="base"></param>
/// <param name="exp"></param>
/// <returns></returns>
int64_t ipow(int64_t base, unsigned exp) {
	int64_t result = 1;
	for (;;) {
		if (exp & 1)
			result *= base;
		exp >>= 1;
		if (!exp)
			break;
		base *= base;
	}

	return result;
}

// super fast ipow, courtesy of 
// Orson Peters
// github: orlp
// location: Leiden, Netherlands 
// email: orsonpeters@gmail.com
int64_t fastipow(int64_t base, uint8_t exp) {
	static const uint8_t highest_bit_set[] = {
		0, 1, 2, 2, 3, 3, 3, 3,
		4, 4, 4, 4, 4, 4, 4, 4,
		5, 5, 5, 5, 5, 5, 5, 5,
		5, 5, 5, 5, 5, 5, 5, 5,
		6, 6, 6, 6, 6, 6, 6, 6,
		6, 6, 6, 6, 6, 6, 6, 6,
		6, 6, 6, 6, 6, 6, 6, 6,
		6, 6, 6, 6, 6, 6, 6, 255, // anything past 63 is a guaranteed overflow with base > 1
		255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255,
	};

	int64_t result = 1;

	switch (highest_bit_set[exp]) {
	case 255: // we use 255 as an overflow marker and return 0 on overflow/underflow
		if (base == 1) {
			return 1;
		}

		if (base == -1) {
			return 1ll - 2ll * static_cast<int64_t>(exp & 0x01u);
		}

		return 0;
	case 6:
		if (exp & 1) result *= base;
		exp >>= 1;
		base *= base;
#if defined(_MSC_VER)
		[[fallthrough]]; // fallthrough is explicit
#endif
	case 5:
		if (exp & 1) result *= base;
		exp >>= 1;
		base *= base;
#if defined(_MSC_VER)
		[[fallthrough]]; // fallthrough is explicit
#endif
	case 4:
		if (exp & 1) result *= base;
		exp >>= 1;
		base *= base;
#if defined(_MSC_VER)
		[[fallthrough]]; // fallthrough is explicit
#endif
	case 3:
		if (exp & 1) result *= base;
		exp >>= 1;
		base *= base;
#if defined(_MSC_VER)
		[[fallthrough]]; // fallthrough is explicit
#endif
	case 2:
		if (exp & 1) result *= base;
		exp >>= 1;
		base *= base;
#if defined(_MSC_VER)
		[[fallthrough]]; // fallthrough is explicit
#endif
	case 1:
		if (exp & 1) result *= base;
#if defined(_MSC_VER)
		[[fallthrough]]; // fallthrough is explicit
#endif
	default:
		return result;
	}
}

////////////////// string operators

// generate a binary string for a native integer
	// optional nbits argument indicating the size of the integer, if 0 use full size of native type
template<typename Integer,
         typename = typename std::enable_if< std::is_integral<Integer>::value, Integer >::type
>
inline std::string to_binary(const Integer& number, int nbits = 0, bool bNibbleMarker = true) {
	std::stringstream ss;
	if (nbits == 0) nbits = 8*sizeof(number);
	uint64_t mask = (uint64_t(1) << (nbits - 1));
	for (int i = int(nbits) - 1; i >= 0; --i) {
		ss << ((number & mask) ? '1' : '0');
		if (bNibbleMarker && i > 0 && i % 4 == 0) ss << '\'';
		mask >>= 1;
	}
	return ss.str();
}

} // namespace sw::universal
