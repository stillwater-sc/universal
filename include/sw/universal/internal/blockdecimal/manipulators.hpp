#pragma once
// manipulators.hpp: text for blockdecimal
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The <iomanip> half of blockdecimal (#1334, #1473): everything that turns a blockdecimal
// into a std::string by way of a stringstream. to_string() concatenates and stays in
// blockdecimal.hpp. Self-contained.
#include <sstream>
#include <string>

#include <universal/internal/blockdecimal/blockdecimal.hpp>

namespace sw { namespace universal {


// Generate a type tag for blockdecimal
template<unsigned N, DecimalEncoding E, typename BT>
inline std::string type_tag(const blockdecimal<N, E, BT>& = {}) {
	std::stringstream s;
	s << "blockdecimal<" << N << '>';
	return s.str();
}

// to_binary: show internal digit storage
template<unsigned N, DecimalEncoding E, typename BT>
inline std::string to_binary(const blockdecimal<N, E, BT>& v) {
	std::stringstream s;
	s << (v.sign() ? '-' : '+') << "[ ";
	for (int i = static_cast<int>(N) - 1; i >= 0; --i) {
		s << v.digit(static_cast<unsigned>(i));
		if (i > 0) s << '.';
	}
	s << " ]";
	return s.str();
}

}} // namespace sw::universal
