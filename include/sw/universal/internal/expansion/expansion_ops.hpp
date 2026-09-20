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
#include <cstddef>  // SIZE_MAX
#include <limits>   // std::numeric_limits
#include <type_traits>  // std::is_floating_point_v, std::type_identity_t

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
 * 3. Precision gain: Each component adds about digits bits of precision (53 for double)
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

// ============================================================================
// EXPANSION LIMB TYPES
// ============================================================================
/*
 * The algorithms below work on any binary floating-point type whose arithmetic is a
 * correctly rounded p-bit IEEE-754 format: float, double, x87 extended, binary128.
 * Their error-free transformations rest on that -- the roundoff of a sum or product
 * must itself be a representable value -- and nothing else in the file assumes double.
 *
 * is_iec559 alone is NOT enough to say so. IBM extended double-double, the default
 * long double on ppc64le, is a pair of doubles: it has no fixed precision, its roundoff
 * is not in general representable, and two_sum is not error-free on it. Yet libstdc++
 * reports is_iec559 == true for it (measured under QEMU, #1563). What does separate it
 * is a property IEEE-754 requires of every binary format: emin = 1 - emax, which in
 * numeric_limits terms is min_exponent == 3 - max_exponent. It holds for float
 * (-125), double (-1021), x87 and binary128 (-16381); double-double has min_exponent
 * -968 against max_exponent 1024.
 */
// the test on a type's numeric_limits, separated out so it can be checked against the
// parameters of a type the host does not have (IBM double-double on x86, say)
constexpr bool expansion_limb_properties(bool is_floating_point, bool is_iec559, int radix,
                                         int min_exponent, int max_exponent) {
    return is_floating_point && is_iec559 && radix == 2 && min_exponent == 3 - max_exponent;
}

template<typename T>
inline constexpr bool is_expansion_limb_v = expansion_limb_properties(
    std::is_floating_point_v<T>, std::numeric_limits<T>::is_iec559, std::numeric_limits<T>::radix,
    std::numeric_limits<T>::min_exponent, std::numeric_limits<T>::max_exponent);

// the check every error-free transformation below makes, with the reason in the message
template<typename FpType>
constexpr void require_expansion_limb() {
    static_assert(is_expansion_limb_v<FpType>,
        "expansion limbs must be a p-bit IEEE-754 binary type (float, double, x87 extended, "
        "binary128): the error-free transformations need the roundoff of every sum and product "
        "to be representable. IBM extended double-double -- the default long double on "
        "ppc64le -- is itself a two-component expansion and cannot serve as a limb; build "
        "with -mabi=ieeelongdouble for a binary128 long double there.");
}

// Where a leading component sits too close to the top of the limb type's range for an
// operation to run unscaled (#1553). Expressed relative to max_exponent, so that for
// double they are the values the range management was written with: 1000 and 1020,
// and -1100 as the exponent that stands in for zero.
template<typename FpType>
struct expansion_range_limits {
    static constexpr int product_limit = std::numeric_limits<FpType>::max_exponent - 24;
    static constexpr int sum_limit     = std::numeric_limits<FpType>::max_exponent - 4;
    static constexpr int zero_exponent = std::numeric_limits<FpType>::min_exponent
                                       - std::numeric_limits<FpType>::digits - 26;
};

