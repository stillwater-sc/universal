#pragma once
// trigonometry.hpp: trigonometry function support for double-double floating-point
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

namespace sw { namespace universal {

    // pi/16
    constexpr dd _pi16(1.963495408493620697e-01, 7.654042494670957545e-18);

    // Table of sin(k * pi/16)
    constexpr dd dd_sin_table[4] = {
        dd(1.950903220161282758e-01, -7.991079068461731263e-18),
        dd(3.826834323650897818e-01, -1.005077269646158761e-17),
        dd(5.555702330196021776e-01,  4.709410940561676821e-17),
        dd(7.071067811865475727e-01, -4.833646656726456726e-17)
    };

    // Table of cos(k * pi/16)
    constexpr dd dd_cos_table[4] = {
        dd(9.807852804032304306e-01, 1.854693999782500573e-17),
        dd(9.238795325112867385e-01, 1.764504708433667706e-17),
        dd(8.314696123025452357e-01, 1.407385698472802389e-18),
        dd(7.071067811865475727e-01, -4.833646656726456726e-17)
    };

    /* Computes sin(a) using Taylor series.
       Assumes |a| <= pi/32.                           */
    inline dd sin_taylor(const dd& a) {
        const double threshold = 0.5 * std::abs(double(a)) * dd_eps;

        if (a.iszero()) return 0.0; 

        dd x = -sqr(a);
        dd s{ a };
        dd r{ a };
        dd t{};
        int i = 0;
        do {
            r *= x;
            t = r * dd_inverse_factorial[i];
            s += t;
            i += 2;
        } while (i < dd_inverse_factorial_table_size && std::abs(double(t)) > threshold);

        return s;
    }

    inline dd cos_taylor(const dd& a) {
        const double threshold = 0.5 * dd_eps;

        if (a.iszero()) return 1.0;

        dd x = -sqr(a);
        dd r{ x };
        dd s = 1.0 + mul_pwr2(r, 0.5);
        dd t{};
        int i = 1;
        do {
            r *= x;
            t = r * dd_inverse_factorial[i];
            s += t;
            i += 2;
        } while (i < dd_inverse_factorial_table_size && std::abs(double(t)) > threshold);

        return s;
    }

