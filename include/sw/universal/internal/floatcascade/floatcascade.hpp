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
    // Components stored in increasing order of magnitude: e[0] <= e[1] <= ... <= e[N-1]
    // Actual value = e[0] + e[1] + ... + e[N-1]
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
    
    constexpr void set(double x) {
        e[0] = x;
        for (size_t i = 1; i < N; ++i) e[i] = 0.0;
    }

    constexpr size_t size() const noexcept { return N; }
    const std::array<double, N>& data() const noexcept { return e; }
    std::array<double, N>& data() { return e; }

    // Conversion to double (estimate)
    constexpr double to_double() const {
        double sum = 0.0;
        for (size_t i = 0; i < N; ++i) {
            sum += e[i];
        }
        return sum;
    }

    // Basic properties
    constexpr bool iszero() const {
        for (size_t i = 0; i < N; ++i) {
            if (e[i] != 0.0) return false;
        }
        return true;
    }

    constexpr int sign() const noexcept {
        // Sign of most significant non-zero component
        for (int i = N-1; i >= 0; --i) {
            if (e[i] > 0.0) return 1;
            if (e[i] < 0.0) return -1;
        }
        return 0;
    }

    // Set all components to zero
    constexpr void clear() {
        for (size_t i = 0; i < N; ++i) e[i] = 0.0;
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
        
        for (size_t i = 0; i < N; ++i) {
            two_sum(q, e[i], q, h);
            result[i] = h;
        }
        result[N] = q;
        
        return result;
    }

    // Add two cascades of same size, result in double-size cascade
    template<size_t N>
    floatcascade<2*N> add_cascades(const floatcascade<N>& a, const floatcascade<N>& b) {
        // Merge the two N-component cascades
        std::array<double, 2*N> merged;
        std::array<double, 2*N> magnitudes;
        
        // Collect all components and their magnitudes
        for (size_t i = 0; i < N; ++i) {
            merged[2*i] = a[i];
            merged[2*i + 1] = b[i];
            magnitudes[2*i] = std::abs(a[i]);
            magnitudes[2*i + 1] = std::abs(b[i]);
        }
        
        // Sort by magnitude (simple bubble sort for small arrays)
        for (size_t i = 0; i < 2*N - 1; ++i) {
            for (size_t j = 0; j < 2*N - 1 - i; ++j) {
                if (magnitudes[j] > magnitudes[j + 1]) {
                    std::swap(merged[j], merged[j + 1]);
                    std::swap(magnitudes[j], magnitudes[j + 1]);
                }
            }
        }
        
        // Accumulate using cascade of two_sum operations
        floatcascade<2*N> result;
        double sum = 0.0;
        size_t result_idx = 0;
        
        for (size_t i = 0; i < 2*N; ++i) {
            double new_sum, error;
            two_sum(sum, merged[i], new_sum, error);
            
            if (error != 0.0 && result_idx < 2*N) {
                result[result_idx++] = error;
            }
            sum = new_sum;
        }
        
        // Store the final sum
        if (result_idx < 2*N) {
            result[result_idx] = sum;
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
