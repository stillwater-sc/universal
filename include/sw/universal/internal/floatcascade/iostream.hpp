#pragma once
// iostream.hpp: stream insertion for floatcascade
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 2b of the floatcascade headers (#1334): the <ostream> half.
//
// floatcascade.hpp declares this operator as a friend template so the arithmetic core
// needs only <iosfwd>; the definition lives here. It was previously an in-class friend
// DEFINITION, which is what forced <iostream> into every consumer of the cascade
// substrate -- dd_cascade, td_cascade, qd_cascade and the floatcascade tests.
#include <ostream>
#include <cstddef>      // size_t
#include <universal/internal/floatcascade/floatcascade.hpp>

namespace sw { namespace universal {

template<size_t N>
inline std::ostream& operator<<(std::ostream& os, const floatcascade<N>& fc) {
	os << "floatcascade<" << N << ">[";
	for (size_t i = 0; i < N; ++i) {
		if (i > 0) os << ", ";
		os << fc[i];
	}
	os << "] ~ " << fc.to_double();
	return os;
}

}} // namespace sw::universal
