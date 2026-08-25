#pragma once
// iostream.hpp: stream insertion for blockfraction
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The <iostream> half of blockfraction's text layer (#1334). manipulators.hpp is the
// <iomanip> half. This operator streams the public double() conversion, so it
// needs no friendship -- see the note in blockfraction.hpp.
#include <ostream>
#include <universal/internal/blockfraction/blockfraction.hpp>

namespace sw { namespace universal {

template<unsigned nbits, typename bt>
inline std::ostream& operator<<(std::ostream& ostr, const blockfraction<nbits, bt>& v) {
	return ostr << double(v);
}

}} // namespace sw::universal
