#pragma once
// error_free_ops.hpp: definition of error free arithmetic functions for native floating-point types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cmath>
#include <limits>
#include <type_traits>

/*
A key property of faithful floating-point arithmetic is that rounding error of an arithmetic operation
can be correctly represented by the arithmetic.

We have the assertion that a + b = s + r for all values in the encoding

 */

// this logic needs to go into a configuration file that is constructed by the build process
// or handrolled for each compiler and its intrinsics

// If fused multiply-add is available, define to correct macro for using it.
// It is invoked as UNIVERSAL_FMA(a, b, c) to compute fl(a * b + c).
// If correctly rounded multiply-add is not available (or if unsure), keep it undefined.
#ifndef RELIABLE_FUSED_MULTIPLY_ADD_OPERATOR
	/* #undef UNIVERSAL_FMA */
#endif

// If fused multiply-subtract is available, define to correct macro for using it.
// It is invoked as UNIVERSAL_FMS(a, b, c) to compute fl(a * b - c).
// If correctly rounded multiply-subtract is not available (or if unsure), keep it undefined.
#ifndef RELIABLE_FUSED_MULTIPLY_SUBTRACT_OPERATOR
   /* #undef UNIVERSAL_FMS */
#endif

namespace sw { namespace universal {

	// Constexpr-safe finiteness check.
	// std::isfinite is not constexpr until C++23; this implementation works
	// during constant evaluation by relying on well-defined IEEE-754 NaN/Inf
	// comparison semantics.
	constexpr bool is_finite_cx(double x) noexcept {
		return !(x != x)
			&& x != std::numeric_limits<double>::infinity()
			&& x != -std::numeric_limits<double>::infinity();
	}

	constexpr bool is_inf_cx(double x) noexcept {
		return x == std::numeric_limits<double>::infinity()
			|| x == -std::numeric_limits<double>::infinity();
	}

	// TwoSums

	/// <summary>
	/// quick_two_sum computes the relationship a + b = s + r
	/// requires its arguments to be |a| >= |b|
	/// </summary>
	/// <param name="a">input</param>
	/// <param name="b">input</param>
	/// <param name="r">reference to the residual</param>
	/// <returns>the sum s</returns>
	constexpr inline double quick_two_sum(double a, double b, double& r) {
		if (std::is_constant_evaluated()) {
			double s = a + b;
			r = is_finite_cx(s) ? b - (s - a) : 0.0;
			return s;
		}
		else {
			volatile double s = a + b;
			r = (std::isfinite(s) ? b - (s - a) : 0.0);
			return s;
		}
	}

	/// <summary>
	/// two_sum computes the relationship a + b = s + r
	/// </summary>
	/// <param name="a">input</param>
	/// <param name="b">input</param>
	/// <param name="r">reference to the residual</param>
	/// <returns>the sum s</returns>
	constexpr inline double two_sum(double a, double b, double& r) {
		if (std::is_constant_evaluated()) {
			double s = a + b;
			if (is_finite_cx(s)) {
				double bb = s - a;
				r = (a - (s - bb)) + (b - bb);
			}
			else {
				r = 0.0;
			}
			return s;
		}
		else {
			volatile double s = a + b;
			if (std::isfinite(s)) {
				volatile double bb = s - a;
				r = (a - (s - bb)) + (b - bb);
			}
			else {
				r = 0.0;
			}
			return s;
		}
	}


	// TwoDiff

	/// <summary>
	/// quick_two_diff computes the relationship a - b = s + r
	/// notice the sign of s + r, this determines the sign of the residual
	/// requires its arguments to be |a| >= |b|
	/// </summary>
	/// <param name="a">input</param>
	/// <param name="b">input</param>
	/// <param name="r">reference to the residual</param>
	/// <returns>the sum s</returns>
	constexpr inline double quick_two_diff(double a, double b, double& r) {
		if (std::is_constant_evaluated()) {
			double s = a - b;
			r = is_finite_cx(s) ? (a - s) - b : 0.0;
			return s;
		}
		else {
			volatile double s = a - b;
			r = (std::isfinite(s) ? (a - s) - b : 0.0);
			return s;
		}
	}

