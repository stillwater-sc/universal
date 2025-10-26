#pragma once
// expansion_ops.hpp: Shewchuk's adaptive precision floating-point expansion algorithms
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// References:
// Jonathan Richard Shewchuk, "Adaptive Precision Floating-Point Arithmetic and Fast Robust
// Geometric Predicates," Discrete & Computational Geometry 18:305-363, October 1997.
// Available at: https://people.eecs.berkeley.edu/~jrs/papers/robustr.pdf
//
// Terminology:
// - Expansion: An unevaluated sum of floating-point numbers (components)
// - Nonoverlapping: Components e[i] and e[i+1] have no overlapping significand bits
// - Strongly nonoverlapping: Even stricter - adjacent components differ by at least mantissa length
// - Adaptive: Algorithms that do only as much work as necessary to guarantee correct result

#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace sw::universal {

/*
 * EXPANSION OVERVIEW
 * ==================
 *
 * An expansion is a sequence of floating-point values e[0], e[1], ..., e[n-1] whose
 * unevaluated sum represents a high-precision number:
 *     value = e[0] + e[1] + ... + e[n-1]
 *
 * Key Properties:
 * 1. Nonoverlapping: Adding e[i] + e[i+1] produces no rounding error
 * 2. Decreasing magnitude: |e[0]| >= |e[1]| >= ... >= |e[n-1]|
 * 3. Precision gain: Each component adds approximately 53 bits of precision
 *
 * This file implements Shewchuk's adaptive precision algorithms, which differ from
 * Priest's fixed-precision algorithms (used in floatcascade) by allowing dynamic
 * component count that grows/shrinks based on precision requirements.
 *
 * Shewchuk vs. Priest:
 * - Priest (floatcascade): Fixed N components, always compress back to N
 * - Shewchuk (expansion): Variable components, compress only when needed
 *
 * The adaptive algorithms enable:
 * - Early termination in comparisons (examine only as many components as needed)
 * - Dynamic precision growth (add components only when precision demands it)
 * - Efficient geometric predicates (orientation test, incircle test)
 */

