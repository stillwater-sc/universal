#pragma once
// exponent.hpp: exponent functions for quad-double cascade (qd_cascade) floating-point
//
// algorithms courtesy Scibuilders, Jack Poulson
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// fwd reference
	qd_cascade ldexp(const qd_cascade&, int);

	// Helper function: multiply by a power of 2 (exact operation, no rounding)
	inline qd_cascade mul_pwr2(const qd_cascade& a, double b) {
		return qd_cascade(a[0] * b, a[1] * b, a[2] * b, a[3] * b);
	}

    constexpr int qdc_inverse_factorial_table_size = 15;
    static const qd_cascade qdc_inverse_factorial[qdc_inverse_factorial_table_size] = {
        qd_cascade(1.66666666666666657e-01,  9.25185853854297066e-18,  5.13581318503262866e-34,  2.85094902409834186e-50),  // 1/3!
        qd_cascade(4.16666666666666644e-02,  2.31296463463574266e-18,  1.28395329625815716e-34,  7.12737256024585466e-51),  // 1/4!
        qd_cascade(8.33333333333333322e-03,  1.15648231731787138e-19,  1.60494162032269652e-36,  2.22730392507682967e-53),  // 1/5!
        qd_cascade(1.38888888888888894e-03, -5.30054395437357706e-20, -1.73868675534958776e-36, -1.63335621172300840e-52),  // 1/6!
        qd_cascade(1.98412698412698413e-04,  1.72095582934207053e-22,  1.49269123913941271e-40,  1.29470326746002471e-58),  // 1/7!
        qd_cascade(2.48015873015873016e-05,  2.15119478667758816e-23,  1.86586404892426588e-41,  1.61837908432503088e-59),  // 1/8!
        qd_cascade(2.75573192239858925e-06, -1.85839327404647208e-22,  8.49175460488199287e-39, -5.72661640789429621e-55),  // 1/9!
        qd_cascade(2.75573192239858883e-07,  2.37677146222502973e-23, -3.26318890334088294e-40,  1.61435111860404415e-56),  // 1/10!
        qd_cascade(2.50521083854417202e-08, -1.44881407093591197e-24,  2.04267351467144546e-41, -8.49632672007163175e-58),  // 1/11!
        qd_cascade(2.08767569878681002e-09, -1.20734505911325997e-25,  1.70222792889287100e-42,  1.41609532150396700e-58),  // 1/12!
        qd_cascade(1.60590438368216133e-10,  1.25852945887520981e-26, -5.31334602762985031e-43,  3.54021472597605528e-59),  // 1/13!
        qd_cascade(1.14707455977297245e-11,  2.06555127528307454e-28,  6.88907923246664603e-45,  5.72920002655109095e-61),  // 1/14!
        qd_cascade(7.64716373181981641e-13,  7.03872877733453001e-30, -7.82753927716258345e-48,  1.92138649443790242e-64),  // 1/15!
        qd_cascade(4.77947733238738525e-14,  4.39920548583408126e-31, -4.89221204822661465e-49,  1.20086655902368901e-65),  // 1/16!
        qd_cascade(2.81145725434552060e-15,  1.65088427308614326e-31, -2.87777179307447918e-50,  4.27110689256293549e-67)   // 1/17!
    };

    inline qd_cascade exp(const qd_cascade& x) {
        /* Strategy:  We first reduce the size of x by noting that

                exp(kr + m * ln(2)) = 2^m * exp(r)^k

           where m and k are integers.  By choosing m appropriately
           we can make |kr| <= ln(2) / 2 = 0.347.  Then exp(r) is
           evaluated using the familiar Taylor series.  Reducing the
           argument substantially speeds up the convergence.

           PRECISION NOTE (2026-08-15, supersedes the 2026-03-16 note):
           The ~10 digit loss this note used to describe was real, but it
           was NOT caused by multiply_cascades(). It came from addition:
           expansion_ops::add_cascades() returned a sum whose value was
           exact but whose components overlapped, and compress_8to4()
           dropped the fourth component on roughly one addition in eight.
           The 16 squarings amplified it, which is why exp() was where it
           showed. Compressing the expansion (universal#1317) brought exp()
           and log() to within a rounding of qd's, measured against an
           exact decimal oracle:

                        before      after       qd
             exp      4.8e-51    4.0e-65   4.0e-65   worst relative error
             log      1.3e-50    2.3e-63   2.3e-63

           Multiplication is a separate, much smaller matter: with full
           4-component operands multiply_cascades() is ~2 decimal digits
           behind qd's hand-tuned qd_mul (1.7e-63 vs 1.6e-65 worst case).
           That is inside the format's 63.6 digits, so it is a quality gap
           rather than a defect; tracked separately.
         */

        constexpr double k = double(1ull << 16);
        constexpr double inv_k = 1.0 / k;

        if (x[0] <= -709.0) return qd_cascade(0.0);

        if (x[0] >= 709.0) return qd_cascade(SpecificValue::infpos);

        if (x.iszero()) return qd_cascade(1.0);

        if (x.isone()) return qdc_e;

        double m = std::floor(x[0] / qdc_ln2[0] + 0.5);
        qd_cascade r = mul_pwr2(x - qdc_ln2 * m, inv_k);
        qd_cascade s, p, t;
        double thresh = inv_k * qdc_eps;

        p = sqr(r);
        s = r + mul_pwr2(p, 0.5);
        int i = 0;
        do {
            p *= r;
            t = p * qdc_inverse_factorial[i++];
            s += t;
        } while (std::abs(double(t)) > thresh && i < 14);

        s = mul_pwr2(s, 2.0) + sqr(s);
        s = mul_pwr2(s, 2.0) + sqr(s);
        s = mul_pwr2(s, 2.0) + sqr(s);
        s = mul_pwr2(s, 2.0) + sqr(s);
        s = mul_pwr2(s, 2.0) + sqr(s);
        s = mul_pwr2(s, 2.0) + sqr(s);
        s = mul_pwr2(s, 2.0) + sqr(s);
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
    inline qd_cascade exp2(const qd_cascade& x) {
	    return exp(x * qdc_ln2);
    }

    // Base-10 exponential function
    inline qd_cascade exp10(const qd_cascade& x) {
	    return exp(x * qdc_ln10);
    }

    // Base-e exponential function exp(x)-1
    // For small |x|, use Taylor series to avoid catastrophic cancellation
    inline qd_cascade expm1(const qd_cascade& x) {
        if (x.iszero()) return qd_cascade(0.0);
        if (std::abs(x[0]) < 0.5) {
            qd_cascade s = x;
            qd_cascade term = x;
            for (int i = 2; i < 50; ++i) {
                term *= x / qd_cascade(i);
                s += term;
                if (std::abs(double(term)) < qdc_eps * std::abs(double(s))) break;
            }
            return s;
        }
        return exp(x) - 1.0;
    }


}} // namespace sw::universal