	/// <summary>
	/// two_diff computes the relationship a - b = s + r
	/// notice the sign of s + r, this determines the sign of the residual
	/// </summary>
	/// <typeparam name="NativeFloat"></typeparam>
	/// <param name="a">input</param>
	/// <param name="b">input</param>
	/// <param name="r">reference to the residual</param>
	/// <returns>the difference s</returns>
	constexpr inline double two_diff(double a, double b, double& r) {
		if (std::is_constant_evaluated()) {
			double s = a - b;
			if (is_finite_cx(s)) {
				double bb = s - a;
				r = (a - (s - bb)) - (b + bb);
			}
			else {
				r = 0.0;
			}
			return s;
		}
		else {
			volatile double s = a - b;
			if (std::isfinite(s)) {
				volatile double bb = s - a;
				r = (a - (s - bb)) - (b + bb);
			}
			else {
				r = 0.0;
			}
			return s;
		}
	}

	// ThreeSum enumerations

	/// <summary>
	/// three_sum computes the relationship x + y + z = r0 + r1 + r2
	/// </summary>
	/// <param name="x">input, yields output r0 (==sum)</param>
	/// <param name="y">input, yields output r1</param>
	/// <param name="z">input, yields output r2</param>
	constexpr inline void three_sum(double& x, double& y, double& z) {
		double u, v, w;
		u = two_sum(x, y, v);
		x = two_sum(z, u, w); // x = r0 (==sum)
		y = two_sum(v, w, z); // y = r1, and z = r2
	}

	/// <summary>
	/// three_sum2 computes the relationship x + y + z = r0 + r1
	/// </summary>
	/// <param name="x">input, yields output r0 (==sum)</param>
	/// <param name="y">input, yields output r1</param>
	/// <param name="z">input</param>
	constexpr inline void three_sum2(double& x, double& y, double z) {
		double u, v, w;
		u = two_sum(x, y, v);
		x = two_sum(z, u, w);  // x = r0 (==sum)
		y = v + w;             // y = r1
	}

	/// <summary>
	/// three_sum3 computes the relationship x + y + z = r0
	/// just the sum of (x, y, z) without any residuals
	/// </summary>
	/// <param name="x">input</param>
	/// <param name="y">input</param>
	/// <param name="z">input</param>
	/// <returns>the (rounded) sum of (x + y + z)</returns>
	constexpr inline double three_sum3(double x, double y, double z) {
		double u = x + y;
		return u + z;  // traditional information loss if z << (x + y) and/or y << x
	}

	/*  */

	/// <summary>
	/// quick_three_accumulate calculates the relationship a + b + c = s + r
	/// s = quick_three_accum(a, b, c) adds c to the dd-pair (a, b).
	/// If the result does not fit in two doubles, then the sum is
	/// output into s and (a, b) contains the remainder.Otherwise
	/// s is zero and (a, b) contains the sum.
	/// </summary>
	/// <param name="a"></param>
	/// <param name="b"></param>
	/// <param name="c"></param>
	/// <returns></returns>
	constexpr inline double quick_three_accumulation(double& a, double& b, double c) {
		double s;
		bool za, zb;

		s = two_sum(b, c, b);
		s = two_sum(a, s, a);

		za = (a != 0.0);
		zb = (b != 0.0);

		if (za && zb)
			return s;

		if (!zb) {
			b = a;
			a = s;
		}
		else {
			a = s;
		}

		return 0.0;
	}

	// Split

#if !defined( RELIABLE_FUSED_MULTIPLY_SUBTRACT_OPERATOR )
	// Computes high word and low word of a double
	//
	// Above SPLIT_THRESHOLD the value is split in a domain scaled down by 2^28 and the halves
	// scaled back up. When the top 27 bits of such a value round up (DBL_MAX does), the high
	// half is 2^996 and comes back as 2^1024 = inf: no 26-bit high half of it is finite.
	// two_prod and two_sqr therefore never split a factor that large; see dekker_residual.
	constexpr inline void split(double a, double& hi, double& lo) {
		constexpr int BITS = 27;  // ( std::numeric_limits< double >::digits + 1 ) / 2;
		constexpr double SPLITTER = 134217729.0;  // std::ldexp(1.0, BITS) + 1.0;
		// SPLIT_THRESHOLD : 0b0.111'1110'0010.1111'1111'1111'1111'1111'1111'1111'1111'1111'1111'1111'1111'1111 : 6.69692879491417e+299
		constexpr double SPLIT_THRESHOLD = 6.6969287949141700e+299; // std::ldexp((std::numeric_limits< double >::max)(), -BITS - 1);
		// 2^(BITS+1) = 2^28 = 268435456.0
		constexpr double POW_2_BITS_PLUS_1 = 268435456.0;
		// 2^-(BITS+1) = 2^-28
		constexpr double POW_2_NEG_BITS_PLUS_1 = 1.0 / POW_2_BITS_PLUS_1;

		if (std::is_constant_evaluated()) {
			double abs_a = (a < 0.0) ? -a : a;
			if (abs_a > SPLIT_THRESHOLD) {
				a = a * POW_2_NEG_BITS_PLUS_1;  // ldexp(a, -BITS - 1)
				double temp = SPLITTER * a;
				hi = temp - (temp - a);
				lo = a - hi;
				hi = hi * POW_2_BITS_PLUS_1;    // ldexp(hi, BITS + 1)
				lo = lo * POW_2_BITS_PLUS_1;    // ldexp(lo, BITS + 1)
			}
			else {
				double temp = SPLITTER * a;
				hi = temp - (temp - a);
				lo = a - hi;
			}
		}
		else {
			volatile double temp;
			if (std::abs(a) > SPLIT_THRESHOLD) {
				a = std::ldexp(a, -BITS - 1);
				temp = SPLITTER * a;
				hi = temp - (temp - a);
				lo = a - hi;
				hi = std::ldexp(hi, BITS + 1);
				lo = std::ldexp(lo, BITS + 1);
			}
			else {
				temp = SPLITTER * a;
				hi = temp - (temp - a);
				lo = a - hi;
			}
		}
	}

