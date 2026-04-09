#pragma once
// generators.hpp: pre-built generator/refinement pairs for bisection
//
// Each number system in the bisection framework is defined by:
//   g(x): generator producing the bracketing sequence a_i = g^i(1)
//   f(a, b): refinement bisecting a bounded interval
//
// All operator() are function templates parameterized on the real type R,
// so the same functor can be used with double, dd, qd, or any type with
// the usual math overloads (sqrt, log2, exp2, ldexp, pow, round).
//
// Reference: Lindstrom, "Universal Coding of the Reals using Bisection",
//            CoNGA'19, Table 1.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
#include <cmath>
#include <algorithm>
#include <limits>

namespace sw { namespace universal {

// -- Refinement functions -----------------------------------------

/// Arithmetic mean: f(a,b) = (a+b)/2
/// Used by Unary, Fibonacci, and as the fraction interpolator for
/// IEEE-like and Posit-like representations.
struct ArithmeticMean {
	template<typename R>
	R operator()(const R& a, const R& b) const {
		return (a + b) / R(2);
	}
};

/// Geometric mean: f(a,b) = sqrt(a*b)
/// Used by LNS and FP representations. Natural refinement for
/// logarithmic encodings.
struct GeometricMean {
	template<typename R>
	R operator()(const R& a, const R& b) const {
		using std::sqrt;
		return sqrt(a * b);
	}
};

/// Hyper mean: bridges bracketing and refinement.
/// Uses arithmetic mean within a binade, geometric mean across binades.
/// Used by Posits, Elias codes, and URR.
struct HyperMean {
	template<typename R>
	R operator()(const R& a, const R& b) const {
		using std::log2;
		using std::exp2;
		if (a <= R(0) || b <= R(0)) return (a + b) / R(2);
		if (b <= R(2) * a) {
			// Within one binade: arithmetic mean
			return (a + b) / R(2);
		}
		// Across binades: geometric mean of exponents
		R la = log2(a);
		R lb = log2(b);
		return exp2((la + lb) / R(2));
	}
};

// -- Generator functions ------------------------------------------

/// Unary: g(x) = x + 1, sequence = 1, 2, 3, 4, ...
/// Simplest possible generator. Linear bracketing.
struct UnaryGenerator {
	template<typename R>
	R operator()(const R& x) const { return x + R(1); }
};

/// Elias gamma: g(x) = 2x, sequence = 1, 2, 4, 8, ...
/// Exponential bracketing with base 2.
struct EliasGammaGenerator {
	template<typename R>
	R operator()(const R& x) const { return R(2) * x; }
};

/// Elias delta: g(x) = 2x^2, sequence = 1, 2, 8, 128, ...
/// Super-exponential bracketing.
struct EliasDeltaGenerator {
	template<typename R>
	R operator()(const R& x) const { return R(2) * x * x; }
};

/// Posit(m): g(x) = b*x where b = 2^(2^m)
/// Posit(0): b=2, Posit(1): b=4, Posit(2): b=16, Posit(3): b=256
template<unsigned m = 0>
struct PositGenerator {
	static_assert(m <= 5, "PositGenerator<m>: 2^(2^m) overflows uint64_t for m > 5");
	static constexpr double base_d = static_cast<double>(uint64_t(1) << (uint64_t(1) << m));
	template<typename R>
	R operator()(const R& x) const { return R(base_d) * x; }
};

/// Fibonacci: g(x) = round(phi * x), sequence = 1, 2, 3, 5, 8, 13, ...
struct FibonacciGenerator {
	template<typename R>
	R operator()(const R& x) const {
		using std::sqrt;
		using std::round;
		// Recompute phi in the target precision so dd/qd get the full
		// precision of the golden ratio rather than a double-truncated value.
		const R phi = (R(1) + sqrt(R(5))) / R(2);
		return round(phi * x);
	}
};

/// Elias omega: g(x) = 2^x, sequence via tetration: 1, 2, 4, 16, 65536, ...
/// The fastest-growing generator in the paper. Clamp output to avoid
/// double overflow (2^(2^16) is far beyond double range).
struct EliasOmegaGenerator {
	template<typename R>
	R operator()(const R& x) const {
		using std::exp2;
		if (x > R(1023)) return R((std::numeric_limits<double>::max)());
		return exp2(x);
	}
};

/// Golden ratio: g(x) = phi * x, sequence = 1, phi, phi^2, phi^3, ...
/// Represents integers as sums of Fibonacci numbers.
struct GoldenRatioGenerator {
	template<typename R>
	R operator()(const R& x) const {
		using std::sqrt;
		const R phi = (R(1) + sqrt(R(5))) / R(2);
		return phi * x;
	}
};

/// URR (Universal Representation of Reals): g(x) = max(2, x^2)
/// Hamada's representation. Uses squaring for super-exponential bracketing
/// after the initial step.
struct URRGenerator {
	template<typename R>
	R operator()(const R& x) const { return (x < R(2)) ? R(2) : x * x; }
};

// FP(m) generator: the paper describes FP(m) with a non-standard
// bracketing sequence g(x) = H(x, 2^(2^(m-1))) that converges to a
// bound rather than growing unboundedly. This requires modifications
// to the bisection framework's bracketing phase and is deferred to
// a future phase. See docs/plans/bisection-coding-analysis.md.

/// LNS(m): Logarithmic number system with m-bit exponent.
/// g(x) = 2^(2^(m-1)) * sqrt(x), sequence = 2^(2^(m-1)(1-2^(-i)))
template<unsigned m = 3>
struct LNSGenerator {
	static_assert(m >= 1, "LNSGenerator<m>: m must be at least 1");
	static_assert(m <= 6, "LNSGenerator<m>: 2^(2^(m-1)) overflows uint64_t for m > 6");
	static constexpr double scale_d = static_cast<double>(uint64_t(1) << (uint64_t(1) << (m - 1)));
	template<typename R>
	R operator()(const R& x) const {
		using std::sqrt;
		return R(scale_d) * sqrt(x);
	}
};

/// Natural refinement: uses the Kolmogorov mean derived from the
/// cumulative distribution function of the number system. For posits,
/// this eliminates wobbling accuracy by providing a smooth mapping
/// from bit strings to reals.
///
/// The natural refinement for a power mean M_p is:
///   f(a, b) = M_p(a, b) = ((a^p + b^p) / 2)^(1/p)
/// For Posit(m), p = -2^(-m).
template<unsigned m = 0>
struct NaturalPositRefinement {
	template<typename R>
	R operator()(const R& a, const R& b) const {
		using std::log2;
		using std::exp2;
		using std::pow;
		using std::ldexp;
		if (a <= R(0) || b <= R(0)) return (a + b) / R(2);
		if (b <= R(2) * a) {
			// Within one binade: use the power mean with p = -2^(-m)
			// ldexp may only be available for double in some toolchains;
			// compute -2^(-m) as a double literal and lift to R.
			const R p = R(-std::ldexp(1.0, -static_cast<int>(m)));
			const R ap = pow(a, p);
			const R bp = pow(b, p);
			return pow((ap + bp) / R(2), R(1) / p);
		}
		// Across binades: geometric mean of exponents
		const R la = log2(a);
		const R lb = log2(b);
		return exp2((la + lb) / R(2));
	}
};

// -- Convenience type aliases -------------------------------------

/// bisection_unary<nbits>: Unary coding with arithmetic mean refinement
template<unsigned nbits, typename bt = uint8_t, typename AuxReal = double>
using bisection_unary = bisection<UnaryGenerator, ArithmeticMean, nbits, bt, AuxReal>;

/// bisection_elias_gamma<nbits>: Elias gamma with hyper mean refinement
template<unsigned nbits, typename bt = uint8_t, typename AuxReal = double>
using bisection_elias_gamma = bisection<EliasGammaGenerator, HyperMean, nbits, bt, AuxReal>;

/// bisection_elias_delta<nbits>: Elias delta with hyper mean refinement
template<unsigned nbits, typename bt = uint8_t, typename AuxReal = double>
using bisection_elias_delta = bisection<EliasDeltaGenerator, HyperMean, nbits, bt, AuxReal>;

/// bisection_posit<nbits, m>: Posit(m) with hyper mean refinement
template<unsigned nbits, unsigned m = 0, typename bt = uint8_t, typename AuxReal = double>
using bisection_posit = bisection<PositGenerator<m>, HyperMean, nbits, bt, AuxReal>;

/// bisection_fibonacci<nbits>: Fibonacci coding with arithmetic mean refinement
template<unsigned nbits, typename bt = uint8_t, typename AuxReal = double>
using bisection_fibonacci = bisection<FibonacciGenerator, ArithmeticMean, nbits, bt, AuxReal>;

/// bisection_lns<nbits>: LNS-like coding with geometric mean refinement
/// (uses Elias gamma generator = base 2, like LNS with 1-bit exponent)
template<unsigned nbits, typename bt = uint8_t, typename AuxReal = double>
using bisection_lns = bisection<EliasGammaGenerator, GeometricMean, nbits, bt, AuxReal>;

/// bisection_elias_omega<nbits>: Elias omega with hyper mean refinement
template<unsigned nbits, typename bt = uint8_t, typename AuxReal = double>
using bisection_elias_omega = bisection<EliasOmegaGenerator, HyperMean, nbits, bt, AuxReal>;

/// bisection_golden<nbits>: Golden ratio base with arithmetic mean.
/// Note: the paper's natural refinement for golden ratio is a power mean
/// with p = -1/log2(phi) ~= -1.44. ArithmeticMean is used here as a
/// working default; a dedicated GoldenRatioPowerMean refinement can be
/// added when the natural refinement framework is generalized.
template<unsigned nbits, typename bt = uint8_t, typename AuxReal = double>
using bisection_golden = bisection<GoldenRatioGenerator, ArithmeticMean, nbits, bt, AuxReal>;

/// bisection_urr<nbits>: Hamada's URR with hyper mean
template<unsigned nbits, typename bt = uint8_t, typename AuxReal = double>
using bisection_urr = bisection<URRGenerator, HyperMean, nbits, bt, AuxReal>;

// bisection_fp: deferred (see FP(m) comment above)

/// bisection_lns_m<nbits, m>: LNS(m) with geometric mean refinement
template<unsigned nbits, unsigned m = 3, typename bt = uint8_t, typename AuxReal = double>
using bisection_lns_m = bisection<LNSGenerator<m>, GeometricMean, nbits, bt, AuxReal>;

/// bisection_natposit<nbits, m>: NaturalPosit with smooth density
template<unsigned nbits, unsigned m = 0, typename bt = uint8_t, typename AuxReal = double>
using bisection_natposit = bisection<PositGenerator<m>, NaturalPositRefinement<m>, nbits, bt, AuxReal>;

}} // namespace sw::universal
