#pragma once
// manipulators.hpp: definitions of helper functions for interval type manipulation
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <sstream>
#include <typeinfo>
#include <cstdint>
#include <universal/utility/bit_cast.hpp>

namespace sw { namespace universal {

// type_tag is defined in interval_impl.hpp

// Pretty print an interval with its components
template<typename Scalar>
std::string pretty_print(const interval<Scalar>& v) {
	std::stringstream s;
	s << "lo: " << v.lo() << " hi: " << v.hi() << " mid: " << v.mid() << " rad: " << v.rad();
	return s.str();
}

// Color print (for terminal display)
template<typename Scalar>
std::string color_print(const interval<Scalar>& v) {
	std::stringstream s;
	s << "\033[32m[" << v.lo() << ", " << v.hi() << "]\033[0m";
	return s.str();
}

// Print the bits of a scalar value into a stream using bit_cast
template<typename Scalar>
void print_scalar_bits(std::ostream& s, Scalar value, bool nibbleMarker) {
	if constexpr (std::is_same_v<Scalar, float>) {
		uint32_t bits = sw::bit_cast<uint32_t>(value);
		s << "0b";
		for (int i = 31; i >= 0; --i) {
			s << ((bits >> i) & 1);
			if (nibbleMarker && i > 0 && i % 4 == 0) s << '\'';
		}
	}
	else if constexpr (std::is_same_v<Scalar, double>) {
		uint64_t bits = sw::bit_cast<uint64_t>(value);
		s << "0b";
		for (int i = 63; i >= 0; --i) {
			s << ((bits >> i) & 1);
			if (nibbleMarker && i > 0 && i % 4 == 0) s << '\'';
		}
	}
	else {
		s << value;  // fallback for non-standard types
	}
}

// Binary representation (shows underlying scalar bits if applicable)
template<typename Scalar>
std::string to_binary(const interval<Scalar>& v, bool nibbleMarker = false) {
	std::stringstream s;
	s << "lo: ";
	print_scalar_bits(s, v.lo(), nibbleMarker);
	s << " hi: ";
	print_scalar_bits(s, v.hi(), nibbleMarker);
	return s.str();
}

// Report interval range information
template<typename Scalar>
void interval_range(std::ostream& ostr = std::cout) {
	ostr << "interval<" << typeid(Scalar).name() << "> range:\n";
	ostr << "  min scalar: " << std::numeric_limits<Scalar>::min() << '\n';
	ostr << "  max scalar: " << std::numeric_limits<Scalar>::max() << '\n';
	ostr << "  lowest scalar: " << std::numeric_limits<Scalar>::lowest() << '\n';
	ostr << "  epsilon: " << std::numeric_limits<Scalar>::epsilon() << '\n';
}

// Check if a value is within the representable range
template<typename Scalar>
bool isInRange(const interval<Scalar>& v) {
	return v.isfinite();
}

}} // namespace sw::universal