	// Dekker's residual r = a * b - p of a finite product p = fl(a * b), exact over the whole
	// finite range (#1881).
	//
	// The split and the partial products must stay finite. Two cases where they would not:
	//  - a factor above the split threshold, whose high half can overflow (see split). The
	//    product is then at most 2^1000, so the other factor is below 2^5: move the large
	//    factor down by 2^28 and the small one up by 2^28. The product, p and r are unchanged.
	//  - a product above 2^1000, where a_hi * b_hi can exceed |a * b| by a relative 2^-25 and
	//    pass DBL_MAX. Move the larger factor (at least 2^500) and p down by 2^28, form the
	//    residual of the scaled product, and scale it back up.
	// Each scaling is by a power of two of a value far above the subnormal range, so is exact.
	// Both cases are rare, so the common path is plain Dekker.
	namespace eft_detail {
		constexpr double SPLIT_THRESHOLD   = 6.6969287949141700e+299;  // DBL_MAX * 2^-28, as in split
		constexpr double PRODUCT_THRESHOLD = 1.0715086071862673e+301;  // 2^1000
		constexpr double UP   = 268435456.0;  // 2^28
		constexpr double DOWN = 1.0 / UP;     // 2^-28

		// Veltkamp's split without split()'s range test. Above SPLIT_THRESHOLD the high half can
		// overflow to a non-finite value, which propagates into the residual: the constant-evaluated
		// path never passes such a value, and the run-time path detects the non-finite residual.
		constexpr inline void split_in_range(double a, double& hi, double& lo) {
			constexpr double SPLITTER = 134217729.0;  // 2^27 + 1
			if (std::is_constant_evaluated()) {
				double temp = SPLITTER * a;
				hi = temp - (temp - a);
			}
			else {
				volatile double temp = SPLITTER * a;
				hi = temp - (temp - a);
			}
			lo = a - hi;
		}

		constexpr inline double dekker_partials(double a, double b, double p) {
			double a_hi = 0.0, a_lo = 0.0, b_hi = 0.0, b_lo = 0.0;
			split_in_range(a, a_hi, a_lo);
			split_in_range(b, b_hi, b_lo);
			return ((a_hi * b_hi - p) + a_hi * b_lo + a_lo * b_hi) + a_lo * b_lo;
		}

		constexpr inline double dekker_residual_scaled(double a, double b, double p, double abs_a, double abs_b, double abs_p) {
			if (abs_p > PRODUCT_THRESHOLD) {
				if (abs_a >= abs_b) a *= DOWN; else b *= DOWN;
				return dekker_partials(a, b, p * DOWN) * UP;
			}
			if (abs_a > SPLIT_THRESHOLD) return dekker_partials(a * DOWN, b * UP, p);
			return dekker_partials(a * UP, b * DOWN, p);
		}
	}

	constexpr inline double dekker_residual(double a, double b, double p) {
		using namespace eft_detail;
		if (std::is_constant_evaluated()) {
			// an overflow is not a constant expression, so test the operands before splitting
			const double abs_a = (a < 0.0) ? -a : a;
			const double abs_b = (b < 0.0) ? -b : b;
			const double abs_p = (p < 0.0) ? -p : p;
			if (abs_p > PRODUCT_THRESHOLD || abs_a > SPLIT_THRESHOLD || abs_b > SPLIT_THRESHOLD) {
				return dekker_residual_scaled(a, b, p, abs_a, abs_b, abs_p);
			}
			return dekker_partials(a, b, p);
		}
		else {
			// Any overflow in the split or the partial products leaves the residual inf or NaN,
			// and without one it is exact; so form it first and redo it scaled only when it is
			// not finite. One test on the result is cheaper than three on the operands.
			const double r = dekker_partials(a, b, p);
			if (r - r == 0.0) [[likely]] return r;
			return dekker_residual_scaled(a, b, p, std::abs(a), std::abs(b), std::abs(p));
		}
	}

