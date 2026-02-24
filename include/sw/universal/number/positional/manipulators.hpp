#pragma once
// manipulators.hpp: definition of manipulation functions for positional integer types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// Generate a type tag for positional type
template<unsigned ndigits, unsigned radix>
std::string type_tag(const positional<ndigits, radix>& = {}) {
	std::stringstream s;
	if constexpr (radix == 8) {
		s << "positional<" << ndigits << ", 8>";
	}
	else if constexpr (radix == 10) {
		s << "positional<" << ndigits << ", 10>";
	}
	else if constexpr (radix == 16) {
		s << "positional<" << ndigits << ", 16>";
	}
	else {
		s << "positional<" << ndigits << ", " << radix << '>';
	}
	return s.str();
}

// to_binary: show internal digit storage
template<unsigned ndigits, unsigned radix>
inline std::string to_binary(const positional<ndigits, radix>& v) {
	return to_binary(v.value());
}

// color_print: show digits with sign
template<unsigned ndigits, unsigned radix>
inline std::string color_print(const positional<ndigits, radix>& v) {
	return to_binary(v);
}

}} // namespace sw::universal
