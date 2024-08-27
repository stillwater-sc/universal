#pragma once
// uint128.hpp : a 128 bit unsigned integer class
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstdint>
#include <cassert>

namespace sw { namespace universal { namespace internal {

// 128 bit unsigned int mapped to two uint64_t elements
class uint128 {
public:
	uint64_t upper;
	uint64_t lower;
};

}}} // namespace sw::universal::internal
