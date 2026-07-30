#pragma once
// bit_manipulation.hpp: shared limb/bit mask helpers for the block storage types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// bit_clear_mask: a mask of limb type `bt` with every bit set EXCEPT bit
// (i % bitsInBlock). Clear a single bit of a limb with:
//     limb = static_cast<bt>(limb & bit_clear_mask<bt>(i, bitsInBlock));
//
// The narrowing to `bt` is intentional -- only the low bitsInBlock bits of the
// complemented mask are meaningful -- and is made EXPLICIT here so the idiom does
// not emit -Wconversion (GCC/Clang) or C4244 (MSVC) at every instantiation site,
// which it previously did when written inline as `bt null = ~(1ull << i)` (#1260).
template<typename bt>
constexpr bt bit_clear_mask(unsigned i, unsigned bitsInBlock) noexcept {
	return static_cast<bt>(~(bt(1) << (i % bitsInBlock)));
}

// bit_high_mask: a mask of limb type `bt` with the highest `count` bits of a
// bitsInBlock-wide limb set (equivalently, all-ones shifted left by
// bitsInBlock - count). Used by the left-shift operators to select the upper
// bits of a limb that move into the next word.
//
// As with bit_clear_mask, the narrowing to `bt` is intentional and EXPLICIT
// here, so the idiom does not emit -Wconversion / MSVC C4244 at every
// instantiation site, which it previously did when written inline as
// `bt mask = 0xFFFFFFFFFFFFFFFF << (bitsInBlock - count)` (#1261). Building the
// mask in `bt` also sidesteps the platform-dependent type of the unsuffixed
// 64-bit literal (unsigned long on LP64 vs unsigned long long on Windows).
template<typename bt>
constexpr bt bit_high_mask(unsigned count, unsigned bitsInBlock) noexcept {
	return static_cast<bt>(static_cast<bt>(~bt(0)) << (bitsInBlock - count));
}

}} // namespace sw::universal
