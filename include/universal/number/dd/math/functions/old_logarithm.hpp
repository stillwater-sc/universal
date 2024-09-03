#pragma once
// logarithm.hpp: logarithm functions for double-double (dd) floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <limits>

namespace sw { namespace universal {


	const dd dd_inv_int[] = {
		dd(std::numeric_limits< dd >::infinity()),			//	1/0
		dd("1.0"),											//	1/1
		dd("0.5"),											//	1/2
		dd("0.3333333333333333333333333333333333333"),		//	1/3
		dd("0.25"),											//	1/4
		dd("0.2"),											//	1/5
		dd("0.1666666666666666666666666666666666667"),		//	1/6
		dd("0.1428571428571428571428571428571428571"),		//	1/7
		dd("0.125"),										//	1/8
		dd("0.1111111111111111111111111111111111111"),		//	1/9
		dd("0.1"),											//	1/10
		dd("0.0909090909090909090909090909090909091"),		//	1/11
		dd("0.0833333333333333333333333333333333333"),		//	1/12
		dd("0.0769230769230769230769230769230769231"),		//	1/13
		dd("0.0714285714285714285714285714285714286"),		//	1/14
		dd("0.0666666666666666666666666666666666667"),		//	1/15
		dd("0.0625"),										//	1/16
		dd("0.0588235294117647058823529411764705882"),		//	1/17
		dd("0.0555555555555555555555555555555555556"),		//	1/18
		dd("0.0526315789473684210526315789473684211"),		//	1/19
		dd("0.05"),											//	1/20
		dd("0.0476190476190476190476190476190476190"),		//	1/21
		dd("0.0454545454545454545454545454545454545"),		//	1/22
		dd("0.0434782608695652173913043478260869565"),		//	1/23
		dd("0.0416666666666666666666666666666666667"),		//	1/24
		dd("0.04"),											//	1/25
		dd("0.0384615384615384615384615384615384615"),		//	1/26
		dd("0.0370370370370370370370370370370370370"),		//	1/27
		dd("0.0357142857142857142857142857142857143"),		//	1/28
		dd("0.0344827586206896551724137931034482759"),		//	1/29
		dd("0.0333333333333333333333333333333333333"),		//	1/30
		dd("0.0322580645161290322580645161290322581"),		//	1/31
		dd("0.03125"),										//	1/32
		dd("0.0303030303030303030303030303030303030"),		//	1/33
		dd("0.0294117647058823529411764705882352941"),		//	1/34
		dd("0.0285714285714285714285714285714285714"),		//	1/35
		dd("0.0277777777777777777777777777777777778"),		//	1/36
		dd("0.0270270270270270270270270270270270270"),		//	1/37
		dd("0.0263157894736842105263157894736842105"),		//	1/38
		dd("0.0256410256410256410256410256410256410"),		//	1/39
		dd("0.025"),										//	1/40
		dd("0.0243902439024390243902439024390243902")		//	1/41
	};

	const dd dd_third("0.333333333333333333333333333");