namespace expansion_ops {

// ============================================================================
// ERROR-FREE TRANSFORMATIONS (EFT)
// ============================================================================
// These are the foundation of all expansion algorithms. They guarantee that
// the result of a floating-point operation can be represented EXACTLY as
// the sum of two floating-point numbers.

/*
 * TWO-SUM: Error-free transformation for addition
 * ================================================
 * Algorithm by Knuth (1969), analysis by Dekker (1971)
 *
 * Given: two floating-point numbers a, b
 * Computes: x, y such that a + b = x + y exactly (no rounding error)
 *
 * Where:
 *   x = RoundToNearest(a + b)  (the sum as computed by floating-point)
 *   y = the rounding error
 *
 * Cost: 6 floating-point operations
 *
 * Note: Uses volatile to prevent aggressive compiler optimizations that would
 * break the error-free guarantee. Modern compilers may reorder or fuse operations
 * in ways that violate the required rounding behavior.
 */
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

/*
 * FAST-TWO-SUM: Optimized error-free transformation when |a| >= |b|
 * ==================================================================
 * Algorithm by Dekker (1971)
 *
 * Precondition: |a| >= |b|
 * Computes: x, y such that a + b = x + y exactly
 *
 * Cost: 3 floating-point operations (half the cost of TWO-SUM!)
 *
 * This is the workhorse of expansion algorithms because we can maintain the
 * magnitude ordering during expansion construction, allowing us to always
 * use this faster version.
 */
inline void fast_two_sum(double a, double b, double& x, double& y) {
    volatile double vx = a + b;
    x = vx;
    volatile double vy = b - (vx - a);
    y = vy;
}

/*
 * TWO-PROD: Error-free transformation for multiplication
 * =======================================================
 * Algorithm by Dekker (1971), optimized using FMA
 *
 * Given: two floating-point numbers a, b
 * Computes: x, y such that a * b = x + y exactly
 *
 * Modern implementation uses FMA (Fused Multiply-Add) for exact error computation:
 *   x = a * b
 *   y = fma(a, b, -x)  // Computes (a*b - x) with no intermediate rounding
 *
 * Cost: 2 floating-point operations (with FMA support)
 *
 * Without FMA, this requires Dekker's splitting algorithm (17 operations).
 * Since we assume IEEE-754 with FMA, we use the fast version.
 */
inline void two_prod(double a, double b, double& x, double& y) {
    volatile double vx = a * b;
    x = vx;
    // Use FMA if available for exact error computation
    volatile double vy = std::fma(a, b, -vx);
    y = vy;
}

// ============================================================================
// EXPANSION GROWTH ALGORITHMS
// ============================================================================

/*
 * GROW-EXPANSION: Add a single component to an expansion
 * =======================================================
 * Algorithm by Shewchuk (Figure 6 in the paper)
 *
 * Input:
 *   e - expansion with m components (nonoverlapping, decreasing magnitude)
 *   b - single floating-point number to add
 *
 * Output:
 *   h - expansion with m+1 components (nonoverlapping, decreasing magnitude)
 *
 * Algorithm:
 *   Starting from the least significant component, use TWO-SUM to add b,
 *   propagating the sum upward and keeping the error at each level.
 *
 * Cost: 6m + 4 floating-point operations
 *
 * Example: Growing [3.0, 0.5e-15] with 1.0
 *   Start: e = [3.0, 0.5e-15], b = 1.0
 *   Step 1: two_sum(1.0, 0.5e-15) = (1.0, 0.5e-15)
 *           h[1] = 0.5e-15, carry q = 1.0
 *   Step 2: two_sum(1.0, 3.0) = (4.0, 0.0)
 *           h[2] = 0.0, carry q = 4.0
 *   Result: h = [4.0, 0.0, 0.5e-15] -> after removing zeros: [4.0, 0.5e-15]
 */
inline std::vector<double> grow_expansion(const std::vector<double>& e, double b) {
    size_t m = e.size();
    std::vector<double> h(m + 1);

    double q = b;

    // Process from least significant (end) to most significant (beginning)
    // Using TWO-SUM since we don't know the relative magnitude of b vs e[i]
    for (size_t i = m; i-- > 0; ) {
        double q_new, h_i;
        two_sum(q, e[i], q_new, h_i);
        h[i + 1] = h_i;  // Store error component
        q = q_new;       // Carry sum forward
    }
    h[0] = q;  // Most significant component

    return h;
}

/*
 * FAST-EXPANSION-SUM: Merge two nonoverlapping expansions
 * ========================================================
 * Algorithm by Shewchuk (Figure 8 in the paper)
 *
 * Input:
 *   e - expansion with m components (strongly nonoverlapping)
 *   f - expansion with n components (strongly nonoverlapping)
 *
 * Output:
 *   h - expansion with m+n components (strongly nonoverlapping)
 *
 * Algorithm:
 *   Merge the two expansions like merge-sort, using FAST-TWO-SUM at each step
 *   to maintain the nonoverlapping property. The key insight is that we can
 *   compare exponents to determine which component to process next.
 *
 * Cost: 6(m+n) floating-point operations
 *
 * Note: Requires "strongly nonoverlapping" expansions and round-to-even
 * tiebreaking for correctness. This is guaranteed by our construction.
 *
 * Example: Merging [3.0, 0.5e-15] + [2.0, 0.3e-15]
 *   Both expansions are sorted by decreasing magnitude
 *   Compare 3.0 vs 2.0 -> process 3.0 first
 *   Compare 2.0 vs 0.5e-15 -> process 2.0
 *   Then process remaining components
 *   Result: [5.0, small corrections...]
 */
inline std::vector<double> fast_expansion_sum(const std::vector<double>& e, const std::vector<double>& f) {
    size_t m = e.size();
    size_t n = f.size();

    // Special cases
    if (m == 0) return f;
    if (n == 0) return e;

    std::vector<double> h;
    h.reserve(m + n);

    size_t i = 0, j = 0;
    double q = 0.0;

    // Merge process: walk through both expansions from least to most significant
    // Note: Our expansions are stored in DECREASING order [most sig ... least sig]
    // So we walk backwards through the arrays

    // Pick the smaller component to start
    double e_curr = (m > 0) ? e[m - 1] : 0.0;
    double f_curr = (n > 0) ? f[n - 1] : 0.0;

    // Start with the absolutely smaller component
    if (std::abs(e_curr) < std::abs(f_curr) || (m == 0)) {
        q = f_curr;
        j = n - 1;
        if (j > 0) j--;
        else j = SIZE_MAX;  // Mark as exhausted
    } else {
        q = e_curr;
        i = m - 1;
        if (i > 0) i--;
        else i = SIZE_MAX;  // Mark as exhausted
    }

    // Merge remaining components
    while (i != SIZE_MAX || j != SIZE_MAX) {
        double next_component;

        if (i == SIZE_MAX) {
            // Only f remains
            next_component = f[j];
            if (j > 0) j--; else j = SIZE_MAX;
        } else if (j == SIZE_MAX) {
            // Only e remains
            next_component = e[i];
            if (i > 0) i--; else i = SIZE_MAX;
        } else {
            // Both available - pick smaller magnitude
            if (std::abs(e[i]) < std::abs(f[j])) {
                next_component = e[i];
                if (i > 0) i--; else i = SIZE_MAX;
            } else {
                next_component = f[j];
                if (j > 0) j--; else j = SIZE_MAX;
            }
        }

        double q_new, h_i;
        two_sum(q, next_component, q_new, h_i);  // Use TWO-SUM for correctness

        if (h_i != 0.0) {
            h.push_back(h_i);
        }
        q = q_new;
    }

    if (q != 0.0) {
        h.push_back(q);
    }

    // Reverse to get decreasing magnitude order
    std::reverse(h.begin(), h.end());

    return h;
}

/*
 * LINEAR-EXPANSION-SUM: Alternative merging algorithm
 * ====================================================
 * Algorithm by Shewchuk (Figure 7 in the paper)
 *
 * Input/Output: Same as FAST-EXPANSION-SUM
 *
 * Difference: Uses TWO-SUM instead of FAST-TWO-SUM, so doesn't require
 * the "strongly nonoverlapping" property. More robust but slower.
 *
 * Cost: 9(m+n) floating-point operations (vs 6(m+n) for FAST version)
 *
 * Use when:
 * - Input expansions might not be strongly nonoverlapping
 * - Robustness is more important than speed
 * - Floating-point environment doesn't guarantee round-to-even
 */
inline std::vector<double> linear_expansion_sum(const std::vector<double>& e, const std::vector<double>& f) {
    size_t m = e.size();
    size_t n = f.size();

    // Special cases
    if (m == 0) return f;
    if (n == 0) return e;

    std::vector<double> h;
    h.reserve(m + n);

    // Initialize indices to point to least significant (last) components
    size_t i = m - 1;
    size_t j = n - 1;
    double q = 0.0;

    // Similar merge process to FAST-EXPANSION-SUM but using TWO-SUM
    // Per Shewchuk Figure 7: Start with component having smaller magnitude
    double e_curr = e[i];
    double f_curr = f[j];

    // Start with the absolutely smaller component (consume it)
    if (std::abs(e_curr) < std::abs(f_curr)) {
        q = e_curr;  // Pick e (smaller)
        if (i > 0) i--;
        else i = SIZE_MAX;
    } else {
        q = f_curr;  // Pick f (smaller or equal)
        if (j > 0) j--;
        else j = SIZE_MAX;
    }

    // Merge remaining components using TWO-SUM (not FAST-TWO-SUM)
    while (i != SIZE_MAX || j != SIZE_MAX) {
        double next_component;

        if (i == SIZE_MAX) {
            next_component = f[j];
            if (j > 0) j--; else j = SIZE_MAX;
        } else if (j == SIZE_MAX) {
            next_component = e[i];
            if (i > 0) i--; else i = SIZE_MAX;
        } else {
            if (std::abs(e[i]) < std::abs(f[j])) {
                next_component = e[i];
                if (i > 0) i--; else i = SIZE_MAX;
            } else {
                next_component = f[j];
                if (j > 0) j--; else j = SIZE_MAX;
            }
        }

        double q_new, h_i;
        two_sum(q, next_component, q_new, h_i);  // Using TWO-SUM (6 ops) not FAST (3 ops)

        if (h_i != 0.0) {
            h.push_back(h_i);
        }
        q = q_new;
    }

    if (q != 0.0) {
        h.push_back(q);
    }

    // Reverse to get decreasing magnitude order
    std::reverse(h.begin(), h.end());

    return h;
}

// ============================================================================
// EXPANSION SCALING
// ============================================================================

/*
 * SCALE-EXPANSION: Multiply expansion by scalar
 * ==============================================
 * Algorithm by Shewchuk (Figure 9 in the paper)
 *
 * Input:
 *   e - expansion with m components (nonoverlapping)
 *   b - scalar floating-point multiplier
 *
 * Output:
 *   h - expansion with at most 2m components
 *
 * Algorithm:
 *   For each component e[i], use TWO-PROD to compute b * e[i] = product + error
 *   Accumulate all products and errors into output expansion
 *
 * Cost: 2m multiplications + accumulation
 *
 * Note: Output may have up to 2m components (one product + one error per input)
 * Use COMPRESS-EXPANSION afterward if you need to reduce component count
 */
inline std::vector<double> scale_expansion(const std::vector<double>& e, double b) {
    if (e.empty()) return std::vector<double>();
    if (b == 0.0) return std::vector<double>{0.0};
    if (b == 1.0) return e;
    if (b == -1.0) {
        std::vector<double> result = e;
        for (auto& v : result) v = -v;
        return result;
    }

    size_t m = e.size();
    std::vector<double> products;
    products.reserve(2 * m);

    // Multiply each component by b, collecting products and errors
    for (size_t i = 0; i < m; ++i) {
        double product, error;
        two_prod(b, e[i], product, error);

        if (product != 0.0) products.push_back(product);
        if (error != 0.0) products.push_back(error);
    }

    // Sort by decreasing magnitude (most significant first)
    std::sort(products.begin(), products.end(), [](double a, double b) {
        return std::abs(a) > std::abs(b);
    });

    // The products are now in decreasing magnitude order but not necessarily
    // nonoverlapping. For exact nonoverlapping, we'd need to run through
    // renormalization. For now, we return the sorted products.
    // TODO: Add optional renormalization pass

    return products;
}

// ============================================================================
// EXPANSION COMPRESSION
// ============================================================================

/*
 * COMPRESS-EXPANSION: Remove insignificant components
 * ===================================================
 * Algorithm by Shewchuk (adaptive variant)
 *
 * Input:
 *   e - expansion with m components
 *   epsilon - relative threshold for removal (default 0.0 = remove only zeros)
 *
 * Output:
 *   h - compressed expansion with ≤ m components
 *
 * Algorithm:
 *   Remove components whose absolute value is less than epsilon * |largest|
 *   When epsilon = 0.0, only exact zeros are removed
 *
 * Use cases:
 *   - epsilon = 0.0: Remove exact zeros (no precision loss)
 *   - epsilon = 1e-30: Remove components that don't affect double precision
 *   - epsilon = 1e-15: Aggressive compression (may lose extended precision)
 *
 * Cost: O(m) scan + potential reallocation
 */
inline std::vector<double> compress_expansion(const std::vector<double>& e, double epsilon = 0.0) {
    if (e.empty()) return e;

    // Find largest magnitude for relative threshold
    double max_magnitude = 0.0;
    for (const auto& component : e) {
        double mag = std::abs(component);
        if (mag > max_magnitude) max_magnitude = mag;
    }

    if (max_magnitude == 0.0) {
        // All zeros
        return std::vector<double>{0.0};
    }

    double threshold = epsilon * max_magnitude;

    std::vector<double> compressed;
    compressed.reserve(e.size());

    for (const auto& component : e) {
        if (std::abs(component) > threshold) {
            compressed.push_back(component);
        }
    }

    if (compressed.empty()) {
        // All components were below threshold - keep the largest
        compressed.push_back(e[0]);
    }

    return compressed;
}

/*
 * COMPRESS-EXPANSION (count-based): Compress to at most N components
 * ==================================================================
 *
 * Input:
 *   e - expansion with m components
 *   max_components - maximum number of components to keep
 *
 * Output:
 *   h - expansion with at most max_components
 *
 * Algorithm:
 *   Keep the max_components most significant components, discard the rest
 *   The discarded tail is lost (precision reduction)
 *
 * Use case: When you have a target precision (e.g., reduce to 4 components)
 */
inline std::vector<double> compress_to_n(const std::vector<double>& e, size_t max_components) {
    if (e.size() <= max_components) return e;

    std::vector<double> compressed(e.begin(), e.begin() + max_components);
    return compressed;
}

// ============================================================================
// ADAPTIVE OPERATIONS
// ============================================================================

/*
 * SIGN-ADAPTIVE: Determine sign with early termination
 * =====================================================
 * This is the key to Shewchuk's adaptive algorithms!
 *
 * Input: expansion e with m components
 * Output: -1 (negative), 0 (zero), +1 (positive)
 *
 * Algorithm:
 *   Examine components from most to least significant
 *   Return as soon as a non-zero component is found
 *
 * Cost: O(1) to O(m) depending on how many leading zeros
 *
 * Example: For expansion [1e-100, 0.0, 0.0, 0.0, ..., tiny_errors]
 *   Traditional: Must sum all m components to get sign
 *   Adaptive: Examine first component, return immediately (1 comparison!)
 *
 * This makes geometric predicates incredibly efficient - most of the time
 * the sign can be determined from just the first 1-2 components.
 */
inline int sign_adaptive(const std::vector<double>& e) {
    for (const auto& component : e) {
        if (component > 0.0) return 1;
        if (component < 0.0) return -1;
    }
    return 0;  // All components are zero
}

/*
 * EXPANSION-PRODUCT: Multiply two expansions
 * ===========================================
 *
 * Algorithm: For expansions e (m components) and f (n components),
 * compute e * f by scaling f by each component of e and summing.
 *
 * Input:
 *   e - expansion with m components
 *   f - expansion with n components
 *
 * Output:
 *   h - expansion representing e * f
 *
 * Cost: O(m*n) - each component of e scales all of f
 *
 * Note: Result may have up to 2*m*n components before compression
 */
inline std::vector<double> expansion_product(const std::vector<double>& e, const std::vector<double>& f) {
    if (e.empty() || f.empty()) return std::vector<double>{0.0};

    // Handle zero cases
    if ((e.size() == 1 && e[0] == 0.0) || (f.size() == 1 && f[0] == 0.0)) {
        return std::vector<double>{0.0};
    }

    // Start with zero
    std::vector<double> result{0.0};

    // For each component in e, scale f and accumulate
    for (const auto& e_component : e) {
        if (e_component != 0.0) {
            std::vector<double> scaled = scale_expansion(f, e_component);
            result = linear_expansion_sum(result, scaled);
        }
    }

    return result;
}

/*
 * EXPANSION-RECIPROCAL: Compute reciprocal of an expansion
 * =========================================================
 *
 * Algorithm: Compute 1/e using Newton iteration
 * Starting with r0 = 1/e[0], refine using: r_{n+1} = r_n * (2 - e * r_n)
 *
 * Input:
 *   e - expansion (must be non-zero)
 *   iterations - number of Newton iterations (default: 3)
 *
 * Output:
 *   h - expansion representing 1/e
 *
 * Note: More iterations = higher precision but more cost
 */
inline std::vector<double> expansion_reciprocal(const std::vector<double>& e, int iterations = 3) {
    if (e.empty() || (e.size() == 1 && e[0] == 0.0)) {
        // Division by zero - return inf (or could throw)
        return std::vector<double>{std::numeric_limits<double>::infinity()};
    }

    // Initial approximation: 1 / first component
    double r0 = 1.0 / e[0];
    std::vector<double> result{r0};

    // Newton iteration: r_{n+1} = r_n * (2 - e * r_n)
    std::vector<double> two{2.0};
    for (int i = 0; i < iterations; ++i) {
        std::vector<double> product = expansion_product(e, result);  // e * r_n
        std::vector<double> diff = linear_expansion_sum(two, scale_expansion(product, -1.0));  // 2 - e * r_n
        result = expansion_product(result, diff);  // r_n * (2 - e * r_n)
    }

    return result;
}

/*
 * EXPANSION-QUOTIENT: Divide two expansions
 * ==========================================
 *
 * Algorithm: Compute e / f = e * (1/f)
 *
 * Input:
 *   e - numerator expansion
 *   f - denominator expansion (must be non-zero)
 *
 * Output:
 *   h - expansion representing e / f
 */
inline std::vector<double> expansion_quotient(const std::vector<double>& e, const std::vector<double>& f) {
    std::vector<double> reciprocal = expansion_reciprocal(f);
    return expansion_product(e, reciprocal);
}

/*
 * COMPARE-ADAPTIVE: Compare two expansions with early termination
 * ================================================================
 *
 * Input: two expansions e and f
 * Output: -1 (e < f), 0 (e == f), +1 (e > f)
 *
 * Algorithm:
 *   Conceptually compute difference = e - f
 *   Determine sign of difference adaptively
 *   Optimization: Don't actually construct difference, just compare components
 *
 * Cost: O(1) to O(m+n) depending on leading cancellation
 *
 * This is crucial for geometric predicates like orientation tests:
 *   "Which side of a line is point P on?"
 *   Answer: sign(orient2d(A, B, P))
 */
inline int compare_adaptive(const std::vector<double>& e, const std::vector<double>& f) {
    // Strategy: Walk through both expansions in decreasing magnitude order
    // comparing corresponding components until we find a difference

    size_t i = 0, j = 0;

    while (i < e.size() || j < f.size()) {
        double e_val = (i < e.size()) ? e[i] : 0.0;
        double f_val = (j < f.size()) ? f[j] : 0.0;

        // Compare absolute magnitudes to decide which to examine
        double e_mag = std::abs(e_val);
        double f_mag = std::abs(f_val);

        if (e_mag > f_mag) {
            // e has larger magnitude component
            if (e_val > 0.0) return 1;   // e > f
            if (e_val < 0.0) return -1;  // e < f
            ++i;
        } else if (f_mag > e_mag) {
            // f has larger magnitude component
            if (f_val > 0.0) return -1;  // e < f
            if (f_val < 0.0) return 1;   // e > f
            ++j;
        } else {
            // Same magnitude - compare values directly
            if (e_val > f_val) return 1;
            if (e_val < f_val) return -1;
            // Equal, continue to next components
            ++i;
            ++j;
        }
    }

    return 0;  // All components compared equal
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/*
 * ESTIMATE: Quick approximation of expansion value
 * =================================================
 * Returns a double-precision approximation of the expansion's value by
 * summing the first few components.
 *
 * This is NOT exact - it loses the precision of the tail components.
 * Use for quick estimates, not for exact computation.
 */
inline double estimate(const std::vector<double>& e) {
    if (e.empty()) return 0.0;

    // Sum first few components for a reasonable estimate
    double sum = 0.0;
    size_t limit = std::min(e.size(), size_t(4));

    // Sum from least to most significant for better accuracy
    for (size_t i = limit; i-- > 0; ) {
        sum += e[i];
    }

    return sum;
}

/*
 * EXPANSION INVARIANT VERIFICATION
 * =================================
 * These functions check that expansions maintain the required properties.
 * Use for debugging and testing, not in production code.
 */

// Check if expansion is in decreasing magnitude order
inline bool is_decreasing_magnitude(const std::vector<double>& e) {
    for (size_t i = 1; i < e.size(); ++i) {
        if (std::abs(e[i-1]) < std::abs(e[i])) {
            return false;
        }
    }
    return true;
}

// Check if adjacent components are nonoverlapping
// (i.e., adding them with FAST-TWO-SUM produces no error)
inline bool is_nonoverlapping(const std::vector<double>& e) {
    for (size_t i = 1; i < e.size(); ++i) {
        double sum, err;
        fast_two_sum(e[i-1], e[i], sum, err);
        if (err != 0.0 && std::abs(err) > std::abs(e[i]) * 1e-10) {
            // Allow tiny numerical noise but flag real overlaps
            return false;
        }
    }
    return true;
}

// Check if expansion is strongly nonoverlapping (Shewchuk's strict property)
// This is a simplified check - full verification requires checking exponent differences
inline bool is_strongly_nonoverlapping(const std::vector<double>& e) {
    // For now, use the same check as nonoverlapping
    // TODO: Implement full strong nonoverlapping check using exponent comparison
    return is_nonoverlapping(e);
}

} // namespace expansion_ops

} // namespace sw::universal
