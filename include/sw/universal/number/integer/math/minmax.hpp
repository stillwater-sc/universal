#pragma once
// minmax.hpp: min/max functions for integers
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

template<unsigned nbits, typename BlockType>
integer<nbits, BlockType> min(integer<nbits, BlockType> x, integer<nbits, BlockType> y) {
	return (x < y ? x : y);
}

template<unsigned nbits, typename BlockType>
integer<nbits, BlockType> max(integer<nbits, BlockType> x, integer<nbits, BlockType> y) {
	return (x > y ? x : y);
}


}} // namespace sw::universal
