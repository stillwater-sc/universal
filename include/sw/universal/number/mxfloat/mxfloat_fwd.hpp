#pragma once
// mxfloat_fwd.hpp: forward declarations for mxblock (MX block floating-point) types
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
template<typename ElementType, size_t BlockSize = 32>
class mxblock;

// compile-time trait: maximum element exponent for each element type
// This is the max biased exponent value for the element type, used in scale computation
template<typename T>
constexpr int max_elem_exponent = 0;

template<>
constexpr int max_elem_exponent<e2m1> = 2;

template<>
constexpr int max_elem_exponent<e2m3> = 2;

template<>
constexpr int max_elem_exponent<e3m2> = 4;

template<>
constexpr int max_elem_exponent<e4m3fn> = 8;

template<>
constexpr int max_elem_exponent<e4m3_saturating> = 8;

template<>
constexpr int max_elem_exponent<e5m2> = 15;

template<>
constexpr int max_elem_exponent<int8_t> = 7;

// type aliases for OCP Microscaling (MX) block formats
using mxfp4     = mxblock<e2m1>;
using mxfp6     = mxblock<e3m2>;
using mxfp6e2m3 = mxblock<e2m3>;
// MX FP8 quantization scales amax into [2^8, 2^9) against an e4m3 maxpos of
// 448, so the largest element of a block lands past the top of the range
// whenever amax/2^shared_exp exceeds 1.8125.  That has to clip, not turn into
// a NaN, which is why the element type here is the saturating policy and not
// the OCP conversion policy that plain e4m3 (= e4m3fn) carries.
using mxfp8     = mxblock<e4m3_saturating>;
using mxfp8e5m2 = mxblock<e5m2>;
using mxint8    = mxblock<int8_t>;

}} // namespace sw::universal
