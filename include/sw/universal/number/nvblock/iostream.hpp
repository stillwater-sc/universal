#pragma once
// iostream.hpp: stream insertion for nvblock
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstddef>       // size_t
//
// Layer 2b of the nvblock headers (#1334): the <ostream> half. It streams the block's
// scale and elements, so it needs the element types' stream layers too.
#include <iostream>
#include <ostream>

#include <universal/number/nvblock/core.hpp>
#include <universal/number/microfloat/iostream.hpp>


namespace sw { namespace universal {

template<typename ElementType, size_t BlockSize, typename ScaleType>
inline std::ostream& operator<<(std::ostream& ostr, const nvblock<ElementType, BlockSize, ScaleType>& blk) {
	ostr << "nvblock(scale=" << blk.block_scale().to_float();
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
