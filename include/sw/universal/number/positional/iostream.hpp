#pragma once
// iostream.hpp: stream insertion for positional
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 2b of the positional headers (#1334, Phase 2 group 5c, #1467): the <iostream>
// half. manipulators.hpp is the <iomanip> half. Self-contained.
//
// operator<< is declared a friend in the class -- it reads the private storage -- and
// defined here. It streams that storage through blockdigit's own operator<<, so this is
// where blockdigit's text layer comes in. Includes only core.hpp of positional's layers.
#include <ostream>

#include <universal/internal/blockdigit/iostream.hpp>   // operator<<(ostream, blockdigit)
#include <universal/number/positional/core.hpp>

namespace sw { namespace universal {

template<unsigned N, unsigned R>
inline std::ostream& operator<<(std::ostream& ostr, const positional<N, R>& v) {
	return ostr << v._value;
}

}} // namespace sw::universal
