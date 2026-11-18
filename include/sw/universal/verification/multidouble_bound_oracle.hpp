#pragma once
// multidouble_bound_oracle.hpp : dd, qd and the cascades checked against their error bounds, exactly (#2124)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <tuple>

#include <universal/verification/exact_reference.hpp>
#include <universal/verification/rounding_oracle_core.hpp>

/*
   A multi-component value is the exact sum of its double limbs. Its arithmetic is not
   correctly rounded to a grid; it is guaranteed to a relative error bound. So the exact bound
   reference (exact_reference.hpp) does not produce a golden value: it decodes the operands
   exactly into dyadics and checks the result R against the exact result X of the operation,
   exactly:

       + - *   |R - X|         <= t * |X|
       /       |R * b - a|     <= t * |a|                    (|R - a/b| <= t * |a/b|)
       sqrt    a * (1 - t)^2   <= R^2 <= a * (1 + t)^2, R >= 0  (|R - sqrt a| <= t * sqrt a)

   with t = bound(op) * epsilon. No quotient or square root is computed. The measured error,
   in units of epsilon, goes with the verdict. A result whose exact value lies outside
   [min(), max()] is not checked: below min() the last limb is subnormal and epsilon no longer
   bounds the relative error (#1882), and past max() the result overflows.

   The bounds, in units of numeric_limits<T>::epsilon() (2^-105 for dd and dd_cascade, 2^-157
   for td_cascade, 2^-209 for qd and qd_cascade), and where each one comes from:

     dd, dd_cascade   + -  2    Joldes, Muller, Popescu, "Tight and rigorous error bounds for
                                basic building blocks of double-word arithmetic", ACM TOMS 44(2),
                                2017: AccurateDWPlusDW, 3u^2 + 13u^3 with u = 2^-53, which is
                                1.5 epsilon and a little more
                      *    4    same paper, DWTimesDW1, 7u^2 = 3.5 epsilon: the least accurate of
                                its double-word products; dd computes all four partial products
                      /    8    same paper, DWDivDW1, 15u^2 + 56u^3 = 7.5 epsilon and a little more:
                                its least accurate double-word quotient; dd takes three quotient terms
                      sqrt 2    no published bound for this algorithm; the #1725/#1883 sqrt audit
                                measured a worst of 1.99 u^2, about 1 epsilon
     td_cascade,      all  4    no published bound for these algorithms (Hida, Li, Bailey, "Algorithms
     qd, qd_cascade             for quad-double precision floating point arithmetic", 2001, gives
                                none that is tight); the worst measured over random normalized
                                operands is below 0.12 epsilon, so 4 leaves a wide margin

   A bound check catches an error larger than the bound, not every error: one unit in the last
   place of the last limb is about 0.25 to 0.5 epsilon, which lies inside every bound above.
*/

