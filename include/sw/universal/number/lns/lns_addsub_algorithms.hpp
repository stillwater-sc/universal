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
// The shipped algorithms (Phases A, B, and C of #777):
//   - DoubleTripAddSub        -- default, preserves current behavior
//   - DirectEvaluationAddSub  -- uses sw::math::constexpr_math::log2/exp2
//   - LookupAddSub            -- Mitchell-style precomputed table + linear interp
//   - PolynomialAddSub        -- (1+x)/(1-x) substitution + degree-7 odd polynomial
//   - ArnoldBaileyAddSub      -- piecewise-linear, no transcendentals
//
// Future phase (#783) will add CORDIC (deferred). All policies are drop-in
// via the same traits specialization.
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
// Shared log-add dispatcher (used by all sb_add / sb_sub-based policies)
// ============================================================================
//
// Routes special values (NaN, +/-0, +/-inf), then dispatches by sign to either
// same-sign log-add or mixed-sign magnitude subtraction. The Gauss log-add path
// only sees finite, non-zero operands. Calls Policy::sb_add(d) for same-sign
// and Policy::sb_sub(d) for mixed-sign with d <= 0.
//
// Policies that build on this dispatcher only need to implement sb_add and
// sb_sub (per the algorithm contract documented at the top of this file).

namespace detail {

template<typename Policy>
constexpr double gauss_log_add(double a, double b) {
	if (a != a || b != b) return a + b;                                   // NaN propagates
	if (a == 0.0) return b;
	if (b == 0.0) return a;
	// Infinities: native double arithmetic gives the correct IEEE result
	// for all combinations (inf + inf = inf, inf + (-inf) = NaN,
	// inf + finite = inf). The Gauss log-add path below assumes finite,
	// non-zero operands; routing inf through log2 would produce inf-inf
	// in the same-sign branch and a spurious 0 from the La == Lb check in
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
		// Same sign: log2(|a + b|) = max(La, Lb) + sb_add(min - max)
		double La = sw::math::constexpr_math::log2(abs_a);
		double Lb = sw::math::constexpr_math::log2(abs_b);
		double Lmax = (La >= Lb) ? La : Lb;
		double Lmin = (La >= Lb) ? Lb : La;
		double Lresult = Lmax + Policy::sb_add(Lmin - Lmax);
		double mag = sw::math::constexpr_math::exp2(Lresult);
		return sign_a ? -mag : mag;
	}
	else {
		// Mixed sign: magnitude subtraction; result sign follows larger magnitude.
		double La = sw::math::constexpr_math::log2(abs_a);
		double Lb = sw::math::constexpr_math::log2(abs_b);
		if (La == Lb) return 0.0;                                            // exact cancellation
		bool a_larger = (La > Lb);
		double Lmax = a_larger ? La : Lb;
		double Lmin = a_larger ? Lb : La;
		double Lresult = Lmax + Policy::sb_sub(Lmin - Lmax);
		// log2 of zero or negative -- happens when the sb_sub argument
		// rounds to 0; treat as exact cancellation.
		if (Lresult != Lresult) return 0.0;
		double mag = sw::math::constexpr_math::exp2(Lresult);
		bool result_neg = a_larger ? sign_a : sign_b;
		return result_neg ? -mag : mag;
	}
}

}  // namespace detail

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
	// log2(1 + 2^d) for d <= 0. As d -> -infinity, correction -> 0.
	// As d -> 0, correction -> 1. The shared dispatcher in detail::gauss_log_add
	// only invokes sb_add with finite d <= 0.
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

	static constexpr Lns& add_assign(Lns& lhs, const Lns& rhs) {
		double result = detail::gauss_log_add<DirectEvaluationAddSub>(double(lhs), double(rhs));
		return lhs = result;
	}
	static constexpr Lns& sub_assign(Lns& lhs, const Lns& rhs) {
		// a - b = a + (-b); negate via the lns operator-() which is a single
		// sign-bit flip.
		double result = detail::gauss_log_add<DirectEvaluationAddSub>(double(lhs), double(-rhs));
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
	// Shift safety: 1 << IndexBits must fit in std::size_t. On narrow ABIs
	// (e.g., 16-bit embedded targets) std::size_t may be smaller than 30 bits,
	// so the SRAM cap below is not by itself a sufficient shift-width guard.
	static_assert(IndexBits < std::numeric_limits<std::size_t>::digits,
	              "LookupAddSub: IndexBits must be smaller than the bit width of std::size_t");
	// SRAM cap: 2^30 doubles = 8 GB, far past any realistic embedded budget.
	static_assert(IndexBits < 30,
	              "LookupAddSub: IndexBits >= 30 would exceed practical SRAM budgets");
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

public:
	// Linear interp: log2(1 + 2^d) for d <= 0. The shared dispatcher in
	// detail::gauss_log_add only invokes sb_add with finite d <= 0.
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

	static constexpr Lns& add_assign(Lns& lhs, const Lns& rhs) {
		double result = detail::gauss_log_add<LookupAddSub>(double(lhs), double(rhs));
		return lhs = result;
	}
	static constexpr Lns& sub_assign(Lns& lhs, const Lns& rhs) {
		double result = detail::gauss_log_add<LookupAddSub>(double(lhs), double(-rhs));
		return lhs = result;
	}
};

