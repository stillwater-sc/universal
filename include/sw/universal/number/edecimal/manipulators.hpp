#pragma once
// manipulators.hpp: definition of manipulation functions for the edecimal type
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 2a of the edecimal headers (#1334): the string-producing half. These build a
// std::string and hand it back, so they need <sstream> but not <iostream>; the stream
// insertion and extraction operators are in iostream.hpp.
#include <string>
#include <sstream>

#include <universal/number/edecimal/core.hpp>

namespace sw { namespace universal {

inline std::string to_binary(const edecimal& d) {
	std::stringstream s;
	if (d.isneg()) s << '-';
	for (edecimal::const_reverse_iterator rit = d.rbegin(); rit != d.rend(); ++rit) {
		s << (int)*rit;
	}
	return s.str();
}

// generate an ASCII edecimal string
inline std::string to_string(const edecimal& d) {
	std::stringstream s;
	if (d.isneg()) s << '-';
	for (edecimal::const_reverse_iterator rit = d.rbegin(); rit != d.rend(); ++rit) {
		s << (int)*rit;
	}
	return s.str();
}

}} // namespace sw::universal
