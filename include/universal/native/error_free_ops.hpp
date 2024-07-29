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
	template<typename NativeFloat>
	inline double quick_two_sum(NativeFloat a, NativeFloat b, NativeFloat& r)
	{
		NativeFloat s = a + b;
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
	template<typename NativeFloat>
	inline double two_sum(NativeFloat a, NativeFloat b, NativeFloat& r)
	{
		double s = a + b;
		if (std::isfinite(s)) {
			double bb = s - a;
			r = (a - (s - bb)) + (b - bb);
			//double sbb = s - bb;
			//double asbb = a - sbb;
			//double bbb = b - bb;
			//r = asbb + bbb;
		}
		else {
			r = 0.0;
		}
		return s;
	}


	// TwoDiff

	/// <summary>
	/// quick_two_diff computes the relationship a - b = s + r
	/// requires its arguments to be |a| >= |b|
	/// </summary>
	/// <param name="a">input</param>
	/// <param name="b">input</param>
	/// <param name="r">reference to the residual</param>
	/// <returns>the sum s</returns>
	template<typename NativeFloat>
	inline double quick_two_diff(NativeFloat a, NativeFloat b, NativeFloat& r)
	{
		double s = a - b;
		r = (std::isfinite(s) ? (a - s) - b : 0.0);
		return s;
	}



	// ThreeSum

	/// <summary>
	/// three_sum calculates the relationship a + b + c = s + r
	/// </summary>
	/// <param name="a">input</param>
	/// <param name="b">input</param>
	/// <param name="c">input value, output residual</param>
	inline void three_sum(double& a, double& b, double& c)
	{
		double t1, t2, t3;

		t1 = two_sum(a, b, t2);
		a = two_sum(c, t1, t3);
		b = two_sum(t2, t3, c);
	}


	// Split

#if !defined( QD_FMS )
	/* Computes high word and lo word of a */
	inline void split(double a, double& hi, double& lo)
	{
		int const QD_BITS = (std::numeric_limits< double >::digits + 1) / 2;
		static double const QD_SPLITTER = std::ldexp(1.0, QD_BITS) + 1.0;
		static double const QD_SPLIT_THRESHOLD = std::ldexp((std::numeric_limits< double >::max)(), -QD_BITS - 1);

		double temp;

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

	/* Computes fl(a*b) and err(a*b). */
	inline double two_prod(double a, double b, double& r)
	{
		double p = a * b;
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

	/* Computes fl(a*a) and err(a*a).  Faster than the above method. */
	inline double two_sqr(double a, double& r)
	{
		double p = a * a;
		if (std::isfinite(p))
		{
#if defined( QD_FMS )
			err = QD_FMS(a, a, p);
#else
			double hi, lo;
			split(a, hi, lo);
			r = ((hi * hi - p) + 2.0 * hi * lo) + lo * lo;
#endif
		}
		else
			r = 0.0;
		return p;
	}



	inline void renorm(double& c0, double& c1, double& c2, double& c3) {
		double s0, s1, s2 = 0.0, s3 = 0.0;

		if (std::isinf(c0)) return;

		s0 = quick_two_sum(c2, c3, c3);
		s0 = quick_two_sum(c1, s0, c2);
		c0 = quick_two_sum(c0, s0, c1);

		s0 = c0;
		s1 = c1;
		if (s1 != 0.0) {
			s1 = quick_two_sum(s1, c2, s2);
			if (s2 != 0.0)
				s2 = quick_two_sum(s2, c3, s3);
			else
				s1 = quick_two_sum(s1, c3, s2);
		}
		else {
			s0 = quick_two_sum(s0, c2, s1);
			if (s1 != 0.0)
				s1 = quick_two_sum(s1, c3, s2);
			else
				s0 = quick_two_sum(s0, c3, s1);
		}

		c0 = s0;
		c1 = s1;
		c2 = s2;
		c3 = s3;
	}

	inline void renorm(double& c0, double& c1, double& c2, double& c3, double& c4) {
		double s0, s1, s2 = 0.0, s3 = 0.0;

		if (std::isinf(c0)) return;

		s0 = quick_two_sum(c3, c4, c4);
		s0 = quick_two_sum(c2, s0, c3);
		s0 = quick_two_sum(c1, s0, c2);
		c0 = quick_two_sum(c0, s0, c1);

		s0 = c0;
		s1 = c1;

		s0 = quick_two_sum(c0, c1, s1);
		if (s1 != 0.0) {
			s1 = quick_two_sum(s1, c2, s2);
			if (s2 != 0.0) {
				s2 = quick_two_sum(s2, c3, s3);
				if (s3 != 0.0)
					s3 += c4;
				else
					s2 += c4;
			}
			else {
				s1 = quick_two_sum(s1, c3, s2);
				if (s2 != 0.0)
					s2 = quick_two_sum(s2, c4, s3);
				else
					s1 = quick_two_sum(s1, c4, s2);
			}
		}
		else
		{
			s0 = quick_two_sum(s0, c2, s1);
			if (s1 != 0.0) {
				s1 = quick_two_sum(s1, c3, s2);
				if (s2 != 0.0)
					s2 = quick_two_sum(s2, c4, s3);
				else
					s1 = quick_two_sum(s1, c4, s2);
			}
			else {
				s0 = quick_two_sum(s0, c3, s1);
				if (s1 != 0.0)
					s1 = quick_two_sum(s1, c4, s2);
				else
					s0 = quick_two_sum(s0, c4, s1);
			}
		}

		c0 = s0;
		c1 = s1;
		c2 = s2;
		c3 = s3;
	}

}} // namespace sw::universal
