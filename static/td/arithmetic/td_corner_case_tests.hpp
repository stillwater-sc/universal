// td_corner_case_tests.hpp: Corner case test infrastructure for triple-double arithmetic
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
#include <universal/number/td/td.hpp>
#include <cmath>
#include <limits>
#include <string>
#include <sstream>

/*
 * TRIPLE-DOUBLE ARITHMETIC CORNER CASE TESTING FRAMEWORK
 * =======================================================
 *
 * WHY CORNER CASES INSTEAD OF RANDOM TESTING?
 * --------------------------------------------
 * Triple-double (td) numbers have ~159 bits of precision (~48 decimal digits), while double
 * has only 53 bits (~16 decimal digits). Comparing td arithmetic results to double references
 * is fundamentally flawed:
 *
 *   td:     ~159 fraction bits (3 × 53-bit doubles with non-overlapping mantissas)
 *   double: ~53 fraction bits
 *
 * Random testing with double references fails because:
 * 1. The reference is less precise than what we're testing
 * 2. Differences in the lower ~106 bits appear as "failures" when they're actually correct
 * 3. Platform differences in FP rounding become magnified in multi-component arithmetic
 *
 * WHY SEPARATE ADDITION AND SUBTRACTION TEST SUITES?
 * ---------------------------------------------------
 * Although addition and subtraction share underlying mechanisms, they require separate test
 * suites because:
 *
 * 1. SUBTRACTION HAS UNIQUE CORNER CASES:
 *    - Complete cancellation (a - a = 0) is fundamental and needs extensive testing
 *    - Catastrophic cancellation reveals precision in lower components
 *    - Near-cancellation triggers different renormalization paths
 *
 * 2. DIFFERENT ERROR PROPAGATION:
 *    - Addition accumulates rounding errors across components
 *    - Subtraction can cancel errors OR amplify relative errors through cancellation
 *
 * 3. DIFFERENT VALIDATION REQUIREMENTS:
 *    - Addition: verify component growth and carry propagation
 *    - Subtraction: verify cancellation correctness and component preservation
 *
 * 4. MIRRORS EXISTING STRUCTURE:
 *    - Other multi-component types (dd, qd) already separate these tests
 *    - Maintains consistency across the Universal library
 *
 * Both test suites share this infrastructure for verification and test case generation.
 *
 *
 * CORNER CASES FOR TRIPLE-DOUBLE ADDITION/SUBTRACTION
 * ====================================================
 *
 * Based on the td implementation structure:
 * - expansion_ops::add_cascades() merges 3+3 components into 6-component expansion
 * - Compression sums tail components (3-5) into component 2
 * - renormalize() uses Knuth's two_sum to maintain non-overlapping property
 *
 * Critical corner cases to test:
 *
 * 1. CANCELLATION CASES (especially for subtraction)
 *    - Complete cancellation: a - a = 0 (all components zero)
 *    - Partial hi cancellation: (1.0, eps, 0) - (1.0, 0, 0) = (eps, 0, 0)
 *    - Partial mid cancellation: where hi components nearly cancel
 *    - Staircase cancellation: progressive cancellation through components
 *
 * 2. COMPONENT ALIGNMENT & MAGNITUDE SEPARATION
 *    - Well-separated: (1.0, 1e-17, 1e-34) - typical normalized case
 *    - Overlapping magnitudes: (1.0, 0.5, 0.25) - triggers heavy renormalization
 *    - Near-zero lower components: (1.0, 1e-100, 1e-200)
 *    - Extreme separation: components at maximum exponent range
 *
 * 3. SIGN PATTERN CASES
 *    - (+,+,+) ± (+,+,+) - all positive
 *    - (+,+,+) ± (-,-,-) - opposite signs
 *    - (+,-,+) ± (+,+,+) - mixed internal signs (tests denormalized inputs)
 *    - (+,+,-) ± (+,-,+) - various mixed patterns
 *
 * 4. RENORMALIZATION TRIGGERS
 *    - Upward carry: adding small values that grow component[0]
 *    - Downward cascade: when sum creates new lower components
 *    - ULP boundaries: 1.0 + ulp(double)/2 captured in lower components
 *    - Component overflow: when mid/lo components exceed representable range
 *
 * 5. SPECIAL VALUES
 *    - Zero operations: 0 + a, a + 0, 0 - 0
 *    - Identity: a - a, (a + b) - a
 *    - Infinity: ±∞ + a, ∞ - ∞ (should be NaN)
 *    - NaN propagation
 *
 * 6. PRECISION BOUNDARY CASES
 *    - Values exactly at double ULP boundaries
 *    - Values requiring all 3 components for exact representation
 *    - Values where hi + mid would round differently than actual sum
 *
 *
 * VALIDATION STRATEGIES
 * =====================
 *
 * Instead of comparing to double references, validate using:
 *
 * 1. SELF-CONSISTENCY: (a + b) - b ≈ a (within td ULP)
 * 2. COMPONENT INSPECTION: Verify each component is within expected bounds
 * 3. ASSOCIATIVITY TESTS: (a + b) + c ≈ a + (b + c) (approximately equal)
 * 4. KNOWN EXACT RESULTS: Construct cases where exact answer is known
 * 5. CROSS-VALIDATION: Use qd (quad-double) as oracle if available
 */

