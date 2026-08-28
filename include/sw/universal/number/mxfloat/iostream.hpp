#pragma once
// iostream.hpp: stream insertion for mxfloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstddef>       // size_t
//
// Layer 2b of the mxfloat headers (#1334): the <ostream> half. It streams the block's
// scale and elements, so it needs the element types' stream layers too.
#include <iostream>
#include <ostream>

#include <universal/number/mxfloat/core.hpp>
#include <universal/number/microfloat/iostream.hpp>
#include <universal/number/e8m0/iostream.hpp>

namespace sw { namespace universal {

template<typename ElementType, size_t BlockSize>
inline std::ostream& operator<<(std::ostream& ostr, const mxblock<ElementType, BlockSize>& blk) {
	ostr << "mxblock(scale=" << blk.scale();
	ostr << ", elements=[";
	for (size_t i = 0; i < BlockSize; ++i) {
		if (i > 0) ostr << ", ";
		ostr << blk[i];
		if (i >= 7 && BlockSize > 10) {
			ostr << ", ... (" << (BlockSize - i - 1) << " more)";
			break;
		}
	}
	ostr << "])";
	return ostr;
}

}} // namespace sw::universal
