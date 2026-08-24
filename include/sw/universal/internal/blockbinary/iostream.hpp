#pragma once
// iostream.hpp: stream insertion for blockbinary
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The <iostream> half of blockbinary's text layer (#1334). manipulators.hpp is
// the <iomanip> half.
#include <ostream>
#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/internal/blockbinary/manipulators.hpp>   // to_binary, used below

namespace sw { namespace universal {

// ostream operator
template<unsigned nbits, typename BlockType, BinaryNumberType NumberType>
std::ostream& operator<<(std::ostream& ostr, const blockbinary<nbits, BlockType, NumberType>& number) {
	ostr << to_decimal(number);
	return ostr;
}


}} // namespace sw::universal
