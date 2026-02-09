#pragma once
// zfparray_fwd.hpp: forward declarations for zfparray (ZFP compressed array container)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <cstddef>

namespace sw { namespace universal {

// forward reference
// zfparray: compressed array container using ZFP fixed-rate codec
// Template parameters:
//   Real - floating-point type (float or double)
//   Dim  - dimensionality (1, 2, or 3)
template<typename Real = float, unsigned Dim = 1>
class zfparray;

// 1D aliases
using zfparray1f = zfparray<float, 1>;
using zfparray1d = zfparray<double, 1>;
// 2D aliases
using zfparray2f = zfparray<float, 2>;
using zfparray2d = zfparray<double, 2>;
// 3D aliases
using zfparray3f = zfparray<float, 3>;
using zfparray3d = zfparray<double, 3>;

}} // namespace sw::universal
