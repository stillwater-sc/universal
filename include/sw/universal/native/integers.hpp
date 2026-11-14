#pragma once
// integers.hpp: manipulators for native integer types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstdint>
#include <limits>
#include <type_traits>
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
	// computed in the unsigned domain, with overflow detected before each multiply: the signed
	// squarings overflowed (UB) for 3^41, and (-2)^63 = INT64_MIN came out 0 (#2059). An overflow
	// returns 0, as it always promised; 0^0 is 1.
	const bool negative = (base < 0) && (exp & 1u);
	std::uint64_t b = (base < 0) ? (std::uint64_t(0) - static_cast<std::uint64_t>(base)) : static_cast<std::uint64_t>(base);
	std::uint64_t result = 1;
	constexpr std::uint64_t umax = ~std::uint64_t(0);
	unsigned e = exp;
	while (e > 0) {
		if (e & 1u) {
			if (b != 0 && result > umax / b) return 0;
			result *= b;
		}
		e >>= 1;
		if (e > 0) {
			if (b > 1 && b > umax / b) return 0;    // the square is needed by a later bit, and overflows
			b *= b;
		}
	}
	constexpr std::uint64_t limit = std::uint64_t(1) << 63;    // |INT64_MIN|
	if (negative) {
		if (result > limit) return 0;
		return (result == limit) ? std::numeric_limits<int64_t>::min() : -static_cast<int64_t>(result);
	}
	if (result >= limit) return 0;
	return static_cast<int64_t>(result);
}

////////////////// string operators

// generate a binary string for a native integer
	// optional nbits argument indicating the size of the integer, if 0 use full size of native type
template<typename Integer,
         typename = typename std::enable_if< std::is_integral<Integer>::value, Integer >::type
>
inline std::string to_binary(const Integer& number, bool bNibbleMarker = true, int nbits = 0) {
	// bit by bit through the unsigned type, so nbits beyond 64 (or the type) is no shift past the
	// width: those bits are the sign extension (#2059)
	// bool has no make_unsigned: read it as the unsigned char of the same width
	using U = std::conditional_t<std::is_same_v<Integer, bool>, unsigned char, std::make_unsigned_t<std::conditional_t<std::is_same_v<Integer, bool>, unsigned char, Integer>>>;
	constexpr int width = static_cast<int>(8 * sizeof(Integer));
	const U u = static_cast<U>(number);
	const bool negative = std::is_signed_v<Integer> && !std::is_same_v<Integer, bool> && (number < Integer(0));
	std::stringstream s;
	if (nbits <= 0) nbits = width;
	s << "0b";
	for (int i = nbits - 1; i >= 0; --i) {
		const bool bit = (i < width) ? (((u >> i) & U(1)) != 0) : negative;
		s << (bit ? '1' : '0');
		if (bNibbleMarker && i > 0 && i % 4 == 0) s << '\'';
	}
	return s.str();
}

// generate a binary string for a native integer
	// optional nbits argument indicating the size of the integer, if 0 use full size of native type
template<typename Integer,
	typename = typename std::enable_if< std::is_integral<Integer>::value, Integer >::type
>
inline std::string to_hex(const Integer& number, bool nibbleMarker = false, bool hexPrefix = true) {
	// nibble by nibble through the unsigned type: a 1ull mask shifted by the width went past 64
	// bits for a 128-bit integer (#2059)
	using U = std::conditional_t<std::is_same_v<Integer, bool>, unsigned char, std::make_unsigned_t<std::conditional_t<std::is_same_v<Integer, bool>, unsigned char, Integer>>>;
	constexpr unsigned nibbles = static_cast<unsigned>(2 * sizeof(Integer));
	const char hex[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	const U u = static_cast<U>(number);
	std::stringstream s;
	if (hexPrefix) s << "0x";
	for (unsigned n = nibbles; n-- > 0;) {
		s << hex[static_cast<unsigned>((u >> (4 * n)) & U(0xF))];
		if (nibbleMarker && n > 0 && n % 4 == 0) s << '\'';
	}
	return s.str();
}

// finding leading non-zeros


}} // namespace sw::universal
