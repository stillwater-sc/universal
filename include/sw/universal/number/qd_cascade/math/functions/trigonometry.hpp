#pragma once
// trigonometry.hpp: trigonometry function support for quad-double floating-point
// 
// algorithms and constants courtesy of Scibuilders, Jack Poulson
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cerrno>
#include <cmath>
#include <universal/number/shared/multidouble_reduce_pio2.hpp>

#include <universal/internal/floatcascade/sincos_tables.hpp>

namespace sw { namespace universal {

    // pi/1024, and sin/cos of k*pi/1024 for k = 1..256, read from the shared cascade tables.
    //
    // The reduction used to stop at pi/16, which leaves an argument as large as pi/32 for the
    // Taylor series. A 15-entry inverse-factorial table cannot carry that argument past
    // double-double accuracy, so every cascade type wider than a double-double delivered 32
    // digits no matter how many limbs it had (universal#1318). Reducing to pi/2048 instead --
    // the same schedule the qd type uses -- brings the series back inside the table.
    //
    // The shared table holds four doubles per entry. Dropping the trailing limbs of a correctly
    // rounded expansion leaves a correctly rounded expansion, so this type reads the leading
    // 4; the generator verifies that prefix against mpmath.
    constexpr qd_cascade qdc_pi1024(cascade::pi1024[0], cascade::pi1024[1], cascade::pi1024[2], cascade::pi1024[3]);

    inline qd_cascade qdc_sin_pi1024(unsigned k) {
        return qd_cascade(cascade::sin_pi1024[k][0], cascade::sin_pi1024[k][1],
                          cascade::sin_pi1024[k][2], cascade::sin_pi1024[k][3]);
    }
    inline qd_cascade qdc_cos_pi1024(unsigned k) {
        return qd_cascade(cascade::cos_pi1024[k][0], cascade::cos_pi1024[k][1],
                          cascade::cos_pi1024[k][2], cascade::cos_pi1024[k][3]);
    }

    /* Computes sin(a) using Taylor series.
       Assumes |a| <= pi/2048.                           */
    inline qd_cascade sin_taylor(const qd_cascade& a) {
        const double threshold = 0.5 * std::abs(double(a)) * qdc_eps;

        if (a.iszero()) return 0.0; 

        qd_cascade x = -sqr(a);
	    qd_cascade s{a};
	    qd_cascade r{a};
	    qd_cascade t{};
        int i = 0;
        do {
            r *= x;
            t = r * qdc_inverse_factorial[i];
            s += t;
            i += 2;
        } while (i < qdc_inverse_factorial_table_size && std::abs(double(t)) > threshold);

        return s;
    }

    inline qd_cascade cos_taylor(const qd_cascade& a) {
        const double threshold = 0.5 * qdc_eps;

        if (a.iszero()) return 1.0;

        qd_cascade x = -sqr(a);
	    qd_cascade r{x};
	    qd_cascade s = 1.0 + mul_pwr2(r, 0.5);
	    qd_cascade t{};
        int i = 1;
        do {
            r *= x;
            t = r * qdc_inverse_factorial[i];
            s += t;
            i += 2;
        } while (i < qdc_inverse_factorial_table_size && std::abs(double(t)) > threshold);

        return s;
    }

    inline void sincos_taylor(const qd_cascade& a, qd_cascade& sin_a, qd_cascade& cos_a) {
        if (a.iszero()) {
            sin_a = 0.0;
            cos_a = 1.0;
            return;
        }

        sin_a = sin_taylor(a);
        cos_a = sqrt(1.0 - sqr(sin_a));
    }


    // a = (j + 4n) pi/2 + t with |t| <= pi/4, to the full precision of the type.  An argument
    // within pi/4 is its own reduction; any other is reduced exactly, by Payne and Hanek's
    // method (number/shared/multidouble_reduce_pio2.hpp), so no finite argument loses digits.
    inline int qd_cascade_reduce_pio2(const qd_cascade& a, qd_cascade& t) {
        if (std::fabs(a[0]) <= 0.78539816339744828) { t = a; return 0; }
        const double limbs[4] = { a[0], a[1], a[2], a[3] };
        return multidouble_reduction::reduce_pio2(limbs, qdc_pi_2, t);
    }

