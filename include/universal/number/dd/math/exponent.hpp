#pragma once
// exponent.hpp: exponent functions for double-double floating-point
//
// algorithms courtesy Scibuilders, Jack Poulson
// 
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// fwd reference
	dd ldexp(dd const&, int);

static const int n_inv_fact = 15;
static const double inv_fact[n_inv_fact][2] = {
  { 1.66666666666666657e-01,  9.25185853854297066e-18},
  { 4.16666666666666644e-02,  2.31296463463574266e-18},
  { 8.33333333333333322e-03,  1.15648231731787138e-19},
  { 1.38888888888888894e-03, -5.30054395437357706e-20},
  { 1.98412698412698413e-04,  1.72095582934207053e-22},
  { 2.48015873015873016e-05,  2.15119478667758816e-23},
  { 2.75573192239858925e-06, -1.85839327404647208e-22},
  { 2.75573192239858883e-07,  2.37677146222502973e-23},
  { 2.50521083854417202e-08, -1.44881407093591197e-24},
  { 2.08767569878681002e-09, -1.20734505911325997e-25},
  { 1.60590438368216133e-10,  1.25852945887520981e-26},
  { 1.14707455977297245e-11,  2.06555127528307454e-28},
  { 7.64716373181981641e-13,  7.03872877733453001e-30},
  { 4.77947733238738525e-14,  4.39920548583408126e-31},
  { 2.81145725434552060e-15,  1.65088427308614326e-31}
};

// Base-e exponential function
dd exp(const dd& a) {
	/* Strategy:  We first reduce the size of x by noting that
     
		exp(kr + m * log(2)) = 2^m * exp(r)^k

	where m and k are integers.  By choosing m appropriately
	we can make |kr| <= log(2) / 2 = 0.347.  Then exp(r) is 
	evaluated using the familiar Taylor series.  Reducing the 
	argument substantially speeds up the convergence.       */  

	const double k = 512.0;
	const double inv_k = 1.0 / k;

	if (a.high() <= -709.0) return dd(0.0);

	if (a.high() >=  709.0) return dd(SpecificValue::infpos);

	if (a.iszero()) return dd(1.0);

	if (a.isone()) return dd_e;

	double m = std::floor(a.high() / dd_log2.high() + 0.5);
	dd r = mul_pwr2(a - dd_log2 * m, inv_k);
	dd s, t, p;

	p = sqr(r);
	s = r + mul_pwr2(p, 0.5);
	p *= r;
	t = p * dd(inv_fact[0][0], inv_fact[0][1]);
	int i = 0;
	do {
		s += t;
		p *= r;
		++i;
		t = p * dd(inv_fact[i][0], inv_fact[i][1]);
	} while (std::abs(double(t)) > inv_k * dd_eps && i < 5);

	s += t;

	s = mul_pwr2(s, 2.0) + sqr(s);
	s = mul_pwr2(s, 2.0) + sqr(s);
	s = mul_pwr2(s, 2.0) + sqr(s);
	s = mul_pwr2(s, 2.0) + sqr(s);
	s = mul_pwr2(s, 2.0) + sqr(s);
	s = mul_pwr2(s, 2.0) + sqr(s);
	s = mul_pwr2(s, 2.0) + sqr(s);
	s = mul_pwr2(s, 2.0) + sqr(s);
	s = mul_pwr2(s, 2.0) + sqr(s);
	s += 1.0;

	return ldexp(s, static_cast<int>(m));
}


// Base-2 exponential function

// Base-10 exponential function
		
// Base-e exponential function exp(x)-1
dd expm1(dd x) {
	return dd(std::expm1(double(x)));
}


}} // namespace sw::universal
