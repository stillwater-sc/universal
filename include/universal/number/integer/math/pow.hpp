#pragma once
// pow.hpp: pow functions for integers
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

template<size_t nbits, typename BlockType, IntegerNumberType NumberType>
integer<nbits, BlockType, NumberType> pow(integer<nbits, BlockType, NumberType> x, integer<nbits, BlockType, NumberType> y) {
	return integer<nbits, BlockType, NumberType>(std::pow(double(x), double(y)));
}

template<size_t nbits, typename BlockType, IntegerNumberType NumberType>
integer<nbits, BlockType, NumberType> pow(integer<nbits, BlockType, NumberType> x, int y) {
	return integer<nbits, BlockType, NumberType>(std::pow(double(x), double(y)));
}

template<size_t nbits, typename BlockType, IntegerNumberType NumberType>
integer<nbits, BlockType, NumberType> pow(integer<nbits, BlockType, NumberType> x, double y) {
	return integer<nbits, BlockType, NumberType>(std::pow(double(x), double(y)));
}


}} // namespace sw::universal
