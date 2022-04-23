#pragma once
// manipulators.hpp: definition of manipulation functions for integer types
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <exception>

namespace sw { namespace universal {

// Generate a type tag for general integer
template<size_t nbits, typename BlockType, IntegerNumberType NumberType>
std::string type_tag(const integer<nbits, BlockType, NumberType>& v) {
	std::stringstream str;
	if (v.iszero()) str << ' '; // remove 'unreferenced formal parameter warning from compilation log
	str << "integer<"
	    << std::setw(4) << nbits << ", "
	    << typeid(BlockType).name() << ", "
		<< typeid(NumberType).name() << '>';
	return str.str();
}

// return in triple form (sign, scale, fraction)
template<size_t nbits, typename BlockType, IntegerNumberType NumberType>
inline std::string to_triple(const integer<nbits, BlockType, NumberType>& number) {
	using Integer = integer<nbits+1, BlockType, NumberType>; // to capture maxneg in magnitude conversion

	std::stringstream str;

	// print sign bit
	str << '(' << (number < 0 ? '-' : '+') << ',';

	Integer magnitude = (number < 0 ? -number : number);

	// scale
	str << scale(magnitude) << ',';

	// print fraction bits
	long msb = findMsb(magnitude);
	if (msb < 0) {
		str << '-';
	}
	else {
		// the msb becomes the hidden bit
		magnitude <<= (nbits - msb);
		for (int i = nbits - 1; i >= 0; --i) {
			str << (magnitude.at(static_cast<size_t>(i)) ? '1' : '0');
		}
	}

	str << ')';
	return str.str();
}

}} // namespace sw::universal