// ============================================================================
// Algorithm 4: PolynomialAddSub -- closed-form via (1+x)/(1-x) substitution
// ============================================================================
//
// Approximates sb_add(d) = log2(1 + 2^d) for d in (-inf, 0] using the classical
// log-domain identity
//
//     log2((1+x)/(1-x)) = (2 / ln 2) * (x + x^3/3 + x^5/5 + ...)
//
// Setting (1+x)/(1-x) = 1 + u (where u = 2^d in (0, 1]), we get
//
//     x = u / (2 + u)
//
// which maps u in (0, 1] to x in (0, 1/3]. The series in odd powers of x then
// converges rapidly: |x|^9 / 9 <= (1/3)^9 / 9 ~= 5.6e-6 over the entire
// domain, so a degree-7 truncation
//
//     log2(1 + u) ~= (2/ln 2) * (x + x^3/3 + x^5/5 + x^7/7)
//
// gives ~5.6e-6 absolute error worst-case. No table, one transcendental at
// runtime (the 2^d to recover u from d), and a single division.
//
// For sb_sub(d) = log2(1 - 2^d) the analogous substitution gives
// x = u / (2 - u), but x stays in [0, 1/3] only for u <= 0.5 (i.e., d <= -1).
// Past that we are in the catastrophic-cancellation regime where any series
// expansion in x diverges; same as Lookup, we fall back to direct
// cm::log2/cm::exp2 evaluation in that band.
//
// Use case: zero memory cost, predictable latency, ~6e-5 ULP error. A good
// default for general-purpose firmware where SRAM is at a premium.

template<typename Lns>
struct PolynomialAddSub {
	// log2(1 + 2^d) for d <= 0. Returns 0 once 2^d is below useful precision.
	static constexpr double sb_add(double d) {
		if (d > 0.0) d = 0.0;
		// Past d ~ -(rbits + 2), 2^d is below the lns ULP and the correction
		// is indistinguishable from 0 after encoding.
		constexpr double d_floor = -(double(Lns::rbits) + 2.0);
		if (d < d_floor) return 0.0;

		double u = sw::math::constexpr_math::exp2(d);    // 2^d in (0, 1]
		double x = u / (2.0 + u);                         // x in (0, 1/3]
		double x2 = x * x;
		double x3 = x * x2;
		double x5 = x3 * x2;
		double x7 = x5 * x2;
		// Coefficients: c_k = 2 / (k * ln 2) for odd k = 1, 3, 5, 7
		constexpr double inv_ln2 = 1.4426950408889634;    // 1 / ln(2)
		constexpr double c1 = 2.0 * inv_ln2;
		constexpr double c3 = c1 / 3.0;
		constexpr double c5 = c1 / 5.0;
		constexpr double c7 = c1 / 7.0;
		return c1 * x + c3 * x3 + c5 * x5 + c7 * x7;
	}

	// log2(1 - 2^d) for d < 0. Falls back to direct evaluation in the
	// cancellation regime u > 0.5 (where x = u/(2-u) > 1/3 and the series
	// no longer converges fast enough to be useful).
	static constexpr double sb_sub(double d) {
		if (d >= 0.0) return 0.0;
		constexpr double d_floor = -(double(Lns::rbits) + 2.0);
		if (d < d_floor) return 0.0;

		double u = sw::math::constexpr_math::exp2(d);     // 2^d in (0, 1)
		if (u > 0.5) {
			// Cancellation regime: direct evaluation.
			double t = 1.0 - u;
			return sw::math::constexpr_math::log2(t);
		}
		// (1-x)/(1+x) = 1 - u  =>  x = u / (2 - u). For u <= 0.5: x <= 1/3.
		double x = u / (2.0 - u);
		double x2 = x * x;
		double x3 = x * x2;
		double x5 = x3 * x2;
		double x7 = x5 * x2;
		constexpr double inv_ln2 = 1.4426950408889634;
		constexpr double c1 = 2.0 * inv_ln2;
		constexpr double c3 = c1 / 3.0;
		constexpr double c5 = c1 / 5.0;
		constexpr double c7 = c1 / 7.0;
		return -(c1 * x + c3 * x3 + c5 * x5 + c7 * x7);
	}

