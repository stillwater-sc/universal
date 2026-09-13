#pragma once
// iostream.hpp: stream insertion and extraction for blockdecimal
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The <iostream> half of blockdecimal (#1334, #1473). The operators are declared friends
// in the class -- operator>> writes the private sign -- and defined here: the shape
// blockdigit's got in Phase N. A translation unit that streams a blockdecimal includes
// this header; one that only computes includes blockdecimal.hpp alone, which pulls no
// I/O-family header. Includes only blockdecimal.hpp; self-contained.
#include <istream>
#include <ostream>
#include <string>

#include <universal/internal/blockdecimal/blockdecimal.hpp>

namespace sw { namespace universal {

template<unsigned N, DecimalEncoding E, typename B>
inline std::ostream& operator<<(std::ostream& os, const blockdecimal<N, E, B>& v) {
	return os << v.to_string();
}

template<unsigned N, DecimalEncoding E, typename B>
inline std::istream& operator>>(std::istream& is, blockdecimal<N, E, B>& v) {
	std::string s;
	is >> s;
	v.clear();
	if (s.empty()) return is;
	unsigned start = 0;
	if (s[0] == '-') { v._negative = true; start = 1; }
	else if (s[0] == '+') { start = 1; }
	unsigned len = static_cast<unsigned>(s.size()) - start;
	for (unsigned i = 0; i < len && i < N; ++i) {
		char c = s[s.size() - 1 - i];
		if (c >= '0' && c <= '9') v.setdigit(i, static_cast<unsigned>(c - '0'));
	}
	if (v.iszero()) v._negative = false;
	return is;
}

}} // namespace sw::universal