	// The square's residual r = a * a - p, as dekker_residual but with one split. A finite
	// square has |a| < 2^512, so only the product case applies: a moves down by 2^14.
	constexpr inline double dekker_square_residual(double a, double p) {
		using namespace eft_detail;
		auto partials = [](double x, double px) {
			double hi = 0.0, lo = 0.0;
			split_in_range(x, hi, lo);
			return ((hi * hi - px) + 2.0 * hi * lo) + lo * lo;
		};
		if (std::is_constant_evaluated()) {
			const double abs_p = (p < 0.0) ? -p : p;
			if (abs_p > PRODUCT_THRESHOLD) return partials(a * (1.0 / 16384.0), p * DOWN) * UP;  // a * 2^-14
			return partials(a, p);
		}
		else {
			const double r = partials(a, p);
			if (r - r == 0.0) [[likely]] return r;
			return partials(a * (1.0 / 16384.0), p * DOWN) * UP;
		}
	}
#endif

	// TwoProd

	/// <summary>
	/// two_prod computes the relationship a * b = p + r
	/// </summary>
	/// <param name="a">input</param>
	/// <param name="b">input</param>
	/// <param name="r">reference to the residual</param>
	/// <returns>the product of a * b</returns>
	constexpr inline double two_prod(double a, double b, double& r) {
		if (std::is_constant_evaluated()) {
			double p = a * b;
			if (is_finite_cx(p)) {
				r = dekker_residual(a, b, p);
			}
			else {
				r = 0.0;
			}
			return p;
		}
		else {
			volatile double p = a * b;
			if (std::isfinite(p)) {
#if defined( RELIABLE_FUSED_MULTIPLY_SUBTRACT_OPERATOR )
				r = UNIVERSAL_FMS(a, b, p);
#else
				r = dekker_residual(a, b, p);
#endif
			}
			else
				r = 0.0;
			return p;
		}
	}

	/// <summary>
	/// two_sqr computes the relationship a * a = square + r
	/// two_sqr is faster than two_prod when calculating the square product
	/// </summary>
	/// <param name="a">input</param>
	/// <param name="r">reference to the residual</param>
	/// <returns>the square product of a</returns>
	constexpr inline double two_sqr(double a, double& r) {
		if (std::is_constant_evaluated()) {
			double p = a * a;
			if (is_finite_cx(p)) {
				r = dekker_square_residual(a, p);
			}
			else {
				r = 0.0;
			}
			return p;
		}
		else {
			volatile double p = a * a;
			if (std::isfinite(p)) {
#if defined( RELIABLE_FUSED_MULTIPLY_SUBTRACT_OPERATOR )
				r = UNIVERSAL_FMS(a, a, p);
#else
				r = dekker_square_residual(a, p);
#endif
			}
			else
				r = 0.0;
			return p;
		}
	}


	// Computes the nearest integer to d
	inline double nint(double d) {
		if (d == std::floor(d)) return d;
		return std::floor(d + 0.5);
	}

	// Computes the truncated integer
	inline double aint(double d) {
		return (d >= 0.0) ? std::floor(d) : std::ceil(d);
	}

	/* These are provided to give consistent
	   interface for double with double-double and quad-double. */
	inline void sincosh(double t, double& sinh_t, double& cosh_t) {
		sinh_t = std::sinh(t);
		cosh_t = std::cosh(t);
	}

	// square of argument t
	constexpr inline double sqr(double t) {
		return t * t;
	}