	static constexpr Lns& add_assign(Lns& lhs, const Lns& rhs) {
		double result = detail::gauss_log_add<PolynomialAddSub>(double(lhs), double(rhs));
		return lhs = result;
	}
	static constexpr Lns& sub_assign(Lns& lhs, const Lns& rhs) {
		double result = detail::gauss_log_add<PolynomialAddSub>(double(lhs), double(-rhs));
		return lhs = result;
	}
};

// ============================================================================
// Algorithm 5: ArnoldBaileyAddSub -- piecewise-linear, zero transcendentals
// ============================================================================
//
// Cheapest of the configurable algorithms: the function sb_add(d) is
// approximated by a small set of linear pieces matching the function value
// at integer d-knots. Each piece evaluates as a + b*d -- two multiplies, one
// add, no transcendentals, no table lookup, no division.
//
// Knots (chosen so the function value is just sw::math::constexpr_math::log2
// at compile time):
//
//     d =  0 -> sb_add =  1.000000
//     d = -1 -> sb_add =  0.584963   (= log2(1.5))
//     d = -2 -> sb_add =  0.321928   (= log2(1.25))
//     d = -3 -> sb_add =  0.169925   (= log2(1.125))
//     d = -4 -> sb_add =  0.087463   (= log2(1.0625))
//     d = -5 -> sb_add =  0.044394   (= log2(1.03125))
//     d <= -6 -> sb_add ~= 0
//
// Within each interval [k-1, k] we use the secant a + b*d (i.e., linear
// interpolation between consecutive knots). The maximum error is bounded by
// the curvature of log2(1 + 2^d), which is largest near d = 0 (~2.5%) and
// drops rapidly as d -> -infinity.
//
// References
// ----------
// The piecewise-linear approximation at integer-d knots traces back to:
//
//   Mitchell, J. N. (1962). "Computer multiplication and division using
//   binary logarithms." IRE Transactions on Electronic Computers, EC-11(4),
//   512-517. doi:10.1109/TEC.1962.5219391
//
// Arnold, M. G. and Bailey, T. A. (and collaborators Cowles, Cuthbertson,
// Walters, Newcomer) extended this in a series of LNS-arithmetic papers in
// the late 1980s and 1990s, exploring multi-knot piecewise approximations
// and hardware-friendly closed forms. Representative entries:
//
//   Arnold, M. G., Bailey, T. A., Cowles, J. R., Cuthbertson, K. (1990).
//   "An Improved Logarithmic Number System Architecture." Journal of VLSI
//   Signal Processing, 1(1), 13-20.
//
//   Arnold, M. G. and Walter, J. (2000). "Unrestricted faithful rounding is
//   good enough for some LNS applications." Proc. 15th IEEE Symposium on
//   Computer Arithmetic, 237-246.
//
// The implementation here is "in the style of" that family rather than a
// direct port of any single paper -- it picks a small, hand-readable knot
// set sufficient to bound worst-case error at ~2.5% over the lns dynamic
// range, leaving more sophisticated multi-segment fits to specialization.
//
// Use case: lowest energy per operation, predictable latency, no SRAM, modest
// accuracy (~2.5% relative error worst-case). Good fit for energy-constrained
// edge AI inference where the LNS storage already costs more than the add
// circuitry.

template<typename Lns>
struct ArnoldBaileyAddSub {
private:
	// Knot values, computed at compile time so we do not bake in approximate
	// constants. f(k) = log2(1 + 2^k), k in [-5, 0].
	static constexpr double f_at_zero    = 1.0;                                                    // log2(2)
	static constexpr double f_at_minus1  = sw::math::constexpr_math::log2(1.0 + 0.5);              // log2(1.5)
	static constexpr double f_at_minus2  = sw::math::constexpr_math::log2(1.0 + 0.25);             // log2(1.25)
	static constexpr double f_at_minus3  = sw::math::constexpr_math::log2(1.0 + 0.125);            // log2(1.125)
	static constexpr double f_at_minus4  = sw::math::constexpr_math::log2(1.0 + 0.0625);           // log2(1.0625)
	static constexpr double f_at_minus5  = sw::math::constexpr_math::log2(1.0 + 0.03125);          // log2(1.03125)

