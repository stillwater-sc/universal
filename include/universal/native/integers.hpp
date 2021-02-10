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