namespace sw {
namespace universal {

class dd;
class qd;
class dd_cascade;
class td_cascade;
class qd_cascade;

// the number of double limbs of a multi-component type; zero for any other type
template<typename T> inline constexpr unsigned multidouble_limbs = 0;
template<> inline constexpr unsigned multidouble_limbs<dd>         = 2;
template<> inline constexpr unsigned multidouble_limbs<dd_cascade> = 2;
template<> inline constexpr unsigned multidouble_limbs<td_cascade> = 3;
template<> inline constexpr unsigned multidouble_limbs<qd>         = 4;
template<> inline constexpr unsigned multidouble_limbs<qd_cascade> = 4;

namespace multidouble_oracle {

using rounding_oracle::Dyadic;
using rounding_oracle::addExact;
using rounding_oracle::mulExact;
using rounding_oracle::cmpMagnitude;

// the bound of op ('+', '-', '*', '/', 'r' for sqrt), in units of epsilon (see above)
template<typename T>
constexpr double bound(char op) {
	if constexpr (multidouble_limbs<T> == 2) {
		switch (op) {
		case '+': case '-': return 2.0;
		case '*':           return 4.0;
		case '/':           return 8.0;
		default:            return 2.0;   // sqrt
		}
	}
	else {
		(void)op;
		return 4.0;
	}
}

// the exact value of a multi-component number: the sum of its limbs
template<typename T>
Dyadic valueOf(const T& v) {
	Dyadic s;
	for (unsigned i = 0; i < multidouble_limbs<T>; ++i) s = addExact(s, rounding_oracle::valueOf(double(v[i])));
	return s;
}

template<typename T>
bool isFinite(const T& v) {
	for (unsigned i = 0; i < multidouble_limbs<T>; ++i) if (!std::isfinite(double(v[i]))) return false;
	return true;
}

inline Dyadic magnitude(Dyadic d) { d.neg = false; return d; }
inline Dyadic negated(Dyadic d) { d.neg = !d.neg; return d; }

// |a| / |b| as a double, for the report; both nonzero
inline double approxRatio(const Dyadic& a, const Dyadic& b) {
	auto top = [](const Dyadic& d, int& e) {
		const std::size_t n = d.m.w.size();
		const std::size_t take = n < 3 ? n : 3;
		double m = 0.0;
		for (std::size_t i = 0; i < take; ++i) m = m * 4294967296.0 + d.m.w[n - 1 - i];
		e = d.e + 32 * static_cast<int>(n - take);
		return m;
	};
	int ea{ 0 }, eb{ 0 };
	const double ma = top(a, ea), mb = top(b, eb);
	return std::ldexp(ma / mb, ea - eb);
}

template<typename T>
T fromLimbs(const std::array<double, multidouble_limbs<T>>& x) {
	return std::apply([](auto... c) { return T(c...); }, x);
}

}  // namespace multidouble_oracle

// the exact bound reference of dd, qd and the cascades
template<typename T>
	requires (multidouble_limbs<T> > 0)
struct exact_bound<T> {
	static constexpr unsigned L = multidouble_limbs<T>;

	static double bound(char op) { return multidouble_oracle::bound<T>(op); }

	static double epsilon() { return double(std::numeric_limits<T>::epsilon()[0]); }

	// t * |m| for the operator's bound t = bound * epsilon, exactly
	static rounding_oracle::Dyadic scaled(char op, const rounding_oracle::Dyadic& m) {
		return rounding_oracle::mulExact(rounding_oracle::valueOf(bound(op) * epsilon()), multidouble_oracle::magnitude(m));
	}

	// the range in which the bound holds: [min(), max()] in magnitude
	static bool inRange(const rounding_oracle::Dyadic& x) {
		using namespace multidouble_oracle;
		const Dyadic lo = rounding_oracle::valueOf(double(std::numeric_limits<T>::min()[0]));
		const Dyadic hi = valueOf(std::numeric_limits<T>::max());
		return cmpMagnitude(x, lo) >= 0 && cmpMagnitude(x, hi) <= 0;
	}

	static bound_verdict binary(char op, const T& a, const T& b, const T& result, double& errorInEps) {
		using namespace multidouble_oracle;
		errorInEps = 0.0;
		if (op != '+' && op != '-' && op != '*' && op != '/') return bound_verdict::unchecked;
		if (!isFinite(a) || !isFinite(b)) return bound_verdict::unchecked;
		const Dyadic x = valueOf(a);
		Dyadic y = valueOf(b);
		if (op == '/') {
			if (y.m.zero()) return bound_verdict::unchecked;
			if (x.m.zero()) return double(result[0]) == 0.0 ? bound_verdict::pass : bound_verdict::fail;
			// |x/y| inside [min, max]: |x| against min * |y| and max * |y|
			const Dyadic lo = rounding_oracle::valueOf(double(std::numeric_limits<T>::min()[0]));
			const Dyadic hi = valueOf(std::numeric_limits<T>::max());
			if (cmpMagnitude(x, mulExact(lo, y)) < 0 || cmpMagnitude(x, mulExact(hi, y)) > 0) return bound_verdict::unchecked;
			if (!isFinite(result)) { errorInEps = std::numeric_limits<double>::infinity(); return bound_verdict::fail; }
			const Dyadic residual = addExact(mulExact(valueOf(result), y), negated(x));   // R*b - a
			errorInEps = residual.m.zero() ? 0.0 : approxRatio(residual, x) / epsilon();
			return cmpMagnitude(residual, scaled(op, x)) <= 0 ? bound_verdict::pass : bound_verdict::fail;
		}
		const Dyadic X = (op == '*') ? mulExact(x, y) : addExact(x, (op == '-') ? negated(y) : y);
		if (X.m.zero()) return double(result[0]) == 0.0 ? bound_verdict::pass : bound_verdict::fail;
		if (!inRange(X)) return bound_verdict::unchecked;
		if (!isFinite(result)) { errorInEps = std::numeric_limits<double>::infinity(); return bound_verdict::fail; }
		const Dyadic E = addExact(valueOf(result), negated(X));   // R - X
		errorInEps = E.m.zero() ? 0.0 : approxRatio(E, X) / epsilon();
		return cmpMagnitude(E, scaled(op, X)) <= 0 ? bound_verdict::pass : bound_verdict::fail;
	}

