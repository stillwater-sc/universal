#pragma once
// logarithm.hpp: logarithm functions for quad-double (qd) floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <limits>

namespace sw { namespace universal {



//	assumes 0.0 < a < inf
qd _log(qd const& a) {
#ifdef LATER
	int k;
	qd f = std::frexp(a, &k);	//	0.5 <= |f| < 1.0
	if (f < _inv_sqrt2) {
		f = std::ldexp(f, 1);
		--k;
	}

	//	sqrt( 0.5 ) <= f < sqrt( 2.0 )
	//	-0.1715... <= s < 0.1715...
	//
	double res[3];
	res[0] = two_sum(f.high(), 1.0, res[1]);
	res[1] = two_sum(f.high(), res[1], res[2]);
	qd f_plus = res[0] == 0.0 ? qd(res[1], res[2]) : qd(res[0], res[1]);
	res[0] = two_sum(f.high(), -1.0, res[1]);
	res[1] = two_sum(f.low(), res[1], res[2]);
	qd f_minus = res[0] == 0.0 ? qd_real(res[1], res[2]) : qd(res[0], res[1]);

	qd s = f_minus / f_plus;

	//	calculate log( f ) = log( 1 + s ) - log( 1 - s )
	//
	//	log( 1+s ) =  s - s^2/2 + s^3/3 - s^4/4 ...
	//	log( 1-s ) = -s + s^2/2 - s^3/3 - s^4/4 ...
	//	log( f ) = 2*s + 2s^3/3 + 2s^5/5 + ...
	//
	qd s2 = s * s;

	//	TODO	- economize the power series using Chebyshev polynomials
	//
	qd x = inv_int[41];
	for (int i = 41; i > 1; i -= 2) {
		x = std::Fma(x, s2, inv_int[i - 2]);
	}
	x *= std::ldexp(s, 1); // x *= 2*s

	return std::Fma(k, _ln2, x);
#endif
	return qd(std::log(a.high()), 0.0);
}

//	assumes -1.0 < a < 2.0
//
qd _log1p(qd const& a) {
#ifdef LATER
	static const qd a_max = _sqrt2 - 1.0;
	static const qd a_min = _inv_sqrt2 - 1.0;

	int ilog = std::ilogb(a) + 1;		//	0.5 <= frac < 1.0 

	if (ilog < -std::numeric_limits<qd>::digits / 2)		//	|a| <= 2^-54 - error O( 2^-108)
		return a;
	if (ilog < -std::numeric_limits<qd>::digits / 3)		//	|a| <= 2^-36 - error O( 2^-108)
		return a * std::Fma(a, -0.5, 1.0);
	if (ilog < -std::numeric_limits<qd>::digits / 4)		//	|a| <= 2^-27 - error O( 2^-108)
		return a * std::Fma(a, -std::Fma(a, -_third, 0.5), 1.0);

	qd f_minus = a;
	int k = 0;

	if ((a > a_max) || (a < a_min))
	{
		double res[3];
		res[0] = two_sum(a.high(), 1.0, res[1]);
		res[1] = two_sum(a.low(), res[1], res[2]);
		qd f_p1 = res[0] == 0.0 ? qd(res[1], res[2]) : qd_real(res[0], res[1]);

		f_p1 = std::frexp(f_p1, &k);	//	0.5 <= |f_p1| < 1.0; k <= 2
		if (f_p1 < _inv_sqrt2) {
			--k;
			std::ldexp(f_p1, 1);
		}

		//	at this point, we have 2^k * ( 1.0 + f ) = 1.0 + a
		//							sqrt( 0.5 ) <= 1.0 + f <= sqrt( 2.0 )
		//
		//							f = 2^-k * a - ( 1.0 - 2^-k )
		double df[2];
		df[0] = two_sum(1.0, -std::ldexp(1.0, -k), df[1]);
		f_minus = std::ldexp(a, -k) - qd_real(df[0], df[1]);
	}

	qd f_plus = f_minus + 2.0;
	qd s = f_minus / f_plus;

	//	calculate log( f ) = log( 1 + s ) - log( 1 - s )
	//
	//	log( 1+s ) =  s - s^2/2 + s^3/3 - s^4/4 ...
	//	log( 1-s ) = -s + s^2/2 - s^3/3 - s^4/4 ...
	//	log( f ) = 2*s + 2s^3/3 + 2s^5/5 + ...
	//
	qd s2 = s * s;

	//	TODO	- economize the power series using Chebyshev polynomials
	//
	qd x = inv_int[41];
	for (int i = 41; i > 1; i -= 2) {
		x = std::Fma(x, s2, inv_int[i - 2]);
	}
	x *= std::ldexp(s, 1);			//	x *= 2*s

	return std::Fma(k, _ln2, x);
#endif
	return qd(std::log1p(a.high()), 0.0);
}

// Natural logarithm of x
qd log(qd const& a) {
	if (a.isnan()) return a;

	if (a.iszero()) return -std::numeric_limits< qd >::infinity();

	if (a.isone()) return 0.0;

	if (a.sign())	{
		std::cerr << "log: non-positive argument";
		errno = EDOM;
		return std::numeric_limits< qd >::quiet_NaN();
	}

	if (a.isinf()) return a;

	return _log(a);
}

// Binary logarithm of x
qd log2(qd const& a)
{
	if (a.isnan()) return a;

	if (a.iszero()) return -std::numeric_limits< qd >::infinity();

	if (a.isone()) return 0.0;

	if (a.sign()) {
		std::cerr << "log2: non-positive argument";
		errno = EDOM;
		return std::numeric_limits< qd >::quiet_NaN();
	}

	if (a.isinf()) return a;

	qd _lge{};
	return _lge * _log(a);
}

// Decimal logarithm of x
qd log10(qd const& a) {
	if (a.isnan()) return a;

	if (a.iszero()) return -std::numeric_limits< qd >::infinity();

	if (a.isone()) return 0.0;

	if (a.sign()) {
		std::cerr << "log10: non-positive argument";
		errno = EDOM;
		return std::numeric_limits< qd >::quiet_NaN();
	}

	if (a.isinf()) return a;

	qd _loge{};
	return _loge * _log(a);
}
		
// Natural logarithm of 1+x
qd log1p(qd const& a)
{
	if (a.isnan()) return a;

	if (a.iszero()) return 0.0;

	if (a == -1.0) return -std::numeric_limits< qd >::infinity();

	if (a < -1.0) {
		std::cerr << "log1p: non-positive argument";
		errno = EDOM;
		return std::numeric_limits< qd >::quiet_NaN();
	}

	if (a.isinf()) return a;


	if ((a >= 2.0) || (a <= -0.5))			//	a >= 2.0 - no loss of significant bits - use log()
		return _log(1.0 + a);

	//	at this point, -1.0 < a < 2.0
	//
	return _log1p(a);
}

}} // namespace sw::universal
