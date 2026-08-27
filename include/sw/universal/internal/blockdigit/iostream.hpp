#pragma once
// iostream.hpp: stream insertion for blockdigit
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 2b of the blockdigit headers (#1334): the <ostream> half.
#include <ostream>

#include <universal/internal/blockdigit/blockdigit.hpp>

namespace sw { namespace universal {

template<unsigned N, unsigned R, typename D>
inline std::ostream& operator<<(std::ostream& ostr, const blockdigit<N, R, D>& v) {
	return ostr << v.to_string();
}

}} // namespace sw::universal