	// Linear interpolation between two knots given d in [d_left, d_right].
	static constexpr double interp(double d, double d_left, double d_right,
	                                double f_left, double f_right) {
		double frac = (d - d_left) / (d_right - d_left);
		return f_left + frac * (f_right - f_left);
	}

public:
	static constexpr double sb_add(double d) {
		if (d > 0.0) d = 0.0;
		if (d <= -6.0) return 0.0;
		// Piecewise-linear over [-5, 0]; tail beyond -5 ramps to 0 by -6.
		if (d > -1.0) return interp(d, -1.0, 0.0, f_at_minus1, f_at_zero);
		if (d > -2.0) return interp(d, -2.0, -1.0, f_at_minus2, f_at_minus1);
		if (d > -3.0) return interp(d, -3.0, -2.0, f_at_minus3, f_at_minus2);
		if (d > -4.0) return interp(d, -4.0, -3.0, f_at_minus4, f_at_minus3);
		if (d > -5.0) return interp(d, -5.0, -4.0, f_at_minus5, f_at_minus4);
		// d in [-6, -5]: ramp f_at_minus5 -> 0
		return interp(d, -6.0, -5.0, 0.0, f_at_minus5);
	}

	// log2(1 - 2^d) for d < 0. Knots derived analogously, with direct-eval
	// fallback in the cancellation regime u > 0.5 (i.e., d > -1) where the
	// function has unbounded slope and any low-degree fit is poor.
	static constexpr double sb_sub(double d) {
		if (d >= 0.0) return 0.0;
		if (d <= -6.0) return 0.0;  // log2(1 - tiny) ~= 0
		if (d > -1.0) {
			// Cancellation regime: direct evaluation.
			double t = 1.0 - sw::math::constexpr_math::exp2(d);
			return sw::math::constexpr_math::log2(t);
		}
		// d in [-6, -1]: g(d) = log2(1 - 2^d) is well-behaved here.
		// Knots: g(-1)=-1, g(-2)=log2(0.75)=-0.4150, g(-3)=log2(0.875)=-0.1926,
		//        g(-4)=-0.0931, g(-5)=-0.0458.
		constexpr double g_at_minus1 = -1.0;
		constexpr double g_at_minus2 = sw::math::constexpr_math::log2(0.75);
		constexpr double g_at_minus3 = sw::math::constexpr_math::log2(0.875);
		constexpr double g_at_minus4 = sw::math::constexpr_math::log2(0.9375);
		constexpr double g_at_minus5 = sw::math::constexpr_math::log2(0.96875);
		if (d > -2.0) return interp(d, -2.0, -1.0, g_at_minus2, g_at_minus1);
		if (d > -3.0) return interp(d, -3.0, -2.0, g_at_minus3, g_at_minus2);
		if (d > -4.0) return interp(d, -4.0, -3.0, g_at_minus4, g_at_minus3);
		if (d > -5.0) return interp(d, -5.0, -4.0, g_at_minus5, g_at_minus4);
		return interp(d, -6.0, -5.0, 0.0, g_at_minus5);
	}

