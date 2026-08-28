#pragma once
// iostream.hpp: stream insertion and extraction for quire
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 2b of the quire headers (#1334): the <iostream> half. manipulators.hpp is the
// string-producing half; this header includes only core.hpp.
#include <iostream>
#include <istream>
#include <ostream>

#include <universal/number/quire/core.hpp>

namespace sw { namespace universal {

template<typename NumberType, unsigned capacity, typename LimbType>
std::ostream& operator<<(std::ostream& ostr, const quire<NumberType, capacity, LimbType>& q) {
	// convert to double for human-readable output; may lose precision for large quires
	return ostr << q.template convert_to<double>();
}

}} // namespace sw::universal
