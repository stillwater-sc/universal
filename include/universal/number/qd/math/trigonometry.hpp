#pragma once
// trigonometry.hpp: trigonometry function support for quad-double (qd) floating-point
// 
// algorithms and constants courtesy of Scibuilders, Jack Poulson
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

    // pi/16
    static constexpr qd _pi16(1.963495408493620697e-01, 7.654042494670957545e-18);

    // Table of sin(k * pi/16) and cos(k * pi/16).
    static const double sin_table[4][2] = {
      {1.950903220161282758e-01, -7.991079068461731263e-18},
      {3.826834323650897818e-01, -1.005077269646158761e-17},
      {5.555702330196021776e-01,  4.709410940561676821e-17},
      {7.071067811865475727e-01, -4.833646656726456726e-17}
    };

    static const double cos_table[4][2] = {
      {9.807852804032304306e-01, 1.854693999782500573e-17},
      {9.238795325112867385e-01, 1.764504708433667706e-17},
      {8.314696123025452357e-01, 1.407385698472802389e-18},
      {7.071067811865475727e-01, -4.833646656726456726e-17}
    };

    /* Computes sin(a) using Taylor series.
       Assumes |a| <= pi/32.                           */
    inline qd sin_taylor(const qd& a) {
        const double thresh = 0.5 * std::abs(double(a)) * d_eps;

        if (a.iszero()) return 0.0; 

        qd x = -sqr(a);
        qd s{ a };
        qd r{ a };
        qd t{};
        int i = 0;
        do {
            r *= x;
            t = r * qd(inv_fact[i][0], inv_fact[i][1]);
            s += t;
            i += 2;
        } while (i < n_inv_fact && std::abs(double(t)) > thresh);

        return s;
    }

    inline qd cos_taylor(const qd& a) {
        const double thresh = 0.5 * d_eps;

        if (a.iszero()) {
            return 1.0;
        }

        qd x = -sqr(a);
        qd r{ x };
        qd s = 1.0 + mul_pwr2(r, 0.5);
        qd t{};
        int i = 1;
        do {
            r *= x;
            t = r * qd(inv_fact[i][0], inv_fact[i][1]);
            s += t;
            i += 2;
        } while (i < n_inv_fact && std::abs(double(t)) > thresh);

        return s;
    }

    inline void sincos_taylor(const qd& a, qd& sin_a, qd& cos_a) {
        if (a.iszero()) {
            sin_a = 0.0;
            cos_a = 1.0;
            return;
        }

        sin_a = sin_taylor(a);
        cos_a = sqrt(1.0 - sqr(sin_a));
    }


    inline qd sin(const qd& a) {

        /* Strategy.  To compute sin(x), we choose integers a, b so that

             x = s + a * (pi/2) + b * (pi/16)

           and |s| <= pi/32.  Using the fact that

             sin(pi/16) = 0.5 * sqrt(2 - sqrt(2 + sqrt(2)))

           we can compute sin(x) from sin(s), cos(s).  This greatly
           increases the convergence of the sine Taylor series. */

        if (a.iszero()) return 0.0;

        // approximately reduce modulo 2*pi
        qd z = nint(a / qd_2pi);
        qd r = a - qd_2pi * z;

        // approximately reduce modulo pi/2 and then modulo pi/16.
        qd t;
        double q = std::floor(r.high() / qd_pi2.high() + 0.5);
        t = r - qd_pi2 * q;
        int j = static_cast<int>(q);
        q = std::floor(t.high() / _pi16.high() + 0.5);
        t -= _pi16 * q;
        int k = static_cast<int>(q);
        int abs_k = std::abs(k);

        if (j < -2 || j > 2) {
            std::cerr << "sin: Cannot reduce modulo pi/2\n";
            return qd(SpecificValue::snan);
        }

        if (abs_k > 4) {
            std::cerr << "(qd::sin): Cannot reduce modulo pi/16\n";
            return qd(SpecificValue::snan);
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

        qd u(cos_table[abs_k - 1][0], cos_table[abs_k - 1][1]);
        qd v(sin_table[abs_k - 1][0], sin_table[abs_k - 1][1]);
        qd sin_t, cos_t;
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

    inline qd cos(const qd& a) {

        if (a.iszero()) return 1.0;

        // approximately reduce modulo 2*pi
        qd z = nint(a / qd_2pi);
        qd r = a - z * qd_2pi;

        // approximately reduce modulo pi/2 and then modulo pi/16
        qd t;
        double q = std::floor(r.high() / qd_pi2.high() + 0.5);
        t = r - qd_pi2 * q;
        int j = static_cast<int>(q);
        q = std::floor(t.high() / _pi16.high() + 0.5);
        t -= _pi16 * q;
        int k = static_cast<int>(q);
        int abs_k = std::abs(k);

        if (j < -2 || j > 2) {
            std::cerr << "cos: Cannot reduce modulo pi/2\n";
            return qd(SpecificValue::snan);
        }

        if (abs_k > 4) {
            std::cerr << "cos: Cannot reduce modulo pi / 16\n";
            return qd(SpecificValue::snan);
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

        qd sin_t, cos_t;
        sincos_taylor(t, sin_t, cos_t);
        qd u(cos_table[abs_k - 1][0], cos_table[abs_k - 1][1]);
        qd v(sin_table[abs_k - 1][0], sin_table[abs_k - 1][1]);

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

    inline void sincos(const qd& a, qd& sin_a, qd& cos_a) {

        if (a.iszero()) {
            sin_a = 0.0;
            cos_a = 1.0;
            return;
        }

        // approximately reduce modulo 2*pi
        qd z = nint(a / qd_2pi);
        qd r = a - qd_2pi * z;

        // approximately reduce module pi/2 and pi/16
        qd t;
        double q = std::floor(r.high() / qd_pi2.high() + 0.5);
        t = r - qd_pi2 * q;
        int j = static_cast<int>(q);
        int abs_j = std::abs(j);
        q = std::floor(t.high() / _pi16.high() + 0.5);
        t -= _pi16 * q;
        int k = static_cast<int>(q);
        int abs_k = std::abs(k);

        if (abs_j > 2) {
            std::cerr << "sincos: Cannot reduce modulo pi/2\n";
            cos_a = sin_a = qd(SpecificValue::snan);
            return;
        }

        if (abs_k > 4) {
            std::cerr << "sincos: Cannot reduce modulo pi/16\n";
            cos_a = sin_a = qd(SpecificValue::snan);
            return;
        }

        qd sin_t, cos_t;
        qd s, c;

        sincos_taylor(t, sin_t, cos_t);

        if (abs_k == 0) {
            s = sin_t;
            c = cos_t;
        }
        else {
            qd u(cos_table[abs_k - 1][0], cos_table[abs_k - 1][1]);
            qd v(sin_table[abs_k - 1][0], sin_table[abs_k - 1][1]);

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

    inline qd atan2(const qd& y, const qd& x) {
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
                return qd(SpecificValue::snan);
            }

            return (y.ispos()) ? qd_pi2 : -qd_pi2;
        }
        else if (y.iszero()) {
            return (x.ispos()) ? qd(0.0) : qd_pi;
        }

        if (x == y) {
            return (y.ispos()) ? qd_pi4 : -qd_3pi4;
        }

        if (x == -y) {
            return (y.ispos()) ? qd_3pi4 : -qd_pi4;
        }

        qd r = sqrt(sqr(x) + sqr(y));
        qd xx = x / r;
        qd yy = y / r;

        // Compute double precision approximation to atan.
        qd z = std::atan2(double(y), double(x));
        qd sin_z, cos_z;

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

    inline qd atan(const qd& a) {
        return atan2(a, qd(1.0));
    }

    inline qd tan(const qd& a) {
        qd s, c;
        sincos(a, s, c);
        return s / c;
    }

    inline qd asin(const qd& a) {
        qd abs_a = abs(a);

        if (abs_a > 1.0) {
            std::cerr << "asin: Argument out of domain\n";
            return qd(SpecificValue::snan);
        }

        if (abs_a.isone()) {
            return (a.ispos()) ? qd_pi2 : -qd_pi2;
        }

        return atan2(a, sqrt(1.0 - sqr(a)));
    }

    inline qd acos(const qd& a) {
        qd abs_a = abs(a);

        if (abs_a > 1.0) {
            std::cerr << "acos: Argument out of domain\n";
            return qd(SpecificValue::snan);
        }

        if (abs_a.isone()) {
            return (a.ispos()) ? qd(0.0) : qd_pi;
        }

        return atan2(sqrt(1.0 - sqr(a)), a);
    }

}} // namespace sw::universal