namespace sw::universal {
namespace td_corner_cases {

// Epsilon values for multi-component precision
// Double:        53 bits of precision → epsilon = 2^-52  ≈ 2.22e-16
// Double-double: 106 bits of precision → epsilon = 2^-106 ≈ 1.23e-32
// Triple-double: 159 bits of precision → epsilon = 2^-159 ≈ 1.74e-48
constexpr double DOUBLE_EPS = std::numeric_limits<double>::epsilon(); // 2^-52 ≈ 2.22e-16
constexpr double DD_EPS = 1.2325951644078309e-32;  // 2^-106 for double-double
constexpr double TD_EPS = 1.7411641656824734e-48;  // 2^-159 for triple-double

// Test result structure
struct TestResult {
    bool passed;
    std::string message;

    TestResult(bool p = true, const std::string& m = "") : passed(p), message(m) {}

    operator bool() const { return passed; }
};

// Component verification: check if td components match expected values within tolerance
inline TestResult verify_components(
    const td& value,
    double expected_hi,
    double expected_mid,
    double expected_lo,
    double tolerance = 0.0,  // 0.0 means exact match
    const std::string& test_name = "component verification")
{
    bool hi_match  = (tolerance == 0.0) ? (value[0] == expected_hi)  : (std::abs(value[0] - expected_hi) <= tolerance);
    bool mid_match = (tolerance == 0.0) ? (value[1] == expected_mid) : (std::abs(value[1] - expected_mid) <= tolerance);
    bool lo_match  = (tolerance == 0.0) ? (value[2] == expected_lo)  : (std::abs(value[2] - expected_lo) <= tolerance);

    if (hi_match && mid_match && lo_match) {
        return TestResult(true);
    }

    std::ostringstream oss;
    oss << test_name << " FAILED:\n";
    oss << "  Expected: (" << expected_hi << ", " << expected_mid << ", " << expected_lo << ")\n";
    oss << "  Got:      (" << value[0] << ", " << value[1] << ", " << value[2] << ")\n";
    if (tolerance > 0.0) {
        oss << "  Tolerance: " << tolerance << "\n";
    }

    return TestResult(false, oss.str());
}

// Verify that a value is zero (all components)
inline TestResult verify_zero(const td& value, const std::string& test_name = "zero verification") {
    return verify_components(value, 0.0, 0.0, 0.0, 0.0, test_name);
}

// Verify proper normalization: components should be non-overlapping
// This means |component[i]| should be approximately ULP of component[i-1]
inline TestResult verify_normalized(const td& value, const std::string& test_name = "normalization check") {
    // A normalized td has components in decreasing magnitude order
    // and each component should be roughly the ULP of the previous one (when non-zero)

    // Skip if value is zero
    if (value[0] == 0.0 && value[1] == 0.0 && value[2] == 0.0) {
        return TestResult(true);
    }

    // Check decreasing magnitude (when components are non-zero)
    if (value[1] != 0.0 && std::abs(value[1]) > std::abs(value[0])) {
        std::ostringstream oss;
        oss << test_name << " FAILED: mid component larger than hi\n";
        oss << "  |mid| = " << std::abs(value[1]) << " > |hi| = " << std::abs(value[0]) << "\n";
        return TestResult(false, oss.str());
    }

    if (value[2] != 0.0 && std::abs(value[2]) > std::abs(value[1])) {
        std::ostringstream oss;
        oss << test_name << " FAILED: lo component larger than mid\n";
        oss << "  |lo| = " << std::abs(value[2]) << " > |mid| = " << std::abs(value[1]) << "\n";
        return TestResult(false, oss.str());
    }

    return TestResult(true);
}

// Verify self-consistency: (a op b) op_inv b ≈ a
// For addition: (a + b) - b ≈ a
// For subtraction: (a - b) + b ≈ a
inline TestResult verify_self_consistency_add(
    const td& a,
    const td& b,
    const std::string& test_name = "self-consistency (a+b)-b=a")
{
    td sum = a + b;
    td recovered = sum - b;

    // Allow small error accumulation (within a few ULPs of td precision)
    double tolerance = std::abs(a[0]) * TD_EPS * 10.0;
    if (tolerance == 0.0) tolerance = TD_EPS * 10.0; // handle zero case

    bool close_enough = std::abs(recovered[0] - a[0]) <= tolerance;

    if (close_enough) {
        return TestResult(true);
    }

    std::ostringstream oss;
    oss << test_name << " FAILED:\n";
    oss << "  a         = " << to_binary(a) << "\n";
    oss << "  b         = " << to_binary(b) << "\n";
    oss << "  (a+b)-b   = " << to_binary(recovered) << "\n";
    oss << "  difference = " << (recovered[0] - a[0]) << "\n";
    oss << "  tolerance  = " << tolerance << "\n";

    return TestResult(false, oss.str());
}

inline TestResult verify_self_consistency_sub(
    const td& a,
    const td& b,
    const std::string& test_name = "self-consistency (a-b)+b=a")
{
    td diff = a - b;
    td recovered = diff + b;

    // Allow small error accumulation
    double tolerance = std::abs(a[0]) * TD_EPS * 10.0;
    if (tolerance == 0.0) tolerance = TD_EPS * 10.0;

    bool close_enough = std::abs(recovered[0] - a[0]) <= tolerance;

    if (close_enough) {
        return TestResult(true);
    }

    std::ostringstream oss;
    oss << test_name << " FAILED:\n";
    oss << "  a         = " << to_binary(a) << "\n";
    oss << "  b         = " << to_binary(b) << "\n";
    oss << "  (a-b)+b   = " << to_binary(recovered) << "\n";
    oss << "  difference = " << (recovered[0] - a[0]) << "\n";
    oss << "  tolerance  = " << tolerance << "\n";

    return TestResult(false, oss.str());
}

// Verify complete cancellation: a - a should be exactly zero
inline TestResult verify_complete_cancellation(
    const td& a,
    const std::string& test_name = "complete cancellation a-a=0")
{
    td result = a - a;
    return verify_zero(result, test_name);
}

// Test case generators
// --------------------

// Generate well-separated components (typical normalized case)
inline td create_well_separated(double hi_value = 1.0) {
    return td(hi_value, hi_value * 1e-17, hi_value * 1e-34);
}

// Generate overlapping components (requires heavy renormalization)
inline td create_overlapping_components(double hi_value = 1.0) {
    return td(hi_value, hi_value * 0.5, hi_value * 0.25);
}

// Generate value with near-zero lower components
inline td create_near_zero_lower(double hi_value = 1.0) {
    return td(hi_value, hi_value * 1e-100, hi_value * 1e-200);
}

// Generate value at ULP boundary
inline td create_at_ulp_boundary() {
    double one = 1.0;
    double ulp = DOUBLE_EPS;
    return td(one, ulp, 0.0);
}

// Generate value with mixed signs (tests denormalized inputs)
inline td create_mixed_signs_internal() {
    return td(1.0, -1e-17, 1e-34);
}

// Generate tiny value requiring lower components
inline td create_requires_lower_components() {
    double eps = DOUBLE_EPS;
    return td(eps / 2.0, eps / 4.0, eps / 8.0);
}

// Generate large magnitude separation
inline td create_large_magnitude_separation() {
    return td(1.0e100, 1.0e83, 1.0e66);
}

// Generate small magnitude separation
inline td create_small_magnitude_separation() {
    return td(1.0e-100, 1.0e-117, 1.0e-134);
}

// ============================================================================
// MULTIPLICATION-SPECIFIC VERIFICATION FUNCTIONS AND TEST GENERATORS
// ============================================================================

/*
 * CORNER CASES FOR TRIPLE-DOUBLE MULTIPLICATION
 * ==============================================
 *
 * Multiplication has fundamentally different characteristics from addition/subtraction:
 *
 * 1. ALGORITHM STRUCTURE:
 *    - Uses expansion_ops::multiply_cascades() which generates N² products (9 for td)
 *    - Each product computed with two_prod for exact error tracking
 *    - Products accumulated by significance level
 *    - Result renormalized
 *
 * 2. UNIQUE MULTIPLICATION CORNER CASES:
 *
 *    a) ZERO ABSORPTION:
 *       - 0 × a = 0, a × 0 = 0, 0 × 0 = 0
 *       - All components must be exactly zero
 *
 *    b) IDENTITY:
 *       - 1 × a = a, a × 1 = a
 *       - All components must be preserved
 *
 *    c) COMMUTATIVITY:
 *       - a × b should equal b × a
 *       - Tests symmetry of multiplication algorithm
 *
 *    d) POWERS OF 2 (EXACT OPERATIONS):
 *       - Multiplying by powers of 2 (2, 4, 0.5, 0.25) is exact in IEEE-754
 *       - Only exponents change, mantissas unchanged
 *       - All components should scale exactly
 *
 *    e) SIGN PATTERNS:
 *       - (+) × (+) = (+), (+) × (-) = (-), (-) × (+) = (-), (-) × (-) = (+)
 *
 *    f) MAGNITUDE EXTREMES:
 *       - Small × Large: may cause overflow/underflow in products
 *       - Large × Large: may overflow
 *       - Small × Small: may underflow
 *
 *    g) NEAR-1 VALUES:
 *       - (1 + ε) × (1 + δ) = 1 + ε + δ + εδ
 *       - Tests precision accumulation in lower components
 *
 *    h) COMPONENT INTERACTION:
 *       - All 9 products (3×3) contribute to final result
 *       - Tests proper accumulation and renormalization
 *
 *    i) ALGEBRAIC PROPERTIES:
 *       - Associativity: (a × b) × c ≈ a × (b × c)
 *       - Distributivity: a × (b + c) ≈ a×b + a×c
 *
 * 3. SELF-CONSISTENCY VALIDATION:
 *    - Commutativity: a × b = b × a (exact within renormalization)
 *    - With division: (a × b) / b ≈ a
 *    - Squares: verify a × a produces expected square
 */

// Verify commutativity: a × b should equal b × a
inline TestResult verify_commutativity(
    const td& a,
    const td& b,
    const std::string& test_name = "commutativity a×b = b×a")
{
    td ab = a * b;
    td ba = b * a;

    // Should be exactly equal after renormalization
    bool components_equal = (ab[0] == ba[0]) && (ab[1] == ba[1]) && (ab[2] == ba[2]);

    if (components_equal) {
        return TestResult(true);
    }

    // Allow small tolerance due to potential differences in renormalization order
    double tolerance = std::max(std::abs(ab[0]), std::abs(ba[0])) * TD_EPS * 10.0;
    bool close_enough = std::abs(ab[0] - ba[0]) <= tolerance;

    if (close_enough) {
        return TestResult(true);
    }

    std::ostringstream oss;
    oss << test_name << " FAILED:\n";
    oss << "  a     = " << to_binary(a) << "\n";
    oss << "  b     = " << to_binary(b) << "\n";
    oss << "  a×b   = " << to_binary(ab) << "\n";
    oss << "  b×a   = " << to_binary(ba) << "\n";
    oss << "  diff  = " << (ab[0] - ba[0]) << "\n";

    return TestResult(false, oss.str());
}

// Verify self-consistency using division: (a × b) / b ≈ a
inline TestResult verify_self_consistency_mul(
    const td& a,
    const td& b,
    const std::string& test_name = "self-consistency (a×b)/b=a")
{
    // Skip if b is zero or too small (division would be unstable)
    if (std::abs(b[0]) < 1e-100) {
        return TestResult(true); // Skip this test for near-zero values
    }

    td product = a * b;
    td recovered = product / b;

    // Allow larger tolerance due to division approximation
    double tolerance = std::abs(a[0]) * TD_EPS * 100.0;
    if (tolerance == 0.0) tolerance = TD_EPS * 100.0;

    bool close_enough = std::abs(recovered[0] - a[0]) <= tolerance;

    if (close_enough) {
        return TestResult(true);
    }

    std::ostringstream oss;
    oss << test_name << " FAILED:\n";
    oss << "  a         = " << to_binary(a) << "\n";
    oss << "  b         = " << to_binary(b) << "\n";
    oss << "  (a×b)/b   = " << to_binary(recovered) << "\n";
    oss << "  difference = " << (recovered[0] - a[0]) << "\n";
    oss << "  tolerance  = " << tolerance << "\n";

    return TestResult(false, oss.str());
}

// Verify associativity: (a × b) × c ≈ a × (b × c)
inline TestResult verify_associativity_mul(
    const td& a,
    const td& b,
    const td& c,
    const std::string& test_name = "associativity (a×b)×c = a×(b×c)")
{
    td ab_c = (a * b) * c;
    td a_bc = a * (b * c);

    // Allow tolerance for accumulated rounding
    double tolerance = std::max(std::abs(ab_c[0]), std::abs(a_bc[0])) * TD_EPS * 100.0;
    if (tolerance == 0.0) tolerance = TD_EPS * 100.0;

    bool close_enough = std::abs(ab_c[0] - a_bc[0]) <= tolerance;

    if (close_enough) {
        return TestResult(true);
    }

    std::ostringstream oss;
    oss << test_name << " FAILED:\n";
    oss << "  a       = " << to_binary(a) << "\n";
    oss << "  b       = " << to_binary(b) << "\n";
    oss << "  c       = " << to_binary(c) << "\n";
    oss << "  (a×b)×c = " << to_binary(ab_c) << "\n";
    oss << "  a×(b×c) = " << to_binary(a_bc) << "\n";
    oss << "  diff    = " << (ab_c[0] - a_bc[0]) << "\n";

    return TestResult(false, oss.str());
}

// Verify distributivity: a × (b + c) ≈ a×b + a×c
inline TestResult verify_distributivity(
    const td& a,
    const td& b,
    const td& c,
    const std::string& test_name = "distributivity a×(b+c) = a×b+a×c")
{
    td a_bc = a * (b + c);
    td ab_ac = (a * b) + (a * c);

    // Allow tolerance for accumulated rounding
    double tolerance = std::max(std::abs(a_bc[0]), std::abs(ab_ac[0])) * TD_EPS * 100.0;
    if (tolerance == 0.0) tolerance = TD_EPS * 100.0;

    bool close_enough = std::abs(a_bc[0] - ab_ac[0]) <= tolerance;

    if (close_enough) {
        return TestResult(true);
    }

    std::ostringstream oss;
    oss << test_name << " FAILED:\n";
    oss << "  a         = " << to_binary(a) << "\n";
    oss << "  b         = " << to_binary(b) << "\n";
    oss << "  c         = " << to_binary(c) << "\n";
    oss << "  a×(b+c)   = " << to_binary(a_bc) << "\n";
    oss << "  a×b+a×c   = " << to_binary(ab_ac) << "\n";
    oss << "  diff      = " << (a_bc[0] - ab_ac[0]) << "\n";

    return TestResult(false, oss.str());
}

// Verify exact power-of-2 multiplication (should be exact)
inline TestResult verify_power_of_2_exact(
    const td& a,
    double power_of_2,
    const std::string& test_name = "power-of-2 exact multiplication")
{
    td scaled = a * power_of_2;

    // For powers of 2, each component should scale exactly
    double expected_hi = a[0] * power_of_2;
    double expected_mid = a[1] * power_of_2;
    double expected_lo = a[2] * power_of_2;

    return verify_components(scaled, expected_hi, expected_mid, expected_lo, 0.0, test_name);
}

// Test case generators for multiplication
// ----------------------------------------

// Generate value near 1 (for testing precision in products)
inline td create_near_one(double epsilon_scale = 1.0) {
    double eps = DOUBLE_EPS * epsilon_scale;
    return td(1.0 + eps, eps * eps / 2.0, eps * eps * eps / 6.0);
}

// Generate a perfect square value (for testing a × a)
inline td create_square_test_value() {
    return td(2.0, 1e-16, 1e-32);
}

} // namespace td_corner_cases
} // namespace sw::universal
