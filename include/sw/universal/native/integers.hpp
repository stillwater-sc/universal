#pragma once
// integers.hpp: manipulators for native integer types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstdint>
#include <universal/native/integer_core.hpp>   // nlz -- the I/O-free bit-manipulation half (#1334)
#include <sstream>
#include <string>
#include <type_traits>

namespace sw { namespace universal {

// fast power of 2 for integers
template<typename Integer,
	typename = typename std::enable_if< std::is_integral<Integer>::value, Integer >::type
>
inline Integer two_to_the_power(Integer n) {
	return (Integer(1) << n);
}

/// <summary>
/// efficient and fast integer power function
/// Note: uses unsigned arithmetic internally to avoid undefined behavior
/// on overflow. Overflow wraps as per unsigned integer semantics.
/// </summary>
/// <param name="base"></param>
/// <param name="exp"></param>
/// <returns></returns>
inline int64_t ipow(int64_t base, unsigned exp) {
	// Use unsigned arithmetic to avoid signed overflow UB
	uint64_t ubase = static_cast<uint64_t>(base);
	uint64_t result = 1;
	for (;;) {
		if (exp & 1)
			result *= ubase;
		exp >>= 1;
		if (!exp)
			break;
		ubase *= ubase;
	}

	return static_cast<int64_t>(result);
}

// super fast ipow, courtesy of 
// Orson Peters
// github: orlp
// location: Leiden, Netherlands 
// email: orsonpeters@gmail.com
inline int64_t fastipow(int64_t base, uint8_t exp) {
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
inline std::string to_binary(const Integer& number, bool bNibbleMarker = true, int nbits = 0) {
	std::stringstream s;
	if (nbits == 0) nbits = 8*sizeof(number);
	s << "0b";
	uint64_t mask = (1ull << (nbits - 1));
	for (int i = nbits - 1; i >= 0; --i) {
		s << ((number & mask) ? '1' : '0');
		if (bNibbleMarker && i > 0 && i % 4 == 0) s << '\'';
		mask >>= 1;
	}
	return s.str();
}

// generate a binary string for a native integer
	// optional nbits argument indicating the size of the integer, if 0 use full size of native type
template<typename Integer,
	typename = typename std::enable_if< std::is_integral<Integer>::value, Integer >::type
>
inline std::string to_hex(const Integer& number, bool nibbleMarker = false, bool hexPrefix = true) {
	std::stringstream s;
	uint64_t nbits = 8 * sizeof(number);
	char hex[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	if (hexPrefix) s << "0x";
	uint64_t mask = (1ull << (nbits - 1));
	uint64_t nibble{ 0 };
	unsigned nibbleIndex = static_cast<unsigned>(nbits / 4 - 1u);
	unsigned rightShift = nibbleIndex * 4;
	for (int i = static_cast<int>(nbits - 1u); i >= 0; --i) {
		nibble |= (mask & number);
		mask >>= 1;
		if ((i % 4) == 0) {
			nibble >>= rightShift;
			s << hex[nibble];
			rightShift -= 4;
			nibble = 0;
			if (nibbleMarker && nibbleIndex > 0 && nibbleIndex % 4 == 0) s << '\'';
			--nibbleIndex;
		}
	}
	return s.str();
}

// finding leading non-zeros


}} // namespace sw::universal
