#pragma once
// minmax.hpp: min/max functions for Google Brain floats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

inline bfloat16 min(bfloat16 x, bfloat16 y) {
	return bfloat16(std::min(float(x), float(y)));
}

inline bfloat16 max(bfloat16 x, bfloat16 y) {
	return bfloat16(std::max(float(x), float(y)));
}


}} // namespace sw::universal
