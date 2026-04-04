#pragma once
// manipulators.hpp: I/O and display utilities for the bisection number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
#include <string>
#include <sstream>

namespace sw { namespace universal {

template<typename G, typename R, unsigned nbits, typename bt>
inline std::string to_binary(const bisection<G, R, nbits, bt>& v, bool nibbleMarker = true) {
	uint64_t bits = v.getbits();
	std::string s = "0b";
	for (int i = static_cast<int>(nbits) - 1; i >= 0; --i) {
		s += ((bits >> i) & 1) ? '1' : '0';
		if (nibbleMarker && i > 0 && (i % 4) == 0) s += '\'';
	}
	return s;
}

template<typename G, typename R, unsigned nbits, typename bt>
inline std::string type_tag(const bisection<G, R, nbits, bt>&) {
	std::ostringstream ss;
	ss << "bisection<" << nbits << ">";
	return ss.str();
}

template<typename G, typename R, unsigned nbits, typename bt>
inline std::string color_print(const bisection<G, R, nbits, bt>& v) {
	return to_binary(v, true);
}

template<typename G, typename R, unsigned nbits, typename bt>
inline std::string components(const bisection<G, R, nbits, bt>& v) {
	std::ostringstream ss;
	if (v.isnan()) {
		ss << "NaN";
	}
	else if (v.iszero()) {
		ss << "zero";
	}
	else {
		ss << "bisection encoding: " << to_binary(v, false)
		   << " = " << double(v);
	}
	return ss.str();
}

}} // namespace sw::universal
