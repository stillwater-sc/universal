#pragma once
// floatcascade.hpp: implementation of a multi-component floating-point number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <array>
#include <cmath>
#include <iostream>
#include <vector>

// supporting types and functions
#include <universal/native/ieee754.hpp>
#include <universal/numerics/error_free_ops.hpp>
#include <universal/number/shared/nan_encoding.hpp>
#include <universal/number/shared/infinite_encoding.hpp>
#include <universal/number/shared/specific_value_encoding.hpp>

namespace sw::universal {

// floatcascade: The fundamental building block for multi-component Real approximations
template<size_t N>
class floatcascade {
private:
    // Components stored in DECREASING order of magnitude: e[0] >= e[1] >= ... >= e[N-1]  
    // Most significant component at e[0], least significant at e[N-1]
    // Actual value = e[0] + e[1] + ... + e[N-1]
    // evaluation order = e[N-1] + e[N-2] + ... + e[0] to capture any non-trivial tail components
    std::array<double, N> e;

public:
    // Constructors
    constexpr floatcascade() : e{} {
        for (size_t i = 0; i < N; ++i) e[i] = 0.0;
    }
    
    explicit constexpr floatcascade(double x) : e{} {
        e[0] = x;
        for (size_t i = 1; i < N; ++i) e[i] = 0.0;
    }

    // Constructor from array of components
    explicit constexpr floatcascade(const std::array<double, N>& components) : e(components) {}

    // Constructor from smaller cascade (zero-extend)
    template<size_t M>
    explicit floatcascade(const floatcascade<M>& other) : e{} {
        static_assert(M <= N, "Cannot construct larger cascade from smaller one automatically");
        for (size_t i = 0; i < M; ++i) e[i] = other[i];
        for (size_t i = M; i < N; ++i) e[i] = 0.0;
    }

    // Assignment from smaller cascade
    template<size_t M>
    floatcascade& operator=(const floatcascade<M>& other) {
        static_assert(M <= N, "Cannot assign larger cascade to smaller one automatically");
        for (size_t i = 0; i < M; ++i) e[i] = other[i];
        for (size_t i = M; i < N; ++i) e[i] = 0.0;
        return *this;
    }

    // Component access
    constexpr double operator[](size_t i) const noexcept { return e[i]; }
    constexpr double& operator[](size_t i) { return e[i]; }
    
    // modifiers
    constexpr void clear() {
        for (size_t i = 0; i < N; ++i) e[i] = 0.0;
    }
    constexpr void set(double x) {
        e[0] = x;
        for (size_t i = 1; i < N; ++i) e[i] = 0.0;
    }
    constexpr void set(const std::array<double, N>& components) {
        e = components;
	}

    // selectors
    constexpr size_t size() const noexcept { return N; }
    const std::array<double, N>& data() const noexcept { return e; }
    std::array<double, N>& data() { return e; }

    constexpr bool iszero()   const noexcept { return testFirstComponent(0.0); }
    constexpr bool isone()    const noexcept { return testFirstComponent(1.0); }
	constexpr bool ispos()    const noexcept { return !std::signbit(e[0]); } // std::signbit deals with inf and NaN correctly
    constexpr bool isneg()    const noexcept { return std::signbit(e[0]); }
    constexpr bool isnan(int NaNType = NAN_TYPE_EITHER)  const noexcept {
        bool negative = isneg();
        int nan_type;
        bool isNaN = checkNaN(e[0], nan_type);
        bool isNegNaN = isNaN && negative;
        bool isPosNaN = isNaN && !negative;
        return (NaNType == NAN_TYPE_EITHER ? (isNegNaN || isPosNaN) :
            (NaNType == NAN_TYPE_SIGNALLING ? isNegNaN :
                (NaNType == NAN_TYPE_QUIET ? isPosNaN : false)));
    }
    constexpr bool isinf(int InfType = INF_TYPE_EITHER)  const noexcept {
        bool negative = isneg();
        int inf_type;
        bool isInf = checkInf(e[0], inf_type);
        bool isNegInf = isInf && negative;
        bool isPosInf = isInf && !negative;
        return (InfType == INF_TYPE_EITHER ? (isNegInf || isPosInf) :
            (InfType == INF_TYPE_NEGATIVE ? isNegInf :
                (InfType == INF_TYPE_POSITIVE ? isPosInf : false)));
	}  



