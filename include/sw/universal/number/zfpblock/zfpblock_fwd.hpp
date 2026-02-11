#pragma once
// zfpblock_fwd.hpp: forward declarations for zfpblock (ZFP compressed floating-point block codec)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <cstdint>
#include <cstddef>

namespace sw { namespace universal {

// ZFP compression modes
enum class zfp_mode {
	fixed_rate,       // fixed number of bits per block
	fixed_precision,  // fixed number of bit planes
	fixed_accuracy,   // fixed absolute error tolerance
	reversible        // bit-exact lossless round-trip
};

// forward reference
// zfpblock: ZFP compressed floating-point block codec
// Template parameters:
//   Real - floating-point type (float or double)
//   Dim  - dimensionality (1, 2, or 3)
template<typename Real = float, unsigned Dim = 1>
class zfpblock;

// 1D aliases
using zfp1f = zfpblock<float, 1>;
using zfp1d = zfpblock<double, 1>;
// 2D aliases
using zfp2f = zfpblock<float, 2>;
using zfp2d = zfpblock<double, 2>;
// 3D aliases
using zfp3f = zfpblock<float, 3>;
using zfp3d = zfpblock<double, 3>;

}} // namespace sw::universal
