#pragma once
// zfp_codec_traits.hpp: type traits mapping native types to ZFP internal types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <cstdint>
#include <cstddef>
#include <climits>
#include <type_traits>

namespace sw { namespace universal {

// ZFP type traits: maps float/double to integer types and codec parameters
template<typename Real>
struct zfp_type_traits;

template<>
struct zfp_type_traits<float> {
	using Int  = int32_t;
	using UInt = uint32_t;
	static constexpr unsigned ebits = 8;
	static constexpr int      ebias = 127;
	static constexpr UInt     nbmask = 0xAAAAAAAAu;
	static constexpr unsigned precision_bits = 32;  // max bit planes for float (CHAR_BIT * sizeof(Int))
	static constexpr int      frac_bits = 30;       // bits for fixed-point fraction (precision_bits - 2)
};

template<>
struct zfp_type_traits<double> {
	using Int  = int64_t;
	using UInt = uint64_t;
	static constexpr unsigned ebits = 11;
	static constexpr int      ebias = 1023;
	static constexpr UInt     nbmask = 0xAAAAAAAAAAAAAAAAull;
	static constexpr unsigned precision_bits = 64;  // max bit planes for double
	static constexpr int      frac_bits = 62;       // bits for fixed-point fraction (precision_bits - 2)
};

// Block size: 4^Dim
template<unsigned Dim>
struct zfp_block_size;

template<> struct zfp_block_size<1> { static constexpr size_t value = 4; };
template<> struct zfp_block_size<2> { static constexpr size_t value = 16; };
template<> struct zfp_block_size<3> { static constexpr size_t value = 64; };

// Maximum compressed bytes for a block (worst case: header + all bits)
// Header: 1 bit (zero flag) + ebits (exponent) = 9 bits for float, 12 for double
// Data: block_size * precision_bits
// Total bits: 1 + ebits + block_size * precision_bits
// Round up to bytes and add safety margin
template<typename Real, unsigned Dim>
struct zfp_max_bytes {
	static constexpr size_t block_size = zfp_block_size<Dim>::value;
	static constexpr size_t max_bits = 1 + zfp_type_traits<Real>::ebits +
	                                   block_size * zfp_type_traits<Real>::precision_bits;
	static constexpr size_t value = (max_bits + 7) / 8 + 8;  // +8 for safety margin
};

}} // namespace sw::universal
