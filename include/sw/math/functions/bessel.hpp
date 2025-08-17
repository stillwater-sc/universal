#pragma once
// bessel.hpp: definition of J and Y Bessel functions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw::math::function {

// Bessel function of the first kind J_n(x)
template<typename T> T bessel_j(unsigned int n, T x, unsigned int max_terms, T tolerance);

// Bessel function of the second kind Y_n(x)
template<typename T> T bessel_y(unsigned int n, T x);

// Modified Bessel function of the first kind I_n(x)
template<typename T> T bessel_i(unsigned int n, T x);

// Modified Bessel function of the second kind K_n(x)
template<typename T> T bessel_k(unsigned int n, T x);


// Bessel function of the first kind J_n(x)
template<typename T>
T bessel_j(unsigned int n, T x, unsigned int max_terms = 50, T tolerance = T(1e-12)) {
	using std::abs, std::pow;
    T sum = T(0);
    T sign = T(1);

    for (unsigned int k = 0; k < max_terms; ++k) {
        T numerator = pow(x / T(2), T(2 * k + n));
        T denominator = factorial<T>(k) * factorial<T>(k + n);
        T term = sign * numerator / denominator;

        sum += term;
        if (abs(term) < tolerance) break;

        sign = -sign;
    }

    return sum;
}

// Optimized J_0(x)
template<typename T>
T bessel_j0(T x, unsigned int max_terms = 50, T tolerance = T(1e-12)) {
	using std::abs, std::pow;
    T sum = T(0);
    T sign = T(1);

    for (unsigned int k = 0; k < max_terms; ++k) {
        T numerator = pow(x / T(2), T(2 * k));
        T denominator = pow(factorial<T>(k), T(2));
        T term = sign * numerator / denominator;

        sum += term;
        if (abs(term) < tolerance) break;

        sign = -sign;
    }

    return sum;
}

// Optimized J_1(x)
template<typename T>
T bessel_j1(T x, unsigned int max_terms = 50, T tolerance = T(1e-12)) {
    using std::abs, std::pow;
    T sum = T(0);
    T sign = T(1);

    for (unsigned int k = 0; k < max_terms; ++k) {
        T numerator = pow(x / T(2), T(2 * k + 1));
        T denominator = factorial<T>(k) * factorial<T>(k + 1);
        T term = sign * numerator / denominator;

        sum += term;
        if (abs(term) < tolerance) break;

        sign = -sign;
    }

    return sum;
}


// Modified Bessel function I_0(x) — series expansion
template<typename T>
T bessel_i0(T x, unsigned int max_terms = 50, T tolerance = T(1e-12)) {
	using std::abs;
    T sum = T(1);
    T term = T(1);
    T x2 = x * x / T(4);

    for (unsigned int k = 1; k < max_terms; ++k) {
        term *= x2 / (T(k) * T(k));
        sum += term;
        if (abs(term) < tolerance) break;
    }

    return sum;
}

// Modified Bessel function I_1(x) — series expansion
template<typename T>
T bessel_i1(T x, unsigned int max_terms = 50, T tolerance = T(1e-12)) {
    using std::abs;
    T sum = x / T(2);
    T term = sum;
    T x2 = x * x / T(4);

    for (unsigned int k = 1; k < max_terms; ++k) {
        term *= x2 / (T(k) * (T(k) + T(1)));
        sum += term;
        if (abs(term) < tolerance) break;
    }

    return sum;
}

// Modified Bessel function K_0(x) — asymptotic approximation
template<typename T>
T bessel_k0(T x) {
    using std::log, std::sqrt, std::exp;
    // Abramowitz & Stegun approximation for x > 0
    if (x <= T(0)) return std::numeric_limits<T>::quiet_NaN();

    T ln_term = -log(x / T(2)) * bessel_i0(x);
    T series = T(0);
    T x2 = x * x;

    // Add correction terms (optional)
    if (x < T(2)) {
        // Series expansion for small x
        T y = x / T(2);
        T y2 = y * y;
        series = -log(y) * bessel_i0(x) + T(0.5772156649);  // Euler-Mascheroni γ
    } else {
        // Asymptotic decay
        series = sqrt(T(3.14159265358979323846) / (T(2) * x)) * exp(-x);
    }

    return series;
}

