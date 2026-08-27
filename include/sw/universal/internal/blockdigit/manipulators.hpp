#pragma once
// manipulators.hpp: type_tag and to_binary for blockdigit -- the string-producing half
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 2a of the blockdigit headers (#1334), matching blockbinary's shape. These build a
// std::string, so they need <sstream> but not <iostream>; the stream insertion operator
// is in iostream.hpp.
#include <string>
#include <sstream>

#include <universal/internal/blockdigit/blockdigit.hpp>

namespace sw { namespace universal {

//////////////////////////////////////////////////////////////////////
// manipulation functions

// Generate a type tag for blockdigit
template<unsigned N, unsigned R, typename D>
inline std::string type_tag(const blockdigit<N, R, D>& = {}) {
	std::stringstream s;
	if constexpr (R == 8) {
		s << "blockoctal<" << N << '>';
	}
	else if constexpr (R == 10) {
		s << "blockdecimal<" << N << '>';
	}
	else if constexpr (R == 16) {
		s << "blockhexadecimal<" << N << '>';
	}
	else {
		s << "blockdigit<" << N << ", " << R << '>';
	}
	return s.str();
}

// to_binary: show internal digit storage
template<unsigned N, unsigned R, typename D>
inline std::string to_binary(const blockdigit<N, R, D>& v) {
	std::stringstream s;
	s << (v.sign() ? '-' : '+') << "[ ";
	for (int i = static_cast<int>(N) - 1; i >= 0; --i) {
		s << static_cast<int>(v.digit(static_cast<unsigned>(i)));
		if (i > 0) s << '.';
	}
	s << " ]";
	return s.str();
}

}} // namespace sw::universal