namespace expansion_ops {

// Every function here is a template on the limb type, deduced from its arguments. The
// default, double, is what keeps the calls written before the templating working: a
// braced list such as is_nonoverlapping({1.0, 1e-17}) deduces nothing, and falls back
// to the double overload it always meant (#1563).

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
template<typename FpType = double>
inline void two_sum(std::type_identity_t<FpType> a, std::type_identity_t<FpType> b, FpType& x, FpType& y) {
    require_expansion_limb<FpType>();
    volatile FpType vx = a + b;
    x = vx;
    volatile FpType b_virtual = vx - a;
    volatile FpType a_virtual = vx - b_virtual;
    volatile FpType b_roundoff = b - b_virtual;
    volatile FpType a_roundoff = a - a_virtual;
    volatile FpType vy = a_roundoff + b_roundoff;
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
template<typename FpType = double>
inline void fast_two_sum(std::type_identity_t<FpType> a, std::type_identity_t<FpType> b, FpType& x, FpType& y) {
    require_expansion_limb<FpType>();
    volatile FpType vx = a + b;
    x = vx;
    volatile FpType vy = b - (vx - a);
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
 * With a correctly rounded FMA the error term is one operation:
 *   x = a * b
 *   y = fma(a, b, -x)  // Computes (a*b - x) with no intermediate rounding
 *
 * That is what float and double use: every platform the library targets has an FMA
 * instruction for them (CI's MinGW toolchain passes -mfma for exactly this reason).
 *
 * No platform has one for a type wider than double. x87 extended has no FMA at all, and
 * binary128 is software everywhere, so std::fma on long double is a library routine --
 * and not always a correct one: mingw-w64's fmal is not correctly rounded, which made
 * two_prod on x87 limbs inexact on MinGW while the same code passed on glibc (#1569).
 * Wider types therefore use Dekker's product with Veltkamp splitting instead, which is
 * exact with ordinary operations and needs nothing from libm.
 */
namespace detail_eft {

    // Veltkamp's split: a == hi + lo, each half holding at most ceil(p/2) significant bits,
    // so that the partial products below are exact
    template<typename FpType>
    inline void veltkamp_split(FpType a, FpType& hi, FpType& lo) {
        constexpr int s = (std::numeric_limits<FpType>::digits + 1) / 2;
        const FpType C = std::ldexp(FpType(1), s) + FpType(1);
        volatile FpType c = C * a;
        volatile FpType t = c - a;
        hi = c - t;
        lo = a - hi;
    }

    template<typename FpType>
    inline void dekker_two_prod(FpType a, FpType b, FpType& x, FpType& y) {
        constexpr int s = (std::numeric_limits<FpType>::digits + 1) / 2;
        constexpr int k = s + 2;
        // largest exponent the split can take: C * a must stay finite
        constexpr int big = std::numeric_limits<FpType>::max_exponent - k;
        if (a == FpType(0) || b == FpType(0) || !std::isfinite(a) || !std::isfinite(b)) {
            x = a * b;
            y = x - x;          // 0, or NaN for an infinite product, as fma(a, b, -x) gives
            return;
        }
        // A factor too large to split is moved down by 2^k and the other up by 2^k: the
        // product, and so x and y, are unchanged. If both are that large, the product
        // overflows whatever is done.
        if (std::ilogb(a) > big || std::ilogb(b) > big) {
            if (std::ilogb(a) > big && std::ilogb(b) > big - k) { x = a * b; y = x - x; return; }
            if (std::ilogb(b) > big && std::ilogb(a) > big - k) { x = a * b; y = x - x; return; }
            if (std::ilogb(a) > big) { a = std::ldexp(a, -k); b = std::ldexp(b, k); }
            else                     { b = std::ldexp(b, -k); a = std::ldexp(a, k); }
        }
        volatile FpType vx = a * b;
        x = vx;
        FpType ah, al, bh, bl;
        veltkamp_split(a, ah, al);
        veltkamp_split(b, bh, bl);
        volatile FpType e1 = ah * bh - x;
        volatile FpType e2 = e1 + ah * bl;
        volatile FpType e3 = e2 + al * bh;
        volatile FpType vy = e3 + al * bl;
        y = vy;
    }

}  // namespace detail_eft

template<typename FpType = double>
inline void two_prod(std::type_identity_t<FpType> a, std::type_identity_t<FpType> b, FpType& x, FpType& y) {
    require_expansion_limb<FpType>();
    if constexpr (std::numeric_limits<FpType>::digits > std::numeric_limits<double>::digits) {
        detail_eft::dekker_two_prod<FpType>(a, b, x, y);
    } else {
        volatile FpType vx = a * b;
        x = vx;
        volatile FpType vy = std::fma(a, b, -vx);
        y = vy;
    }
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
template<typename FpType = double>
inline std::vector<FpType> grow_expansion(const std::vector<FpType>& e, std::type_identity_t<FpType> b) {
    size_t m = e.size();
    std::vector<FpType> h(m + 1);

    FpType q = b;

    // Process from least significant (end) to most significant (beginning)
    // Using TWO-SUM since we don't know the relative magnitude of b vs e[i]
    for (size_t i = m; i-- > 0; ) {
        FpType q_new, h_i;
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
 * Based on Shewchuk's FastExpansionSum (Figure 8), but NOTE this implementation
 * is a magnitude-ordered merge that accumulates with TWO-SUM (not FAST-TWO-SUM)
 * and drops zero residuals; canonical non-overlapping form is delegated to
 * renormalize_expansion. It is NOT on the ereal hot path (ereal uses
 * linear_expansion_sum). See docs/bugs/ereal-priest-conformance-audit.md.
 *
 * Input:
 *   e - expansion with m components (strongly nonoverlapping)
 *   f - expansion with n components (strongly nonoverlapping)
 *
 * Output:
 *   h - expansion with m+n components (strongly nonoverlapping)
 *
 * Algorithm:
 *   Merge the two expansions like merge-sort, using TWO-SUM at each step
 *   (chosen over FAST-TWO-SUM because the merged operands are not guaranteed to
 *   be magnitude-ordered pairwise). The non-overlapping property is restored by
 *   the caller via renormalize_expansion.
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
template<typename FpType = double>
inline std::vector<FpType> fast_expansion_sum(const std::vector<FpType>& e, const std::vector<FpType>& f) {
    size_t m = e.size();
    size_t n = f.size();

    // Special cases
    if (m == 0) return f;
    if (n == 0) return e;

    std::vector<FpType> h;
    h.reserve(m + n);

    size_t i = 0, j = 0;
    FpType q = FpType(0);

    // Merge process: walk through both expansions from least to most significant
    // Note: Our expansions are stored in DECREASING order [most sig ... least sig]
    // So we walk backwards through the arrays

    // Pick the smaller component to start
    FpType e_curr = (m > 0) ? e[m - 1] : FpType(0);
    FpType f_curr = (n > 0) ? f[n - 1] : FpType(0);

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
        FpType next_component;

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

        FpType q_new, h_i;
        two_sum(q, next_component, q_new, h_i);  // Use TWO-SUM for correctness

        if (h_i != FpType(0)) {
            h.push_back(h_i);
        }
        q = q_new;
    }

    if (q != FpType(0)) {
        h.push_back(q);
    }

    // Ensure we always have at least one component (even if zero)
    // This maintains the ereal invariant: limb vector is never empty
    if (h.empty()) {
        h.push_back(FpType(0));
    }

    // Reverse to get decreasing magnitude order
    std::reverse(h.begin(), h.end());

    return h;
}

/*
 * LINEAR-EXPANSION-SUM: Alternative merging algorithm
 * ====================================================
 * Based on Shewchuk's LinearExpansionSum (Figure 7). NOTE this is not a literal
 * transcription of the Figure 7 (Q,q) recurrence: it is a magnitude-ordered
 * TWO-SUM merge that drops zero residuals. Its output is non-overlapping and
 * value-exact; ereal always follows it with renormalize_expansion to obtain
 * canonical Priest form. This is the add/subtract hot path for ereal.
 * See docs/bugs/ereal-priest-conformance-audit.md.
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
template<typename FpType = double>
inline std::vector<FpType> linear_expansion_sum(const std::vector<FpType>& e, const std::vector<FpType>& f) {
    size_t m = e.size();
    size_t n = f.size();

    // Special cases
    if (m == 0) return f;
    if (n == 0) return e;

    std::vector<FpType> h;
    h.reserve(m + n);

    // Initialize indices to point to least significant (last) components
    size_t i = m - 1;
    size_t j = n - 1;
    FpType q = FpType(0);

    // Similar merge process to FAST-EXPANSION-SUM but using TWO-SUM
    // Per Shewchuk Figure 7: Start with component having smaller magnitude
    FpType e_curr = e[i];
    FpType f_curr = f[j];

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
        FpType next_component;

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

        FpType q_new, h_i;
        two_sum(q, next_component, q_new, h_i);  // Using TWO-SUM (6 ops) not FAST (3 ops)

        if (h_i != FpType(0)) {
            h.push_back(h_i);
        }
        q = q_new;
    }

    if (q != FpType(0)) {
        h.push_back(q);
    }

    // Ensure we always have at least one component (even if zero)
    // This maintains the ereal invariant: limb vector is never empty
    if (h.empty()) {
        h.push_back(FpType(0));
    }

    // Reverse to get decreasing magnitude order
    std::reverse(h.begin(), h.end());

    return h;
}

// ============================================================================
// EXPANSION RENORMALIZATION
// ============================================================================

/*
 * RENORMALIZE-EXPANSION: produce Priest's canonical non-overlapping form.
 * ==============================================================================
 * Earlier versions of this function used grow_expansion sequentially. That
 * produced non-overlapping output but NOT value-canonical output: two
 * expansions representing the same real value could renormalize to
 * different limb sequences depending on input order. That broke
 * associativity in `compare_adaptive`-based equality (issue #959).
 *
 * The Priest canonical algorithm (sweepUp -> recurse -> sweepDown) produces
 * the same limb sequence for value-equal inputs, restoring associativity.
 *
 * Input:
 *   e - expansion in decreasing magnitude order (may overlap, may have zeros)
 *
 * Output:
 *   h - expansion in canonical Priest form:
 *       * Non-overlapping: |h[i+1]| <= ulp(h[i]) / 2
 *       * Decreasing magnitude
 *       * No zero components (unless h represents zero, in which case h is
 *         empty by convention; callers that need a canonical zero limb
 *         re-add it explicitly)
 *       * Value-canonical: equal-valued inputs produce equal outputs
 *
 * Reference: Priest 1992 (renormalize for finite expansions); the algorithm
 * also appears as sweepUp/sweepDown in elreal's threeAdd (Phase 4 of the
 * McCleeary work) and in Bailey/Hida's qd Renormalize.
 *
 * Cost: O(m^2) two_sum operations in the worst case (m sweepUp + recursive
 * priest_renormalize on the tail). For ereal<maxlimbs <= 19> this is at
 * most ~190 two_sum calls per renormalize -- still O(1) for fixed maxlimbs.
 */
namespace detail_priest {

    // sweepUpRec: cumulative twoSum from the back; emits residuals as we go
    // and the final carry as the leading element of the (reversed) result.
    template<typename FpType = double>
    inline std::vector<FpType>
    sweepUpRec(const std::vector<FpType>& as, size_t pos, FpType b) {
        if (pos == as.size()) return { b };
        FpType s, e;
        two_sum(as[pos], b, s, e);
        std::vector<FpType> rest = sweepUpRec(as, pos + 1, s);
        std::vector<FpType> result;
        result.reserve(rest.size() + 1);
        result.push_back(e);
        result.insert(result.end(), rest.begin(), rest.end());
        return result;
    }

    // sweepUp: build a "loose" non-overlapping form by sweeping from least to
    // most significant. Returns the result in decreasing magnitude order.
    template<typename FpType = double>
    inline std::vector<FpType> sweepUp(const std::vector<FpType>& as) {
        if (as.empty()) return {};
        // Walk the input back-to-front by passing a position index; the
        // recursion produces the residual-first sequence, which we reverse
        // at the end to get decreasing magnitude order.
        std::vector<FpType> reversed_input(as.rbegin(), as.rend());
        FpType ra = reversed_input.front();
        std::vector<FpType> result = sweepUpRec(reversed_input, 1, ra);
        std::reverse(result.begin(), result.end());
        return result;
    }

    template<typename FpType = double>
    inline std::vector<FpType> sweepDown(const std::vector<FpType>& as);

    template<typename FpType = double>

    inline std::vector<FpType>
    sweepDownRec(const std::vector<FpType>& as, size_t pos, FpType b) {
        if (pos == as.size()) return { b };
        FpType s, e;
        two_sum(as[pos], b, s, e);
        std::vector<FpType> tail;
        if (e == FpType(0)) {
            // Re-enter sweepDown on the remaining tail (effectively dropping
            // the zero residual).
            std::vector<FpType> remaining(as.begin() + pos + 1, as.end());
            tail = sweepDown(remaining);
        } else {
            tail = sweepDownRec(as, pos + 1, e);
        }
        std::vector<FpType> result;
        result.reserve(tail.size() + 1);
        result.push_back(s);
        result.insert(result.end(), tail.begin(), tail.end());
        return result;
    }

    template<typename FpType>
    inline std::vector<FpType> sweepDown(const std::vector<FpType>& as) {
        if (as.empty()) return {};
        return sweepDownRec(as, 1, as.front());
    }

    template<typename FpType = double>
    inline std::vector<FpType> remove_zeros(const std::vector<FpType>& xs) {
        std::vector<FpType> out;
        out.reserve(xs.size());
        for (FpType v : xs) {
            if (v != FpType(0)) out.push_back(v);
        }
        return out;
    }

    template<typename FpType = double>

    inline std::vector<FpType>
    priest_renormalize(std::vector<FpType> as) {
        if (as.empty()) return {};
        std::vector<FpType> up = sweepUp(as);
        if (up.empty()) return {};
        FpType f = up.front();
        std::vector<FpType> fs(up.begin() + 1, up.end());
        std::vector<FpType> recursed = priest_renormalize(std::move(fs));
        std::vector<FpType> cleaned = remove_zeros(recursed);
        std::vector<FpType> down = sweepDown(cleaned);
        std::vector<FpType> result;
        result.reserve(down.size() + 1);
        result.push_back(f);
        result.insert(result.end(), down.begin(), down.end());
        return result;
    }

} // namespace detail_priest

template<typename FpType = double>
inline std::vector<FpType> renormalize_expansion(const std::vector<FpType>& e) {
    if (e.empty()) return {};
    if (e.size() == 1) return std::vector<FpType>{ e[0] }; // Single-component input is trivially canonical.
    std::vector<FpType> result = detail_priest::priest_renormalize(e);
    // Strip trailing zeros (matches the historical contract of this function).
    while (!result.empty() && result.back() == FpType(0)) {
        result.pop_back();
    }
    return result;
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
template<typename FpType = double>
inline std::vector<FpType> scale_expansion(const std::vector<FpType>& e, std::type_identity_t<FpType> b) {
    if (e.empty()) return std::vector<FpType>();
    if (b == FpType(0)) return std::vector<FpType>{FpType(0)};
    if (b == FpType(1)) return e;
    if (b == FpType(-1)) {
        std::vector<FpType> result = e;
        for (auto& v : result) v = -v;
        return result;
    }

    size_t m = e.size();
    std::vector<FpType> products;
    products.reserve(2 * m);

    // Multiply each component by b, collecting products and errors
    for (size_t i = 0; i < m; ++i) {
        FpType product, error;
        two_prod(b, e[i], product, error);

        if (product != FpType(0)) products.push_back(product);
        if (error != FpType(0)) products.push_back(error);
    }

    // Sort by decreasing magnitude (most significant first)
    std::sort(products.begin(), products.end(), [](FpType a, FpType b) {
        return std::abs(a) > std::abs(b);
    });

    // Renormalize to ensure non-overlapping property
    // The sorted products may have overlapping components (adjacent components
    // with insufficient magnitude separation). Renormalization uses two_sum
    // to extract non-overlapping parts and ensures Shewchuk invariants.
    return renormalize_expansion(products);
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
 *   h - compressed expansion with <= m components
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
template<typename FpType = double>
inline std::vector<FpType> compress_expansion(const std::vector<FpType>& e, std::type_identity_t<FpType> epsilon = FpType(0)) {
    if (e.empty()) return e;

    // Find largest magnitude for relative threshold
    FpType max_magnitude = FpType(0);
    for (const auto& component : e) {
        FpType mag = std::abs(component);
        if (mag > max_magnitude) max_magnitude = mag;
    }

    if (max_magnitude == FpType(0)) {
        // All zeros
        return std::vector<FpType>{FpType(0)};
    }

    FpType threshold = epsilon * max_magnitude;

    std::vector<FpType> compressed;
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
template<typename FpType = double>
inline std::vector<FpType> compress_to_n(const std::vector<FpType>& e, size_t max_components) {
    if (e.size() <= max_components) return e;

    std::vector<FpType> compressed(e.begin(), e.begin() + max_components);
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
template<typename FpType = double>
inline int sign_adaptive(const std::vector<FpType>& e) {
    for (const auto& component : e) {
        if (component > FpType(0)) return 1;
        if (component < FpType(0)) return -1;
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
/*
 * RANGE MANAGEMENT AT THE TOP OF THE LIMB TYPE'S RANGE (#1553)
 * ==============================================================
 *
 * The error-free transformations underneath these operations compute a rounded
 * result and then its roundoff. When the rounded LEADING term overflows, the roundoff
 * comes out as inf - inf = NaN, and renormalization spreads that NaN through every
 * component: DBL_MAX + DBL_MAX was [nan, nan] rather than inf. The same happens inside
 * an algorithm whose true result is in range -- DBL_MAX / 2 was NaN, because the
 * product with the divisor's reciprocal overflowed on the way.
 *
 * So each operation near the top of the range scales its operands down by an exact
 * power of two, computes where nothing can overflow, and scales the result back up.
 * Rounding commutes with power-of-two scaling, so this is exact; and a result whose
 * leading component overflows on the way back up is a genuine overflow, for which the
 * IEEE answer is a single signed infinity. The components beneath it would be
 * meaningless, so they are dropped.
 */
namespace expansion_range {

    // scale every component by 2^s: exact unless a component leaves the normal range
    template<typename FpType = double>
    inline std::vector<FpType> scaled(const std::vector<FpType>& e, int s) {
        std::vector<FpType> r(e.size());
        for (std::size_t i = 0; i < e.size(); ++i) r[i] = std::ldexp(e[i], s);
        return r;
    }

    // a leading component that overflowed on the way back up is the whole answer
    // Split e into the components that scaling by 2^-s leaves exact (hi) and those it would
    // not (lo): a component whose low bits would fall below the smallest subnormal. Only the
    // hi part may go through a scaled computation; the lo part is added back unscaled.
    // Dropping it -- as the range management did at first -- lost exact bits:
    // {max, denorm_min} + {-max} came out 0 rather than denorm_min. A component failing the
    // test has every component below it failing too, so lo is always a suffix of e, and both
    // parts keep e's decreasing order.
    template<typename FpType = double>
    inline void split_for_scaling(const std::vector<FpType>& e, int s, std::vector<FpType>& hi, std::vector<FpType>& lo) {
        for (FpType c : e) {
            if (std::ldexp(std::ldexp(c, -s), s) == c) hi.push_back(c); else lo.push_back(c);
        }
    }

    template<typename FpType = double>
    inline std::vector<FpType> canonical_overflow(std::vector<FpType> r) {
        if (!r.empty() && std::isinf(r[0])) return std::vector<FpType>{ r[0] };
        return r;
    }

    // exponent of an expansion's leading component, or a floor for zero / non-finite
    template<typename FpType = double>
    inline int leading_exponent(const std::vector<FpType>& e) {
        if (e.empty() || e[0] == FpType(0) || !std::isfinite(e[0])) return expansion_range_limits<FpType>::zero_exponent;
        return std::ilogb(e[0]);
    }

}

// expansion_product without range management: the caller guarantees the product of the
// leading components cannot overflow
template<typename FpType = double>
inline std::vector<FpType> expansion_product_in_range(const std::vector<FpType>& e, const std::vector<FpType>& f) {
    if (e.empty() || f.empty()) return std::vector<FpType>{FpType(0)};

    // Handle zero cases
    if ((e.size() == 1 && e[0] == FpType(0)) || (f.size() == 1 && f[0] == FpType(0))) {
        return std::vector<FpType>{FpType(0)};
    }

    // Start with zero
    std::vector<FpType> result{FpType(0)};

    // For each component in e, scale f and accumulate
    for (const auto& e_component : e) {
        if (e_component != FpType(0)) {
            std::vector<FpType> scaled = scale_expansion(f, e_component);
            result = linear_expansion_sum(result, scaled);
        }
    }

    // Renormalize the accumulated partial products into Priest canonical form.
    // linear_expansion_sum keeps the running sum ordered but leaves overlapping,
    // uncompressed components; without this final renormalize the product (and
    // division, which is e * reciprocal) returns a non-canonical expansion that
    // violates the non-overlapping invariant (issue #981). renormalize_expansion
    // is exactly value-preserving.
    result = renormalize_expansion(result);
    if (result.empty()) result.push_back(FpType(0));  // canonical zero
    return result;
}

template<typename FpType = double>
inline std::vector<FpType> expansion_product(const std::vector<FpType>& e, const std::vector<FpType>& f) {
    using namespace expansion_range;
    // Leading components at 2^a and 2^b multiply to at most 2^(a+b+2). Below a combined
    // exponent of 1000 nothing can overflow, and the product runs unscaled, exactly as it
    // always has. Above it, the excess is taken off both operands -- split between them,
    // so neither has its smallest components pushed toward the subnormal range -- and put
    // back on the result.
    const int combined = leading_exponent(e) + leading_exponent(f);
    constexpr int limit = expansion_range_limits<FpType>::product_limit;   // 1000 for double
    if (combined <= limit) return expansion_product_in_range(e, f);
    const int excess = combined - limit;
    const int se = excess / 2;
    const int sf = excess - se;
    // e * f = ehi * fhi, scaled, plus the cross terms with the parts scaling would not keep;
    // those involve a tiny factor, so they run unscaled and cannot overflow
    std::vector<FpType> ehi, elo, fhi, flo;
    split_for_scaling(e, se, ehi, elo);
    split_for_scaling(f, sf, fhi, flo);
    std::vector<FpType> result = canonical_overflow(scaled(expansion_product_in_range(scaled(ehi, -se), scaled(fhi, -sf)), excess));
    if (elo.empty() && flo.empty()) return result;
    if (!result.empty() && std::isinf(result[0])) return result;
    if (!elo.empty())                 result = linear_expansion_sum(result, expansion_product_in_range(elo, f));
    if (!flo.empty() && !ehi.empty()) result = linear_expansion_sum(result, expansion_product_in_range(ehi, flo));
    result = renormalize_expansion(result);
    if (result.empty()) result.push_back(FpType(0));  // canonical zero
    return result;
}

/*
 * EXPANSION-SUM (range managed): the sum of two expansions, renormalized
 * =====================================================================
 *
 * linear_expansion_sum followed by renormalize_expansion, with the operands scaled
 * down first when either leading component is within a factor of four of overflow:
 * two values below 2^1021 cannot sum past 2^1022, so a shift of at most three bits
 * is all the headroom a sum ever needs (#1553).
 */
template<typename FpType = double>
inline std::vector<FpType> expansion_sum_normalized(const std::vector<FpType>& e, const std::vector<FpType>& f) {
    using namespace expansion_range;
    const int top = std::max(leading_exponent(e), leading_exponent(f));
    constexpr int limit = expansion_range_limits<FpType>::sum_limit;   // 1020 for double
    if (top <= limit) return renormalize_expansion(linear_expansion_sum(e, f));
    const int shift = top - limit;
    // only the components the shift keeps exact are summed scaled; the rest are added back
    std::vector<FpType> ehi, elo, fhi, flo;
    split_for_scaling(e, shift, ehi, elo);
    split_for_scaling(f, shift, fhi, flo);
    std::vector<FpType> result = canonical_overflow(scaled(renormalize_expansion(linear_expansion_sum(scaled(ehi, -shift), scaled(fhi, -shift))), shift));
    if (elo.empty() && flo.empty()) return result;
    if (!result.empty() && std::isinf(result[0])) return result;
    return renormalize_expansion(linear_expansion_sum(result, linear_expansion_sum(elo, flo)));
}

/*
 * Truncate an expansion to a limb budget. Safe for any expansion in Priest normal form:
 * the components descend in magnitude and do not overlap, so a prefix is the leading-order
 * value, and what is dropped lies below the retained precision. A budget of 0 means no
 * bound. (#1572)
 */
template<typename FpType = double>
inline void truncate_expansion(std::vector<FpType>& e, std::size_t budget) {
    if (budget != 0 && e.size() > budget) e.resize(budget);
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
template<typename FpType = double>
inline std::vector<FpType> expansion_reciprocal(const std::vector<FpType>& e, int iterations = 3, std::size_t budget = 0) {
    if (e.empty() || (e.size() == 1 && e[0] == FpType(0))) {
        // Division by zero - return inf (or could throw)
        return std::vector<FpType>{std::numeric_limits<FpType>::infinity()};
    }

    // Initial approximation: 1 / first component
    FpType r0 = FpType(1) / e[0];
    std::vector<FpType> result{r0};

    // Newton iteration: r_{n+1} = r_n * (2 - e * r_n)
    //
    // Each iteration squares the iterate's limb count, and nothing in the loop prunes it.
    // With double limbs the growth stops by accident -- the trailing components fall below
    // the smallest normal and renormalization drops them -- but a wide-exponent limb has no
    // such floor, so the iterate grew until it spanned the whole exponent range: 250 limbs
    // for x87, whatever precision the caller actually wanted, at seconds per division
    // (#1572).
    //
    // `budget` bounds the working precision. Truncating an expansion in Priest normal form
    // discards only low-order bits, and Newton's iteration is self-correcting: an iterate
    // truncated to b limbs still converges to b limbs of accuracy, because each step
    // recomputes the residual (2 - e*r) from scratch rather than accumulating it. The
    // truncation is applied to the intermediates as well as the iterate, since e * r_n is
    // where the quadratic blowup actually lands.
    //
    // budget == 0 means unbounded, which is what the non-ereal callers get.
    //
    // The bound tightens as the iteration proceeds. Newton doubles the correct digits each
    // step, so from a one-limb seed the iterate after step i is accurate to 2^(i+1) limbs
    // and the limbs past that are noise. Carrying them is not just wasteful, it is the
    // whole cost: each step multiplies e by the iterate, so a full-width iterate makes
    // every step pay a full budget-by-budget product.
    //
    // The step widths therefore grow geometrically and saturate at the budget: the early
    // steps are cheap and only the last step or two run at full width (how many depends on
    // how far 2^iterations overshoots the budget). The total is a geometric sum, roughly
    // twice the final width rather than iterations times it.
    std::vector<FpType> two{FpType(2)};
    for (int i = 0; i < iterations; ++i) {
        std::vector<FpType> product = expansion_product(e, result);  // e * r_n
        std::vector<FpType> diff = linear_expansion_sum(two, scale_expansion(product, FpType(-1)));  // 2 - e * r_n
        result = expansion_product(result, diff);  // r_n * (2 - e * r_n)
        // 2^(i+1) plus two guard limbs: the doubling is the asymptotic rate, and rounding
        // within a step eats into it, so trimming to exactly 2^(i+1) cost the last couple
        // of digits (ereal<8>'s Newton sqrt(2) went from 257 digits to 255).
        // Saturating rather than overflowing for a large iteration count.
        const std::size_t reached = (i < 30) ? ((std::size_t{1} << (i + 1)) + 2) : budget;
        const std::size_t step_budget = (budget == 0) ? 0 : ((reached < budget) ? reached : budget);
        truncate_expansion(result, step_budget);
    }

    return result;
}

/*
 * EXPANSION-QUOTIENT: Divide two expansions
 * ==========================================
 *
 * Algorithm: Compute e / f = (e * (1/f')) * 2^-k, where f = f' * 2^k with
 * f' in [0.5, 1).
 *
 * Why scale the divisor to near-unit magnitude first: forming 1/f directly
 * fails when |f| is large. For example 1/10^300 ~= 1e-300, whose second
 * expansion component (~1e-316) is below DBL_MIN and therefore subnormal --
 * which Shewchuk's algorithms forbid -- so the reciprocal collapses to ~1
 * normal component (~16 correct digits). The reciprocal accuracy degrades by
 * ~1 decimal digit per decimal order of magnitude of the divisor (issue #1006).
 *
 * Scaling f to f' in [0.5, 1) by an exact power of two (ldexp, lossless) keeps
 * the Newton reciprocal operating on a near-unit, fully-normal operand whose
 * result is also near-unit and fully representable. The product e * (1/f') is
 * formed in normal range, and only then is the exact 2^-k applied. The quotient
 * therefore carries full precision whenever the *result* is in normal range
 * (e.g. parsing 0.142857... = M / 10^e); it can only lose precision when the
 * quotient itself is near DBL_MIN, which is an inherent representational limit,
 * not an algorithmic one.
 *
 * Input:
 *   e - numerator expansion
 *   f - denominator expansion (must be non-zero, finite)
 *
 * Output:
 *   h - expansion representing e / f
 */
template<typename FpType = double>
inline std::vector<FpType> expansion_quotient(const std::vector<FpType>& e, const std::vector<FpType>& f, int iterations = 3, std::size_t budget = 0) {
    // Divide-by-zero / non-finite divisor: fall back to the direct reciprocal,
    // which yields the IEEE special value (Inf/NaN) the callers expect.
    if (f.empty() || f[0] == FpType(0) || !std::isfinite(f[0])) {
        std::vector<FpType> reciprocal = expansion_reciprocal(f, iterations, budget);
        return expansion_product(e, reciprocal);
    }
    // f = f' * 2^k with f' in [0.5, 1): k = ilogb(f[0]) + 1.
    int k = std::ilogb(f[0]) + 1;
    std::vector<FpType> fscaled(f.size());
    for (std::size_t i = 0; i < f.size(); ++i) fscaled[i] = std::ldexp(f[i], -k);
    std::vector<FpType> reciprocal = expansion_reciprocal(fscaled, iterations, budget);
    // The reciprocal of f' lies in (1, 2], so the product below can be up to twice the
    // dividend. For a dividend near the top of double's range that product overflowed
    // even when the true quotient is nowhere near the limit: DBL_MAX / 2 and DBL_MAX / 1e10
    // both came back NaN in every limb (#1553). A large dividend is scaled down first --
    // only as far as needed, so its smallest components are not pushed into the subnormal
    // range -- and the scaling is undone by the same exact ldexp that undoes 2^k.
    const int headroom = (!e.empty() && e[0] != FpType(0) && std::isfinite(e[0]))
                       ? std::max(0, std::ilogb(e[0]) - expansion_range_limits<FpType>::product_limit) : 0;
    std::vector<FpType> escaled(e.size());
    for (std::size_t i = 0; i < e.size(); ++i) escaled[i] = std::ldexp(e[i], -headroom);
    std::vector<FpType> quotient = expansion_product(escaled, reciprocal);
    truncate_expansion(quotient, budget);
    for (auto& v : quotient) v = std::ldexp(v, headroom - k);  // exact: * 2^(headroom - k)
    // a leading component that overflowed on the way back is a genuine overflow
    if (!quotient.empty() && std::isinf(quotient[0])) return std::vector<FpType>{ quotient[0] };
    // ldexp can underflow the smallest components to 0; renormalize to strip
    // those zeros and restore Priest canonical (non-overlapping, no interior
    // zero) form.
    quotient = renormalize_expansion(quotient);
    if (quotient.empty()) quotient.push_back(FpType(0));  // canonical zero
    return quotient;
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
template<typename FpType = double>
inline int compare_adaptive(const std::vector<FpType>& e, const std::vector<FpType>& f) {
    // Strategy: Walk through both expansions in decreasing magnitude order
    // comparing corresponding components until we find a difference

    size_t i = 0, j = 0;

    while (i < e.size() || j < f.size()) {
        FpType e_val = (i < e.size()) ? e[i] : FpType(0);
        FpType f_val = (j < f.size()) ? f[j] : FpType(0);

        // Compare absolute magnitudes to decide which to examine
        FpType e_mag = std::abs(e_val);
        FpType f_mag = std::abs(f_val);

        if (e_mag > f_mag) {
            // e has larger magnitude component
            if (e_val > FpType(0)) return 1;   // e > f
            if (e_val < FpType(0)) return -1;  // e < f
            ++i;
        } else if (f_mag > e_mag) {
            // f has larger magnitude component
            if (f_val > FpType(0)) return -1;  // e < f
            if (f_val < FpType(0)) return 1;   // e > f
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
 * Returns a limb-precision (FpType) approximation of the expansion's value by
 * summing the first few components.
 *
 * This is NOT exact - it loses the precision of the tail components.
 * Use for quick estimates, not for exact computation.
 */
template<typename FpType = double>
inline FpType estimate(const std::vector<FpType>& e) {
    if (e.empty()) return FpType(0);

    // Sum first few components for a reasonable estimate
    FpType sum = FpType(0);
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
template<typename FpType = double>
inline bool is_decreasing_magnitude(const std::vector<FpType>& e) {
    for (size_t i = 1; i < e.size(); ++i) {
        if (std::abs(e[i-1]) < std::abs(e[i])) {
            return false;
        }
    }
    return true;
}

// Check if adjacent components are nonoverlapping.
//
// Nonoverlapping (Shewchuk / Priest): each component's significant bits lie
// strictly below the previous component's least significant bit, i.e. for a
// decreasing-magnitude expansion |e[i]| <= ulp(e[i-1])/2 == 2^(ilogb(e[i-1]) - p)
// for a normal e[i-1], with p = numeric_limits<FpType>::digits (53 for double). (Equivalent to the verified check_priest_normal
// gap test in verification/ereal_test_support.hpp.)
//
// NOTE: a prior implementation used a fast_two_sum error heuristic that was
// inverted -- it flagged genuinely non-overlapping pairs (the small component
// passes through fast_two_sum unchanged as the error term) as overlapping, and
// declared exactly-combining overlapping pairs non-overlapping (issue #999).
template<typename FpType = double>
inline bool is_nonoverlapping(const std::vector<FpType>& e) {
    for (size_t i = 1; i < e.size(); ++i) {
        FpType prev = e[i - 1];
        FpType cur  = e[i];
        if (cur == FpType(0)) continue;                       // a zero component never overlaps
        if (!std::isfinite(prev) || !std::isfinite(cur)) return false;
        if (prev == FpType(0)) return false;                  // nonzero below a zero: not canonical
        // |cur| <= ulp(prev)/2 ; ilogb(prev) is well-defined here (prev finite, nonzero)
        if (std::abs(cur) > std::ldexp(FpType(1), std::ilogb(prev) - std::numeric_limits<FpType>::digits)) return false;
    }
    return true;
}

// Check if expansion is strongly nonoverlapping (Shewchuk's strict property).
//
// Strongly nonoverlapping == nonoverlapping AND, for any adjacent pair whose
// exponents are exactly one ulp apart (the smaller sits right at the previous
// component's half-ulp boundary, ilogb(cur) == ilogb(prev) - p), the smaller
// component must be a power of two. A full bit of separation
// (ilogb(cur) <= ilogb(prev) - 54) imposes no extra constraint.
template<typename FpType = double>
inline bool is_strongly_nonoverlapping(const std::vector<FpType>& e) {
    for (size_t i = 1; i < e.size(); ++i) {
        FpType prev = e[i - 1];
        FpType cur  = e[i];
        if (cur == FpType(0)) continue;
        if (!std::isfinite(prev) || !std::isfinite(cur)) return false;
        if (prev == FpType(0)) return false;
        int eprev = std::ilogb(prev);
        int ecur  = std::ilogb(cur);
        constexpr int p = std::numeric_limits<FpType>::digits;   // 53 for double
        if (ecur > eprev - p) return false;             // overlapping
        if (ecur == eprev - p) {                        // adjacent: smaller must be a power of two
            if (std::abs(cur) != std::ldexp(FpType(1), ecur)) return false;
        }
    }
    return true;
}

} // namespace expansion_ops

} // namespace sw::universal
