#pragma once
// iostream.hpp: stream insertion and extraction for elreal
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 2b of the elreal headers (#1334, Phase 2 group 5b, #1455): the <iostream> half.
// manipulators.hpp is the <iomanip> half. Self-contained.
//
// This header includes only core.hpp: both operators go through a host double, and
// neither half of the text layer depends on the other, so there is no cycle for
// #pragma once to mask (#1427).
#include <istream>
#include <ostream>

#include <universal/number/elreal/core.hpp>

namespace sw { namespace universal {

// stream output: the value at its current precision as a host-double approximation.
// (A full high-precision decimal printer is tracked as later manipulators work.)
template <typename FpType>
inline std::ostream& operator<<(std::ostream& ostr, const elreal<FpType>& v) {
	return ostr << static_cast<double>(v);
}

// stream input: parse a host-double literal into an elreal (exact for values a
// double represents exactly; otherwise the nearest double).
template <typename FpType>
inline std::istream& operator>>(std::istream& istr, elreal<FpType>& v) {
	double d{};
	istr >> d;
	if (!istr.fail()) v = d;
	return istr;
}

}} // namespace sw::universal
