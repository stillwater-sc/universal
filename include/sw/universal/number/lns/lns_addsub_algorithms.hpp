#pragma once
// lns_addsub_algorithms.hpp: configurable algorithms for lns operator+ / operator-
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// -----------------------------------------------------------------------------
// Customization point for lns add/sub
// -----------------------------------------------------------------------------
//
// Logarithmic addition is the hard case in any LNS implementation -- in the
// log domain, multiplication and division are simple integer ops, but addition
// requires the Gauss correction
//
//     log2(a + b) = La + sb_add(d),   d = Lb - La,  d <= 0
//     sb_add(d)  = log2(1 + 2^d)
//
// (and an analogous sb_sub for subtraction with care near d=0 cancellation).
// Different hardware targets want different sb_add/sb_sub implementations:
// table lookup for ML accelerators, polynomial for memory-constrained DSP,
// CORDIC for hardware codesign, direct evaluation for high-precision software.
//
// To support all of these without locking lns into a single algorithm,
// lns selects its add/sub algorithm via a traits-class customization point.
// The default is the placeholder DoubleTripAddSub (cast to double, do native
// double arithmetic, cast back) which preserves the long-standing behavior
// from before the constexpr_math facility (#763) existed.
//
// To override for a specific lns instantiation, specialize lns_addsub_traits:
//
//     #include <universal/number/lns/lns.hpp>
//     #include <universal/number/lns/lns_addsub_algorithms.hpp>
//
//     namespace sw::universal {
//         template<>
//         struct lns_addsub_traits<lns<16, 8, std::uint16_t>> {
//             using type = DirectEvaluationAddSub<lns<16, 8, std::uint16_t>>;
//         };
//     }
//
// The shipped algorithms (Phases A and B of #777):
//   - DoubleTripAddSub      -- default, preserves current behavior
//   - DirectEvaluationAddSub -- uses sw::math::constexpr_math::log2/exp2
//   - LookupAddSub           -- Mitchell-style precomputed table + linear interp
//
// Future phases (#781, #783) will add Polynomial, ArnoldBailey, and (deferred)
// CORDIC. All will be drop-in via the same traits specialization.
//
// -----------------------------------------------------------------------------
// Algorithm contract
// -----------------------------------------------------------------------------
// Each algorithm class must provide two static member functions:
//
//     static constexpr Lns& add_assign(Lns& lhs, const Lns& rhs);
//     static constexpr Lns& sub_assign(Lns& lhs, const Lns& rhs);
//
// They return lhs (mutated in place) and are responsible for honoring the
// lns Saturating-vs-Modulo Behavior of the Lns instantiation. Special-value
// handling (NaN propagation, zero, infinity) follows IEEE-754 conventions and
// is the responsibility of the algorithm.

#include <array>
#include <cstddef>
#include <limits>

#include <math/constexpr_math.hpp>
#include <universal/number/lns/lns_fwd.hpp>

