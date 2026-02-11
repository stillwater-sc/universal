#pragma once
// floatcascade.hpp: implementation of a multi-component floating-point number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <algorithm>
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

    constexpr int scale() const noexcept {
        return sw::universal::scale(e[0]); 
    }

    // Decimal conversion methods
    // Declaration only - implementations after expansion_ops namespace
    void to_digits(std::vector<char>& s, int& exponent, int precision) const;

    std::string to_string(
        std::streamsize precision = 7,
        std::streamsize width = 15,
        bool fixed = false,
        bool scientific = true,
        bool internal = false,
        bool left = false,
        bool showpos = false,
        bool uppercase = false,
        char fill = ' '
    ) const;

    // Parse a decimal string into this floatcascade
    // Returns true on success, false on parse error
    bool parse(const std::string& str);

    // Arithmetic operators (implementations after expansion_ops namespace)
    floatcascade& operator+=(const floatcascade& rhs) noexcept;
    floatcascade& operator-=(const floatcascade& rhs) noexcept;
    floatcascade& operator*=(const floatcascade& rhs) noexcept;
    floatcascade& operator/=(const floatcascade& rhs) noexcept;

    // Arithmetic operators with double
    floatcascade& operator+=(double rhs) noexcept { return *this += floatcascade(rhs); }
    floatcascade& operator-=(double rhs) noexcept { return *this -= floatcascade(rhs); }
    floatcascade& operator*=(double rhs) noexcept { return *this *= floatcascade(rhs); }
    floatcascade& operator/=(double rhs) noexcept { return *this /= floatcascade(rhs); }

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

    // Helper methods for to_digits

    void round_string(std::vector<char>& s, int precision, int* decimalPoint) const {
        int nrDigits = precision;
        // round decimal string and propagate carry
        int lastDigit = nrDigits - 1;
        if (s[static_cast<unsigned>(lastDigit)] >= '5') {
            int i = nrDigits - 2;
            s[static_cast<unsigned>(i)]++;
            while (i > 0 && s[static_cast<unsigned>(i)] > '9') {
                s[static_cast<unsigned>(i)] -= 10;
                s[static_cast<unsigned>(--i)]++;
            }
        }

        // if first digit is 10, shift everything
        if (s[0] > '9') {
            for (int i = precision; i >= 2; --i)
                s[static_cast<unsigned>(i)] = s[static_cast<unsigned>(i - 1)];
            s[0u] = '1';
            s[1u] = '0';

            (*decimalPoint)++;  // increment decimal point
            ++precision;
        }
    }

    void append_exponent(std::string& str, int exp) const {
        str += (exp < 0 ? '-' : '+');
        exp = std::abs(exp);
        int k;
        if (exp >= 100) {
            k = (exp / 100);
            str += static_cast<char>('0' + k);
            exp -= 100 * k;
        }

        k = (exp / 10);
        str += static_cast<char>('0' + k);
        exp -= 10 * k;

        str += static_cast<char>('0' + exp);
    }

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
    for (size_t i = 0; (i < N) && (i < 3); ++i) acc += scaled[i];
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
    // Volatiles are handled inside two_sum, so locals don't need to be volatile
    template<size_t N>
    floatcascade<N+1> grow_expansion(const floatcascade<N>& e, double b) {
        floatcascade<N+1> result;
        double q = b;
        double h;

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
        // Volatiles are handled inside two_sum, so locals don't need to be volatile
        floatcascade<2 * N> result;
        double sum = 0.0;
        std::vector<double> corrections;

        // Process from end (smallest) to beginning (largest)
        for (int i = 2 * N - 1; i >= 0; --i) {  // Changed: reverse iteration
            double new_sum, error;
            two_sum(sum, merged[static_cast<size_t>(i)], new_sum, error);

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
    // Volatiles are handled inside two_sum, so temporaries don't need to be volatile
    inline void three_sum(double& a, double& b, double& c) {
        double t1, t2, t3;
        two_sum(a, b, t1, t2);
        two_sum(t1, c, a, t3);
        two_sum(t2, t3, b, c);
    }

    // THREE-SUM2: variant that doesn't reorder inputs
    // Volatiles are handled inside two_sum, so temporaries don't need to be volatile
    inline void three_sum2(double a, double b, double c, double& x, double& y, double& z) {
        double t1, t2, t3;
        two_sum(a, b, t1, t2);
        two_sum(t1, c, x, t3);
        two_sum(t2, t3, y, z);
    }

    // Renormalize N components to maintain non-overlapping property
    // Improved two-phase algorithm based on Hida-Li-Bailey QD library
    // Volatiles are handled inside quick_two_sum, so locals don't need to be volatile
    //
    // Phase 1: Compression - bottom-up accumulation using quick_two_sum
    // Phase 2: Conditional refinement - carry propagation with zero detection
    //
    // This ensures the non-overlapping property: |component[i+1]| ≤ ulp(component[i])/2
    // Fixes precision loss in iterative algorithms (e.g., pow() improved from 77-92 bits to 200+ bits)

    // Generic version for arbitrary N
    template<size_t N>
    floatcascade<N> renormalize(const floatcascade<N>& e) {
        floatcascade<N> result = e;
        if (std::isinf(result[0])) return result;

        // Phase 1: Compression
        std::array<double, N> s;
        volatile double sum = result[N-1];
        for (int i = static_cast<int>(N) - 2; i >= 0; --i) {
            sum = quick_two_sum(result[i], sum, s[i+1]);
        }
        s[0] = sum;

        // Phase 2: Simple linear propagation for arbitrary N
        for (size_t i = 0; i < N-1; ++i) {
            result[i] = quick_two_sum(s[i], s[i+1], result[i+1]);
        }
        result[N-1] = s[N-1];

        return result;
    }

    // Specialization for N=2 (double-double)
    template<>
    inline floatcascade<2> renormalize<2>(const floatcascade<2>& e) {
        floatcascade<2> result = e;
        if (std::isinf(result[0])) return result;

        result[0] = quick_two_sum(result[0], result[1], result[1]);
        return result;
    }

    // Specialization for N=3 (triple-double)
    template<>
    inline floatcascade<3> renormalize<3>(const floatcascade<3>& e) {
        floatcascade<3> result = e;
        if (std::isinf(result[0])) return result;

        double s0, s1, s2 = 0.0;

        // Phase 1: Compression
        s0 = quick_two_sum(result[1], result[2], result[2]);
        result[0] = quick_two_sum(result[0], s0, result[1]);

        // Phase 2: Conditional refinement
        s0 = result[0];
        s1 = result[1];

        if (s1 != 0.0) {
            s1 = quick_two_sum(s1, result[2], s2);
        } else {
            s0 = quick_two_sum(s0, result[2], s1);
        }

        result[0] = s0;
        result[1] = s1;
        result[2] = s2;
        return result;
    }

    // Specialization for N=4 (quad-double) - matches QD library exactly
    template<>
    inline floatcascade<4> renormalize<4>(const floatcascade<4>& e) {
        floatcascade<4> result = e;
        if (std::isinf(result[0])) return result;

        double s0, s1, s2 = 0.0, s3 = 0.0;

        // Phase 1: Compression
        s0 = quick_two_sum(result[2], result[3], result[3]);
        s0 = quick_two_sum(result[1], s0, result[2]);
        result[0] = quick_two_sum(result[0], s0, result[1]);

        // Phase 2: Conditional refinement (matches QD library algorithm exactly)
        s0 = result[0];
        s1 = result[1];

        if (s1 != 0.0) {
            s1 = quick_two_sum(s1, result[2], s2);
            if (s2 != 0.0)
                s2 = quick_two_sum(s2, result[3], s3);
            else
                s1 = quick_two_sum(s1, result[3], s2);
        } else {
            s0 = quick_two_sum(s0, result[2], s1);
            if (s1 != 0.0)
                s1 = quick_two_sum(s1, result[3], s2);
            else
                s0 = quick_two_sum(s0, result[3], s1);
        }

        result[0] = s0;
        result[1] = s1;
        result[2] = s2;
        result[3] = s3;
        return result;
    }

    /*
     * EXPANSION COMPRESSION FUNCTIONS
     * ================================
     *
     * Why Naive Compression Fails
     * ---------------------------
     * When adding two N-component cascades, add_cascades() produces a 2N-component expansion
     * that must be compressed back to N components. The naive approach:
     *
     *   compressed[last] = result[N-1] + result[N] + result[N+1] + ... + result[2N-1]
     *
     * is FUNDAMENTALLY BROKEN because it uses floating-point addition without capturing
     * rounding errors. Each '+' operation loses precision in the tail bits.
     *
     * Testing Insights
     * ----------------
     * The bug manifests most severely in the identity test: (a+b)-a = b
     *
     * For qd_cascade (8→4 compression):
     *   - Naive sum: result[3] + result[4] + result[5] + result[6] + result[7]
     *   - Loses cumulative rounding errors across 4 additions
     *   - Test FAILED: recovered b had wrong sign and magnitude in component[3]
     *   - Error: -1.5e-51 instead of 5e-52 (212-bit precision destroyed)
     *
     * For td_cascade (6→3 compression):
     *   - Naive sum: result[2] + result[3] + result[4] + result[5]
     *   - Loses cumulative rounding errors across 3 additions
     *   - Test PASSES but only because renormalize() afterward salvages precision
     *   - Still incorrect in principle - relies on post-processing to fix errors
     *
     * For dd_cascade (4→2 compression):
     *   - Naive sum: result[1] + result[2] + result[3]
     *   - Loses cumulative rounding errors across 2 additions
     *   - Test PASSES but same caveat as td_cascade
     *
     * The Proven Solution: Error-Free Compression
     * --------------------------------------------
     * Based on Hida, Li, Bailey "Library for Double-Double and Quad-Double Arithmetic"
     *
     * Two-phase algorithm:
     * 1. Bottom-up accumulation: Use fast_two_sum to capture all rounding errors
     * 2. Conditional extraction: Dynamically extract non-overlapping components
     *
     * Key insight: fast_two_sum(a, b, sum, error) guarantees a + b = sum + error EXACTLY
     * By chaining these operations and carefully managing where errors flow, we maintain
     * the non-overlapping property required for multi-component arithmetic.
     *
     * Why conditional extraction? Components may cancel to zero during accumulation.
     * The algorithm dynamically shifts remaining precision into available slots, ensuring
     * we extract the N most significant non-overlapping components.
     */

    // Compress 4-component cascade to 2 components following proven QD algorithm
    // Used by dd_cascade for (2+2)→2 compression after addition/subtraction
    inline floatcascade<2> compress_4to2(const floatcascade<4>& e) {
        double r0 = e[0];
        double r1 = e[1];
        double r2 = e[2];
        double r3 = e[3];

        double s0, s1, s2;

        // Phase 1: Bottom-up accumulation using fast_two_sum
        fast_two_sum(r2, r3, s0, r3);  // s0 = r2+r3, error->r3
        fast_two_sum(r1, s0, s0, r2);  // s0 = r1+s0, error->r2
        fast_two_sum(r0, s0, r0, r1);  // r0 = r0+s0, error->r1

        // Phase 2: Extract 2 non-overlapping components with conditional logic
        fast_two_sum(r0, r1, s0, s1);
        if (s1 != 0.0) {
            fast_two_sum(s1, r2, s1, s2);
            if (s2 != 0.0)
                s2 += r3;  // Final residual absorbed
            else
                s1 += r3;
        } else {
            fast_two_sum(s0, r2, s0, s1);
            if (s1 != 0.0)
                s1 += r3;
            else
                s0 += r3;
        }

        floatcascade<2> result;
        result[0] = s0;
        result[1] = s1;
        return result;
    }

    // Compress 6-component cascade to 3 components following proven QD algorithm
    // Used by td_cascade for (3+3)→3 compression after addition/subtraction
    inline floatcascade<3> compress_6to3(const floatcascade<6>& e) {
        double r0 = e[0];
        double r1 = e[1];
        double r2 = e[2];
        double r3 = e[3];
        double r4 = e[4];
        double r5 = e[5];

        double s0, s1, s2 = 0.0, s3;

        // Phase 1: Bottom-up accumulation using fast_two_sum
        fast_two_sum(r4, r5, s0, r5);  // s0 = r4+r5, error->r5
        fast_two_sum(r3, s0, s0, r4);  // s0 = r3+s0, error->r4
        fast_two_sum(r2, s0, s0, r3);  // s0 = r2+s0, error->r3
        fast_two_sum(r1, s0, s0, r2);  // s0 = r1+s0, error->r2
        fast_two_sum(r0, s0, r0, r1);  // r0 = r0+s0, error->r1

        // Phase 2: Extract 3 non-overlapping components with conditional logic
        fast_two_sum(r0, r1, s0, s1);
        if (s1 != 0.0) {
            fast_two_sum(s1, r2, s1, s2);
            if (s2 != 0.0) {
                fast_two_sum(s2, r3, s2, s3);
                if (s3 != 0.0)
                    s3 += r4 + r5;  // Final residual absorbed
                else
                    s2 += r4 + r5;
            } else {
                fast_two_sum(s1, r3, s1, s2);
                if (s2 != 0.0)
                    s2 += r4 + r5;
                else
                    s1 += r4 + r5;
            }
        } else {
            fast_two_sum(s0, r2, s0, s1);
            if (s1 != 0.0) {
                fast_two_sum(s1, r3, s1, s2);
                if (s2 != 0.0)
                    s2 += r4 + r5;
                else
                    s1 += r4 + r5;
            } else {
                fast_two_sum(s0, r3, s0, s1);
                if (s1 != 0.0)
                    s1 += r4 + r5;
                else
                    s0 += r4 + r5;
            }
        }

        floatcascade<3> result;
        result[0] = s0;
        result[1] = s1;
        result[2] = s2;
        return result;
    }

    // Compress 8-component cascade to 4 components following proven QD algorithm
    // Used by qd_cascade for (4+4)→4 compression after addition/subtraction
    // This implements the same algorithm as sw::universal::renorm(a0,a1,a2,a3,a4,...,a7)
    // Based on: Hida, Li, Bailey "Library for Double-Double and Quad-Double Arithmetic"
    inline floatcascade<4> compress_8to4(const floatcascade<8>& e) {
        // Note: Volatiles are used inside two_sum/fast_two_sum, so we don't need them here
        double r0 = e[0];
        double r1 = e[1];
        double r2 = e[2];
        double r3 = e[3];
        double r4 = e[4];
        double r5 = e[5];
        double r6 = e[6];
        double r7 = e[7];

        double s0, s1, s2 = 0.0, s3 = 0.0, s4;

        // Phase 1: Bottom-up accumulation using fast_two_sum
        // Following proven QD algorithm: accumulate from least to most significant
        // Pattern: s0 = sum(a[i], s0), error goes back to a[i]
        fast_two_sum(r6, r7, s0, r7);  // s0 = r6+r7, error->r7
        fast_two_sum(r5, s0, s0, r6);  // s0 = r5+s0, error->r6
        fast_two_sum(r4, s0, s0, r5);  // s0 = r4+s0, error->r5
        fast_two_sum(r3, s0, s0, r4);  // s0 = r3+s0, error->r4
        fast_two_sum(r2, s0, s0, r3);  // s0 = r2+s0, error->r3
        fast_two_sum(r1, s0, s0, r2);  // s0 = r1+s0, error->r2
        fast_two_sum(r0, s0, r0, r1);  // r0 = r0+s0, error->r1

        // Now we have redistributed: r0 (most sig), r1, r2, r3, r4, r5, r6

        // Phase 2: Extract 4 non-overlapping components with conditional logic
        // This is the proven algorithm from QD library
        s0 = r0;
        s1 = r1;

        fast_two_sum(r0, r1, s0, s1);
        if (s1 != 0.0) {
            fast_two_sum(s1, r2, s1, s2);
            if (s2 != 0.0) {
                fast_two_sum(s2, r3, s2, s3);
                if (s3 != 0.0) {
                    fast_two_sum(s3, r4, s3, s4);
                    if (s4 != 0.0)
                        s4 += r5 + r6;  // Final residual absorbed (unavoidable precision loss)
                    else
                        s3 += r5 + r6;
                } else {
                    fast_two_sum(s2, r4, s2, s3);
                    if (s3 != 0.0)
                        s3 += r5 + r6;
                    else
                        s2 += r5 + r6;
                }
            } else {
                fast_two_sum(s1, r3, s1, s2);
                if (s2 != 0.0) {
                    fast_two_sum(s2, r4, s2, s3);
                    if (s3 != 0.0)
                        s3 += r5 + r6;
                    else
                        s2 += r5 + r6;
                } else {
                    fast_two_sum(s1, r4, s1, s2);
                    if (s2 != 0.0)
                        s2 += r5 + r6;
                    else
                        s1 += r5 + r6;
                }
            }
        } else {
            fast_two_sum(s0, r2, s0, s1);
            if (s1 != 0.0) {
                fast_two_sum(s1, r3, s1, s2);
                if (s2 != 0.0) {
                    fast_two_sum(s2, r4, s2, s3);
                    if (s3 != 0.0)
                        s3 += r5 + r6;
                    else
                        s2 += r5 + r6;
                } else {
                    fast_two_sum(s1, r4, s1, s2);
                    if (s2 != 0.0)
                        s2 += r5 + r6;
                    else
                        s1 += r5 + r6;
                }
            } else {
                fast_two_sum(s0, r3, s0, s1);
                if (s1 != 0.0) {
                    fast_two_sum(s1, r4, s1, s2);
                    if (s2 != 0.0)
                        s2 += r5 + r6;
                    else
                        s1 += r5 + r6;
                } else {
                    fast_two_sum(s0, r4, s0, s1);
                    if (s1 != 0.0)
                        s1 += r5 + r6;
                    else
                        s0 += r5 + r6;
                }
            }
        }

        floatcascade<4> result;
        result[0] = s0;
        result[1] = s1;
        result[2] = s2;
        result[3] = s3;

        return result;
    }

    // Multiply two N-component cascades using diagonal partitioning
    template<size_t N>
    floatcascade<N> multiply_cascades(const floatcascade<N>& a, const floatcascade<N>& b) {
        // Generate N×N products (partial products matrix)
        std::array<double, N * N> products;
        std::array<double, N * N> errors;

        // Compute all products with their errors using two_prod
        for (size_t i = 0; i < N; ++i) {
            for (size_t j = 0; j < N; ++j) {
                two_prod(a[i], b[j], products[i * N + j], errors[i * N + j]);
            }
        }

        // Diagonal partitioning: diagonal k contains all products[i*N+j] where i+j == k
        // Diagonals represent significance levels: smaller k = more significant
        // We have 2*N-1 diagonals total (k = 0 to 2*N-2)

        // Accumulate each diagonal separately using stable two_sum chains
        // Each diagonal produces a sum and carries errors to the next diagonal
        std::array<double, 2 * N - 1> diagonal_sums;
        std::array<double, 2 * N - 1> diagonal_errors;

        // Initialize all to zero
        for (size_t k = 0; k < 2 * N - 1; ++k) {
            diagonal_sums[k] = 0.0;
            diagonal_errors[k] = 0.0;
        }

        // Process each diagonal: accumulate products and errors from previous diagonal
        for (size_t diag = 0; diag < 2 * N - 1; ++diag) {
            // Collect all terms for this diagonal
            std::vector<double> terms;

            // Add products where i+j == diag
            for (size_t i = 0; i <= diag && i < N; ++i) {
                size_t j = diag - i;
                if (j < N) {
                    terms.push_back(products[i * N + j]);
                }
            }

            // Add errors from previous diagonal (errors[i*N+j] where i+j == diag-1)
            if (diag > 0) {
                for (size_t i = 0; i <= diag - 1 && i < N; ++i) {
                    size_t j = diag - 1 - i;
                    if (j < N) {
                        terms.push_back(errors[i * N + j]);
                    }
                }
            }

            // Accumulate all terms using stable two_sum chain
            if (!terms.empty()) {
                double sum = terms[0];
                double accumulated_error = 0.0;

                for (size_t t = 1; t < terms.size(); ++t) {
                    double new_sum, err;
                    two_sum(sum, terms[t], new_sum, err);
                    sum = new_sum;
                    // Accumulate errors for propagation to next diagonal
                    double err_sum, err_err;
                    two_sum(accumulated_error, err, err_sum, err_err);
                    accumulated_error = err_sum;
                    // Higher-order errors go to next diagonal
                    if (diag + 1 < 2 * N - 1) {
                        diagonal_errors[diag + 1] += err_err;
                    }
                }

                diagonal_sums[diag] = sum;
                diagonal_errors[diag] = accumulated_error;
            }
        }

        // Extract top N components by merging diagonals from most to least significant
        // Build complete expansion including both diagonal sums and errors
        std::vector<double> expansion;
        expansion.reserve(2 * (2 * N - 1));

        for (size_t k = 0; k < 2 * N - 1; ++k) {
            if (std::abs(diagonal_sums[k]) > 0.0) {
                expansion.push_back(diagonal_sums[k]);
            }
            if (std::abs(diagonal_errors[k]) > 0.0) {
                expansion.push_back(diagonal_errors[k]);
            }
        }

        // Sort by decreasing absolute magnitude to get most significant terms first
        std::sort(expansion.begin(), expansion.end(),
                  [](double a, double b) { return std::abs(a) > std::abs(b); });

        // Now accumulate into result using proper two_sum chains to maintain precision
        floatcascade<N> result;

        // Initialize all components to zero explicitly
        for (size_t i = 0; i < N; ++i) {
            result[i] = 0.0;
        }

        // Accumulate expansion terms into result components
        if (!expansion.empty()) {
            result[0] = expansion[0];  // Most significant term

            // Accumulate remaining terms using two_sum cascade
            for (size_t i = 1; i < expansion.size(); ++i) {
                double carry = expansion[i];

                // Try to add carry into result components, propagating errors down
                for (size_t j = 0; j < N && std::abs(carry) > 0.0; ++j) {
                    double sum, err;
                    two_sum(result[j], carry, sum, err);
                    result[j] = sum;
                    carry = err;  // Error propagates to next component
                }

                // CRITICAL: If carry is still non-zero after exhausting all N components,
                // fold it into the least significant component to avoid silent data loss
                if (std::abs(carry) > 0.0) {
                    double sum, err;
                    two_sum(result[N-1], carry, sum, err);
                    result[N-1] = sum;
                    // Remaining err is truly sub-ULP for N components and can be safely discarded
                    // (it represents precision beyond what N floats can represent)
                }
            }
        }

        // Renormalize to maintain non-overlapping property
        return renormalize(result);
    }

} // namespace expansion_ops

///////////////////////////////////////////////////////////////////////////////
// Decimal conversion implementation (defined after expansion_ops)

template<size_t N>
void floatcascade<N>::to_digits(std::vector<char>& s, int& exponent, int precision) const {
    constexpr floatcascade<N> _one(1.0), _ten(10.0);
    constexpr double _log2(0.301029995663981);

    if (iszero()) {
        exponent = 0;
        for (int i = 0; i < precision; ++i)
            s[static_cast<unsigned>(i)] = '0';
        return;
    }

    // First determine the (approximate) exponent
    int exp;
    (void)std::frexp(e[0], &exp);  // Only need exponent, not mantissa
    --exp;  // adjust exp as frexp gives a binary exp that is 1 too big
    exp = static_cast<int>(_log2 * exp);  // estimate the power of ten exponent

    floatcascade<N> r = abs(*this);
    if (exp < 0) {
        if (exp < -300) {
            // Scale up to avoid underflow
            floatcascade<N> scaled;
            for (size_t i = 0; i < N; ++i) {
                scaled[i] = std::ldexp(r[i], 53);
            }
            r = scaled;
            r *= pown(_ten, -exp);
            // Scale back down
            for (size_t i = 0; i < N; ++i) {
                r[i] = std::ldexp(r[i], -53);
            }
        }
        else {
            r *= pown(_ten, -exp);
        }
    }
    else {
        if (exp > 0) {
            if (exp > 300) {
                // Scale down to avoid overflow
                floatcascade<N> scaled;
                for (size_t i = 0; i < N; ++i) {
                    scaled[i] = std::ldexp(r[i], -53);
                }
                r = scaled;
                r /= pown(_ten, exp);
                // Scale back up
                for (size_t i = 0; i < N; ++i) {
                    r[i] = std::ldexp(r[i], 53);
                }
            }
            else {
                r /= pown(_ten, exp);
            }
        }
    }

    // Fix exponent if we have gone too far
    if (r >= _ten) {
        r /= _ten;
        ++exp;
    }
    else {
        if (r < _one) {
            r *= _ten;
            --exp;
        }
    }

    // Use full floatcascade comparison (not just r[0]) to match dd behavior
    if ((r >= _ten) || (r < _one)) {
        std::cerr << "to_digits() failed to compute exponent\n";
        return;
    }

    // at this point the value is normalized to a decimal value between [1.0, 10.0)
    // generate the digits
    int nrDigits = precision + 1;
    for (int i = 0; i < nrDigits; ++i) {
        int mostSignificantDigit = static_cast<int>(r[0]);

        // Subtract the digit
        floatcascade<N> digit_value(static_cast<double>(mostSignificantDigit));

        // Negate and add using expansion_ops (since we don't have operator-= yet)
        floatcascade<N> neg_digit;
        for (size_t j = 0; j < N; ++j) {
            neg_digit[j] = -digit_value[j];
        }
        floatcascade<2*N> temp_expanded = expansion_ops::add_cascades(r, neg_digit);

        // Compress back to N components
        if constexpr (N == 2) {
            r = expansion_ops::compress_4to2(temp_expanded);
        }
        else if constexpr (N == 3) {
            r = expansion_ops::compress_6to3(temp_expanded);
        }
        else if constexpr (N == 4) {
            r = expansion_ops::compress_8to4(temp_expanded);
        }

        // Multiply by 10
        r *= _ten;

        s[static_cast<unsigned>(i)] = static_cast<char>(mostSignificantDigit + '0');
    }

    // Fix out of range digits
    for (int i = nrDigits - 1; i > 0; --i) {
        if (s[static_cast<unsigned>(i)] < '0') {
            s[static_cast<unsigned>(i - 1)]--;
            s[static_cast<unsigned>(i)] += 10;
        }
        else {
            if (s[static_cast<unsigned>(i)] > '9') {
                s[static_cast<unsigned>(i - 1)]++;
                s[static_cast<unsigned>(i)] -= 10;
            }
        }
    }

    if (s[0] <= '0') {
        std::cerr << "to_digits() non-positive leading digit\n";
        return;
    }

    // Round and propagate carry
    int lastDigit = nrDigits - 1;
    if (s[static_cast<unsigned>(lastDigit)] >= '5') {
        int i = nrDigits - 2;
        s[static_cast<unsigned>(i)]++;
        while (i > 0 && s[static_cast<unsigned>(i)] > '9') {
            s[static_cast<unsigned>(i)] -= 10;
            s[static_cast<unsigned>(--i)]++;
        }
    }

    // If first digit is 10, shift left and increment exponent
    if (s[0] > '9') {
        ++exp;
        for (int i = precision; i >= 2; --i) {
            s[static_cast<unsigned>(i)] = s[static_cast<unsigned>(i - 1)];
        }
        s[0] = '1';
        s[1] = '0';
    }

    s[static_cast<unsigned>(precision)] = 0;  // termination null
    exponent = exp;
}

template<size_t N>
std::string floatcascade<N>::to_string(
    std::streamsize precision,
    std::streamsize width,
    bool fixed,
    bool scientific,
    bool internal,
    bool left,
    bool showpos,
    bool uppercase,
    char fill
) const {
    std::string s;
    bool negative = (e[0] < 0.0);
    int exponent_value = 0;

    if (fixed && scientific)
        fixed = false;  // scientific format takes precedence

    if (isnan()) {
        s = uppercase ? "NAN" : "nan";
        negative = false;
    }
    else {
        if (negative) {
            s += '-';
        }
        else {
            if (showpos)
                s += '+';
        }

        if (isinf()) {
            s += uppercase ? "INF" : "inf";
        }
        else if (iszero()) {
            s += '0';
            if (precision > 0) {
                s += '.';
                s.append(static_cast<unsigned int>(precision), '0');
            }
        }
        else {
            int powerOfTenScale = static_cast<int>(std::log10(std::fabs(e[0])));
            int integerDigits = (fixed ? (powerOfTenScale + 1) : 1);
            int nrDigits = integerDigits + static_cast<int>(precision);

            int nrDigitsForFixedFormat = nrDigits;
            if (fixed)
                nrDigitsForFixedFormat = std::max(60, nrDigits);

            // a number in the range of [0.5, 1.0) to be printed with zero precision
            // must be rounded up to 1 to print correctly
            if (fixed && (precision == 0) && (std::abs(e[0]) < 1.0)) {
                s += (std::abs(e[0]) >= 0.5) ? '1' : '0';
                return s;
            }

            if (fixed && nrDigits <= 0) {
                // process values that are near zero
                s += '0';
                if (precision > 0) {
                    s += '.';
                    s.append(static_cast<unsigned int>(precision), '0');
                }
            }
            else {
                std::vector<char> t;

                if (fixed) {
                    t.resize(static_cast<size_t>(nrDigitsForFixedFormat + 1));
                    to_digits(t, exponent_value, nrDigitsForFixedFormat);
                }
                else {
                    t.resize(static_cast<size_t>(nrDigits + 1));
                    to_digits(t, exponent_value, nrDigits);
                }

                if (fixed) {
                    // round the decimal string
                    round_string(t, nrDigits + 1, &integerDigits);

                    if (integerDigits > 0) {
                        int i;
                        for (i = 0; i < integerDigits; ++i)
                            s += t[static_cast<unsigned>(i)];
                        if (precision > 0) {
                            s += '.';
                            for (int j = 0; j < precision; ++j, ++i)
                                s += t[static_cast<unsigned>(i)];
                        }
                    }
                    else {
                        s += "0.";
                        if (integerDigits < 0)
                            s.append(static_cast<size_t>(-integerDigits), '0');
                        for (int i = 0; i < nrDigits; ++i)
                            s += t[static_cast<unsigned>(i)];
                    }
                }
                else {
                    s += t[0ull];
                    if (precision > 0)
                        s += '.';

                    for (int i = 1; i <= precision; ++i)
                        s += t[static_cast<unsigned>(i)];
                }
            }
        }

        // TBD: this is seriously broken and needs a redesign
        //
        // fix for improper offset with large values and small values
        // without this trap, output of values of the for 10^j - 1 fail for j > 28
        // and are output with the point in the wrong place, leading to a significant error
        if (fixed && (precision > 0)) {
            // make sure that the value isn't dramatically larger
            double from_string = atof(s.c_str());

            // if this ratio is large, then we've got problems
            if (std::fabs(from_string / e[0]) > 3.0) {
                // loop on the string, find the point, move it up one
                // don't act on the first character
                for (std::string::size_type i = 1; i < s.length(); ++i) {
                    if (s[i] == '.') {
                        s[i] = s[i - 1];
                        s[i - 1] = '.';  // this will destroy the leading 0 when s[i==1] == '.';
                        break;
                    }
                }
                // BUG: the loop above, in particular s[i-1] = '.', destroys the leading 0
                // in the fixed point representation if the point is located at i = 1;
                // it also breaks the precision request as it adds a new digit to the fixed representation

                from_string = atof(s.c_str());
                // if this ratio is large, then the string has not been fixed
                if (std::fabs(from_string / e[0]) > 3.0) {
                    std::cerr << "re-rounding unsuccessful in fixed point fix\n";
                }
            }
        }

        if (!fixed && !isinf()) {
            // construct the exponent
            s += uppercase ? 'E' : 'e';
            append_exponent(s, exponent_value);
        }
    }

    // process any fill
    size_t strLength = s.length();
    if (strLength < static_cast<size_t>(width)) {
        size_t nrCharsToFill = (width - strLength);
        if (internal) {
            if (negative)
                s.insert(static_cast<std::string::size_type>(1), nrCharsToFill, fill);
            else
                s.insert(static_cast<std::string::size_type>(0), nrCharsToFill, fill);
        }
        else if (left) {
            s.append(nrCharsToFill, fill);
        }
        else {
            s.insert(static_cast<std::string::size_type>(0), nrCharsToFill, fill);
        }
    }

    return s;
}

template<size_t N>
bool floatcascade<N>::parse(const std::string& number) {
    const char* p = number.c_str();

    // Skip any leading spaces
    while (std::isspace(*p)) ++p;

    floatcascade<N> r;  // result accumulator
    for (size_t i = 0; i < N; ++i) r[i] = 0.0;

    int nrDigits = 0;
    int decimalPoint = -1;
    int sign = 0, eSign = 1;
    int exp = 0;
    bool done = false, parsingMantissa = true;
    char ch;

    while (!done && (ch = *p) != '\0') {
        if (std::isdigit(ch)) {
            if (parsingMantissa) {
                int digit = ch - '0';
                r *= 10.0;
                r += static_cast<double>(digit);
                ++nrDigits;
            }
            else {
                // parsing exponent section
                int digit = ch - '0';
                exp *= 10;
                exp += digit;
            }
        }
        else {
            switch (ch) {
            case '.':
                if (decimalPoint >= 0) return false;  // multiple decimal points
                decimalPoint = nrDigits;
                break;

            case '-':
            case '+':
                if (parsingMantissa) {
                    if (sign != 0 || nrDigits > 0) return false;  // sign in wrong place
                    sign = (ch == '-' ? -1 : 1);
                }
                else {
                    eSign = (ch == '-' ? -1 : 1);
                }
                break;

            case 'E':
            case 'e':
                parsingMantissa = false;
                break;

            default:
                return false;  // invalid character
            }
        }

        ++p;
    }

    exp *= eSign;

    // Adjust exponent based on decimal point position
    if (decimalPoint >= 0) exp -= (nrDigits - decimalPoint);

    // Apply exponent using power of 10
    floatcascade<N> ten(10.0);
    if (exp > 0) {
        r *= pown(ten, exp);
    }
    else if (exp < 0) {
        r /= pown(ten, -exp);
    }

    // Apply sign
    if (sign == -1) {
        for (size_t i = 0; i < N; ++i) {
            r[i] = -r[i];
        }
    }

    // Copy result to this
    *this = r;
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// Arithmetic operator implementations

// Addition operator
template<size_t N>
floatcascade<N>& floatcascade<N>::operator+=(const floatcascade<N>& rhs) noexcept {
    floatcascade<2*N> temp = expansion_ops::add_cascades(*this, rhs);

    // Compress back to N components
    if constexpr (N == 2) {
        *this = expansion_ops::compress_4to2(temp);
    }
    else if constexpr (N == 3) {
        *this = expansion_ops::compress_6to3(temp);
    }
    else if constexpr (N == 4) {
        *this = expansion_ops::compress_8to4(temp);
    }
    else {
        *this = expansion_ops::renormalize(temp);
    }

    return *this;
}

// Subtraction operator
template<size_t N>
floatcascade<N>& floatcascade<N>::operator-=(const floatcascade<N>& rhs) noexcept {
    // Negate rhs and add
    floatcascade<N> neg_rhs;
    for (size_t i = 0; i < N; ++i) {
        neg_rhs[i] = -rhs[i];
    }
    return *this += neg_rhs;
}

// Multiplication operator
template<size_t N>
floatcascade<N>& floatcascade<N>::operator*=(const floatcascade<N>& rhs) noexcept {
    *this = expansion_ops::multiply_cascades(*this, rhs);
    return *this;
}

// Division operator (Newton-Raphson method, generalized from dd_cascade)
template<size_t N>
floatcascade<N>& floatcascade<N>::operator/=(const floatcascade<N>& rhs) noexcept {
    if (isnan())
        return *this;
    if (rhs.isnan())
        return *this = rhs;
    if (rhs.iszero()) {
        if (iszero()) {
            // Set to NaN
            e[0] = std::numeric_limits<double>::quiet_NaN();
            for (size_t i = 1; i < N; ++i) e[i] = 0.0;
        } else {
            // Set to infinity with appropriate sign
            double inf_val = (sign() == rhs.sign()) ? INFINITY : -INFINITY;
            e[0] = inf_val;
            for (size_t i = 1; i < N; ++i) e[i] = 0.0;
        }
        return *this;
    }

    // Newton-Raphson division: compute reciprocal then multiply
    // Initial approximation q0 = a/b using highest component
    double q0 = e[0] / rhs.e[0];

    // Compute residual: *this - q0 * rhs
    floatcascade<N> q0_fc;
    q0_fc[0] = q0;
    for (size_t i = 1; i < N; ++i) q0_fc[i] = 0.0;

    floatcascade<N> q0_times_rhs = expansion_ops::multiply_cascades(q0_fc, rhs);

    // Use add_cascades for subtraction (add negative)
    floatcascade<N> neg_q0_times_rhs;
    for (size_t i = 0; i < N; ++i) {
        neg_q0_times_rhs[i] = -q0_times_rhs[i];
    }
    floatcascade<2*N> residual_expanded = expansion_ops::add_cascades(*this, neg_q0_times_rhs);

    // Compress residual back to N components
    floatcascade<N> residual;
    if constexpr (N == 2) {
        residual = expansion_ops::compress_4to2(residual_expanded);
    }
    else if constexpr (N == 3) {
        residual = expansion_ops::compress_6to3(residual_expanded);
    }
    else if constexpr (N == 4) {
        residual = expansion_ops::compress_8to4(residual_expanded);
    }

    // Refine: q1 = q0 + residual/rhs
    double q1 = residual.e[0] / rhs.e[0];

    // Combine quotients using two_sum for error-free addition
    floatcascade<N> result_cascade;
    result_cascade[0] = q0;
    result_cascade[1] = q1;
    for (size_t i = 2; i < N; ++i) result_cascade[i] = 0.0;

    *this = expansion_ops::renormalize(result_cascade);
    return *this;
}

///////////////////////////////////////////////////////////////////////////////
// Helper functions for decimal conversion and mathematical operations

// absolute value
template<size_t N>
inline floatcascade<N> abs(const floatcascade<N>& a) {
    floatcascade<N> result(a);
    if (a[0] < 0.0) {
        // Negate all components
        for (size_t i = 0; i < N; ++i) {
            result[i] = -result[i];
        }
    }
    return result;
}

// square: x^2
template<size_t N>
inline floatcascade<N> sqr(const floatcascade<N>& a) {
    return expansion_ops::multiply_cascades(a, a);
}

// reciprocal: 1/x
template<size_t N>
inline floatcascade<N> reciprocal(const floatcascade<N>& a) {
    if (a.iszero()) {
        floatcascade<N> result;
        result[0] = (a[0] < 0.0) ? -INFINITY : INFINITY;
        for (size_t i = 1; i < N; ++i) result[i] = 0.0;
        return result;
    }

    if (a.isinf()) {
        floatcascade<N> result;
        for (size_t i = 0; i < N; ++i) result[i] = 0.0;
        return result;
    }

    // Use division operator: 1.0 / a
    floatcascade<N> one(1.0);
    floatcascade<N> result = one;
    result /= a;
    return result;
}

// power to integer exponent using binary exponentiation
template<size_t N>
inline floatcascade<N> pown(const floatcascade<N>& a, int n) {
    if (a.isnan()) return a;

    int abs_n = (n < 0) ? -n : n;
    floatcascade<N> result;

    // Handle special cases
    if (abs_n == 0) {
        if (a.iszero()) {
            std::cerr << "pown: 0^0 is undefined\n";
            result[0] = std::numeric_limits<double>::quiet_NaN();
            for (size_t i = 1; i < N; ++i) result[i] = 0.0;
            return result;
        }
        // x^0 = 1
        result[0] = 1.0;
        for (size_t i = 1; i < N; ++i) result[i] = 0.0;
        return result;
    }

    if (abs_n == 1) {
        result = a;
    }
    else if (abs_n == 2) {
        result = sqr(a);
    }
    else {
        // Binary exponentiation
        floatcascade<N> base = a;
        result[0] = 1.0;
        for (size_t i = 1; i < N; ++i) result[i] = 0.0;

        int exp = abs_n;
        while (exp > 0) {
            if (exp % 2 == 1) {
                result = expansion_ops::multiply_cascades(result, base);
            }
            exp /= 2;
            if (exp > 0) {
                base = sqr(base);
            }
        }
    }

    // If exponent was negative, return reciprocal
    if (n < 0) {
        result = reciprocal(result);
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
// Comparison operators

// Equality: compare all components
template<size_t N>
inline bool operator==(const floatcascade<N>& lhs, const floatcascade<N>& rhs) {
    for (size_t i = 0; i < N; ++i) {
        if (lhs[i] != rhs[i]) return false;
    }
    return true;
}

template<size_t N>
inline bool operator!=(const floatcascade<N>& lhs, const floatcascade<N>& rhs) {
    return !(lhs == rhs);
}

// Less than: lexicographic comparison (compare high to low components)
template<size_t N>
inline bool operator<(const floatcascade<N>& lhs, const floatcascade<N>& rhs) {
    for (size_t i = 0; i < N; ++i) {
        if (lhs[i] < rhs[i]) return true;
        if (lhs[i] > rhs[i]) return false;
    }
    return false; // equal
}

template<size_t N>
inline bool operator>(const floatcascade<N>& lhs, const floatcascade<N>& rhs) {
    return rhs < lhs;
}

template<size_t N>
inline bool operator<=(const floatcascade<N>& lhs, const floatcascade<N>& rhs) {
    return !(rhs < lhs);
}

template<size_t N>
inline bool operator>=(const floatcascade<N>& lhs, const floatcascade<N>& rhs) {
    return !(lhs < rhs);
}

// Comparison with double (convert double to floatcascade<N>)
template<size_t N>
inline bool operator==(const floatcascade<N>& lhs, double rhs) {
    return lhs == floatcascade<N>(rhs);
}

template<size_t N>
inline bool operator!=(const floatcascade<N>& lhs, double rhs) {
    return lhs != floatcascade<N>(rhs);
}

template<size_t N>
inline bool operator<(const floatcascade<N>& lhs, double rhs) {
    return lhs < floatcascade<N>(rhs);
}

template<size_t N>
inline bool operator>(const floatcascade<N>& lhs, double rhs) {
    return lhs > floatcascade<N>(rhs);
}

template<size_t N>
inline bool operator<=(const floatcascade<N>& lhs, double rhs) {
    return lhs <= floatcascade<N>(rhs);
}

template<size_t N>
inline bool operator>=(const floatcascade<N>& lhs, double rhs) {
    return lhs >= floatcascade<N>(rhs);
}

template<size_t N>
inline bool operator==(double lhs, const floatcascade<N>& rhs) {
    return floatcascade<N>(lhs) == rhs;
}

template<size_t N>
inline bool operator!=(double lhs, const floatcascade<N>& rhs) {
    return floatcascade<N>(lhs) != rhs;
}

template<size_t N>
inline bool operator<(double lhs, const floatcascade<N>& rhs) {
    return floatcascade<N>(lhs) < rhs;
}

template<size_t N>
inline bool operator>(double lhs, const floatcascade<N>& rhs) {
    return floatcascade<N>(lhs) > rhs;
}

template<size_t N>
inline bool operator<=(double lhs, const floatcascade<N>& rhs) {
    return floatcascade<N>(lhs) <= rhs;
}

template<size_t N>
inline bool operator>=(double lhs, const floatcascade<N>& rhs) {
    return floatcascade<N>(lhs) >= rhs;
}

} // namespace sw::universal
