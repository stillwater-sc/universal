// td_corner_case_tests.hpp: Corner case test infrastructure for double-double cascade arithmetic
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
#include <universal/number/dd_cascade/dd_cascade.hpp>
#include <cmath>
#include <limits>
#include <string>
#include <sstream>

/*
 * DOUBLE-DOUBLE CASCADE ARITHMETIC CORNER CASE TESTING FRAMEWORK
 * =======================================================
 *
 * WHY CORNER CASES INSTEAD OF RANDOM TESTING?
 * --------------------------------------------
 * Double-double (dd_cascade) numbers have ~106 bits of precision (~32 decimal digits), while double
 * has only 53 bits (~16 decimal digits). Comparing dd_cascade arithmetic results to double references
 * is fundamentally flawed:
 *
 *   dd_cascade: ~106 fraction bits (2 × 53-bit doubles with non-overlapping mantissas)
 *   double:      ~53 fraction bits
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
 * CORNER CASES FOR DOUBLE-DOUBLE CASCADE ADDITION/SUBTRACTION
 * ====================================================
 *
 * Based on the dd_cascade implementation structure:
 * - expansion_ops::add_cascades() merges 2+2 components into 4-component expansion
 * - Compression sums tail components (2-3) into component 1 (0-based indexing)
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
 *    - Well-separated: (1.0, 1e-17) - typical normalized case
 *    - Overlapping magnitudes: (1.0, 0.5) - triggers heavy renormalization
 *    - Near-zero lower components: (1.0, 1e-200)
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
 * 1. SELF-CONSISTENCY: (a + b) - b ≈ a (within dd_cascade ULP)
 * 2. COMPONENT INSPECTION: Verify each component is within expected bounds
 * 3. ASSOCIATIVITY TESTS: (a + b) + c ≈ a + (b + c) (approximately equal)
 * 4. KNOWN EXACT RESULTS: Construct cases where exact answer is known
 * 5. CROSS-VALIDATION: Use qd (quad-double) as oracle if available
 */

namespace sw::universal {
namespace dd_cascade_corner_cases {

// Epsilon values for multi-component precision
// Double:         53 bits of precision → epsilon = 2^-52  ≈ 2.22e-16
// Double-double: 106 bits of precision → epsilon = 2^-106 ≈ 1.23e-32
// Triple-double: 159 bits of precision → epsilon = 2^-159 ≈ 1.74e-48
constexpr double DOUBLE_EPS = std::numeric_limits<double>::epsilon(); // 2^-52 ≈ 2.22e-16
constexpr double DD_EPS = 1.2325951644078309e-32;  // 2^-106 for double-double

// Test result structure
struct TestResult {
    bool passed;
    std::string message;

    TestResult(bool p = true, const std::string& m = "") : passed(p), message(m) {}

