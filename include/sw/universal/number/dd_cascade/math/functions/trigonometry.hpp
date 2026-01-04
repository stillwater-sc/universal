#pragma once
// trigonometry.hpp: trigonometry function support for double-double floating-point
// 
// algorithms and constants courtesy of Scibuilders, Jack Poulson
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

    // pi/16
    constexpr dd_cascade ddc_pi16(1.963495408493620697e-01, 7.654042494670957545e-18);

    // Table of sin(k * pi/16)
    constexpr dd_cascade ddc_sin_table[4] = {
        dd_cascade(1.950903220161282758e-01, -7.991079068461731263e-18),
        dd_cascade(3.826834323650897818e-01, -1.005077269646158761e-17),
        dd_cascade(5.555702330196021776e-01,  4.709410940561676821e-17),
        dd_cascade(7.071067811865475727e-01, -4.833646656726456726e-17)
    };

    // Table of cos(k * pi/16)
    constexpr dd_cascade ddc_cos_table[4] = {
        dd_cascade(9.807852804032304306e-01, 1.854693999782500573e-17),
        dd_cascade(9.238795325112867385e-01, 1.764504708433667706e-17),
        dd_cascade(8.314696123025452357e-01, 1.407385698472802389e-18),
        dd_cascade(7.071067811865475727e-01, -4.833646656726456726e-17)
    };

    /* Computes sin(a) using Taylor series.
       Assumes |a| <= pi/32.                           */
    inline dd_cascade sin_taylor(const dd_cascade& a) {
        const double threshold = 0.5 * std::abs(double(a)) * ddc_eps;

        if (a.iszero()) return 0.0; 

        dd_cascade x = -sqr(a);
	    dd_cascade s{a};
	    dd_cascade r{a};
	    dd_cascade t{};
        int i = 0;
        do {
            r *= x;
            t = r * ddc_inverse_factorial[i];
            s += t;
            i += 2;
        } while (i < ddc_inverse_factorial_table_size && std::abs(double(t)) > threshold);

        return s;
    }

    inline dd_cascade cos_taylor(const dd_cascade& a) {
        const double threshold = 0.5 * ddc_eps;

        if (a.iszero()) return 1.0;

        dd_cascade x = -sqr(a);
	    dd_cascade r{x};
	    dd_cascade s = 1.0 + mul_pwr2(r, 0.5);
	    dd_cascade t{};
        int i = 1;
        do {
            r *= x;
            t = r * ddc_inverse_factorial[i];
            s += t;
            i += 2;
        } while (i < ddc_inverse_factorial_table_size && std::abs(double(t)) > threshold);

        return s;
    }

    inline void sincos_taylor(const dd_cascade& a, dd_cascade& sin_a, dd_cascade& cos_a) {
        if (a.iszero()) {
            sin_a = 0.0;
            cos_a = 1.0;
            return;
        }

        sin_a = sin_taylor(a);
        cos_a = sqrt(1.0 - sqr(sin_a));
    }


    inline dd_cascade sin(const dd_cascade& a) {

        /* Strategy.  To compute sin(x), we choose integers a, b so that

             x = s + a * (pi/2) + b * (pi/16)

           and |s| <= pi/32.  Using the fact that

             sin(pi/16) = 0.5 * sqrt(2 - sqrt(2 + sqrt(2)))

           we can compute sin(x) from sin(s), cos(s).  This greatly
           increases the convergence of the sine Taylor series. */

        if (a.iszero()) return 0.0;

        // approximately reduce modulo 2*pi
	    dd_cascade z = nint(a / ddc_2pi);
	    dd_cascade r = a - ddc_2pi * z;

        // approximately reduce modulo pi/2 and then modulo pi/16.
	    dd_cascade t;
        double q = std::floor(r.high() / ddc_pi_2.high() + 0.5);
        t = r - ddc_pi_2 * q;
        int j = static_cast<int>(q);
        q = std::floor(t.high() / ddc_pi16.high() + 0.5);
        t -= ddc_pi16 * q;
        int k = static_cast<int>(q);
        int abs_k = std::abs(k);

        if (j < -2 || j > 2) {
            std::cerr << "sin: Cannot reduce modulo pi/2\n";
		    return dd_cascade(SpecificValue::snan);
        }

        if (abs_k > 4) {
            std::cerr << "(dd::sin): Cannot reduce modulo pi/16\n";
		    return dd_cascade(SpecificValue::snan);
        }

        if (k == 0) {
            switch (j) {
            case 0:
                return sin_taylor(t);
            case 1:
                return cos_taylor(t);
            case -1:
                return -cos_taylor(t);
            default:
                return -sin_taylor(t);
            }
        }

        dd_cascade u(ddc_cos_table[abs_k - 1]);
	    dd_cascade v(ddc_sin_table[abs_k - 1]);
	    dd_cascade sin_t, cos_t;
        sincos_taylor(t, sin_t, cos_t);
        if (j == 0) {
            if (k > 0) {
                r = u * sin_t + v * cos_t;
            }
            else {
                r = u * sin_t - v * cos_t;
            }
        }
        else if (j == 1) {
            if (k > 0) {
                r = u * cos_t - v * sin_t;
            }
            else {
                r = u * cos_t + v * sin_t;
            }
        }
        else if (j == -1) {
            if (k > 0) {
                r = v * sin_t - u * cos_t;
            }
            else if (k < 0) {
                r = -u * cos_t - v * sin_t;
            }
        }
        else {
            if (k > 0) {
                r = -u * sin_t - v * cos_t;
            }
            else {
                r = v * cos_t - u * sin_t;
            }
        }

        return r;
    }

    inline dd_cascade cos(const dd_cascade& a) {

        if (a.iszero()) return 1.0;

        // approximately reduce modulo 2*pi
	    dd_cascade z = nint(a / ddc_2pi);
	    dd_cascade r = a - z * ddc_2pi;

        // approximately reduce modulo pi/2 and then modulo pi/16
	    dd_cascade t;
        double q = std::floor(r.high() / ddc_pi_2.high() + 0.5);
        t = r - ddc_pi_2 * q;
        int j = static_cast<int>(q);
        q = std::floor(t.high() / ddc_pi16.high() + 0.5);
        t -= ddc_pi16 * q;
        int k = static_cast<int>(q);
        int abs_k = std::abs(k);

        if (j < -2 || j > 2) {
            std::cerr << "cos: Cannot reduce modulo pi/2\n";
            return dd_cascade(SpecificValue::snan);
        }

        if (abs_k > 4) {
            std::cerr << "cos: Cannot reduce modulo pi / 16\n";
            return dd_cascade(SpecificValue::snan);
        }

        if (k == 0) {
            switch (j) {
            case 0:
                return cos_taylor(t);
            case 1:
                return -sin_taylor(t);
            case -1:
                return sin_taylor(t);
            default:
                return -cos_taylor(t);
            }
        }

        dd_cascade sin_t, cos_t;
        sincos_taylor(t, sin_t, cos_t);
	    dd_cascade u(ddc_cos_table[abs_k - 1]);
	    dd_cascade v(ddc_sin_table[abs_k - 1]);

        if (j == 0) {
            if (k > 0) {
                r = u * cos_t - v * sin_t;
            }
            else {
                r = u * cos_t + v * sin_t;
            }
        }
        else if (j == 1) {
            if (k > 0) {
                r = -u * sin_t - v * cos_t;
            }
            else {
                r = v * cos_t - u * sin_t;
            }
        }
        else if (j == -1) {
            if (k > 0) {
                r = u * sin_t + v * cos_t;
            }
            else {
                r = u * sin_t - v * cos_t;
            }
        }
        else {
            if (k > 0) {
                r = v * sin_t - u * cos_t;
            }
            else {
                r = -u * cos_t - v * sin_t;
            }
        }

        return r;
    }

    inline void sincos(const dd_cascade& a, dd_cascade& sin_a, dd_cascade& cos_a) {

        if (a.iszero()) {
            sin_a = 0.0;
            cos_a = 1.0;
            return;
        }

        // approximately reduce modulo 2*pi
	    dd_cascade z = nint(a / ddc_2pi);
	    dd_cascade r = a - ddc_2pi * z;

        // approximately reduce module pi/2 and pi/16
	    dd_cascade t;
        double q = std::floor(r.high() / ddc_pi_2.high() + 0.5);
        t = r - ddc_pi_2 * q;
        int j = static_cast<int>(q);
        int abs_j = std::abs(j);
        q = std::floor(t.high() / ddc_pi16.high() + 0.5);
        t -= ddc_pi16 * q;
        int k = static_cast<int>(q);
        int abs_k = std::abs(k);

        if (abs_j > 2) {
            std::cerr << "sincos: Cannot reduce modulo pi/2\n";
		    cos_a = sin_a = dd_cascade(SpecificValue::snan);
            return;
        }

        if (abs_k > 4) {
            std::cerr << "sincos: Cannot reduce modulo pi/16\n";
		    cos_a = sin_a = dd_cascade(SpecificValue::snan);
            return;
        }

        dd_cascade sin_t, cos_t;
	    dd_cascade s, c;

        sincos_taylor(t, sin_t, cos_t);

        if (abs_k == 0) {
            s = sin_t;
            c = cos_t;
        }
        else {
		    dd_cascade u(ddc_cos_table[abs_k - 1]);
		    dd_cascade v(ddc_sin_table[abs_k - 1]);

            if (k > 0) {
                s = u * sin_t + v * cos_t;
                c = u * cos_t - v * sin_t;
            }
            else {
                s = u * sin_t - v * cos_t;
                c = u * cos_t + v * sin_t;
            }
        }

        if (abs_j == 0) {
            sin_a = s;
            cos_a = c;
        }
        else if (j == 1) {
            sin_a = c;
            cos_a = -s;
        }
        else if (j == -1) {
            sin_a = -c;
            cos_a = s;
        }
        else {
            sin_a = -s;
            cos_a = -c;
        }

    }

    inline dd_cascade atan2(const dd_cascade& y, const dd_cascade& x) {
        /* Strategy: Instead of using Taylor series to compute
           arctan, we instead use Newton's iteration to solve
           the equation

              sin(z) = y/r    or    cos(z) = x/r

           where r = sqrt(x^2 + y^2).
           The iteration is given by

              z' = z + (y - sin(z)) / cos(z)          (for equation 1)
              z' = z - (x - cos(z)) / sin(z)          (for equation 2)

           Here, x and y are normalized so that x^2 + y^2 = 1.
           If |x| > |y|, then first iteration is used since the
           denominator is larger.  Otherwise, the second is used.
        */

        if (x.iszero()) {

            if (y.iszero()) {
                /* Both x and y is zero. */
                std::cerr << "atan2: Both arguments zero\n";
			    return dd_cascade(SpecificValue::snan);
            }

            return (y.ispos()) ? ddc_pi_2 : -ddc_pi_2;
        }
        else if (y.iszero()) {
            return (x.ispos()) ? dd_cascade(0.0) : ddc_pi;
        }

        if (x == y) {
            return (y.ispos()) ? ddc_pi_4 : -ddc_3pi_4;
        }

        if (x == -y) {
            return (y.ispos()) ? ddc_3pi_4 : -ddc_pi_4;
        }

        dd_cascade r  = sqrt(sqr(x) + sqr(y));
	    dd_cascade xx = x / r;
	    dd_cascade yy = y / r;

        // Compute double precision approximation to atan.
	    dd_cascade z = std::atan2(double(y), double(x));
	    dd_cascade sin_z, cos_z;

        if (std::abs(xx.high()) > std::abs(yy.high())) {
            // Use Newton iteration 1.  z' = z + (y - sin(z)) / cos(z)
            sincos(z, sin_z, cos_z);
            z += (yy - sin_z) / cos_z;
        }
        else {
            // Use Newton iteration 2.  z' = z - (x - cos(z)) / sin(z)
            sincos(z, sin_z, cos_z);
            z -= (xx - cos_z) / sin_z;
        }

        return z;
    }

    inline dd_cascade atan(const dd_cascade& a) {
	    return atan2(a, dd_cascade(1.0));
    }

    inline dd_cascade tan(const dd_cascade& a) {
	    dd_cascade s, c;
        sincos(a, s, c);
        return s / c;
    }

    inline dd_cascade asin(const dd_cascade& a) {
	    dd_cascade abs_a = abs(a);

        if (abs_a > 1.0) {
            std::cerr << "asin: Argument out of domain\n";
            return dd_cascade(SpecificValue::snan);
        }

        if (abs_a.isone()) {
            return (a.ispos()) ? ddc_pi_2 : -ddc_pi_2;
        }

        return atan2(a, sqrt(1.0 - sqr(a)));
    }

    inline dd_cascade acos(const dd_cascade& a) {
	    dd_cascade abs_a = abs(a);

        if (abs_a > 1.0) {
            std::cerr << "acos: Argument out of domain\n";
            return dd_cascade(SpecificValue::snan);
        }

        if (abs_a.isone()) {
		    return (a.ispos()) ? dd_cascade(0.0) : ddc_pi;
        }

        return atan2(sqrt(1.0 - sqr(a)), a);
    }

}} // namespace sw::universal
