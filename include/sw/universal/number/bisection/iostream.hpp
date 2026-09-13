#pragma once
// iostream.hpp: stream insertion for the bisection number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
//
// Layer 2b of the bisection headers (#1334, Phase 2 group 5c, #1467): the <iostream>
// half. manipulators.hpp is the <iomanip> half. Includes only core.hpp; self-contained.
#include <ostream>

#include <universal/number/bisection/core.hpp>

namespace sw { namespace universal {

template<typename G, typename R, unsigned n, typename b, typename A>
inline std::ostream& operator<<(std::ostream& ostr, const bisection<G, R, n, b, A>& v) {
	double d = double(v);
	return ostr << d;
}

}} // namespace sw::universal
