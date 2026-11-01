#pragma once
// range_limit_add.hpp: multi-double expansion addition at the top of the double range
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <array>
#include <cstddef>
#include <limits>
#include <universal/numerics/error_free_ops.hpp>

namespace sw { namespace universal {

	// The multi-double types (dd, qd, dd_cascade, td_cascade, qd_cascade) all add through
	// error-free sums of their limbs. Near the top of the double range those sums can overflow
	// even when the complete sum is finite, e.g. (DBL_MAX, -2^969) + 2^970 = (DBL_MAX, 2^969),
	// and the result would be decided from an overflowed leading-limb sum (#1891).
	// The rule below is shared by all five types so they agree at the range limit (#1890, #1891).

	// true when the sum of two finite expansions with these leading limbs needs add_at_range_limit
	constexpr bool reaches_range_limit(double a0, double b0) noexcept {
		auto magnitude = [](double x) { return x < 0.0 ? -x : x; };
		return magnitude(a0) >= 0x1p+1022 || magnitude(b0) >= 0x1p+1022;
	}

	// Adds two N-limb expansions with finite leading limbs, at least one of which is 2^1022 or more.
	// add is the type's own expansion addition, called as
	//   std::array<double, N> add(const std::array<double, N>&, const std::array<double, N>&)
	// and only ever given operands below 2^1021, where its error-free sums cannot overflow.
	//
	// - Each operand splits into the prefix of limbs that quarter scaling keeps exact, which is
	//   scaled by 1/4, and the suffix it would not (subnormal limbs), moved to the front of its own
	//   expansion. Half scale is not enough: max() + max() at half scale is max(), and the
	//   error-free sums overflow on the way.
	// - The scaled prefixes are added and scaled back. The result overflows when the leading limb is
	//   inf, or is DBL_MAX with a same-signed tail of half an ulp of DBL_MAX (2^970) or more.
	// - Otherwise the suffixes, which are far below the result, are added in.
	template<std::size_t N, typename Add>
	constexpr std::array<double, N> add_at_range_limit(const std::array<double, N>& a, const std::array<double, N>& b, Add add) {
		static_assert(N >= 2, "add_at_range_limit is for multi-limb expansions");
		auto magnitude = [](double x) { return x < 0.0 ? -x : x; };
		auto split = [](const std::array<double, N>& x, std::array<double, N>& scaled_prefix, std::array<double, N>& suffix) {
			scaled_prefix.fill(0.0);
			suffix.fill(0.0);
			std::size_t k = 0;
			bool in_suffix = false;
			for (std::size_t i = 0; i < N; ++i) {
				const double q = x[i] * 0.25;
				if (!in_suffix && q * 4.0 == x[i]) scaled_prefix[i] = q;
				else { in_suffix = true; suffix[k++] = x[i]; }
			}
			return k > 0;
		};
		std::array<double, N> qa{}, qb{}, la{}, lb{};
		const bool tail_a = split(a, qa, la);
		const bool tail_b = split(b, qb, lb);
		std::array<double, N> r = add(qa, qb);
		for (std::size_t i = 0; i < N; ++i) r[i] *= 4.0;
		const bool overflow = !is_finite_cx(r[0]) ||
			(magnitude(r[0]) == std::numeric_limits<double>::max() && (r[0] < 0.0) == (r[1] < 0.0) && magnitude(r[1]) >= 0x1p+970);
		if (overflow) {
			const double inf = r[0] < 0.0 ? -std::numeric_limits<double>::infinity() : std::numeric_limits<double>::infinity();
			r.fill(0.0);
			r[0] = inf;
			return r;
		}
		if (tail_a || tail_b) r = add(r, add(la, lb));
		return r;
	}

}} // namespace sw::universal
