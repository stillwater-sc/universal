#pragma once
// limb_multiplication.hpp: product of multi-limb unsigned integers, a 32-bit word at a time
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Schoolbook multiplication on 32-bit limbs with 64-bit partial products: O(m n) word
// multiply-adds, where the shift-and-add loops it replaces cost a full-width add and shift for
// every BIT of one operand (#1743). Like limb::divide in limb_division.hpp, it is constexpr and
// allocation-free.
#include <cstddef>
#include <cstdint>

namespace sw { namespace universal { namespace limb {

// w = (u * v) mod 2^(32 k): the low k limbs of the product of u (m limbs) and v (n limbs), all
// least significant limb first. w holds k limbs and must not overlap u or v. Partial products
// that land at or above limb k are never formed.
inline constexpr void multiply_low(const std::uint32_t* u, std::size_t m, const std::uint32_t* v, std::size_t n,
                                   std::uint32_t* w, std::size_t k) noexcept {
	for (std::size_t i = 0; i < k; ++i) w[i] = 0;
	for (std::size_t i = 0; i < m && i < k; ++i) {
		const std::uint64_t ui = u[i];
		if (ui == 0) continue;
		std::uint64_t carry = 0;
		std::size_t j = 0;
		for (; j < n && i + j < k; ++j) {
			// ui * v[j] + w + carry < 2^64: (2^32 - 1)^2 + 2 (2^32 - 1) = 2^64 - 1
			const std::uint64_t t = ui * v[j] + w[i + j] + carry;
			w[i + j] = static_cast<std::uint32_t>(t);
			carry = t >> 32;
		}
		if (i + j < k) w[i + j] = static_cast<std::uint32_t>(carry);   // that limb is still zero here
	}
}

}}}  // namespace sw::universal::limb
