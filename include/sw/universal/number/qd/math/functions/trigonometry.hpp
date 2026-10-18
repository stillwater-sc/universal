#pragma once
// trigonometry.hpp: trigonometry function support for quad-double (qd) floating-point
// 
// algorithms and constants courtesy of Scibuilders, Jack Poulson
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cerrno>
#include <cmath>
#include <universal/number/qd/math/constants/sincos_tables.hpp>

namespace sw { namespace universal {

	// value representing an angle expressed in radians
	// One radian is equivalent to 180/PI degrees

/// <summary>
/// Compute sin(a) and cos(a) using a Taylor series approximation. Assumes |a| <= pi/2048
/// </summary>
/// <param name="a">angle in radians</param>
/// <param name="sin_a">result sin(a)</param>
/// <param name="cos_a">result cos(a)</param>
inline void sincos_taylor(const qd& a, qd& sin_a, qd& cos_a) {
    const double threshold = 0.5 * qd_eps * std::abs(double(a));
    qd p, s, t, x;

    if (a.iszero()) {
        sin_a = 0.0;
        cos_a = 1.0;
        return;
    }

    x = -sqr(a);
    s = a;
    p = a;
    unsigned int i = 0;
    do {
        p *= x;
        t = p * qd_inverse_factorial[i];
        s += t;
        i += 2;
    } while (i < qd_inverse_factorial_table_size && std::abs(double(t)) > threshold);

    sin_a = s;
    cos_a = sqrt(1.0 - sqr(s));
}

inline qd sin_taylor(const qd& a) {
    const double threshold = 0.5 * qd_eps * std::abs(double(a));
    qd p, s, t, x;

    if (a.iszero()) {
        return 0.0;
    }

    x = -sqr(a);
    s = a;
    p = a;
    unsigned int i = 0;
    do {
        p *= x;
        t = p * qd_inverse_factorial[i];
        s += t;
        i += 2;
    } while (i < qd_inverse_factorial_table_size && std::abs(double(t)) > threshold);

    return s;
}

inline qd cos_taylor(const qd& a) {
    constexpr double threshold = 0.5 * qd_eps;
    qd p, s, t, x;

    if (a.iszero()) {
        return 1.0;
    }

    x = -sqr(a);
    s = 1.0 + mul_pwr2(x, 0.5);
    p = x;
    unsigned int i = 1;
    do {
        p *= x;
        t = p * qd_inverse_factorial[i];
        s += t;
        i += 2;
    } while (i < qd_inverse_factorial_table_size && std::abs(double(t)) > threshold);

    return s;
}