namespace sw { namespace universal {

// ============================================================================
// Algorithm 1: DoubleTripAddSub -- the historical placeholder, preserved
// ============================================================================
//
// Casts both operands to double, does native double arithmetic, casts the
// result back. Round-trips through IEEE-754 (one cast each direction = up to
// 2 ulp error in lns precision). Slow because it pays for the exp2 (decode)
// + add + log2 (encode) every operation -- which is the worst possible path
// for a number system that exists specifically to avoid those transcendentals.
//
// Default for backward compatibility, and a correctness oracle that all other
// algorithms cross-validate against.

template<typename Lns>
struct DoubleTripAddSub {
	static constexpr Lns& add_assign(Lns& lhs, const Lns& rhs) {
		double sum = double(lhs) + double(rhs);
		// The assignment routes through Lns::operator=(double), which honors
		// the Saturating Behavior in its convert_ieee754 implementation.
		return lhs = sum;
	}
	static constexpr Lns& sub_assign(Lns& lhs, const Lns& rhs) {
		double diff = double(lhs) - double(rhs);
		return lhs = diff;
	}
};

// ============================================================================
// Algorithm 2: DirectEvaluationAddSub -- exact via cm::log2 + cm::exp2
// ============================================================================
//
// Implements the Gauss log-add formulation directly using the constexpr_math
// facility (Epic #763):
//
//     a + b > 0:
//       Let La = log2(|a|), Lb = log2(|b|), and arrange La >= Lb.
//       log2(a + b) = La + log2(1 + 2^(Lb - La))
//
// The "1 + 2^d" form for d <= 0 stays well-behaved: as d -> -infinity, the
// correction goes to 0 (and the sum equals the larger operand, which is the
// correct answer in the lns domain). At d = 0, the correction is exactly 1
// (i.e., a + a = 2a in the log domain is +1).
//
// For subtraction with same-sign operands, the analogous formula is
//     log2(a - b) = La + log2(1 - 2^(Lb - La))
// which has catastrophic cancellation when d -> 0. This algorithm doesn't
// special-case that -- callers needing exact cancellation should use a
// higher-precision intermediate or a different algorithm. The behavior
// matches what std::log2(1 - small) does in IEEE-754 double.
//
// Mixed-sign: a + (-b) reduces to a - b; (-a) + b reduces to b - a. We
// dispatch on signs and route to the single same-sign sb_add / sb_sub paths.
//
// Slow (two transcendentals per add) but accurate to within ~3-4 ulp of
// std::log2/std::exp2 over the lns dynamic range. Useful as a correctness
// oracle for the faster Phase B/C algorithms, and as the recommended default
// for high-precision software targets.
//
// All work happens at the double level; we then encode the result back into
// the lns. The double-precision intermediate has more dynamic range than any
// reasonable lns target (which is at most 64 bits significand), so no overflow
// concerns until the final encode.

template<typename Lns>
struct DirectEvaluationAddSub {
private:
	// log2(1 + 2^d) for d <= 0. As d -> -infinity, correction -> 0.
	// As d -> 0, correction -> 1.
	static constexpr double sb_add(double d) {
		// 2^d for d <= 0 stays in [0, 1]. The 1 + 2^d sum is exact in
		// double for any d, and cm::log2 handles the result.
		return sw::math::constexpr_math::log2(1.0 + sw::math::constexpr_math::exp2(d));
	}

	// log2(1 - 2^d) for d < 0. (At d == 0 the result is -infinity which is
	// the correct lns answer for a - a, but the caller should typically have
	// special-cased that as zero before reaching here.) For d close to 0,
	// the function returns a large negative value (approaching -infinity);
	// this correctly encodes near-zero results in the lns domain.
	static constexpr double sb_sub(double d) {
		double t = 1.0 - sw::math::constexpr_math::exp2(d);
		return sw::math::constexpr_math::log2(t);
	}

