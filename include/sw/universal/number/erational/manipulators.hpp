#pragma once
// manipulators.hpp: definition of manipulation functions for adaptive precision rational types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 2a of the erational headers (#1334): the string-producing half. These build a
// std::string, so they need <sstream> but not <iostream>; the stream operators are in
// iostream.hpp.
//
// This header had NO includes at all while using std::string and std::stringstream.
#include <string>
#include <sstream>

#include <universal/number/erational/core.hpp>

namespace sw { namespace universal {

	// Generate a type tag for rational type
	//
	// inline: this is a non-template free function in a header. Without it, any two
	// translation units that both included erational.hpp failed to link.
	inline std::string type_tag(const erational& = {}) {
		return "erational";
	}

// generate an ASCII erational string
inline std::string to_string(const erational& d) {
	std::stringstream str;
	if (d.isneg()) str << '-';
	str << "TBD";
	return str.str();
}

}} // namespace sw::universal