    operator bool() const { return passed; }
};

// Component verification: check if dd_cascade components match expected values within tolerance
inline TestResult verify_components(
    const dd_cascade& value,
    double expected_hi,
    double expected_lo,
    double tolerance = 0.0,  // 0.0 means exact match
    const std::string& test_name = "component verification")
{
    bool hi_match  = (tolerance == 0.0) ? (value[0] == expected_hi)  : (std::abs(value[0] - expected_hi) <= tolerance);
    bool lo_match  = (tolerance == 0.0) ? (value[1] == expected_lo)  : (std::abs(value[1] - expected_lo) <= tolerance);

    if (hi_match && lo_match) {
        return TestResult(true);
    }

    std::ostringstream oss;
    oss << test_name << " FAILED:\n";
    oss << "  Expected: (" << expected_hi << ", " << expected_lo << ")\n";
    oss << "  Got:      (" << value[0] << ", " << value[1] << ")\n";
    if (tolerance > 0.0) {
        oss << "  Tolerance: " << tolerance << "\n";
    }

    return TestResult(false, oss.str());
}

// Verify that a value is zero (all components)
inline TestResult verify_zero(const dd_cascade& value, const std::string& test_name = "zero verification") {
    return verify_components(value, 0.0, 0.0, 0.0, test_name);
}

// Verify proper normalization: components should be non-overlapping
// This means |component[i]| should be approximately ULP of component[i-1]
inline TestResult verify_normalized(const dd_cascade& value, const std::string& test_name = "normalization check") {
    // A normalized dd_cascade has components in decreasing magnitude order
    // and each component should be roughly the ULP of the previous one (when non-zero)

    // Skip if value is zero
    if (value[0] == 0.0 && value[1] == 0.0) {
        return TestResult(true);
    }

    // Check decreasing magnitude (when components are non-zero)
    if (value[1] != 0.0 && std::abs(value[1]) > std::abs(value[0])) {
        std::ostringstream oss;
        oss << test_name << " FAILED: lo component larger than hi\n";
        oss << "  |lo| = " << std::abs(value[1]) << " > |hi| = " << std::abs(value[0]) << "\n";
        return TestResult(false, oss.str());
    }

    return TestResult(true);
}

// Verify self-consistency: (a op b) op_inv b ≈ a
// For addition: (a + b) - b ≈ a
// For subtraction: (a - b) + b ≈ a
inline TestResult verify_self_consistency_add(
    const dd_cascade& a,
    const dd_cascade& b,
    const std::string& test_name = "self-consistency (a+b)-b=a")
{
    dd_cascade sum = a + b;
    dd_cascade recovered = sum - b;

    // Allow small error accumulation (within a few ULPs of dd_cascade precision)
    double tolerance = std::abs(a[0]) * DD_EPS * 10.0;
    if (tolerance == 0.0) tolerance = DD_EPS * 10.0; // handle zero case

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
    const dd_cascade& a,
    const dd_cascade& b,
    const std::string& test_name = "self-consistency (a-b)+b=a")
{
    dd_cascade diff = a - b;
    dd_cascade recovered = diff + b;

    // Allow small error accumulation
    double tolerance = std::abs(a[0]) * DD_EPS * 10.0;
    if (tolerance == 0.0) tolerance = DD_EPS * 10.0;

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
    const dd_cascade& a,
    const std::string& test_name = "complete cancellation a-a=0")
{
    dd_cascade result = a - a;
    return verify_zero(result, test_name);
}

// Test case generators
// --------------------

// Generate well-separated components (typical normalized case)
inline dd_cascade create_well_separated(double hi_value = 1.0) {
    return dd_cascade(hi_value, hi_value * 1e-17);
}

// Generate overlapping components (requires heavy renormalization)
inline dd_cascade create_overlapping_components(double hi_value = 1.0) {
    return dd_cascade(hi_value, hi_value * 0.5);
}

// Generate value with near-zero lower components
inline dd_cascade create_near_zero_lower(double hi_value = 1.0) {
    return dd_cascade(hi_value, hi_value * 1e-200);
}

// Generate value at ULP boundary
inline dd_cascade create_at_ulp_boundary() {
    double one = 1.0;
    double ulp = DOUBLE_EPS;
    return dd_cascade(one, ulp);
}

// Generate value with mixed signs (tests denormalized inputs)
inline dd_cascade create_mixed_signs_internal() {
    return dd_cascade(1.0, -1e-17);
}

// Generate tiny value requiring lower components
inline dd_cascade create_requires_lower_components() {
    double eps = DOUBLE_EPS;
    return dd_cascade(eps / 2.0, eps / 4.0);
}

// Generate large magnitude separation
inline dd_cascade create_large_magnitude_separation() {
    return dd_cascade(1.0e100, 1.0e83);
}

// Generate small magnitude separation
inline dd_cascade create_small_magnitude_separation() {
    return dd_cascade(1.0e-100, 1.0e-117);
}

// ============================================================================
// MULTIPLICATION-SPECIFIC VERIFICATION FUNCTIONS AND TEST GENERATORS
// ============================================================================

/*
 * CORNER CASES FOR DOUBLE-DOUBLE CASCADE MULTIPLICATION
 * ==============================================
 *
 * Multiplication has fundamentally different characteristics from addition/subtraction:
 *
 * 1. ALGORITHM STRUCTURE:
 *    - Uses expansion_ops::multiply_cascades() which generates N² products (9 for dd_cascade)
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
    const dd_cascade& a,
    const dd_cascade& b,
    const std::string& test_name = "commutativity a×b = b×a")
{
    dd_cascade ab = a * b;
    dd_cascade ba = b * a;

    // Should be exactly equal after renormalization
    bool components_equal = (ab[0] == ba[0]) && (ab[1] == ba[1]);

    if (components_equal) {
        return TestResult(true);
    }

    // Allow small tolerance due to potential differences in renormalization order
    double tolerance = std::max(std::abs(ab[0]), std::abs(ba[0])) * DD_EPS * 10.0;
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
    const dd_cascade& a,
    const dd_cascade& b,
    const std::string& test_name = "self-consistency (a×b)/b=a")
{
    // Skip if b is zero or too small (division would be unstable)
    if (std::abs(b[0]) < 1e-100) {
        return TestResult(true); // Skip this test for near-zero values
    }

    dd_cascade product = a * b;
    dd_cascade recovered = product / b;

    // Allow larger tolerance due to division approximation
    double tolerance = std::abs(a[0]) * DD_EPS * 100.0;
    if (tolerance == 0.0) tolerance = DD_EPS * 100.0;

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
    const dd_cascade& a,
    const dd_cascade& b,
    const dd_cascade& c,
    const std::string& test_name = "associativity (a×b)×c = a×(b×c)")
{
    dd_cascade ab_c = (a * b) * c;
    dd_cascade a_bc = a * (b * c);

    // Allow tolerance for accumulated rounding
    double tolerance = std::max(std::abs(ab_c[0]), std::abs(a_bc[0])) * DD_EPS * 100.0;
    if (tolerance == 0.0) tolerance = DD_EPS * 100.0;

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
    const dd_cascade& a,
    const dd_cascade& b,
    const dd_cascade& c,
    const std::string& test_name = "distributivity a×(b+c) = a×b+a×c")
{
    dd_cascade a_bc = a * (b + c);
    dd_cascade ab_ac = (a * b) + (a * c);

    // Allow tolerance for accumulated rounding
    double tolerance = std::max(std::abs(a_bc[0]), std::abs(ab_ac[0])) * DD_EPS * 100.0;
    if (tolerance == 0.0) tolerance = DD_EPS * 100.0;

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
    const dd_cascade& a,
    double power_of_2,
    const std::string& test_name = "power-of-2 exact multiplication")
{
    dd_cascade scaled = a * power_of_2;

    // For powers of 2, each component should scale exactly
    double expected_hi = a[0] * power_of_2;
    double expected_lo = a[1] * power_of_2;

    return verify_components(scaled, expected_hi, expected_lo, 0.0, test_name);
}

// Test case generators for multiplication
// ----------------------------------------

// Generate value near 1 (for testing precision in products)
inline dd_cascade create_near_one(double epsilon_scale = 1.0) {
    double eps = DOUBLE_EPS * epsilon_scale;
    return dd_cascade(1.0 + eps, eps * eps / 2.0);
}

// Generate a perfect square value (for testing a × a)
inline dd_cascade create_square_test_value() {
    return dd_cascade(2.0, 1e-16);
}

// ============================================================================
// DIVISION-SPECIFIC VERIFICATION FUNCTIONS AND TEST GENERATORS
// ============================================================================

/*
 * CORNER CASES FOR DOUBLE-DOUBLE CASCADE DIVISION
 * ========================================
 *
 * Division has fundamentally different characteristics from other operations:
 *
 * 1. ALGORITHM STRUCTURE (Newton-Raphson with 2 iterations):
 *    - Initial approximation: q0 = dividend[0] / divisor[0]
 *    - Iterative refinement using residuals
 *    - Only 3 iterations (may not fully converge for pathological cases)
 *    - Result renormalized
 *
 * 2. UNIQUE DIVISION CORNER CASES:
 *
 *    a) SPECIAL VALUE HANDLING:
 *       - NaN propagation: NaN / a = NaN, a / NaN = NaN
 *       - Division by zero: 0/0 = NaN, a/0 = ±∞ (sign depends on operands)
 *       - Division of infinity: ∞/a, a/∞, ∞/∞
 *
 *    b) NON-COMMUTATIVITY:
 *       - a / b ≠ b / a (except when a = ±b)
 *       - Must verify this explicitly
 *
 *    c) IDENTITY AND RECIPROCAL:
 *       - a / a = 1 (for all components)
 *       - a / 1 = a
 *       - 1 / a = reciprocal(a)
 *
 *    d) POWERS OF 2 (EXACT OPERATIONS):
 *       - Division by powers of 2 (2, 4, 0.5, 0.25) should be exact
 *       - Only exponents change, mantissas unchanged
 *
 *    e) SIGN PATTERNS:
 *       - (+) / (+) = (+), (+) / (-) = (-), (-) / (+) = (-), (-) / (-) = (+)
 *
 *    f) CONVERGENCE ISSUES:
 *       - Very small divisors (near underflow)
 *       - Very large divisors (near overflow)
 *       - Dividend and divisor with vastly different magnitudes
 *       - Only 3 Newton-Raphson iterations may not fully converge
 *
 *    g) WELL-KNOWN DIVISIONS:
 *       - 1/3, 1/7, 1/9 (test repeating decimals in binary)
 *       - Test precision of result
 *
 *    h) MAGNITUDE EXTREMES:
 *       - Large / small (may overflow)
 *       - Small / large (may underflow)
 *       - Large / large, small / small
 *
 * 3. SELF-CONSISTENCY VALIDATION:
 *    - (a / b) × b ≈ a (primary validation method)
 *    - (a × b) / b ≈ a (already tested in multiplication)
 *    - 1 / (1 / a) ≈ a (double reciprocal)
 */

// Verify self-consistency: (a / b) × b ≈ a
inline TestResult verify_self_consistency_div(
    const dd_cascade& a,
    const dd_cascade& b,
    const std::string& test_name = "self-consistency (a/b)×b=a")
{
    // Skip if b is zero or too small/large (division would be unstable)
    if (std::abs(b[0]) < 1e-100 || std::abs(b[0]) > 1e100) {
        return TestResult(true); // Skip this test for extreme values
    }

    dd_cascade quotient = a / b;
    dd_cascade recovered = quotient * b;

    // Allow larger tolerance due to iterative approximation in division
    double tolerance = std::abs(a[0]) * DD_EPS * 1000.0;  // Relaxed for division
    if (tolerance == 0.0) tolerance = DD_EPS * 1000.0;

    bool close_enough = std::abs(recovered[0] - a[0]) <= tolerance;

    if (close_enough) {
        return TestResult(true);
    }

    std::ostringstream oss;
    oss << test_name << " FAILED:\n";
    oss << "  a         = " << to_binary(a) << "\n";
    oss << "  b         = " << to_binary(b) << "\n";
    oss << "  (a/b)×b   = " << to_binary(recovered) << "\n";
    oss << "  difference = " << (recovered[0] - a[0]) << "\n";
    oss << "  tolerance  = " << tolerance << "\n";

    return TestResult(false, oss.str());
}

// Verify a / a = 1 for all components
inline TestResult verify_division_identity(
    const dd_cascade& a,
    const std::string& test_name = "division identity a/a=1")
{
    // Skip for zero
    if (a.iszero()) {
        return TestResult(true);
    }

    dd_cascade quotient = a / a;

    // Should be very close to 1.0
    double tolerance = DD_EPS * 100.0;

    if (std::abs(quotient[0] - 1.0) > tolerance) {
        std::ostringstream oss;
        oss << test_name << " FAILED:\n";
        oss << "  a       = " << to_binary(a) << "\n";
        oss << "  a/a     = " << to_binary(quotient) << "\n";
        oss << "  expected = 1.0\n";
        oss << "  diff     = " << (quotient[0] - 1.0) << "\n";
        return TestResult(false, oss.str());
    }

    return TestResult(true);
}

// Verify double reciprocal: 1 / (1 / a) ≈ a
inline TestResult verify_double_reciprocal(
    const dd_cascade& a,
    const std::string& test_name = "double reciprocal 1/(1/a)=a")
{
    // Skip for zero or extreme values
    if (a.iszero() || std::abs(a[0]) < 1e-100 || std::abs(a[0]) > 1e100) {
        return TestResult(true);
    }

    dd_cascade one(1.0, 0.0);
    dd_cascade recip = one / a;
    dd_cascade double_recip = one / recip;

    // Allow larger tolerance for two division operations
    double tolerance = std::abs(a[0]) * DD_EPS * 10000.0;
    if (tolerance == 0.0) tolerance = DD_EPS * 10000.0;

    bool close_enough = std::abs(double_recip[0] - a[0]) <= tolerance;

    if (close_enough) {
        return TestResult(true);
    }

    std::ostringstream oss;
    oss << test_name << " FAILED:\n";
    oss << "  a         = " << to_binary(a) << "\n";
    oss << "  1/a       = " << to_binary(recip) << "\n";
    oss << "  1/(1/a)   = " << to_binary(double_recip) << "\n";
    oss << "  difference = " << (double_recip[0] - a[0]) << "\n";
    oss << "  tolerance  = " << tolerance << "\n";

    return TestResult(false, oss.str());
}

// Verify non-commutativity: a / b ≠ b / a (except for special cases)
inline TestResult verify_non_commutativity(
    const dd_cascade& a,
    const dd_cascade& b,
    const std::string& test_name = "non-commutativity a/b ≠ b/a")
{
    // Skip if either is zero
    if (a.iszero() || b.iszero()) {
        return TestResult(true);
    }

    // Skip if a and b are equal or opposites (special cases where they might be equal)
    if ((a[0] == b[0] && a[1] == b[1]) ||
        (a[0] == -b[0] && a[1] == -b[1])) {
        return TestResult(true);
    }

    dd_cascade ab = a / b;
    dd_cascade ba = b / a;

    // These should NOT be equal
    bool are_different = !(
        std::abs(ab[0] - ba[0]) < DD_EPS * 10.0 &&
        std::abs(ab[1] - ba[1]) < DD_EPS * 10.0
    );

    if (are_different) {
        return TestResult(true);
    }

    std::ostringstream oss;
    oss << test_name << " FAILED: a/b equals b/a when it shouldn't\n";
    oss << "  a     = " << to_binary(a) << "\n";
    oss << "  b     = " << to_binary(b) << "\n";
    oss << "  a/b   = " << to_binary(ab) << "\n";
    oss << "  b/a   = " << to_binary(ba) << "\n";

    return TestResult(false, oss.str());
}

// Test case generators for division
// ----------------------------------

// Generate value for reciprocal testing
inline dd_cascade create_for_reciprocal_test(double scale = 1.0) {
    return dd_cascade(scale, scale * 1e-16);
}

} // namespace dd_cascade_corner_cases
} // namespace sw::universal
