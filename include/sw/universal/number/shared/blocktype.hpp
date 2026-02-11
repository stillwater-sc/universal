#pragma once
// blocktype.hpp: definition of blocktype manipulators
//
// Copyright (C) 2021-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <cstdint>

namespace sw { namespace universal {

template<typename bt>
struct ExpandBlockType {
};

template<>
struct ExpandBlockType<uint8_t> {
	typedef uint16_t BlockType;
};

template<>
struct ExpandBlockType<uint16_t> {
	typedef uint32_t BlockType;
};

template<>
struct ExpandBlockType<uint32_t> {
	typedef uint64_t BlockType;
};

}} // namespace sw::universal
