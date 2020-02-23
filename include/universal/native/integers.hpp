#pragma once
// integers.hpp: manipulators for native integer types
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <sstream>
#include <string>
#include <type_traits>
#include "bit_functions.hpp"   // findMostSignificantBit

namespace sw {
namespace unum {

////////////////// string operators

// generate a binary string for a native integer
	// optional nbits argument indicating the size of the integer, if 0 use full size of native type
template<typename Integer,
         typename = typename std::enable_if< std::is_integral<Integer>::value, Integer >::type
>
inline std::string to_binary(const Integer& number, int nbits = 0) {
	std::stringstream ss;
	if (nbits == 0) nbits = 8*sizeof(number);
	uint64_t mask = (uint64_t(1) << (nbits - 1));
	for (int i = int(nbits) - 1; i >= 0; --i) {
		ss << ((number & mask) ? '1' : '0');
		mask >>= 1;
	}
	return ss.str();
}

/*
 * DEPRECATED
 *
// nbits binary representation of a signed 64-bit number
template<size_t nbits>
std::string to_binary_(long long int number) {
	std::stringstream ss;
	uint64_t mask = uint64_t(1) << (nbits - 1);                // parenthesis to avoid clang's prio warning
	for (int i = nbits - 1; i >= 0; --i) {
		ss << (mask & number ? "1" : "0");
		mask >>= 1;
	}
	return ss.str();
}

// full binary representation of a signed 64-bit number
inline std::string to_binary(long long int number) {
	std::stringstream ss;
	unsigned int msb = findMostSignificantBit(number);
	if (msb == 0) {
		ss << "-";
	}
	else {
		uint64_t mask = (uint64_t(1) << msb);
		for (int i = msb; i >= 0; --i) {
			ss << (mask & number ? "1" : "0");
			mask >>= 1;
		}
	}
	return ss.str();
}
// full binary representation of a unsigned 64-bit number
inline std::string to_binary(unsigned long long number) {
	std::stringstream ss;
	unsigned int msb = findMostSignificantBit(number);
	if (msb == 0) {
		ss << "-";
	}
	else {
		uint64_t mask = (uint64_t(1) << msb);
		for (int i = msb; i >= 0; --i) {
			ss << (mask & number ? "1" : "0");
			if (i > 0 && i % 4 == 0) ss << "_";
			mask >>= 1;
		}
	}
	return ss.str();
}
*/

} // namespace unum
} // namespace sw
