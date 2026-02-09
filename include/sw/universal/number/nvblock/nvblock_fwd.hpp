#pragma once
// nvblock_fwd.hpp: forward declarations for nvblock (NVIDIA NVFP4 block floating-point) types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <cstdint>
#include <type_traits>
#include <universal/number/microfloat/microfloat_fwd.hpp>

namespace sw { namespace universal {

// forward reference
// nvblock: NVIDIA two-level block scaling format
// Template parameters:
//   ElementType - microfloat element (default: e2m1 for NVFP4)
//   BlockSize   - elements per block (default: 16 per NVIDIA spec)
//   ScaleType   - block scale type (default: e4m3, fractional unlike MX's e8m0)
template<typename ElementType = e2m1, size_t BlockSize = 16, typename ScaleType = e4m3>
class nvblock;

// type alias for the canonical NVFP4 configuration
using nvfp4 = nvblock<e2m1, 16, e4m3>;

}} // namespace sw::universal
