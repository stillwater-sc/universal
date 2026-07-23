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
#include <cstdint>
#include <universal/behavior/rounding.hpp>          // RoundingMode
#include <universal/number/elreal/block.hpp>
#include <universal/number/elreal/zbcl.hpp>
#include <universal/number/dd/dd.hpp>
#include <universal/number/qd/qd.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/traits/cfloat_traits.hpp>

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

	// Round a widened non-overlapping expansion `blk` at absolute bit position
	// `cut` under `mode`, returning the rounded kept expansion in the same units
	// (descending, non-overlapping; empty for zero). `moreBelow` marks exact
	// content strictly below the retained blocks, so an exactly-half round-off is
	// really more-than-half. Splitting at `cut`, renormalising the round-off, and
	// classifying against the half-ulp 2^(cut-1) are all shift-invariant, so the
	// caller is free to pass an expansion scaled by any common power of two (used
	// by the cfloat path to keep working doubles near 1.0 and dodge overflow).
	inline std::vector<double> round_expansion_at(const std::vector<double>& blk, int cut, RoundingMode mode, bool moreBelow) {
		if (blk.empty()) return {};
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

		const int e0  = std::ilogb(blk[0]);
		const int cut = e0 - P + 1;                             // lowest kept bit
		return round_expansion_at(blk, cut, mode, moreBelow);
	}

	// ---- cfloat conversion -----------------------------------------------------
	//
	// round_to_cfloat<Cf>(x, mode) rounds an exact-real elreal stream to an
	// arbitrary cfloat<nbits,es,...> target. Unlike the native-double targets, a
	// cfloat has a finite es-bit exponent range, so the result can overflow to
	// +/-inf (or maxpos/maxneg for saturating configs), underflow to a subnormal
	// or to zero, and the subnormal path rounds at a reduced, fixed precision. The
	// significand rounding reuses round_expansion_at (all four modes); the final
	// bit construction mirrors cfloat's own convert() so the two agree on the
	// exponent field, the carry-into-hidden-bit binade bump, and the projection of
	// an all-ones exponent field onto inf.

	// overflow: magnitude is above the largest finite cfloat value.
	template<typename Cf>
	inline Cf cfloat_overflow(bool sign, RoundingMode mode) {
		Cf r;
		bool toInf;
		switch (mode) {
		case RoundingMode::RoundToZero:         toInf = false;  break;
		case RoundingMode::RoundTowardPositive: toInf = !sign;  break;   // +inf only when positive
		case RoundingMode::RoundTowardNegative: toInf =  sign;  break;   // -inf only when negative
		case RoundingMode::RoundToNearest:
		default:                                toInf = true;   break;
		}
		if constexpr (Cf::isSaturating) toInf = false;                    // saturating never yields inf
		if (toInf) r.setinf(sign);
		else { if (sign) r.maxneg(); else r.maxpos(); }
		return r;
	}

	// underflow: magnitude is strictly below the half-ulp of the smallest
	// representable value, so it rounds to zero except when a directed mode pushes
	// it out to the smallest representable value on its own side.
	template<typename Cf>
	inline Cf cfloat_underflow(bool sign, RoundingMode mode) {
		Cf r;
		if (mode == RoundingMode::RoundTowardPositive && !sign) { r.minpos(); return r; }
		if (mode == RoundingMode::RoundTowardNegative &&  sign) { r.minneg(); return r; }
		r.setzero(); r.setsign(sign);
		return r;
	}

	// assemble a finite cfloat from a sign, a biased exponent field, and a fraction
	// field, then project an all-ones exponent encoding onto inf/maxpos exactly as
	// cfloat's convert() does. Requires nbits <= 64 (the raw path).
	template<typename Cf>
	inline Cf cfloat_assemble(bool sign, std::uint64_t biasedExponent, std::uint64_t fracbits) {
		std::uint64_t raw = (sign ? 1ull : 0ull);
		raw <<= Cf::es;
		raw |= biasedExponent;
		raw <<= Cf::fbits;
		raw |= fracbits;
		Cf r; r.setbits(raw);
		if constexpr (Cf::isSaturating) {
			if (r.isnan()) { if (sign) r.maxneg(); else r.maxpos(); }
		}
		else {
			if (r.isnan()) r.setinf(sign);
		}
		return r;
	}

	template<typename Cf, typename FpType>
	inline Cf round_to_cfloat(const ZBCL<FpType>& x, RoundingMode mode) {
		static_assert(Cf::nbits <= 64, "round_to<cfloat>: only nbits <= 64 targets are supported");
		static_assert(Cf::fbits <= 52, "round_to<cfloat>: fraction bits (nbits-es-1) must fit a double significand (<= 52)");
		using exp_t = typename block<FpType>::exp_t;
		constexpr int  fbits             = static_cast<int>(Cf::fbits);
		constexpr int  MIN_EXP_NORMAL    = Cf::MIN_EXP_NORMAL;
		constexpr int  MIN_EXP_SUBNORMAL = Cf::MIN_EXP_SUBNORMAL;
		constexpr int  MAX_EXP           = Cf::MAX_EXP;
		constexpr int  EXP_BIAS          = Cf::EXP_BIAS;
		constexpr bool hasSubnormals     = Cf::hasSubnormals;
		constexpr int  kbits             = block<FpType>::k;

		// take enough blocks to resolve fbits+guard significand bits plus a borrow bit
		const std::size_t need = static_cast<std::size_t>((fbits + 3 * kbits) / kbits) + 4;
		std::vector<block<FpType>> blks = x.take(need + 1);
		const bool moreBelow = (blks.size() > need);
		if (blks.size() > need) blks.resize(need);

		// locate the leading (dominant) block: it fixes the sign and the exponent
		// estimate. Do it with the wide combined exponent so extreme scales cannot
		// overflow int.
		const block<FpType>* lead = nullptr;
		for (const auto& b : blks) { if (!b.is_zero_block()) { lead = &b; break; } }
		if (lead == nullptr) { Cf z; z.setzero(); return z; }        // exact zero -> +0
		const bool  sign = (lead->sign() < 0);
		const exp_t E0   = lead->exponent();

		// range guards using the wide exponent (a signed tail can only shift the
		// true binade by one, so a two-binade margin is always safe).
		if (E0 > exp_t(MAX_EXP + 1)) return cfloat_overflow<Cf>(sign, mode);
		const int lowGuard = hasSubnormals ? (MIN_EXP_SUBNORMAL - 2) : (MIN_EXP_NORMAL - 2);
		if (E0 < exp_t(lowGuard)) return cfloat_underflow<Cf>(sign, mode);

		// inside the comfortable window E0 fits int; widen blocks RELATIVE to it so
		// the working doubles stay near 1.0 (exact: block value = v * 2^exp).
		const int e0est = static_cast<int>(E0);
		std::vector<double> rel;
		rel.reserve(blks.size());
		for (const auto& b : blks) {
			if (b.is_zero_block()) continue;
			double m = static_cast<double>(b.v);                 // the block mantissa piece
			int    e = static_cast<int>(b.exp) - e0est;          // block's own exponent field, relative
			double d = std::ldexp(m, e);
			if (d != 0.0) rel.push_back(d);
		}
		if (rel.empty()) { Cf z; z.setzero(); return z; }
		std::vector<double> relN = renorm(rel);                  // descending, exact

		// true leading exponent: a leading exact power-of-two (|relN[0]| == 1.0)
		// with an opposite-sign tail means the magnitude dipped below 2^e0est, so
		// the true binade is one lower (the signed-non-overlapping-tail case).
		int e0 = e0est;
		if (std::fabs(relN[0]) == 1.0 && relN.size() > 1 && ((relN[1] < 0.0) == (relN[0] > 0.0))) --e0;

		// classify against the cfloat's range and pick the absolute cut (lowest
		// representable bit) for this value.
		int cut;
		bool normalRegion;
		if (e0 >= MIN_EXP_NORMAL) { cut = e0 - fbits;          normalRegion = true;  }
		else if (hasSubnormals)   { cut = MIN_EXP_SUBNORMAL;   normalRegion = false; }
		else                      { cut = MIN_EXP_NORMAL;      normalRegion = false; } // flush: ULP = minpos

		const int relCut = cut - e0est;                          // cut in relative units
		std::vector<double> keptRel = round_expansion_at(relN, relCut, mode, moreBelow);

		// mantissa integer M = |rounded value| / 2^cut (each kept component is a
		// multiple of 2^relCut, so the descaled contributions are exact integers).
		long long M = 0;
		for (double d : keptRel) M += std::llround(std::ldexp(d, -relCut));
		std::uint64_t absM = static_cast<std::uint64_t>(M < 0 ? -M : M);

		if (normalRegion) {
			int expv = e0;
			if (absM == (1ull << (fbits + 1))) { absM >>= 1; ++expv; }   // 1.11..1 -> 10.00.. carry
			if (expv > MAX_EXP) return cfloat_overflow<Cf>(sign, mode);
			std::uint64_t biasedExponent = static_cast<std::uint64_t>(expv + EXP_BIAS);
			std::uint64_t fracbits = absM & Cf::ALL_ONES_FR;             // drop the hidden bit
			return cfloat_assemble<Cf>(sign, biasedExponent, fracbits);
		}
		else {
			if (absM == 0) { Cf z; z.setzero(); z.setsign(sign); return z; }
			if constexpr (!hasSubnormals) {
				// flush region (no subnormals): the ULP is minpos_normal itself, so
				// absM is in {0,1}; absM == 1 is the smallest normal, not a subnormal
				// encoding (an exp field of 0 with a nonzero fraction is invalid here).
				return cfloat_assemble<Cf>(sign, static_cast<std::uint64_t>(MIN_EXP_NORMAL + EXP_BIAS), 0ull);
			}
			if (absM >= (1ull << fbits)) {                               // promoted to smallest normal
				return cfloat_assemble<Cf>(sign, static_cast<std::uint64_t>(MIN_EXP_NORMAL + EXP_BIAS), 0ull);
			}
			// genuine subnormal (hasSubnormals): exponent field 0, fraction = M
			return cfloat_assemble<Cf>(sign, 0ull, absM);
		}
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
	else if constexpr (is_cfloat<Target>) {
		return round_to_cfloat<Target>(x, mode);
	}
	else {
		static_assert(std::is_same_v<Target, double>,
		              "round_to<Target>: unsupported target (use double, float, dd, qd, or cfloat<nbits,es,...>)");
		return Target{};
	}
}

}} // namespace sw::universal