    inline qd_cascade sin(const qd_cascade& a) {

        /* Strategy.  To compute sin(x), we choose integers a, b so that

             x = s + a * (pi/2) + b * (pi/1024)

           and |s| <= pi/2048.  Using a precomputed table of
           sin(k pi / 1024) and cos(k pi / 1024), we can compute
           sin(x) from sin(s) and cos(s).  This greatly increases the
           convergence of the sine Taylor series. */

        if (a.iszero()) return 0.0;

        if (a.isnan()) return qd_cascade(SpecificValue::qnan);
        if (a.isinf()) { errno = EDOM; return qd_cascade(SpecificValue::qnan); }

        // reduce exactly modulo pi/2: a = (j + 4n) pi/2 + t with |t| <= pi/4 (#1723)
        qd_cascade t, r;
        int j = qd_cascade_reduce_pio2(a, t);
        double q = std::floor(t[0] / qdc_pi1024[0] + 0.5);
        if (!(std::fabs(q) <= 256.0)) return qd_cascade(SpecificValue::qnan);   // unreachable: |t| <= pi/4
        t -= qdc_pi1024 * q;
        int k = static_cast<int>(q);
        int abs_k = std::abs(k);

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

        qd_cascade u(qdc_cos_pi1024(static_cast<unsigned>(abs_k) - 1));
	    qd_cascade v(qdc_sin_pi1024(static_cast<unsigned>(abs_k) - 1));
	    qd_cascade sin_t, cos_t;
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

    inline qd_cascade cos(const qd_cascade& a) {

        if (a.iszero()) return 1.0;

        if (a.isnan()) return qd_cascade(SpecificValue::qnan);
        if (a.isinf()) { errno = EDOM; return qd_cascade(SpecificValue::qnan); }

        // reduce exactly modulo pi/2: a = (j + 4n) pi/2 + t with |t| <= pi/4 (#1723)
        qd_cascade t, r;
        int j = qd_cascade_reduce_pio2(a, t);
        double q = std::floor(t[0] / qdc_pi1024[0] + 0.5);
        if (!(std::fabs(q) <= 256.0)) return qd_cascade(SpecificValue::qnan);   // unreachable: |t| <= pi/4
        t -= qdc_pi1024 * q;
        int k = static_cast<int>(q);
        int abs_k = std::abs(k);

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

        qd_cascade sin_t, cos_t;
        sincos_taylor(t, sin_t, cos_t);
	    qd_cascade u(qdc_cos_pi1024(static_cast<unsigned>(abs_k) - 1));
	    qd_cascade v(qdc_sin_pi1024(static_cast<unsigned>(abs_k) - 1));

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

    inline void sincos(const qd_cascade& a, qd_cascade& sin_a, qd_cascade& cos_a) {

        if (a.iszero()) {
            sin_a = 0.0;
            cos_a = 1.0;
            return;
        }

        if (a.isnan()) { sin_a = cos_a = qd_cascade(SpecificValue::qnan); return; }
        if (a.isinf()) { errno = EDOM; sin_a = cos_a = qd_cascade(SpecificValue::qnan); return; }

        // reduce exactly modulo pi/2: a = (j + 4n) pi/2 + t with |t| <= pi/4 (#1723)
        qd_cascade t;
        int j = qd_cascade_reduce_pio2(a, t);
        int abs_j = std::abs(j);
        double q = std::floor(t[0] / qdc_pi1024[0] + 0.5);
        if (!(std::fabs(q) <= 256.0)) { sin_a = cos_a = qd_cascade(SpecificValue::qnan); return; }   // unreachable: |t| <= pi/4
        t -= qdc_pi1024 * q;
        int k = static_cast<int>(q);
        int abs_k = std::abs(k);

        qd_cascade sin_t, cos_t;
	    qd_cascade s, c;

        sincos_taylor(t, sin_t, cos_t);

        if (abs_k == 0) {
            s = sin_t;
            c = cos_t;
        }
        else {
		    qd_cascade u(qdc_cos_pi1024(static_cast<unsigned>(abs_k) - 1));
		    qd_cascade v(qdc_sin_pi1024(static_cast<unsigned>(abs_k) - 1));

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

    inline qd_cascade atan2(const qd_cascade& y, const qd_cascade& x) {
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
                // both zero: std::atan2 gives +-0 for x = +0 and +-pi for x = -0, signed as y
                if (std::signbit(x[0])) return std::signbit(y[0]) ? -qdc_pi : qdc_pi;
                return y;
            }

            return (y.ispos()) ? qdc_pi_2 : -qdc_pi_2;
        }
        else if (y.iszero()) {
            return (x.ispos()) ? y : (std::signbit(y[0]) ? -qdc_pi : qdc_pi);   // signed as y, as std::atan2
        }

        if (x == y) {
            return (y.ispos()) ? qdc_pi_4 : -qdc_3pi_4;
        }

        if (x == -y) {
            return (y.ispos()) ? qdc_3pi_4 : -qdc_pi_4;
        }

        qd_cascade r  = sqrt(sqr(x) + sqr(y));
	    qd_cascade xx = x / r;
	    qd_cascade yy = y / r;

        // Compute double precision approximation to atan.
	    qd_cascade z = std::atan2(double(y), double(x));
	    qd_cascade sin_z, cos_z;

        // Newton doubles the number of correct bits per step, and the seed above is a double:
        // 53 -> 106 -> 212 -> 424. A quad-double's 212 bits need three.
        // This file was derived from the double-double one, where a single step is right, and the
        // extra steps did not come with it -- so atan, asin and acos all stopped at 106 bits no
        // matter how wide the type was (universal#1318).
        constexpr int newtonSteps = 3;

        if (std::abs(xx[0]) > std::abs(yy[0])) {
            // Use Newton iteration 1.  z' = z + (y - sin(z)) / cos(z)
            for (int i = 0; i < newtonSteps; ++i) {
                sincos(z, sin_z, cos_z);
                z += (yy - sin_z) / cos_z;
            }
        }
        else {
            // Use Newton iteration 2.  z' = z - (x - cos(z)) / sin(z)
            for (int i = 0; i < newtonSteps; ++i) {
                sincos(z, sin_z, cos_z);
                z -= (xx - cos_z) / sin_z;
            }
        }

        return z;
    }

    inline qd_cascade atan(const qd_cascade& a) {
	    return atan2(a, qd_cascade(1.0));
    }

    inline qd_cascade tan(const qd_cascade& a) {
	    qd_cascade s, c;
        sincos(a, s, c);
        return s / c;
    }

    inline qd_cascade asin(const qd_cascade& a) {
	    qd_cascade abs_a = abs(a);

        if (abs_a > 1.0) {
            errno = EDOM;
            return qd_cascade(SpecificValue::qnan);
        }

        if (abs_a.isone()) {
            return (a.ispos()) ? qdc_pi_2 : -qdc_pi_2;
        }

        return atan2(a, sqrt(1.0 - sqr(a)));
    }

    inline qd_cascade acos(const qd_cascade& a) {
	    qd_cascade abs_a = abs(a);

        if (abs_a > 1.0) {
            errno = EDOM;
            return qd_cascade(SpecificValue::qnan);
        }

        if (abs_a.isone()) {
		    return (a.ispos()) ? qd_cascade(0.0) : qdc_pi;
        }

        return atan2(sqrt(1.0 - sqr(a)), a);
    }

}} // namespace sw::universal
