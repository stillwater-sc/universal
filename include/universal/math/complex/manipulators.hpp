#pragma once
// manipulators: helper functions for complex<> types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <sstream>
#include <complex>

namespace sw { namespace universal {

// complex<> type adapter for to_binary() operator
template<typename NumberType>
std::string to_binary(const std::complex<NumberType>& c, bool nibbleMarker = false) {
	std::stringstream ss;
	NumberType r = c.real();  // real() and imag() provided by the number types math library
	NumberType i = c.imag();
	ss << '(' << to_binary(r, nibbleMarker) << ", " << to_binary(i, nibbleMarker) << ')';
	return ss.str();
}

}} // namespace sw::universal