    inline void sincos_taylor(const dd& a, dd& sin_a, dd& cos_a) {
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
    inline int dd_reduce_pio2(const dd& a, dd& t) {
        if (std::fabs(a[0]) <= 0.78539816339744828) { t = a; return 0; }
        const double limbs[2] = { a[0], a[1] };
        return multidouble_reduction::reduce_pio2(limbs, dd_pi_2, t);
    }

    inline dd sin(const dd& a) {

        /* Strategy.  To compute sin(x), we choose integers a, b so that

             x = s + a * (pi/2) + b * (pi/16)

           and |s| <= pi/32.  Using the fact that

             sin(pi/16) = 0.5 * sqrt(2 - sqrt(2 + sqrt(2)))

           we can compute sin(x) from sin(s), cos(s).  This greatly
           increases the convergence of the sine Taylor series. */

        if (a.iszero()) return 0.0;

        if (a.isnan()) return dd(SpecificValue::qnan);
        if (a.isinf()) { errno = EDOM; return dd(SpecificValue::qnan); }

        // reduce exactly modulo pi/2: a = (j + 4n) pi/2 + t with |t| <= pi/4 (#1723)
        dd t, r;
        int j = dd_reduce_pio2(a, t);
        double q = std::floor(t.high() / _pi16.high() + 0.5);
        if (!(std::fabs(q) <= 4.0)) return dd(SpecificValue::qnan);   // unreachable: |t| <= pi/4
        t -= _pi16 * q;
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

        dd u(dd_cos_table[abs_k - 1]);
        dd v(dd_sin_table[abs_k - 1]);
        dd sin_t, cos_t;
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

    inline dd cos(const dd& a) {

        if (a.iszero()) return 1.0;

        if (a.isnan()) return dd(SpecificValue::qnan);
        if (a.isinf()) { errno = EDOM; return dd(SpecificValue::qnan); }

        // reduce exactly modulo pi/2: a = (j + 4n) pi/2 + t with |t| <= pi/4 (#1723)
        dd t, r;
        int j = dd_reduce_pio2(a, t);
        double q = std::floor(t.high() / _pi16.high() + 0.5);
        if (!(std::fabs(q) <= 4.0)) return dd(SpecificValue::qnan);   // unreachable: |t| <= pi/4
        t -= _pi16 * q;
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

        dd sin_t, cos_t;
        sincos_taylor(t, sin_t, cos_t);
        dd u(dd_cos_table[abs_k - 1]);
        dd v(dd_sin_table[abs_k - 1]);

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

    inline void sincos(const dd& a, dd& sin_a, dd& cos_a) {

        if (a.iszero()) {
            sin_a = 0.0;
            cos_a = 1.0;
            return;
        }

        if (a.isnan()) { sin_a = cos_a = dd(SpecificValue::qnan); return; }
        if (a.isinf()) { errno = EDOM; sin_a = cos_a = dd(SpecificValue::qnan); return; }

        // reduce exactly modulo pi/2: a = (j + 4n) pi/2 + t with |t| <= pi/4 (#1723)
        dd t;
        int j = dd_reduce_pio2(a, t);
        int abs_j = std::abs(j);
        double q = std::floor(t.high() / _pi16.high() + 0.5);
        if (!(std::fabs(q) <= 4.0)) { sin_a = cos_a = dd(SpecificValue::qnan); return; }   // unreachable: |t| <= pi/4
        t -= _pi16 * q;
        int k = static_cast<int>(q);
        int abs_k = std::abs(k);

        dd sin_t, cos_t;
        dd s, c;

        sincos_taylor(t, sin_t, cos_t);

        if (abs_k == 0) {
            s = sin_t;
            c = cos_t;
        }
        else {
            dd u(dd_cos_table[abs_k - 1]);
            dd v(dd_sin_table[abs_k - 1]);

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

    inline dd atan2(const dd& y, const dd& x) {
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
                if (std::signbit(x.high())) return std::signbit(y.high()) ? -dd_pi : dd_pi;
                return y;
            }

            return (y.ispos()) ? dd_pi_2 : -dd_pi_2;
        }
        else if (y.iszero()) {
            return (x.ispos()) ? y : (std::signbit(y.high()) ? -dd_pi : dd_pi);   // signed as y, as std::atan2
        }

        if (x == y) {
            return (y.ispos()) ? dd_pi_4 : -dd_3pi_4;
        }

        if (x == -y) {
            return (y.ispos()) ? dd_3pi_4 : -dd_pi_4;
        }

        dd r = sqrt(sqr(x) + sqr(y));
        dd xx = x / r;
        dd yy = y / r;

        // Compute double precision approximation to atan.
        dd z = std::atan2(double(y), double(x));
        dd sin_z, cos_z;

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

    inline dd atan(const dd& a) {
        return atan2(a, dd(1.0));
    }

    inline dd tan(const dd& a) {
        dd s, c;
        sincos(a, s, c);
        return s / c;
    }

    inline dd asin(const dd& a) {
        dd abs_a = abs(a);

        if (abs_a > 1.0) {
            errno = EDOM;
            return dd(SpecificValue::qnan);
        }

        if (abs_a.isone()) {
            return (a.ispos()) ? dd_pi_2 : -dd_pi_2;
        }

        return atan2(a, sqrt(1.0 - sqr(a)));
    }

    inline dd acos(const dd& a) {
        dd abs_a = abs(a);

        if (abs_a > 1.0) {
            errno = EDOM;
            return dd(SpecificValue::qnan);
        }

        if (abs_a.isone()) {
            return (a.ispos()) ? dd(0.0) : dd_pi;
        }

        return atan2(sqrt(1.0 - sqr(a)), a);
    }

}} // namespace sw::universal
