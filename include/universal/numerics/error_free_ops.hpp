#pragma once
// error_free_ops.hpp: definition of error free arithmetic functions for native floating-point types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>

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

	// TwoSums

	/// <summary>
	/// quick_two_sum computes the relationship a + b = s + r
	/// requires its arguments to be |a| >= |b|
	/// </summary>
	/// <param name="a">input</param>
	/// <param name="b">input</param>
	/// <param name="r">reference to the residual</param>
	/// <returns>the sum s</returns>
	inline double quick_two_sum(double a, double b, volatile double& r) {
		volatile double s = a + b;
		r = (std::isfinite(s) ? b - (s - a) : 0.0);
		return s;
	}

	/// <summary>
	/// two_sum computes the relationship a + b = s + r
	/// </summary>
	/// <param name="a">input</param>
	/// <param name="b">input</param>
	/// <param name="r">reference to the residual</param>
	/// <returns>the sum s</returns>
	inline double two_sum(double a, double b, volatile double& r) {
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
	inline double quick_two_diff(double a, double b, volatile double& r) {
		volatile double s = a - b;
		r = (std::isfinite(s) ? (a - s) - b : 0.0);
		return s;
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
	inline double two_diff(double a, double b, volatile double& r) {
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

	// ThreeSum enumerations

	/// <summary>
	/// three_sum computes the relationship x + y + z = r0 + r1 + r2
	/// </summary>
	/// <param name="x">input, yields output r0 (==sum)</param>
	/// <param name="y">input, yields output r1</param>
	/// <param name="z">input, yields output r2</param>
	inline void three_sum(volatile double& x, volatile double& y, volatile double& z) {
		volatile double u, v, w;

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
	inline void three_sum2(volatile double& x, volatile double& y, double z) {
		volatile double u, v, w;

		u = two_sum(x, y, v);
		x = two_sum(z, u, w);  // x = r0 (==sum)
		y = v + w;                       // y = r1
	}

	/// <summary>
	/// three_sum3 computes the relationship x + y + z = r0
	/// just the sum of (x, y, z) without any residuals
	/// </summary>
	/// <param name="x">input</param>
	/// <param name="y">input</param>
	/// <param name="z">input</param>
	/// <returns>the (rounded) sum of (x + y + z)</returns>
	inline double three_sum3(double x, double y, double z) {
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
	inline double quick_three_accumulation(volatile double& a, volatile double& b, double c) {
		volatile double s;
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
	inline void split(double a, volatile double& hi, volatile double& lo) {
		constexpr int BITS = 27;  // ( std::numeric_limits< double >::digits + 1 ) / 2;
		constexpr double SPLITTER = 134217729.0;  // std::ldexp(1.0, BITS) + 1.0;
		// SPLIT_THRESHOLD : 0b0.111'1110'0010.1111'1111'1111'1111'1111'1111'1111'1111'1111'1111'1111'1111'1111 : 6.69692879491417e+299
		constexpr double SPLIT_THRESHOLD = 6.6969287949141700e+299; // std::ldexp((std::numeric_limits< double >::max)(), -BITS - 1);

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
#endif

	// TwoProd

	/// <summary>
	/// two_prod computes the relationship a * b = p + r
	/// </summary>
	/// <param name="a">input</param>
	/// <param name="b">input</param>
	/// <param name="r">reference to the residual</param>
	/// <returns>the product of a * b</returns>
	inline double two_prod(double a, double b, volatile double& r) {
		volatile double p = a * b;
		if (std::isfinite(p)) {
#if defined( RELIABLE_FUSED_MULTIPLY_SUBTRACT_OPERATOR )
			r = UNIVERSAL_FMS(a, b, p);
#else
			double a_hi, a_lo, b_hi, b_lo;
			split(a, a_hi, a_lo);
			split(b, b_hi, b_lo);
			r = ((a_hi * b_hi - p) + a_hi * b_lo + a_lo * b_hi) + a_lo * b_lo;
#endif
		}
		else
			r = 0.0;
		return p;
	}

	/// <summary>
	/// two_sqr computes the relationship a * a = square + r
	/// two_sqr is faster than two_prod when calculating the square product
	/// </summary>
	/// <param name="a">input</param>
	/// <param name="r">reference to the residual</param>
	/// <returns>the square product of a</returns>
	inline double two_sqr(double a, volatile double& r) {
		volatile double p = a * a;
		if (std::isfinite(p)) {
#if defined( RELIABLE_FUSED_MULTIPLY_SUBTRACT_OPERATOR )
			err = UNIVERSAL_FMS(a, a, p);
#else
			volatile double hi, lo;
			split(a, hi, lo);
			r = ((hi * hi - p) + 2.0 * hi * lo) + lo * lo;
#endif
		}
		else
			r = 0.0;
		return p;
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
	inline double sqr(double t) {
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
	inline void renorm(volatile double& a0, volatile double& a1, volatile double& a2, volatile double& a3) {
		volatile double s0, s1, s2{ 0.0 }, s3{ 0.0 };

		if (std::isinf(a0)) return;

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
	inline void renorm(volatile double& a0, volatile double& a1, volatile double& a2, volatile double& a3, volatile double& a4) {
		volatile double s0, s1, s2{ 0.0 }, s3{ 0.0 };

		if (std::isinf(a0)) return;

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