    // The argument of sin, cos or sincos is finite but too large to reduce: the remainder
    // modulo a 2*pi held to this precision is not within [-pi, pi], so every digit of the
    // result would be noise.  Reported as a NaN with errno = ERANGE, or thrown.
    inline qd qd_trig_reduction_failed() {
#if QUADDOUBLE_THROW_ARITHMETIC_EXCEPTION
        throw qd_trig_argument_too_large();
#else
        errno = ERANGE;
        return qd(SpecificValue::qnan);
#endif
    }

inline qd sin(const qd& a) {

    /* Strategy.  To compute sin(x), we choose integers a, b so that

         x = s + a * (pi/2) + b * (pi/1024)

       and |s| <= pi/2048.  Using a precomputed table of
       sin(k pi / 1024) and cos(k pi / 1024), we can compute
       sin(x) from sin(s) and cos(s).  This greatly increases the
       convergence of the sine Taylor series.                          */

    if (a.iszero()) return 0.0;

    if (a.isnan()) return qd(SpecificValue::qnan);
    if (a.isinf()) { errno = EDOM; return qd(SpecificValue::qnan); }

    // approximately reduce modulo 2*pi
    qd z = nint(a / qd_2pi);
    qd r = a - qd_2pi * z;

    // approximately reduce modulo pi/2 and then modulo pi/1024
    double q = std::floor(r[0] / qd_pi_2[0] + 0.5);
    // |q| > 2 means the reduction modulo 2*pi left more than pi: the argument is too
    // large for this precision.  Test before the casts to int, which overflow for such q.
    if (!(std::fabs(q) <= 2.0)) return qd_trig_reduction_failed();
    qd t = r - qd_pi_2 * q;
    int j = static_cast<int>(q);
    q = std::floor(t[0] / qd_pi1024[0] + 0.5);
    if (!(std::fabs(q) <= 256.0)) return qd_trig_reduction_failed();   // guards the table index
    t -= qd_pi1024 * q;
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

    qd sin_t, cos_t;
    qd u = qd_cos_table[abs_k - 1];
    qd v = qd_sin_table[abs_k - 1];
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
        else {
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

    if (a.isnan()) return qd(SpecificValue::qnan);
    if (a.isinf()) { errno = EDOM; return qd(SpecificValue::qnan); }

    // approximately reduce modulo 2*pi
    qd z = nint(a / qd_2pi);
    qd r = a - qd_2pi * z;

    // approximately reduce modulo pi/2 and then modulo pi/1024
    double q = std::floor(r[0] / qd_pi_2[0] + 0.5);
    // |q| > 2 means the reduction modulo 2*pi left more than pi: the argument is too
    // large for this precision.  Test before the casts to int, which overflow for such q.
    if (!(std::fabs(q) <= 2.0)) return qd_trig_reduction_failed();
    qd t = r - qd_pi_2 * q;
    int j = static_cast<int>(q);
    q = std::floor(t[0] / qd_pi1024[0] + 0.5);
    if (!(std::fabs(q) <= 256.0)) return qd_trig_reduction_failed();   // guards the table index
    t -= qd_pi1024 * q;
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

    qd sin_t, cos_t;
    sincos_taylor(t, sin_t, cos_t);

    qd u = qd_cos_table[abs_k - 1];
    qd v = qd_sin_table[abs_k - 1];

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

    if (a.isnan()) { sin_a = cos_a = qd(SpecificValue::qnan); return; }
    if (a.isinf()) { errno = EDOM; sin_a = cos_a = qd(SpecificValue::qnan); return; }

    // approximately reduce by 2*pi
    qd z = nint(a / qd_2pi);
    qd t = a - qd_2pi * z;

    // approximately reduce by pi/2 and then by pi/1024.
    double q = std::floor(t[0] / qd_pi_2[0] + 0.5);
    // |q| > 2 means the reduction modulo 2*pi left more than pi: the argument is too
    // large for this precision.  Test before the casts to int, which overflow for such q.
    if (!(std::fabs(q) <= 2.0)) { sin_a = cos_a = qd_trig_reduction_failed(); return; }
    t -= qd_pi_2 * q;
    int j = static_cast<int>(q);
    q = std::floor(t[0] / qd_pi1024[0] + 0.5);
    if (!(std::fabs(q) <= 256.0)) { sin_a = cos_a = qd_trig_reduction_failed(); return; }   // guards the table index
    t -= qd_pi1024 * q;
    int k = static_cast<int>(q);
    int abs_k = std::abs(k);

    qd sin_t, cos_t;
    sincos_taylor(t, sin_t, cos_t);

    if (k == 0) {
        if (j == 0) {
            sin_a = sin_t;
            cos_a = cos_t;
        }
        else if (j == 1) {
            sin_a = cos_t;
            cos_a = -sin_t;
        }
        else if (j == -1) {
            sin_a = -cos_t;
            cos_a = sin_t;
        }
        else {
            sin_a = -sin_t;
            cos_a = -cos_t;
        }
        return;
    }

    qd u = qd_cos_table[abs_k - 1];
    qd v = qd_sin_table[abs_k - 1];

    if (j == 0) {
        if (k > 0) {
            sin_a = u * sin_t + v * cos_t;
            cos_a = u * cos_t - v * sin_t;
        }
        else {
            sin_a = u * sin_t - v * cos_t;
            cos_a = u * cos_t + v * sin_t;
        }
    }
    else if (j == 1) {
        if (k > 0) {
            cos_a = -u * sin_t - v * cos_t;
            sin_a = u * cos_t - v * sin_t;
        }
        else {
            cos_a = v * cos_t - u * sin_t;
            sin_a = u * cos_t + v * sin_t;
        }
    }
    else if (j == -1) {
        if (k > 0) {
            cos_a = u * sin_t + v * cos_t;
            sin_a = v * sin_t - u * cos_t;
        }
        else {
            cos_a = u * sin_t - v * cos_t;
            sin_a = -u * cos_t - v * sin_t;
        }
    }
    else {
        if (k > 0) {
            sin_a = -u * sin_t - v * cos_t;
            cos_a = v * sin_t - u * cos_t;
        }
        else {
            sin_a = v * cos_t - u * sin_t;
            cos_a = -u * cos_t - v * sin_t;
        }
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
            // both zero: std::atan2 gives +-0 for x = +0 and +-pi for x = -0, signed as y
            if (std::signbit(x[0])) return std::signbit(y[0]) ? -qd_pi : qd_pi;
            return y;
        }

        return (y.ispos()) ? qd_pi_2 : -qd_pi_2;
    }
    else if (y.iszero()) {
        return (x.ispos()) ? y : (std::signbit(y[0]) ? -qd_pi : qd_pi);   // signed as y, as std::atan2
    }

    if (x == y) {
        return (y.ispos()) ? qd_pi_4 : -qd_3pi_4;
    }

    if (x == -y) {
        return (y.ispos()) ? qd_3pi_4 : -qd_pi_4;
    }

    qd r = sqrt(sqr(x) + sqr(y));
    qd xx = x / r;
    qd yy = y / r;

    /* Compute double precision approximation to atan. */
    qd z = std::atan2(double(y), double(x));
    qd sin_z, cos_z;

    if (std::abs(xx[0]) > std::abs(yy[0])) {
        /* Use Newton iteration 1.  z' = z + (y - sin(z)) / cos(z)  */
        sincos(z, sin_z, cos_z);
        z += (yy - sin_z) / cos_z;
        sincos(z, sin_z, cos_z);
        z += (yy - sin_z) / cos_z;
        sincos(z, sin_z, cos_z);
        z += (yy - sin_z) / cos_z;
    }
    else {
        /* Use Newton iteration 2.  z' = z - (x - cos(z)) / sin(z)  */
        sincos(z, sin_z, cos_z);
        z -= (xx - cos_z) / sin_z;
        sincos(z, sin_z, cos_z);
        z -= (xx - cos_z) / sin_z;
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
        errno = EDOM;
        return qd(SpecificValue::qnan);
    }

    if (abs_a.isone()) {
        return (a.ispos()) ? qd_pi_2 : -qd_pi_2;
    }

    return atan2(a, sqrt(1.0 - sqr(a)));
}

inline qd acos(const qd& a) {
    qd abs_a = abs(a);

    if (abs_a > 1.0) {
        errno = EDOM;
        return qd(SpecificValue::qnan);
    }

    if (abs_a.isone()) {
        return (a.ispos()) ? qd(0.0) : qd_pi;
    }

    return atan2(sqrt(1.0 - sqr(a)), a);
}

}} // namespace sw::universal