    // Conversion to double (estimate)
    constexpr double to_double() const {
        double sum{ 0 };
        if constexpr (1 == N) {
            sum = e[0];
        }
        else if constexpr (2 == N) {
            sum = e[0] + e[1];
        }
        else if constexpr (3 == N) {
            sum  = e[2] + e[1];
            sum += e[0];
        }
        else if constexpr (4 == N) {
            double l = e[3] + e[2];
            double r = e[1] + e[0];
			sum = l + r;
        }
        else if constexpr (5 == N) {
            double l = e[4] + e[3] + e[2];
            double r = e[1] + e[0];
            sum = l + r;
        }
        else if constexpr (6 == N) {
            double p1 = e[5] + e[4];
            double p2 = e[3] + e[2];
			p2 += p1;
            p1 = e[1] + e[0];
            sum = p1 + p2;
        }
        else {
            // general case
			sum = e[N - 1] + e[N - 2];
            for (size_t i = 2; i < N; ++i) {
                sum += e[N - 1 - i];
            }
        }
        return sum;
    }

    // Basic properties

    constexpr int sign() const noexcept {
		return (e[0] > 0.0) ? 1 : ((e[0] < 0.0) ? -1 : 0);
    }



    // Debug output
	template<size_t M>
    friend std::string to_tuple(const floatcascade<M>& fc);

    friend std::ostream& operator<<(std::ostream& os, const floatcascade& fc) {
        os << "floatcascade<" << N << ">[";
        for (size_t i = 0; i < N; ++i) {
            if (i > 0) os << ", ";
            os << fc.e[i];
        }
        os << "] ~ " << fc.to_double();
        return os;
    }

private:

    constexpr bool testFirstComponent(double v) const noexcept {
        if constexpr (2 == N) {
            return e[0] == v && e[1] == 0.0;
        }
        else if constexpr (3 == N) {
            return e[0] == v && e[1] == 0.0 && e[2] == 0.0;
        }
        else if constexpr (4 == N) {
            return e[0] == v && e[1] == 0.0 && e[2] == 0.0 && e[3] == 0.0;
        }
        else if constexpr (5 == N) {
            return e[0] == v && e[1] == 0.0 && e[2] == 0.0 && e[3] == 0.0 && e[4] == 0.0;
        }
        else if constexpr (6 == N) {
            return e[0] == v && e[1] == 0.0 && e[2] == 0.0 && e[3] == 0.0 && e[4] == 0.0 && e[5] == 0.0;
        }
        else {
            // general case
            if (e[0] != v) return false;
            for (size_t i = 1; i < N; ++i) {
                if (e[i] != 0.0) return false;
            }
            return true;
        }
    }

};

template<size_t N>
std::string to_tuple(const floatcascade<N>& fc) {
    std::stringstream ss;
	ss << std::scientific;
	//ss.setprecision(17); // max precision of double
    ss << "{ ";
    for (size_t i = 0; i < N; ++i) {
        if (i > 0) ss << ", ";
        ss << fc.e[i];
    }
    ss << '}';
    return ss.str();
}

template<size_t N>
std::string to_scientific(const floatcascade<N>& fc,
    int precision = N * 17,
    bool showpos = false,
    bool uppercase = false,
    bool trailing_zeros = true) {
    // precondition: fc is a normalized floatcascade

    // Handle special cases early
    if (fc.isnan(NAN_TYPE_QUIET)) return std::string("qNaN");
    if (fc.isnan(NAN_TYPE_SIGNALLING)) return std::string("sNaN");
    if (fc.isinf(INF_TYPE_POSITIVE)) return std::string("Inf");
    if (fc.isinf(INF_TYPE_NEGATIVE)) return std::string("-Inf");
    if (fc.iszero()) return std::string(showpos ? "+0.0e+0" : "0.0e+0");

    // Step 1: Estimate exponent from the most significant non-zero component
    double hi = fc[0];
    int exp10 = static_cast<int>(std::floor(std::log10(std::abs(hi))));
    if (!std::isfinite(exp10)) exp10 = 0; // handle log10(0) = -inf case
    double scale = std::pow(10.0, -exp10);

	// Step 2: Scale all components so we are in the range [0.0, 10.0)
    double scaled[N];
    for (size_t i = 0; i < N; ++i) {
        scaled[i] = fc[i] * scale;
    }
    double acc = 0.0;
    for (int i = 0; i < 3; ++i) acc += scaled[i];
	bool negative = std::signbit(acc);
	acc = std::abs(acc);

    // Step 3: Generate digits iteratively
    std::string digits;
	digits.reserve(static_cast<size_t>(precision) + 2); // leading digit + precision digits

    for (int i = 0; i <= precision; ++i) {
        int digit = static_cast<int>(acc);
		if (digit > 9) digit = 9; // defensive clamp
        digits.push_back(static_cast<char>('0' + digit));
        acc = (acc - digit) * 10.0;
    }

	// Step 4: Round last digit and propagate carry (with exponent adjustment)
    if (acc >= 5.0) {
		int i = precision;
        for (; i >= 0 && digits[i] == '9'; --i) {
			digits[i] = '0';
        }
        if (i >= 0) {
            digits[i] += 1; // increment this digit
        }
        else {
			// overflowed the leading digit: 9.99... 9 -> 10.0...0
            digits.insert(digits.begin(), '1');
            exp10 += 1;
		}
    }

    // Step 5: Format string
    std::string result;
    if (negative) result += '-';
    else if (showpos) result += '+';

    result += digits[0]; // leading digit
    result += '.';
    if (precision > 0) {
        result.append(digits.begin() + 1, digits.begin() + 1 + precision);
        if (trailing_zeros) {
            // digits already has length precision + 1
        }
        else {
			// trim trailing zeros after decimal point
            while (!result.empty() && result.back() == '0') {
                result.pop_back();
			}
            if (!result.empty() && result.back() == '.') {
                result.pop_back(); // remove decimal point if no digits follow
			}
        }
    }
    else {
		result += '0'; // no precision requested, just add a zero
    }

    result += uppercase ? "E" : "e";
    result += (exp10 >= 0 ? "+" : "-");
    result += std::to_string(std::abs(exp10));

    return result;
}


// Core expansion operations - the "engine" for all cascade operations
namespace expansion_ops {

