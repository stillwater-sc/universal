#pragma once
// exponent.hpp: exponent functions for double-double (dd) floating-point
//
// base algorithm strategy courtesy Scibuilder, Jack Poulson
// 
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// fwd reference
	inline dd_cascade ldexp(const dd_cascade&, int);

	constexpr int ddc_inverse_factorial_table_size = 15;
    constexpr dd_cascade ddc_inverse_factorial[ddc_inverse_factorial_table_size] = {
		dd_cascade(1.66666666666666657e-01,  9.25185853854297066e-18),  // 1/3!
        dd_cascade(4.16666666666666644e-02, 2.31296463463574266e-18),   // 1/4!
        dd_cascade(8.33333333333333322e-03, 1.15648231731787138e-19),   // 1/5!
        dd_cascade(1.38888888888888894e-03, -5.30054395437357706e-20),  // 1/6!
        dd_cascade(1.98412698412698413e-04, 1.72095582934207053e-22),   // 1/7!
        dd_cascade(2.48015873015873016e-05, 2.15119478667758816e-23),   // 1/8!
        dd_cascade(2.75573192239858925e-06, -1.85839327404647208e-22),  // 1/9!
        dd_cascade(2.75573192239858883e-07, 2.37677146222502973e-23),   // 1/10!
        dd_cascade(2.50521083854417202e-08, -1.44881407093591197e-24),  // 1/11!
        dd_cascade(2.08767569878681002e-09, -1.20734505911325997e-25),  // 1/12!
        dd_cascade(1.60590438368216133e-10, 1.25852945887520981e-26),   // 1/13!
        dd_cascade(1.14707455977297245e-11, 2.06555127528307454e-28),   // 1/14!
        dd_cascade(7.64716373181981641e-13, 7.03872877733453001e-30),   // 1/15!
        dd_cascade(4.77947733238738525e-14, 4.39920548583408126e-31),   // 1/16!
        dd_cascade(2.81145725434552060e-15, 1.65088427308614326e-31)    // 1/17!
	};

	// Base-e exponential function: returns the exponential e^x
    inline dd_cascade exp(const dd_cascade& a) {
		/* Strategy:  We first reduce the size of x by noting that
     
			exp(kr + m * log(2)) = 2^m * exp(r)^k

		where m and k are integers.  By choosing m appropriately
		we can make |kr| <= log(2) / 2 = 0.347.  Then exp(r) is 
		evaluated using the familiar Taylor series.  Reducing the 
		argument substantially speeds up the convergence.       */  

		const double k = 512.0;
		const double inv_k = 1.0 / k;

		if (a.high() <= -709.0) return dd_cascade(0.0);

		if (a.high() >=  709.0) return dd_cascade(SpecificValue::infpos);

		if (a.iszero()) return dd_cascade(1.0);

		if (a.isone()) return ddc_e;

		double m = std::floor(a.high() / ddc_log2.high() + 0.5);
	    dd_cascade r = mul_pwr2(a - ddc_log2 * m, inv_k);
	    dd_cascade s, t, p;

		p = sqr(r);
		s = r + mul_pwr2(p, 0.5);
		p *= r;
		t = p * ddc_inverse_factorial[0];
		int i = 0;
		do {
			s += t;
			p *= r;
			++i;
			t = p * ddc_inverse_factorial[i];
		} while (std::abs(double(t)) > inv_k * ddc_eps && i < 5);

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
    inline dd_cascade exp2(const dd_cascade& x) {
	    return dd_cascade(std::exp2(double(x)));
	}

	// Base-10 exponential function
    inline dd_cascade exp10(const dd_cascade& x) {
	    return dd_cascade(std::pow(10.0, double(x)));
	}
		
	// Base-e exponential function exp(x)-1
    inline dd_cascade expm1(const dd_cascade& x) {
	    return dd_cascade(std::expm1(double(x)));
	}


}} // namespace sw::universal
