#pragma once
// debug.hpp: introspection for einteger
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 3 of the einteger headers (#1334, Phase 2 group 5b, #1455). The limb dumps are
// member functions -- declared in the class, defined here -- because they build their
// text with a stringstream, the shape blocktriple's introspection got in #1388.
// Self-contained.
#include <iomanip>   // std::setw
#include <sstream>
#include <string>

#include <universal/native/integers.hpp>   // to_binary on a native limb
#include <universal/number/einteger/core.hpp>

namespace sw { namespace universal {

// show the binary encodings of the limbs
template<typename BlockType>
std::string einteger<BlockType>::showLimbs() const {
	if (_block.empty()) return "no limbs";
	std::stringstream s;
	size_t i = _block.size() - 1;
	while (i > 0) {
		s << to_binary(_block[i], sizeof(BlockType) * 8, true) << ' ';
		--i;
	}
	s << to_binary(_block[0], sizeof(BlockType) * 8, true);
	return s.str();
}

// show the values of the limbs as a radix-BlockType number
template<typename BlockType>
std::string einteger<BlockType>::showLimbValues() const {
	if (_block.empty()) return "no limbs";
	std::stringstream s;
	size_t i = _block.size() - 1;
	while (i > 0) {
		s << std::setw(5) << unsigned(_block[i]) << ", ";
		--i;
	}
	s << std::setw(5) << unsigned(_block[0]);
	return s.str();
}

}} // namespace sw::universal
