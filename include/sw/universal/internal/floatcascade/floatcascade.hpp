#pragma once

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
    constexpr bool ispos()    const noexcept { return e[0] >= 0.0; }
    constexpr bool isneg()    const noexcept { return e[0] < 0.0; }
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

// Core expansion operations - the "engine" for all cascade operations
namespace expansion_ops {

    // Knuth's TWO-SUM: computes a + b = x + y exactly
    inline void two_sum(double a, double b, double& x, double& y) {
        x = a + b;
        double b_virtual = x - a;
        double a_virtual = x - b_virtual;
        double b_roundoff = b - b_virtual;
        double a_roundoff = a - a_virtual;
        y = a_roundoff + b_roundoff;
    }

    // Dekker's FAST-TWO-SUM: assumes |a| >= |b|
    inline void fast_two_sum(double a, double b, double& x, double& y) {
        x = a + b;
        y = b - (x - a);
    }

    // Add single double to N-component cascade, result in (N+1)-component cascade
    template<size_t N>
    floatcascade<N+1> grow_expansion(const floatcascade<N>& e, double b) {
        floatcascade<N+1> result;
        double q = b;
        double h;
        
        // Process from least significant (end) to most significant (beginning)
        for (int i = N - 1; i >= 0; --i) {  // Changed: reverse order
            two_sum(q, e[i], q, h);
            result[i + 1] = h;  // Changed: shift components right
        }
        result[0] = q;  // Changed: most significant component at [0]
        
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
        floatcascade<2 * N> result;
        double sum = 0.0;
        std::vector<double> corrections;

        // Process from end (smallest) to beginning (largest)
        for (int i = 2 * N - 1; i >= 0; --i) {  // Changed: reverse iteration
            double new_sum, error;
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

} // namespace expansion_ops

} // namespace sw::universal