// Modified Bessel function K_1(x) — asymptotic approximation
template<typename T>
T bessel_k1(T x) {
    using std::log, std::sqrt, std::exp;
    if (x <= T(0)) return std::numeric_limits<T>::quiet_NaN();

    T series = T(0);

    if (x < T(2)) {
        // Series expansion for small x
        T y = x / T(2);
        series = log(y) * bessel_i1(x) + T(1) / x;
    } else {
        // Asymptotic decay
        series = sqrt(T(3.14159265358979323846) / (T(2) * x)) * exp(-x) * (T(1) + T(1) / x);
    }

    return series;
}

// Bessel function Y_0(x) — series expansion
template<typename T>
T bessel_y0(T x, unsigned int max_terms = 50, T tolerance = T(1e-12)) {
	using std::abs, std::pow, std::log;
    if (x <= T(0)) return std::numeric_limits<T>::quiet_NaN();

    T gamma = T(0.5772156649);  // Euler-Mascheroni constant
    T sum = T(0);
    T x2 = x * x / T(4);

    for (unsigned int k = 1; k < max_terms; ++k) {
        T term = pow(x2, T(k)) / (pow(factorial<T>(k), T(2)) * T(k));
        sum += term;
        if (abs(term) < tolerance) break;
    }

    return (T(2) / T(3.14159265358979323846)) * (log(x / T(2)) + gamma) * bessel_j0<T>(x) - sum;
}

// Bessel function Y_1(x) — series expansion
template<typename T>
T bessel_y1(T x, unsigned int max_terms = 50, T tolerance = T(1e-12)) {
	using std::abs, std::pow, std::log;
    if (x <= T(0)) return std::numeric_limits<T>::quiet_NaN();

    T gamma = T(0.5772156649);
    T x2 = x * x / T(4);
    T sum = T(0);

    for (unsigned int k = 0; k < max_terms; ++k) {
        T term = pow(x2, T(k)) / (factorial<T>(k) * factorial<T>(k + 1) * (T(k) + T(1)));
        sum += term;
        if (abs(term) < tolerance) break;
    }

    return (T(2) / T(3.14159265358979323846)) * ((log(x / T(2)) + gamma) * bessel_j1<T>(x) - T(1) / x - sum);
}

// Bessel Y_n(x) via recurrence
template<typename T>
T bessel_y(unsigned int n, T x) {
    if (n == 0) return bessel_y0<T>(x);
    if (n == 1) return bessel_y1<T>(x);

    T y0 = bessel_y0<T>(x);
    T y1 = bessel_y1<T>(x);
    T yn;

    for (unsigned int k = 2; k <= n; ++k) {
        yn = (T(2 * (k - 1)) / x) * y1 - y0;
        y0 = y1;
        y1 = yn;
    }

    return yn;
}

// Modified Bessel I_n(x) via recurrence
template<typename T>
T bessel_i(unsigned int n, T x) {
    if (n == 0) return bessel_i0<T>(x);
    if (n == 1) return bessel_i1<T>(x);

    T i0 = bessel_i0<T>(x);
    T i1 = bessel_i1<T>(x);
    T in;

    for (unsigned int k = 2; k <= n; ++k) {
        in = i0 - (T(2 * (k - 1)) / x) * i1;
        i0 = i1;
        i1 = in;
    }

    return in;
}

// Modified Bessel K_n(x) via recurrence
template<typename T>
T bessel_k(unsigned int n, T x) {
    if (n == 0) return bessel_k0<T>(x);
    if (n == 1) return bessel_k1<T>(x);

    T k0 = bessel_k0<T>(x);
    T k1 = bessel_k1<T>(x);
    T kn;

    for (unsigned int k = 2; k <= n; ++k) {
        kn = k0 + (T(2 * (k - 1)) / x) * k1;
        k0 = k1;
        k1 = kn;
    }

    return kn;
}

}  // namespace sw::math::function