	/// <summary>
	/// renorm adjusts the quad-double to a canonical form
	/// A quad-double number is an unevaluated sum of four IEEE double numbers.
	/// The quad-double (a0 a1 a2 a3) represents the exact sum a = a0 + a1 + a2 + a3.
	/// Note that for any given representable number x, there can be many representations
	/// as an unevaluated sum of four doubles.
	/// Hence we require that the quadruple(a0 a1 a2 a3) to satisfy
	///  | a_(i+1) | leq ulp(a_i) / 2
	/// for i =0, 1, 2, with equality only occuring when ai = 0, or the last bit of ai is 0
	/// Note that the first a0 is the double precision approximation of the quad-double number,
	/// accurate to almost half an ulp.
	/// </summary>
	/// <param name="a0">reference to higest limb of the quad-double</param>
	/// <param name="a1">reference to second highest limb</param>
	/// <param name="a2">reference to third highest limb</param>
	/// <param name="a3">reference to fourth and lowest limb of the quad-double</param>
	constexpr inline void renorm(double& a0, double& a1, double& a2, double& a3) {
		if (std::is_constant_evaluated()) {
			if (is_inf_cx(a0)) return;
		}
		else {
			if (std::isinf(a0)) return;
		}

		double s0, s1, s2{ 0.0 }, s3{ 0.0 };

		s0 = quick_two_sum(a2, a3, a3);
		s0 = quick_two_sum(a1, s0, a2);
		a0 = quick_two_sum(a0, s0, a1);

		s0 = a0;
		s1 = a1;
		if (s1 != 0.0) {
			s1 = quick_two_sum(s1, a2, s2);
			if (s2 != 0.0)
				s2 = quick_two_sum(s2, a3, s3);
			else
				s1 = quick_two_sum(s1, a3, s2);
		}
		else {
			s0 = quick_two_sum(s0, a2, s1);
			if (s1 != 0.0)
				s1 = quick_two_sum(s1, a3, s2);
			else
				s0 = quick_two_sum(s0, a3, s1);
		}

		a0 = s0;
		a1 = s1;
		a2 = s2;
		a3 = s3;
	}

	/// <summary>
	/// renorm adjusts an intermediate five-element double to a quad-double in canonical form
	/// A quad-double number is an unevaluated sum of four IEEE double numbers.
	/// The quad-double (a0 a1 a2 a3) represents the exact sum a = a0 + a1 + a2 + a3.
	/// Note that for any given representable number x, there can be many representations
	/// as an unevaluated sum of four doubles.
	/// Hence we require that the quadruple(a0 a1 a2 a3) to satisfy
	///  | a_(i+1) | leq ulp(a_i) / 2
	/// for i =0, 1, 2, with equality only occuring when ai = 0, or the last bit of ai is 0
	/// Note that the first a0 is the double precision approximation of the quad-double number,
	/// accurate to almost half an ulp.
	/// </summary>
	/// <param name="a0">reference to higest limb of the quad-double</param>
	/// <param name="a1">reference to second highest limb</param>
	/// <param name="a2">reference to third highest limb</param>
	/// <param name="a3">reference to fourth and lowest limb of the quad-double</param>
	/// <param name="a4">reference to residual value that might influence the lowest bits if higher limbs exhibit overlap</param>
	constexpr inline void renorm(double& a0, double& a1, double& a2, double& a3, double& a4) {
		if (std::is_constant_evaluated()) {
			if (is_inf_cx(a0)) return;
		}
		else {
			if (std::isinf(a0)) return;
		}

		double s0, s1, s2{ 0.0 }, s3{ 0.0 };

		s0 = quick_two_sum(a3, a4, a4);
		s0 = quick_two_sum(a2, s0, a3);
		s0 = quick_two_sum(a1, s0, a2);
		a0 = quick_two_sum(a0, s0, a1);

		s0 = a0;
		s1 = a1;

		s0 = quick_two_sum(a0, a1, s1);
		if (s1 != 0.0) {
			s1 = quick_two_sum(s1, a2, s2);
			if (s2 != 0.0) {
				s2 = quick_two_sum(s2, a3, s3);
				if (s3 != 0.0)
					s3 += a4;
				else
					s2 += a4;
			}
			else {
				s1 = quick_two_sum(s1, a3, s2);
				if (s2 != 0.0)
					s2 = quick_two_sum(s2, a4, s3);
				else
					s1 = quick_two_sum(s1, a4, s2);
			}
		}
		else {
			s0 = quick_two_sum(s0, a2, s1);
			if (s1 != 0.0) {
				s1 = quick_two_sum(s1, a3, s2);
				if (s2 != 0.0)
					s2 = quick_two_sum(s2, a4, s3);
				else
					s1 = quick_two_sum(s1, a4, s2);
			}
			else {
				s0 = quick_two_sum(s0, a3, s1);
				if (s1 != 0.0)
					s1 = quick_two_sum(s1, a4, s2);
				else
					s0 = quick_two_sum(s0, a4, s1);
			}
		}

		a0 = s0;
		a1 = s1;
		a2 = s2;
		a3 = s3;
	}


}} // namespace sw::universal
