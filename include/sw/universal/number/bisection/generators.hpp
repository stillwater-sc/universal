#pragma once
// generators.hpp: pre-built generator/refinement pairs for bisection
//
// Each number system in the bisection framework is defined by:
//   g(x): generator producing the bracketing sequence a_i = g^i(1)
//   f(a, b): refinement bisecting a bounded interval
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

namespace sw { namespace universal {

// ── Refinement functions ─────────────────────────────────────────

/// Arithmetic mean: f(a,b) = (a+b)/2
/// Used by Unary, Fibonacci, and as the fraction interpolator for
/// IEEE-like and Posit-like representations.
struct ArithmeticMean {
	double operator()(double a, double b) const {
		return (a + b) / 2.0;
	}
};

/// Geometric mean: f(a,b) = sqrt(a*b)
/// Used by LNS and FP representations. Natural refinement for
/// logarithmic encodings.
struct GeometricMean {
	double operator()(double a, double b) const {
		return std::sqrt(a * b);
	}
};

/// Hyper mean: bridges bracketing and refinement.
/// Uses arithmetic mean within a binade, geometric mean across binades.
/// Used by Posits, Elias codes, and URR.
struct HyperMean {
	double operator()(double a, double b) const {
		if (a <= 0.0 || b <= 0.0) return (a + b) / 2.0;
		if (b <= 2.0 * a) {
			// Within one binade: arithmetic mean
			return (a + b) / 2.0;
		}
		// Across binades: geometric mean of exponents
		double la = std::log2(a);
		double lb = std::log2(b);
		return std::exp2((la + lb) / 2.0);
	}
};

// ── Generator functions ──────────────────────────────────────────

/// Unary: g(x) = x + 1, sequence = 1, 2, 3, 4, ...
/// Simplest possible generator. Linear bracketing.
struct UnaryGenerator {
	double operator()(double x) const { return x + 1.0; }
};

/// Elias gamma: g(x) = 2x, sequence = 1, 2, 4, 8, ...
/// Exponential bracketing with base 2.
struct EliasGammaGenerator {
	double operator()(double x) const { return 2.0 * x; }
};

/// Elias delta: g(x) = 2x^2, sequence = 1, 2, 8, 128, ...
/// Super-exponential bracketing.
struct EliasDeltaGenerator {
	double operator()(double x) const { return 2.0 * x * x; }
};

/// Posit(m): g(x) = b*x where b = 2^(2^m)
/// Posit(0): b=2, Posit(1): b=4, Posit(2): b=16, Posit(3): b=256
template<unsigned m = 0>
struct PositGenerator {
	static constexpr double base = static_cast<double>(uint64_t(1) << (uint64_t(1) << m));
	double operator()(double x) const { return base * x; }
};

/// Fibonacci: g(x) = round(phi * x), sequence = 1, 2, 3, 5, 8, 13, ...
struct FibonacciGenerator {
	double operator()(double x) const {
		static const double phi = (1.0 + std::sqrt(5.0)) / 2.0;
		return std::round(phi * x);
	}
};

// ── Convenience type aliases ─────────────────────────────────────

/// bisection_unary<nbits>: Unary coding with arithmetic mean refinement
template<unsigned nbits, typename bt = uint8_t>
using bisection_unary = bisection<UnaryGenerator, ArithmeticMean, nbits, bt>;

/// bisection_elias_gamma<nbits>: Elias gamma with hyper mean refinement
template<unsigned nbits, typename bt = uint8_t>
using bisection_elias_gamma = bisection<EliasGammaGenerator, HyperMean, nbits, bt>;

/// bisection_elias_delta<nbits>: Elias delta with hyper mean refinement
template<unsigned nbits, typename bt = uint8_t>
using bisection_elias_delta = bisection<EliasDeltaGenerator, HyperMean, nbits, bt>;

/// bisection_posit<nbits, m>: Posit(m) with hyper mean refinement
template<unsigned nbits, unsigned m = 0, typename bt = uint8_t>
using bisection_posit = bisection<PositGenerator<m>, HyperMean, nbits, bt>;

/// bisection_fibonacci<nbits>: Fibonacci coding with arithmetic mean refinement
template<unsigned nbits, typename bt = uint8_t>
using bisection_fibonacci = bisection<FibonacciGenerator, ArithmeticMean, nbits, bt>;

/// bisection_lns<nbits>: LNS-like coding with geometric mean refinement
template<unsigned nbits, typename bt = uint8_t>
using bisection_lns = bisection<EliasGammaGenerator, GeometricMean, nbits, bt>;

}} // namespace sw::universal