    // Knuth's TWO-SUM: computes a + b = x + y exactly
    // Uses volatile to prevent compiler optimizations that break error-free transformation
    inline void two_sum(double a, double b, double& x, double& y) {
        volatile double vx = a + b;
        x = vx;
        volatile double b_virtual = vx - a;
        volatile double a_virtual = vx - b_virtual;
        volatile double b_roundoff = b - b_virtual;
        volatile double a_roundoff = a - a_virtual;
        volatile double vy = a_roundoff + b_roundoff;
        y = vy;
    }

    // Dekker's FAST-TWO-SUM: assumes |a| >= |b|
    // Uses volatile to prevent compiler optimizations that break error-free transformation
    inline void fast_two_sum(double a, double b, double& x, double& y) {
        volatile double vx = a + b;
        x = vx;
        volatile double vy = b - (vx - a);
        y = vy;
    }

    // Add single double to N-component cascade, result in (N+1)-component cascade
    // Uses volatile to prevent compiler optimizations that break error-free transformation
    template<size_t N>
    floatcascade<N+1> grow_expansion(const floatcascade<N>& e, double b) {
        floatcascade<N+1> result;
        volatile double q = b;
        volatile double h;

        // Process from least significant (end) to most significant (beginning)
        for (size_t i = N; i-- > 0; ) {
            two_sum(q, e[i], q, h);
            result[i + 1] = h;  // shift components right
        }
        result[0] = q;  // most significant component at [0]

        return result;
    }

    // Add two cascades of same size, result in double-size cascade
    template<size_t N>
    floatcascade<2 * N> add_cascades(const floatcascade<N>& a, const floatcascade<N>& b) {
        // Merge the two N-component cascades
        std::array<double, 2 * N> merged;
        std::array<double, 2 * N> magnitudes;

        // Collect all components and their magnitudes
        for (size_t i = 0; i < N; ++i) {
            merged[2 * i] = a[i];
            merged[2 * i + 1] = b[i];
            magnitudes[2 * i] = std::abs(a[i]);
            magnitudes[2 * i + 1] = std::abs(b[i]);
        }

        // Sort by magnitude - LARGEST FIRST for decreasing order
        for (size_t i = 0; i < 2 * N - 1; ++i) {
            for (size_t j = 0; j < 2 * N - 1 - i; ++j) {
                if (magnitudes[j] < magnitudes[j + 1]) {  // Changed: < instead of >
                    std::swap(merged[j], merged[j + 1]);
                    std::swap(magnitudes[j], magnitudes[j + 1]);
                }
            }
        }

        // Accumulate from smallest to largest (reverse order of sorted array)
        // Uses volatile to prevent compiler optimizations that break error-free transformation
        floatcascade<2 * N> result;
        volatile double sum = 0.0;
        std::vector<double> corrections;

        // Process from end (smallest) to beginning (largest)
        for (int i = 2 * N - 1; i >= 0; --i) {  // Changed: reverse iteration
            volatile double new_sum, error;
            two_sum(sum, merged[i], new_sum, error);

            if (error != 0.0) {
                corrections.push_back(error);
            }
            sum = new_sum;
        }

        // Store most significant component first
        result[0] = sum;

        // Store corrections in decreasing order of significance
        for (size_t i = 0; i < std::min(corrections.size(), size_t(2 * N - 1)); ++i) {
            result[i + 1] = corrections[corrections.size() - 1 - i];  // Reverse order
        }

        return result;
    }

