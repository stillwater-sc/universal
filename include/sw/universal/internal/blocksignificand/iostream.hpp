#pragma once
// iostream.hpp: stream insertion for blocksignificand
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The <iostream> half of blocksignificand's text layer (#1334). manipulators.hpp is the
// <iomanip> half. This operator streams the public double() conversion, so it
// needs no friendship -- see the note in blocksignificand.hpp.
#include <ostream>
#include <universal/internal/blocksignificand/blocksignificand.hpp>

namespace sw { namespace universal {

template<unsigned nbits, typename bt>
inline std::ostream& operator<<(std::ostream& ostr, const blocksignificand<nbits, bt>& v) {
	return ostr << double(v);
}

}} // namespace sw::universal
