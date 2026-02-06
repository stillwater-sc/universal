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

// Binary representation (shows underlying scalar bits if applicable)
template<typename Scalar>
std::string to_binary(const interval<Scalar>& v, bool nibbleMarker = false) {
	std::stringstream s;
	s << "lo: ";
	if constexpr (std::is_same_v<Scalar, float>) {
		union { float f; uint32_t u; } conv;
		conv.f = v.lo();
		s << "0b";
		for (int i = 31; i >= 0; --i) {
			s << ((conv.u >> i) & 1);
			if (nibbleMarker && i > 0 && i % 4 == 0) s << '\'';
		}
	}
	else if constexpr (std::is_same_v<Scalar, double>) {
		union { double d; uint64_t u; } conv;
		conv.d = v.lo();
		s << "0b";
		for (int i = 63; i >= 0; --i) {
			s << ((conv.u >> i) & 1);
			if (nibbleMarker && i > 0 && i % 4 == 0) s << '\'';
		}
	}
	else {
		s << v.lo();  // fallback for non-standard types
	}
	s << " hi: ";
	if constexpr (std::is_same_v<Scalar, float>) {
		union { float f; uint32_t u; } conv;
		conv.f = v.hi();
		s << "0b";
		for (int i = 31; i >= 0; --i) {
			s << ((conv.u >> i) & 1);
			if (nibbleMarker && i > 0 && i % 4 == 0) s << '\'';
		}
	}
	else if constexpr (std::is_same_v<Scalar, double>) {
		union { double d; uint64_t u; } conv;
		conv.d = v.hi();
		s << "0b";
		for (int i = 63; i >= 0; --i) {
			s << ((conv.u >> i) & 1);
			if (nibbleMarker && i > 0 && i % 4 == 0) s << '\'';
		}
	}
	else {
		s << v.hi();
	}
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
