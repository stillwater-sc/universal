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

We have the assertion that a + b = s + r

 */

 /* If fused multiply-add is available, define to correct macro for
	using it.  It is invoked as QD_FMA(a, b, c) to compute fl(a * b + c).
	If correctly rounded multiply-add is not available (or if unsure),
	keep it undefined.*/
#ifndef QD_FMA
	/* #undef QD_FMA */
#endif

/* If fused multiply-subtract is available, define to correct macro for
   using it.  It is invoked as QD_FMS(a, b, c) to compute fl(a * b - c).
   If correctly rounded multiply-add is not available (or if unsure),
   keep it undefined.*/
#ifndef QD_FMS
   /* #undef QD_FMS */
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

	// ThreeSum

	/// <summary>
	/// three_sum computes the relationship a + b + c = s + r
	/// </summary>
	/// <param name="a">input, output sum</param>
	/// <param name="b">input, output residual</param>
	/// <param name="c">input value</param>
	inline void three_sum(volatile double& a, volatile double& b, volatile double& c) {
		volatile double t1, t2, t3;

		t1 = two_sum(a, b, t2);
		a = two_sum(c, t1, t3);
		b = two_sum(t2, t3, c);
	}

	/// <summary>
	/// three_sum2 computes the relationship a + b + c = s + r
	/// </summary>
	/// <param name="a">input, output sum</param>
	/// <param name="b">input, output residual</param>
	/// <param name="c">input</param>
	inline void three_sum2(volatile double& a, volatile double& b, volatile double& c) {
		double t1, t2, t3;
		t1 = two_sum(a, b, t2);
		a = two_sum(c, t1, t3);
		b = t2 + t3;
	}


	// Split

#if !defined( QD_FMS )
	/* Computes high word and lo word of a */
	inline void split(double a, volatile double& hi, volatile double& lo) {
		int const QD_BITS = (std::numeric_limits< double >::digits + 1) / 2;
		static double const QD_SPLITTER = std::ldexp(1.0, QD_BITS) + 1.0;
		static double const QD_SPLIT_THRESHOLD = std::ldexp((std::numeric_limits< double >::max)(), -QD_BITS - 1);

		volatile double temp;

		if (std::abs(a) > QD_SPLIT_THRESHOLD)
		{
			a = std::ldexp(a, -QD_BITS - 1);
			temp = QD_SPLITTER * a;
			hi = temp - (temp - a);
			lo = a - hi;
			hi = std::ldexp(hi, QD_BITS + 1);
			lo = std::ldexp(lo, QD_BITS + 1);
		}
		else
		{
			temp = QD_SPLITTER * a;
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
	inline double two_prod(double a, double b, volatile double& r)
	{
		volatile double p = a * b;
		if (std::isfinite(p)) {
#if defined( QD_FMS )
			r = QD_FMS(a, b, p);
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
		if (std::isfinite(p))
		{
#if defined( QD_FMS )
			err = QD_FMS(a, a, p);
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
	/// <param name="a0"></param>
	/// <param name="a1"></param>
	/// <param name="a2"></param>
	/// <param name="a3"></param>
	inline void renorm(volatile double& a0, volatile double& a1, volatile double& a2, volatile double& a3) {
		volatile double s0, s1, s2 = 0.0, s3 = 0.0;

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
	/// <param name="a0">reference to a0</param>
	/// <param name="a1">reference to a1</param>
	/// <param name="a2">reference to a2</param>
	/// <param name="a3">reference to a3</param>
	/// <param name="a4">reference to a4</param>
	inline void renorm(volatile double& a0, volatile double& a1, volatile double& a2, volatile double& a3, volatile double& a4) {
		volatile double s0, s1, s2 = 0.0, s3 = 0.0;

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
