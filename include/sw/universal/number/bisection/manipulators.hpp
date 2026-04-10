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

template<typename G, typename R, unsigned nbits, typename bt, typename A>
inline std::string to_binary(const bisection<G, R, nbits, bt, A>& v, bool nibbleMarker = true) {
	std::string s = "0b";
	for (int i = static_cast<int>(nbits) - 1; i >= 0; --i) {
		s += v.at(static_cast<unsigned>(i)) ? '1' : '0';
		if (nibbleMarker && i > 0 && (i % 4) == 0) s += '\'';
	}
	return s;
}

template<typename G, typename R, unsigned nbits, typename bt, typename A>
inline std::string type_tag(const bisection<G, R, nbits, bt, A>&) {
	std::ostringstream ss;
	ss << "bisection<" << nbits << ">";
	return ss.str();
}

template<typename G, typename R, unsigned nbits, typename bt, typename A>
inline std::string color_print(const bisection<G, R, nbits, bt, A>& v) {
	return to_binary(v, true);
}

template<typename G, typename R, unsigned nbits, typename bt, typename A>
inline std::string components(const bisection<G, R, nbits, bt, A>& v) {
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
