#pragma once
// microfloat_fwd.hpp: forward declarations for microfloat types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <type_traits>

namespace sw { namespace universal {

// forward reference
template<unsigned nbits, unsigned es, bool hasInf, bool hasNaN, bool isSaturating>
class microfloat;

// type aliases for OCP Microscaling (MX) element types
using e2m1 = microfloat<4, 2, false, false, true>;
using e2m3 = microfloat<6, 2, false, false, true>;
using e3m2 = microfloat<6, 3, false, false, true>;

// e4m3 comes in two conversion policies over the same encoding.  The encoding
// is the OCP OFP8 E4M3 one either way -- no infinity, NaN at S.1111.111,
// maxpos 448 at 0x7E -- and all 256 patterns decode identically.  They differ
// only in what a conversion from a wider type does with a value beyond maxpos:
//
//   e4m3fn          NaN, with the sign preserved.  This is the OCP 8-bit
//                   Floating Point Specification, and what ml_dtypes, JAX and
//                   PyTorch call float8_e4m3fn ("fn" = finite: no infinities,
//                   NaN signals overflow).  e4m3 names this one, because that
//                   is what the ecosystem reads e4m3 to mean.
//   e4m3_saturating clamp to maxpos/maxneg.  This is what block quantization
//                   wants: MX and NVFP4 both scale amax to just under 2^9
//                   against a maxpos of 448, so the largest element of a block
//                   routinely lands past the top of the range and must clip
//                   rather than poison the block with a NaN.
using e4m3fn          = microfloat<8, 4, false, true,  false>;
using e4m3_saturating = microfloat<8, 4, false, true,  true>;
using e4m3            = e4m3fn;

using e5m2 = microfloat<8, 5, true,  true,  false>;

// true for either e4m3 conversion policy: both carry the OCP E4M3 encoding,
// so anything that keys off the encoding rather than the overflow behavior
// (a name, a max element exponent, a bit layout) wants both.
template<typename T>
constexpr bool is_e4m3_encoding = std::is_same_v<T, e4m3fn> || std::is_same_v<T, e4m3_saturating>;

}}  // namespace sw::universal