	dd Fma(const dd& a, const dd& b, const dd& c) {
		double p[4];
		qd_mul(a, b, p);
		qd_add(p, c, p);
		p[0] = two_sum(p[0], p[1] + p[2] + p[3], p[1]);
		return dd(p[0], p[1]);
	}

//	assumes 0.0 < a < inf
dd _log(dd const& a) {
	int k;
	dd fraction = frexp(a, &k);	  // 0.5 <= |fraction| < 1.0
	if (fraction < dd_inv_sqrt2) {
		fraction = ldexp(fraction, 1);
		--k;
	}

	//	sqrt( 0.5 ) <= f < sqrt( 2.0 )
	//	-0.1715... <= s < 0.1715...
	//
	double res[3];
	res[0] = two_sum(fraction.high(), 1.0, res[1]);
	res[1] = two_sum(fraction.high(), res[1], res[2]);
	dd f_plus = res[0] == 0.0 ? dd(res[1], res[2]) : dd(res[0], res[1]);
	res[0] = two_sum(fraction.high(), -1.0, res[1]);
	res[1] = two_sum(fraction.low(), res[1], res[2]);
	dd f_minus = res[0] == 0.0 ? dd(res[1], res[2]) : dd(res[0], res[1]);

	dd s = f_minus / f_plus;

	//	calculate log( f ) = log( 1 + s ) - log( 1 - s )
	//
	//	log( 1+s ) =  s - s^2/2 + s^3/3 - s^4/4 ...
	//	log( 1-s ) = -s + s^2/2 - s^3/3 - s^4/4 ...
	//	log( f ) = 2*s + 2s^3/3 + 2s^5/5 + ...
	//
	dd s2 = s * s;

	//	TODO	- economize the power series using Chebyshev polynomials
	//
	dd x = dd_inv_int[41];
	for (int i = 41; i > 1; i -= 2) {
		x = fma(x, s2, dd_inv_int[i - 2]);
	}
	x *= ldexp(s, 1); // x *= 2*s

	return fma(k, dd_ln2, x);
}

//	assumes -1.0 < a < 2.0
//
dd _log1p(dd const& a) {
	static const dd a_max = dd_sqrt2 - 1.0;
	static const dd a_min = dd_inv_sqrt2 - 1.0;

	int ilog = std::ilogb(a) + 1;		//	0.5 <= frac < 1.0 

	if (ilog < -std::numeric_limits<dd>::digits / 2)		//	|a| <= 2^-54 - error O( 2^-108)
		return a;
	if (ilog < -std::numeric_limits<dd>::digits / 3)		//	|a| <= 2^-36 - error O( 2^-108)
		return a * Fma(a, -0.5, 1.0);
	if (ilog < -std::numeric_limits<dd>::digits / 4)		//	|a| <= 2^-27 - error O( 2^-108)
		return a * Fma(a, -Fma(a, -dd_third, 0.5), 1.0);

	dd f_minus = a;
	int k = 0;

	if ((a > a_max) || (a < a_min))
	{
		double res[3];
		res[0] = two_sum(a.high(), 1.0, res[1]);
		res[1] = two_sum(a.low(), res[1], res[2]);
		dd f_p1 = res[0] == 0.0 ? dd(res[1], res[2]) : dd(res[0], res[1]);

		f_p1 = frexp(f_p1, &k);	//	0.5 <= |f_p1| < 1.0; k <= 2
		if (f_p1 < dd_inv_sqrt2) {
			--k;
			ldexp(f_p1, 1);
		}

		//	at this point, we have 2^k * ( 1.0 + f ) = 1.0 + a
		//							sqrt( 0.5 ) <= 1.0 + f <= sqrt( 2.0 )
		//
		//							f = 2^-k * a - ( 1.0 - 2^-k )
		double df[2];
		df[0] = two_sum(1.0, -std::ldexp(1.0, -k), df[1]);
		f_minus = ldexp(a, -k) - dd(df[0], df[1]);
	}

	dd f_plus = f_minus + 2.0;
	dd s = f_minus / f_plus;

	//	calculate log( f ) = log( 1 + s ) - log( 1 - s )
	//
	//	log( 1+s ) =  s - s^2/2 + s^3/3 - s^4/4 ...
	//	log( 1-s ) = -s + s^2/2 - s^3/3 - s^4/4 ...
	//	log( f ) = 2*s + 2s^3/3 + 2s^5/5 + ...
	//
	dd s2 = s * s;

	//	TODO	- economize the power series using Chebyshev polynomials
	//
	dd x = dd_inv_int[41];
	for (int i = 41; i > 1; i -= 2) {
		x = Fma(x, s2, dd_inv_int[i - 2]);
	}
	x *= ldexp(s, 1);			//	x *= 2*s

	return Fma(k, dd_ln2, x);

}

// Natural logarithm of x
dd log(dd const& a) {
	if (a.isnan()) return a;

	if (a.iszero()) return -std::numeric_limits< dd >::infinity();

	if (a.isone()) return 0.0;

	if (a.sign())	{
		std::cerr << "log: non-positive argument\n";
		errno = EDOM;
		return std::numeric_limits< dd >::quiet_NaN();
	}

	if (a.isinf()) return a;

	return _log(a);
}

// Binary logarithm of x
dd log2(dd const& a)
{
	if (a.isnan()) return a;

	if (a.iszero()) return -std::numeric_limits< dd >::infinity();

	if (a.isone()) return 0.0;

	if (a.sign()) {
		std::cerr << "log2: non-positive argument\n";
		errno = EDOM;
		return std::numeric_limits< dd >::quiet_NaN();
	}

	if (a.isinf()) return a;

	dd _lge{};
	return _lge * _log(a);
}

// Decimal logarithm of x
dd log10(dd const& a) {
	if (a.isnan()) return a;

	if (a.iszero()) return -std::numeric_limits< dd >::infinity();

	if (a.isone()) return 0.0;

	if (a.sign()) {
		std::cerr << "log10: non-positive argument\n";
		errno = EDOM;
		return std::numeric_limits< dd >::quiet_NaN();
	}

	if (a.isinf()) return a;

	dd _loge{};
	return _loge * _log(a);
}
		
// Natural logarithm of 1+x
dd log1p(dd const& a)
{
	if (a.isnan()) return a;

	if (a.iszero()) return 0.0;

	if (a == -1.0) return -std::numeric_limits< dd >::infinity();

	if (a < -1.0) {
		std::cerr << "log1p: non-positive argument\n";
		errno = EDOM;
		return std::numeric_limits< dd >::quiet_NaN();
	}

	if (a.isinf()) return a;


	if ((a >= 2.0) || (a <= -0.5))			//	a >= 2.0 - no loss of significant bits - use log()
		return _log(1.0 + a);

	//	at this point, -1.0 < a < 2.0
	//
	return _log1p(a);
}

}} // namespace sw::universal
