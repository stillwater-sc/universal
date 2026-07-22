// round.hpp: real floating-point conversion for elreal (Phase 8, #932).
//
// round_to<Target>(ZBCL<FpType> x, RoundingMode) rounds an exact-real elreal
// stream to a target native floating-point format -- double, float, dd, qd --
// with correctly-rounded semantics under all four IEEE rounding modes. This is
// how elreal hands an exact intermediate to the rest of the library.
//
// Method (dissertation 4.3): a ZBCL is a non-overlapping (0-overlap) expansion,
// so its blocks widened to double form a Shewchuk expansion of the exact value.
// To round to P significand bits, split every block at the cut bit e0-P+1 into a
// kept part (>= cut) and a round-off part (< cut); renormalise the round-off so
// opposite-sign cancellation collapses; classify it against the half-ulp
// (2^(cut-1)); then apply the mode (nearest ties-to-even, toward-zero, toward
// +inf, toward -inf) and renormalise the kept expansion. double/float take one
// limb (P=53/24); dd/qd take two/four (P=106/212).
//
// Guarantees (validated exhaustively against the exact einteger dyadic oracle):
//   idempotence, monotonicity across nested precisions, and path-independent
//   determinism -- the result depends only on the exact value and the mode.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
#include <cmath>
#include <algorithm>
#include <vector>
#include <universal/behavior/rounding.hpp>          // RoundingMode
#include <universal/number/elreal/block.hpp>
#include <universal/number/elreal/zbcl.hpp>
#include <universal/number/dd/dd.hpp>
#include <universal/number/qd/qd.hpp>

namespace sw { namespace universal {

namespace elreal_round_detail {

	// error-free transformation: s = a+b (rounded), e = the exact round-off
	inline void two_sum(double a, double b, double& s, double& e) noexcept {
		s = a + b;
		double bb = s - a;
		e = (a - (s - bb)) + (b - bb);
	}

	// renormalise a bag of doubles into a non-overlapping expansion, ordered by
	// descending magnitude (a Shewchuk grow-then-compress).
	inline std::vector<double> renorm(const std::vector<double>& v) {
		std::vector<double> h;
		for (double x : v) {
			double c = x;
			std::vector<double> nh;
			for (double hi : h) { double s, e; two_sum(hi, c, s, e); if (e != 0.0) nh.push_back(e); c = s; }
			if (c != 0.0) nh.push_back(c);
			h.swap(nh);
		}
		std::sort(h.begin(), h.end(), [](double a, double b) { return std::fabs(a) > std::fabs(b); });
		std::vector<double> g;
		double q = 0.0;
		for (double hv : h) { double s, e; two_sum(q, hv, s, e); q = s; if (e != 0.0) g.push_back(e); }
		if (q != 0.0) g.insert(g.begin(), q);
		std::sort(g.begin(), g.end(), [](double a, double b) { return std::fabs(a) > std::fabs(b); });
		return g;
	}

	// split double d at bit exponent c: hi holds the bits >= c (a multiple of
	// 2^c), lo = d - hi holds the bits below c (exact).
	inline void split_at(double d, int c, double& hi, double& lo) noexcept {
		if (d == 0.0) { hi = 0.0; lo = 0.0; return; }
		int e = std::ilogb(d);
		if (e < c) { hi = 0.0; lo = d; return; }               // wholly below the cut
		if (e - 52 >= c) { hi = d; lo = 0.0; return; }          // wholly at/above the cut
		double t = std::trunc(std::ldexp(d, -c));               // |scaled| in [1, 2^53)
		hi = std::ldexp(t, c);
		lo = d - hi;                                            // exact
	}