	// Compute a + b in the log domain, returning the result as a double in
	// linear units (caller encodes to Lns).
	static constexpr double log_add(double a, double b) {
		// Special values
		if (a != a || b != b) return a + b;                             // NaN propagates
		if (a == 0.0) return b;
		if (b == 0.0) return a;
		// Infinities: native double arithmetic gives the correct IEEE result
		// for all combinations (inf + inf = inf, inf + (-inf) = NaN,
		// inf + finite = inf). The Gauss log-add path below assumes finite,
		// non-zero operands; routing inf through log2 would produce inf-inf
		// in the same-sign branch and a spurious 0 from the La==Lb check in
		// the mixed-sign branch.
		{
			constexpr double dinf = std::numeric_limits<double>::infinity();
			if (a == dinf || a == -dinf || b == dinf || b == -dinf) return a + b;
		}

		bool sign_a = a < 0.0;
		bool sign_b = b < 0.0;
		double abs_a = sign_a ? -a : a;
		double abs_b = sign_b ? -b : b;

		if (sign_a == sign_b) {
			// Same sign: pure addition in the log domain.
			// log2(|a + b|) = max(La, Lb) + sb_add(min - max)
			double La = sw::math::constexpr_math::log2(abs_a);
			double Lb = sw::math::constexpr_math::log2(abs_b);
			double Lmax = (La >= Lb) ? La : Lb;
			double Lmin = (La >= Lb) ? Lb : La;
			double Lresult = Lmax + sb_add(Lmin - Lmax);
			double mag = sw::math::constexpr_math::exp2(Lresult);
			return sign_a ? -mag : mag;
		}
		else {
			// Mixed sign: a + b = sign(a)*|a| - sign(a)*|b| ... reduces to
			// magnitude subtraction. The result's sign follows the larger
			// magnitude.
			double La = sw::math::constexpr_math::log2(abs_a);
			double Lb = sw::math::constexpr_math::log2(abs_b);
			if (La == Lb) return 0.0;                                    // exact cancellation
			bool a_larger = (La > Lb);
			double Lmax = a_larger ? La : Lb;
			double Lmin = a_larger ? Lb : La;
			double Lresult = Lmax + sb_sub(Lmin - Lmax);
			// log2 of zero or negative -- happens only when sb_sub argument
			// rounds to 0; treat as exact cancellation.
			if (Lresult != Lresult) return 0.0;
			double mag = sw::math::constexpr_math::exp2(Lresult);
			bool result_neg = a_larger ? sign_a : sign_b;
			return result_neg ? -mag : mag;
		}
	}

public:
	static constexpr Lns& add_assign(Lns& lhs, const Lns& rhs) {
		double result = log_add(double(lhs), double(rhs));
		return lhs = result;
	}
	static constexpr Lns& sub_assign(Lns& lhs, const Lns& rhs) {
		// a - b = a + (-b); negate via the lns operator-() which is a single
		// sign-bit flip.
		double result = log_add(double(lhs), double(-rhs));
		return lhs = result;
	}
};

// ============================================================================
// Algorithm 3: LookupAddSub -- Mitchell-style precomputed table + linear interp
// ============================================================================
//
// The original Mitchell 1962 LNS algorithm: precompute sb_add(d) over a finite
// d-grid, then index + linearly interpolate at runtime. For ML inference and
// embedded DSP this is the SRAM-friendly hardware default -- one indexed
// memory read + one mul-add per operation, no transcendentals at runtime.
//
// Table generation runs at compile time via sw::math::constexpr_math::log2/exp2
// (Epic #763), so no offline tooling is needed and the table follows the
// instantiation: a different (Lns, IndexBits) pair gets its own table.
//
// Parameterization
// ----------------
//   IndexBits -- log2 of the number of table entries.
//                Default: min(Lns::rbits + 2, 10) -- caps at 1024 entries so a
//                64-bit lns doesn't try to allocate a 4-million-entry table.
//                Override for higher accuracy or smaller SRAM as needed.
//
// d-range
// -------
// d = Lb - La with d <= 0. As d -> -infinity, sb_add(d) -> 0; in lns precision
// (2^-rbits) the correction is below ULP once |d| > rbits + ~2. We pick that
// as the table cutoff: |d| >= d_range => sb_add returns 0 directly.
//
// sb_sub special-case
// -------------------
// sb_sub(d) = log2(1 - 2^d) has unbounded slope as d -> 0 (catastrophic
// cancellation). Linear interpolation in the lowest table cell is not safe;
// for |d| < step we fall back to direct evaluation via cm::log2/cm::exp2.
// This preserves accuracy in the cancellation regime at the cost of one
// transcendental call per sub in that narrow band.

template<typename Lns,
         unsigned IndexBits = ((Lns::rbits + 2u < 10u) ? Lns::rbits + 2u : 10u)>
struct LookupAddSub {
	// Shift safety: table_entries = 1 << IndexBits is computed in std::size_t,
	// so the limit is sizeof(size_t)*8 bits. We cap well below that to keep
	// table sizes sane (2^30 doubles = 8 GB, far past any realistic budget).
	static_assert(IndexBits < 30,
	              "LookupAddSub: IndexBits >= 30 would overflow practical SRAM budgets and "
	              "approaches the shift width of std::size_t");
private:
	static constexpr std::size_t table_entries = std::size_t(1) << IndexBits;
	static constexpr double      d_range = double(Lns::rbits) + 2.0;
	static constexpr double      step    = d_range / double(table_entries);

	// log2(1 + 2^d) for d = -i*step, i in [0, table_entries].
	// Endpoint i = table_entries gives the value at d = -d_range.
	static constexpr std::array<double, table_entries + 1u> make_add_table() {
		std::array<double, table_entries + 1u> t{};
		for (std::size_t i = 0; i <= table_entries; ++i) {
			double d = -double(i) * step;
			t[i] = sw::math::constexpr_math::log2(
			           1.0 + sw::math::constexpr_math::exp2(d));
		}
		return t;
	}

	// log2(1 - 2^d) for d = -i*step, i in [1, table_entries].
	// Index 0 is unused (corresponds to d = 0, where the function is -infinity);
	// callers route the |d| < step cell through direct evaluation instead.
	static constexpr std::array<double, table_entries + 1u> make_sub_table() {
		std::array<double, table_entries + 1u> t{};
		t[0] = 0.0;
		for (std::size_t i = 1; i <= table_entries; ++i) {
			double d = -double(i) * step;
			double v = 1.0 - sw::math::constexpr_math::exp2(d);
			t[i] = sw::math::constexpr_math::log2(v);
		}
		return t;
	}

