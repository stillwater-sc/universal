#pragma once
// poxel_fwd.hpp: forward declarations of the poxel, a posit with an uncertainty bit
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstdint>

namespace sw { namespace universal {

// poxel<nbits, es, bt>: the lattice of a posit<nbits - 1, es> plus a trailing ubit
template<unsigned nbits, unsigned es, typename bt> class poxel;

// tiles over the standard posit lattices: posit<8,2>, <16,2>, <32,2>, <64,2>
using poxel9  = poxel< 9, 2, std::uint16_t>;
using poxel17 = poxel<17, 2, std::uint32_t>;
using poxel33 = poxel<33, 2, std::uint64_t>;

}} // namespace sw::universal
