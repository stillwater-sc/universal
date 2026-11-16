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

// compile-time trait: emax of each element type, the exponent of its largest power of two
// (OCP MX v1.0, Table 1). quantize() picks the scale 2^(floor(log2(amax)) - emax), so the
// block's largest element lands in [2^emax, 2^(emax+1)).
// INT8 is two's complement with an implicit scale of 2^-6 (emax 0); read as an integer
// code, as mxblock stores it, its largest power of two is 2^6, so emax is 6. With 7, every
// block's largest code fell in [128, 256) and was clamped to 127 (#2083).
// The specializations are inline: an explicit specialization of a variable template is not
// implicitly inline, so two translation units including this header failed to link (#2083).
template<typename T>
inline constexpr int max_elem_exponent = 0;

template<>
inline constexpr int max_elem_exponent<e2m1> = 2;

template<>
inline constexpr int max_elem_exponent<e2m3> = 2;

template<>
inline constexpr int max_elem_exponent<e3m2> = 4;

template<>
inline constexpr int max_elem_exponent<e4m3fn> = 8;

template<>
inline constexpr int max_elem_exponent<e4m3_saturating> = 8;

template<>
inline constexpr int max_elem_exponent<e5m2> = 15;

template<>
inline constexpr int max_elem_exponent<e5m2_saturating> = 15;

template<>
inline constexpr int max_elem_exponent<int8_t> = 6;

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
// The same holds for e5m2 (#2096): the block maximum lands in [2^15, 2^16) against a maxpos of 57344,
// and with plain e5m2 every scaled value of 61440 or more became inf.
using mxfp8e5m2 = mxblock<e5m2_saturating>;
using mxint8    = mxblock<int8_t>;

}} // namespace sw::universal
