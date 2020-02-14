#pragma once
// integers.hpp: manipulators for native integer types
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw {
namespace native {

////////////////// string operators

// generate a binary string for a native integer
	// optional nbits argument indicating the size of the integer, if 0 use full size of native type
template<typename Integer,
         typename = std::enable_if< std::is_integral<Integer>::value, Integer >::type
>
inline std::string to_binary(const Integer& number, int nbits = 0) {
	std::stringstream ss;
	if (nbits == 0) nbits = sizeof(number);
	uint64_t mask = (uint64_t(1) << (nbits - 1));
	for (int i = int(nbits) - 1; i >= 0; --i) {
		ss << ((number & mask) ? '1' : '0');
		mask >>= 1;
	}
	return ss.str();
}

} // namespace native
} // namespace sw