	static constexpr auto add_table = make_add_table();
	static constexpr auto sub_table = make_sub_table();

	// Linear interp: log2(1 + 2^d) for d <= 0.
	static constexpr double sb_add(double d) {
		if (d > 0.0) d = 0.0;
		double abs_d = -d;
		if (abs_d >= d_range) return 0.0;
		double idx_f = abs_d / step;
		std::size_t idx = static_cast<std::size_t>(idx_f);
		double frac = idx_f - double(idx);
		return add_table[idx] + frac * (add_table[idx + 1u] - add_table[idx]);
	}

	// log2(1 - 2^d) for d < 0. d == 0 is exact-cancellation territory and
	// must be handled by the caller (we return 0.0 as a safe sentinel).
	static constexpr double sb_sub(double d) {
		if (d >= 0.0) return 0.0;
		double abs_d = -d;
		if (abs_d >= d_range) return 0.0;  // 1 - 2^d ~= 1, log2 ~= 0
		double idx_f = abs_d / step;
		std::size_t idx = static_cast<std::size_t>(idx_f);
		if (idx == 0) {
			// Cancellation regime: log2(1 - 2^d) has unbounded slope.
			// Direct evaluation -- one transcendental pair, only in this band.
			double t = 1.0 - sw::math::constexpr_math::exp2(d);
			return sw::math::constexpr_math::log2(t);
		}
		double frac = idx_f - double(idx);
		return sub_table[idx] + frac * (sub_table[idx + 1u] - sub_table[idx]);
	}

	// Same dispatcher as DirectEvaluationAddSub: routes special values, then
	// dispatches by sign to same-sign add or mixed-sign magnitude subtraction.
	static constexpr double log_add(double a, double b) {
		if (a != a || b != b) return a + b;                                 // NaN propagates
		if (a == 0.0) return b;
		if (b == 0.0) return a;
		// Infinities -- delegate to native double for correct IEEE behavior;
		// see DirectEvaluationAddSub::log_add for the rationale.
		{
			constexpr double dinf = std::numeric_limits<double>::infinity();
			if (a == dinf || a == -dinf || b == dinf || b == -dinf) return a + b;
		}

		bool sign_a = a < 0.0;
		bool sign_b = b < 0.0;
		double abs_a = sign_a ? -a : a;
		double abs_b = sign_b ? -b : b;

		if (sign_a == sign_b) {
			double La = sw::math::constexpr_math::log2(abs_a);
			double Lb = sw::math::constexpr_math::log2(abs_b);
			double Lmax = (La >= Lb) ? La : Lb;
			double Lmin = (La >= Lb) ? Lb : La;
			double Lresult = Lmax + sb_add(Lmin - Lmax);
			double mag = sw::math::constexpr_math::exp2(Lresult);
			return sign_a ? -mag : mag;
		}
		else {
			double La = sw::math::constexpr_math::log2(abs_a);
			double Lb = sw::math::constexpr_math::log2(abs_b);
			if (La == Lb) return 0.0;
			bool a_larger = (La > Lb);
			double Lmax = a_larger ? La : Lb;
			double Lmin = a_larger ? Lb : La;
			double Lresult = Lmax + sb_sub(Lmin - Lmax);
			if (Lresult != Lresult) return 0.0;
			double mag = sw::math::constexpr_math::exp2(Lresult);
			bool result_neg = a_larger ? sign_a : sign_b;
			return result_neg ? -mag : mag;
		}
	}

public:
	static constexpr Lns& add_assign(Lns& lhs, const Lns& rhs) {
		double result = log_add(double(lhs), double(rhs));
		return lhs = result;
	}
	static constexpr Lns& sub_assign(Lns& lhs, const Lns& rhs) {
		double result = log_add(double(lhs), double(-rhs));
		return lhs = result;
	}
};

// ============================================================================
// Customization point: traits class
// ============================================================================
//
// Default specialization picks DoubleTripAddSub. Users override per
// instantiation:
//
//     namespace sw::universal {
//         template<>
//         struct lns_addsub_traits<lns<16, 8, std::uint16_t>> {
//             using type = DirectEvaluationAddSub<lns<16, 8, std::uint16_t>>;
//         };
//     }

template<typename Lns>
struct lns_addsub_traits {
	using type = DoubleTripAddSub<Lns>;
};

template<typename Lns>
using lns_addsub_algorithm_t = typename lns_addsub_traits<Lns>::type;

}}  // namespace sw::universal
