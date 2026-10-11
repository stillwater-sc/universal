#pragma once
// truncate.hpp: truncation functions (floor, ceil, trunc, round) for the bounded posit
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Exact for every configuration: the integral part is read from the encoding's fields, and
// it is itself a b-posit (it lies in the argument's binade or a lower one, or is a power of
// two), so nothing is rounded.
#include <cstdint>

namespace sw { namespace universal {

namespace bposit_math {

enum class integral_rule { floor, ceil, trunc, round };

template<integral_rule R, typename B>
B integral(const B& x) {
	if (x.iszero() || x.isnar()) return x;
	const auto d = x.decode_fields();
	if (d.scale >= static_cast<int>(d.nf)) return x;                 // no fraction bits
	const std::uint64_t sig = (std::uint64_t(1) << d.nf) | d.frac;
	std::uint64_t mag = 0;
	bool inexact = true, half = false;
	if (d.scale >= 0) {
		const unsigned k = d.nf - static_cast<unsigned>(d.scale);    // fraction bits, 1..nf
		const std::uint64_t rem = sig & ((std::uint64_t(1) << k) - 1u);
		mag     = sig >> k;
		inexact = rem != 0u;
		half    = ((rem >> (k - 1)) & 1u) != 0u;
	}
	else {
		half = (d.scale == -1);                                      // |x| in [1/2, 1)
	}
	bool away = false;                                               // one step away from zero
	switch (R) {
	case integral_rule::trunc: away = false; break;
	case integral_rule::floor: away = d.negative && inexact; break;
	case integral_rule::ceil:  away = !d.negative && inexact; break;
	case integral_rule::round: away = half; break;                   // ties away from zero
	}
	if (away) ++mag;
	if (mag == 0u) return B(0);
	const B r(static_cast<unsigned long long>(mag));
	return d.negative ? -r : r;
}

} // namespace bposit_math

// the largest integral value not greater than x
template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> floor(const bposit<nbits, rs, es, bt>& x) { return bposit_math::integral<bposit_math::integral_rule::floor>(x); }

// the smallest integral value not less than x
template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> ceil(const bposit<nbits, rs, es, bt>& x) { return bposit_math::integral<bposit_math::integral_rule::ceil>(x); }

// the nearest integral value not larger in magnitude than x
template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> trunc(const bposit<nbits, rs, es, bt>& x) { return bposit_math::integral<bposit_math::integral_rule::trunc>(x); }

// the nearest integral value, halfway cases away from zero
template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> round(const bposit<nbits, rs, es, bt>& x) { return bposit_math::integral<bposit_math::integral_rule::round>(x); }

}} // namespace sw::universal
