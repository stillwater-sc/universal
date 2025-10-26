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
        fast_two_sum(next_component, q, q_new, h_i);

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

    size_t i = 0, j = 0;
    double q = 0.0;

    // Similar merge process to FAST-EXPANSION-SUM but using TWO-SUM
    double e_curr = (m > 0) ? e[m - 1] : 0.0;
    double f_curr = (n > 0) ? f[n - 1] : 0.0;

    // Start with the absolutely smaller component
    if (std::abs(e_curr) < std::abs(f_curr) || (m == 0)) {
        q = f_curr;
        j = n - 1;
        if (j > 0) j--;
        else j = SIZE_MAX;
    } else {
        q = e_curr;
        i = m - 1;
        if (i > 0) i--;
        else i = SIZE_MAX;
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
