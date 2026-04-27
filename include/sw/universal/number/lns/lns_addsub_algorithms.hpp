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
// The shipped algorithms (Phase A of #777):
//   - DoubleTripAddSub      -- default, preserves current behavior
//   - DirectEvaluationAddSub -- uses sw::math::constexpr_math::log2/exp2
//
// Future phases (#780, #781, #783) will add Lookup, Polynomial, ArnoldBailey,
// and (deferred) CORDIC. All will be drop-in via the same traits specialization.
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
	// special-cased that as zero before reaching here.)
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