	// Round a ZBCL to P significand bits under `mode`, returning the result as a
	// normalised non-overlapping double expansion (descending; empty for zero).
	template<typename FpType>
	inline std::vector<double> round_to_bits(const ZBCL<FpType>& x, int P, RoundingMode mode) {
		constexpr int kbits = block<FpType>::k;
		const std::size_t need = static_cast<std::size_t>((P + 2 * kbits) / kbits) + 3;
		std::vector<block<FpType>> blks = x.take(need + 1);
		const bool moreBelow = (blks.size() > need);
		if (blks.size() > need) blks.resize(need);

		// widen blocks to doubles (exact for float/double/bfloat16 hosts)
		std::vector<double> blk;
		blk.reserve(blks.size());
		for (const auto& b : blks) { double d = b.template value_as<double>(); if (d != 0.0) blk.push_back(d); }
		if (blk.empty()) return {};

		const int  e0   = std::ilogb(blk[0]);
		const int  cut  = e0 - P + 1;                           // lowest kept bit
		const bool Vpos = (blk[0] > 0.0);

		std::vector<double> kept, ro;
		for (double d : blk) { double hi, lo; split_at(d, cut, hi, lo); if (hi != 0.0) kept.push_back(hi); if (lo != 0.0) ro.push_back(lo); }

		// classify the round-off against the half-ulp 2^(cut-1). Renormalise ro so
		// opposite-sign cancellation collapses to the true leading magnitude.
		std::vector<double> ror = renorm(ro);
		int  tsign  = 0;
		bool geHalf = false, eqHalf = false;
		if (!ror.empty()) {
			double lead = ror[0];
			tsign = (lead < 0.0) ? -1 : 1;
			if (std::ilogb(lead) == cut - 1) {
				geHalf = true;
				eqHalf = (ror.size() == 1) && (std::fabs(lead) == std::ldexp(1.0, cut - 1)) && !moreBelow;
			}
		}

		const double oneUlp = std::ldexp(1.0, cut);
		// parity of the pre-round result at bit `cut` (for ties-to-even)
		std::vector<double> keptExp = renorm(kept);
		bool lsbOdd = false;
		if (!keptExp.empty()) lsbOdd = (std::fmod(std::fabs(std::ldexp(keptExp.back(), -cut)), 2.0) >= 1.0);

		int adj = 0;   // add adj*ulp to the kept expansion
		switch (mode) {
		case RoundingMode::RoundToZero:
			if (tsign < 0 && Vpos)  adj = -1;
			if (tsign > 0 && !Vpos) adj = +1;
			break;
		case RoundingMode::RoundTowardPositive:
			adj = (tsign > 0) ? +1 : 0;
			break;
		case RoundingMode::RoundTowardNegative:
			adj = (tsign < 0) ? -1 : 0;
			break;
		case RoundingMode::RoundToNearest:
		default:
			if (geHalf) { if (!eqHalf) adj = (tsign > 0) ? +1 : -1; else adj = lsbOdd ? (tsign > 0 ? +1 : -1) : 0; }
			break;
		}
		if (adj != 0) kept.push_back(static_cast<double>(adj) * oneUlp);
		return renorm(kept);
	}

}  // namespace elreal_round_detail

// round_to<Target>(x, mode): correctly-rounded conversion of an exact-real elreal
// stream to a native floating-point target. Supported targets: double, float,
// dd, qd. Default mode is round-to-nearest, ties-to-even.
template<typename Target, typename FpType>
inline Target round_to(const ZBCL<FpType>& x, RoundingMode mode = RoundingMode::RoundToNearest) {
	using namespace elreal_round_detail;
	if constexpr (std::is_same_v<Target, double>) {
		auto r = round_to_bits(x, 53, mode);
		return r.empty() ? 0.0 : r[0];
	}
	else if constexpr (std::is_same_v<Target, float>) {
		auto r = round_to_bits(x, 24, mode);
		return r.empty() ? 0.0f : static_cast<float>(r[0]);   // r[0] is a 24-bit value -> exact
	}
	else if constexpr (std::is_same_v<Target, dd>) {
		auto r = round_to_bits(x, dd::fbits, mode);            // 106
		double h = r.size() > 0 ? r[0] : 0.0;
		double l = r.size() > 1 ? r[1] : 0.0;
		dd result(h, l);
		result.renorm();
		return result;
	}
	else if constexpr (std::is_same_v<Target, qd>) {
		auto r = round_to_bits(x, qd::fbits, mode);            // 212
		double c[4] = { 0.0, 0.0, 0.0, 0.0 };
		for (std::size_t i = 0; i < r.size() && i < 4; ++i) c[i] = r[i];
		qd result(c[0], c[1], c[2], c[3]);
		result.renorm();
		return result;
	}
	else {
		static_assert(std::is_same_v<Target, double>,
		              "round_to<Target>: unsupported target (use double, float, dd, or qd; cfloat is tracked separately)");
		return Target{};
	}
}

}} // namespace sw::universal