    // Compress cascade to remove unnecessary precision
    template<size_t N>
    floatcascade<N> compress(const floatcascade<N>& e) {
        // Simple compression - could be much more sophisticated
        floatcascade<N> result = e;

        // Remove tiny components that don't contribute significantly
        double threshold = std::abs(result.to_double()) * 1e-16;
        for (size_t i = 0; i < N; ++i) {
            if (std::abs(result[i]) < threshold) {
                result[i] = 0.0;
            }
        }

        return result;
    }

    // Priest's TWO-PROD: computes a * b = x + y exactly
    // Uses volatile to prevent compiler optimizations that break error-free transformation
    inline void two_prod(double a, double b, double& x, double& y) {
        volatile double vx = a * b;
        x = vx;
        // Use FMA if available for exact error
        volatile double vy = std::fma(a, b, -vx);
        y = vy;
    }

    // THREE-SUM: sum three doubles, accumulate errors
    // Uses volatile to prevent compiler optimizations that break error-free transformation
    inline void three_sum(double& a, double& b, double& c) {
        volatile double t1, t2, t3;
        two_sum(a, b, t1, t2);
        two_sum(t1, c, a, t3);
        two_sum(t2, t3, b, c);
    }

    // THREE-SUM2: variant that doesn't reorder inputs
    // Uses volatile to prevent compiler optimizations that break error-free transformation
    inline void three_sum2(double a, double b, double c, double& x, double& y, double& z) {
        volatile double t1, t2, t3;
        two_sum(a, b, t1, t2);
        two_sum(t1, c, x, t3);
        two_sum(t2, t3, y, z);
    }

    // Renormalize N components to maintain non-overlapping property
    // Uses volatile to prevent compiler optimizations that break error-free transformation
    template<size_t N>
    floatcascade<N> renormalize(const floatcascade<N>& e) {
        floatcascade<N> result;
        volatile double s = e[N-1];

        // Accumulate from least significant to most significant
        for (int i = N - 2; i >= 0; --i) {
            volatile double hi, lo;
            two_sum(s, e[i], hi, lo);
            result[i+1] = lo;
            s = hi;
        }
        result[0] = s;

        return result;
    }

    // Multiply two N-component cascades
    template<size_t N>
    floatcascade<N> multiply_cascades(const floatcascade<N>& a, const floatcascade<N>& b) {
        // Generate N^2 products (partial products matrix)
        std::array<double, N * N> products;
        std::array<double, N * N> errors;

        // Compute all products with their errors
        for (size_t i = 0; i < N; ++i) {
            for (size_t j = 0; j < N; ++j) {
                two_prod(a[i], b[j], products[i * N + j], errors[i * N + j]);
            }
        }

        // Accumulate products by significance level
        // Level 0: products[0,0]
        // Level 1: products[0,1], products[1,0], errors[0,0]
        // Level 2: products[0,2], products[1,1], products[2,0], errors[0,1], errors[1,0]
        // etc.

        floatcascade<N> result;

        // Start with most significant product
        result[0] = products[0];

        if constexpr (N >= 2) {
            // Level 1: accumulate cross products and error from level 0
            double level1_sum = products[1] + products[N] + errors[0];
            double level1_err;
            two_sum(result[0], level1_sum, result[0], level1_err);
            result[1] = level1_err;
        }

        if constexpr (N >= 3) {
            // Level 2: more cross products
            double level2_sum = products[2] + products[N+1] + products[2*N] + errors[1] + errors[N];
            double level2_err;
            two_sum(result[1], level2_sum, result[1], level2_err);
            result[2] = level2_err;

            // Accumulate remaining terms into last component
            for (size_t i = 3; i < N * N; ++i) {
                if (i < N || i % N < N - 1) { // Not already counted
                    result[2] += products[i] + errors[i];
                }
            }
        }

        // Renormalize to maintain non-overlapping property
        return renormalize(result);
    }

} // namespace expansion_ops

} // namespace sw::universal