	static bound_verdict sqrt(const T& a, const T& result, double& errorInEps) {
		using namespace multidouble_oracle;
		errorInEps = 0.0;
		if (!isFinite(a)) return bound_verdict::unchecked;
		const Dyadic x = valueOf(a);
		if (x.m.zero()) return double(result[0]) == 0.0 ? bound_verdict::pass : bound_verdict::fail;
		if (x.neg) return bound_verdict::unchecked;
		if (!isFinite(result)) { errorInEps = std::numeric_limits<double>::infinity(); return bound_verdict::fail; }
		const Dyadic r = valueOf(result);
		if (r.neg) { errorInEps = std::numeric_limits<double>::infinity(); return bound_verdict::fail; }
		const Dyadic r2 = mulExact(r, r);
		const Dyadic t = rounding_oracle::valueOf(bound('r') * epsilon());
		const Dyadic one = rounding_oracle::valueOf(1.0);
		const Dyadic up = addExact(one, t), down = addExact(one, negated(t));
		const Dyadic diff = addExact(r2, negated(x));
		errorInEps = diff.m.zero() ? 0.0 : approxRatio(diff, x) / 2.0 / epsilon();   // |R^2 - a| / 2a ~ |R - sqrt a| / sqrt a
		const bool above = cmpMagnitude(r2, mulExact(x, mulExact(down, down))) >= 0;
		const bool below = cmpMagnitude(r2, mulExact(x, mulExact(up, up))) <= 0;
		return (above && below) ? bound_verdict::pass : bound_verdict::fail;
	}

	// a normalized random value: a leading double of random sign, exponent in [-300, 300] and a
	// full random fraction, each further limb random and below half an ulp of the one above it;
	// one in eight with the lowest limbs zero
	template<typename Engine>
	static void random(T& v, Engine& eng) {
		std::uniform_real_distribution<double> unit(-1.0, 1.0);
		std::uniform_int_distribution<int> exponent(-300, 300);
		std::uniform_int_distribution<unsigned> shortTail(0, 7);
		std::array<double, L> x{};
		const double lead = 1.0 + std::fabs(unit(eng));
		x[0] = std::ldexp(unit(eng) < 0.0 ? -lead : lead, exponent(eng));
		const unsigned nonzero = (shortTail(eng) == 0) ? 1u + static_cast<unsigned>(eng() % L) : L;
		for (unsigned i = 1; i < nonzero && i < L; ++i) {
			int e{ 0 };
			(void)std::frexp(x[i - 1], &e);
			x[i] = std::ldexp(0.5 * unit(eng), e - 53);   // |x[i]| < ulp(x[i-1]) / 2
		}
		v = multidouble_oracle::fromLimbs<T>(x);
	}

	// b close to -a: a's limbs negated with the last one replaced by a random value below half
	// an ulp of the lowest nonzero limb above it, so a + b cancels all but that last scale
	template<typename Engine>
	static void randomNearNegation(const T& a, T& b, Engine& eng) {
		std::uniform_real_distribution<double> unit(-1.0, 1.0);
		std::array<double, L> x{};
		for (unsigned i = 0; i < L; ++i) x[i] = -double(a[i]);
		unsigned k = 0;   // the lowest nonzero limb above the last
		for (unsigned i = 1; i + 1 < L; ++i) if (x[i] != 0.0) k = i;
		int e{ 0 };
		(void)std::frexp(x[k], &e);
		for (unsigned i = k + 1; i < L; ++i) x[i] = 0.0;
		x[L - 1] = std::ldexp(0.5 * unit(eng), e - 53);
		b = multidouble_oracle::fromLimbs<T>(x);
	}
};

}  // namespace universal
}  // namespace sw