	static constexpr Lns& add_assign(Lns& lhs, const Lns& rhs) {
		double result = detail::gauss_log_add<ArnoldBaileyAddSub>(double(lhs), double(rhs));
		return lhs = result;
	}
	static constexpr Lns& sub_assign(Lns& lhs, const Lns& rhs) {
		double result = detail::gauss_log_add<ArnoldBaileyAddSub>(double(lhs), double(-rhs));
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

// ============================================================================
// Per-algorithm tolerance contract (for regression suite consumers)
// ============================================================================
//
// Each policy advertises an upper bound on its **log-domain** error in
// sb_add(d) / sb_sub(d), so a regression test that must work with any
// configured algorithm can derive the right value-domain tolerance.
//
// We track log-domain rather than value-domain error because of the
// encoding-boundary amplification:
//
//   Algorithm log-domain error E can shift the encoded result by up to
//   ceil(E / 2^-rbits) ULPs (plus 1 for rounding noise). One lns ULP in
//   the encoding maps to a value-domain relative error of (2^(2^-rbits) - 1),
//   which is 19% at rbits=2 but only 0.27% at rbits=8 -- so a flat
//   per-algorithm value-domain bound would be wrong by orders of magnitude
//   across configurations. The helper below derives the correct tolerance
//   from the algorithm's log-domain bound and the LnsType's rbits.
//
// Bounds (matching the benchmark envelopes from
// benchmark/performance/arithmetic/lns/log_add_algorithms.cpp):
//
//   DoubleTripAddSub          -- 0 (routes through native double, exact)
//   DirectEvaluationAddSub    -- 0 (oracle, by construction)
//   LookupAddSub<Lns, 10>     -- ~1e-4 log-domain at default IndexBits
//   PolynomialAddSub          -- ~5e-6 log-domain (degree-7 truncation)
//   ArnoldBaileyAddSub        -- ~0.025 log-domain (piecewise-linear secant)
//
// Users specializing the traits to a custom algorithm must specialize this
// trait too (default is 0, forcing bit-exact comparison).

template<typename Alg>
struct lns_addsub_log_error_bound {
	static constexpr double value = 0.0;  // default: exact
};

template<typename Lns>
struct lns_addsub_log_error_bound<DoubleTripAddSub<Lns>> {
	static constexpr double value = 0.0;
};
template<typename Lns>
struct lns_addsub_log_error_bound<DirectEvaluationAddSub<Lns>> {
	static constexpr double value = 0.0;
};
template<typename Lns, unsigned IndexBits>
struct lns_addsub_log_error_bound<LookupAddSub<Lns, IndexBits>> {
	// Linear-interp error in sb_add at default IndexBits.
	static constexpr double value = 1.0e-4;
};
template<typename Lns>
struct lns_addsub_log_error_bound<PolynomialAddSub<Lns>> {
	// Degree-7 (1+x)/(1-x) truncation: x^9/9 at x=1/3 gives 5.6e-6.
	static constexpr double value = 1.0e-5;
};
template<typename Lns>
struct lns_addsub_log_error_bound<ArnoldBaileyAddSub<Lns>> {
	// Piecewise-linear secant error worst case ~0.02 log-domain.
	static constexpr double value = 2.5e-2;
};

template<typename Alg>
inline constexpr double lns_addsub_log_error_bound_v = lns_addsub_log_error_bound<Alg>::value;

// Compare two lns values using the active algorithm's tolerance contract.
// Bit-exact for exact algorithms (rel_tol == 0); for approximate algorithms,
// derive a value-domain relative tolerance from the log-domain error bound
// and the LnsType's rbits, accounting for encoding-boundary ULP shifts.
template<typename LnsType>
constexpr bool lns_eq_within_alg_tolerance(const LnsType& c, const LnsType& cref) {
	using Alg = lns_addsub_algorithm_t<LnsType>;
	constexpr double E = lns_addsub_log_error_bound_v<Alg>;
	// Both-NaN counts as equivalent in regression terms (lns has no payload
	// distinction, and IEEE-defined NaN != NaN would otherwise spuriously
	// fail symmetric NaN-propagation cases). One-sided NaN is always a fail.
	if (c.isnan() && cref.isnan()) return true;
	if (c.isnan() != cref.isnan()) return false;
	if constexpr (E == 0.0) {
		return c == cref;
	}
	else {
		if (c == cref) return true;

		// One lns ULP in log domain.
		constexpr double log_ulp = 1.0 / static_cast<double>(1ull << LnsType::rbits);
		// Max ULPs the algorithm error can shift the encoded result, plus
		// 2 for boundary-rounding noise. (The +2 covers a half-ULP rounding
		// on each side of a boundary.)
		constexpr double ulp_shift = (E / log_ulp) + 2.0;
		// Value-domain relative error for that ULP shift:
		//   |2^(N * log_ulp) - 1| where N = ulp_shift.
		// For small N * log_ulp this is approximately N * log_ulp * ln(2);
		// for large it grows exponentially. Use cm::exp2 for the exact form.
		constexpr double rel_tol =
			sw::math::constexpr_math::exp2(ulp_shift * log_ulp) - 1.0;

		double vc   = double(c);
		double vref = double(cref);
		double diff = vc - vref;
		if (diff < 0.0) diff = -diff;
		double mag = (vref < 0.0 ? -vref : vref);
		// Floor the magnitude at 1.0 so small-value cases use absolute
		// (rather than meaninglessly-large relative) error.
		if (mag < 1.0) mag = 1.0;
		return (diff / mag) <= rel_tol;
	}
}

}}  // namespace sw::universal
