#pragma once
// integer_fwd.hpp: type forwards of fixed-size arbitrary precision integer numbers
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// forward references
enum class IntegerNumberType;
template<unsigned nbits, typename BlockType, IntegerNumberType NumberType> class integer;
template<unsigned nbits, typename BlockType, IntegerNumberType NumberType> integer<nbits, BlockType, NumberType> max_int();
template<unsigned nbits, typename BlockType, IntegerNumberType NumberType> integer<nbits, BlockType, NumberType> min_int();
template<unsigned nbits, typename BlockType, IntegerNumberType NumberType> struct idiv_t;
template<unsigned nbits, typename BlockType, IntegerNumberType NumberType> idiv_t<nbits, BlockType, NumberType> idiv(const integer<nbits, BlockType, NumberType>&, const integer<nbits, BlockType, NumberType>&b);

}} // namespace sw::universal
