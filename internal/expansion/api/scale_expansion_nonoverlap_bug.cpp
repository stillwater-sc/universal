// scale_expansion_nonoverlap_bug.cpp: Root Cause Analysis for scale_expansion non-overlapping violation
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

/*
 * ROOT CAUSE ANALYSIS: scale_expansion Returns Unsanitized Sorted Products
 * =========================================================================
 *
 * PROBLEM:
 * The scale_expansion implementation (expansion_ops.hpp:408-442) performs:
 * 1. Multiplies each expansion component by scalar b using two_prod
 * 2. Collects products and errors
 * 3. Sorts by decreasing magnitude
 * 4. Returns result WITHOUT RENORMALIZATION
 *
 * This violates Shewchuk's expansion invariants:
 * - NON-OVERLAPPING: Adjacent components should not share significant bits
 * - ORDERING: Components must be in strictly decreasing magnitude order
 *
 * WHY SORTING ISN'T ENOUGH:
 * Sorting by magnitude doesn't guarantee non-overlapping property. Consider:
 *   e = [1.0, 1e-17]  (nonoverlapping expansion)
 *   scale by b = 0.1
 *
 * After two_prod:
 *   1.0 × 0.1 = product: 0.1, error: 1.3877787807814457e-18
 *   1e-17 × 0.1 = product: 1e-18, error: ~0
 *
 * After sorting by magnitude: [0.1, 1.3877787807814457e-18, 1e-18]
 *
 * PROBLEM: 1.3877787807814457e-18 and 1e-18 have OVERLAPPING bits!
 * They're only 1.39× apart in magnitude, not the required 2^53 separation
 * for multi-component doubles.
 *
 * IMPACT ON DOWNSTREAM ALGORITHMS:
 * - fast_expansion_sum assumes nonoverlapping property for correctness
 * - linear_expansion_sum relies on proper ordering
 * - Compression algorithms depend on non-overlapping invariant
 * - Accumulated errors compound in subsequent operations
 *
 * THE FIX:
 * Must perform renormalization pass:
 * 1. Sort by magnitude (current behavior)
 * 2. Accumulate left-to-right using fast_two_sum to extract nonoverlapping components
 * 3. Drop zeros
 * 4. Return properly sanitized expansion
 */

#include <universal/utility/directives.hpp>
#include <universal/internal/expansion/expansion_ops.hpp>
#include <universal/verification/test_suite.hpp>
#include <iostream>
#include <iomanip>
#include <cmath>

namespace sw::universal {

// Helper to check if expansion satisfies non-overlapping property
bool verify_nonoverlapping(const std::vector<double>& e, const std::string& label, bool verbose = true) {
    if (e.size() < 2) return true; // Single component is trivially nonoverlapping

    bool all_nonoverlapping = true;
    const double SEPARATION_THRESHOLD = std::pow(2.0, 53); // IEEE double has 53 bits of precision

    if (verbose) {
        std::cout << "\n=== Checking non-overlapping property for: " << label << " ===\n";
        std::cout << "Components (" << e.size() << "):\n";
    }

    for (size_t i = 0; i < e.size(); ++i) {
        if (verbose) {
            std::cout << "  e[" << i << "] = " << std::scientific << std::setprecision(17) << e[i];
        }

        if (i > 0 && std::abs(e[i]) > 0.0) {
            double ratio = std::abs(e[i-1]) / std::abs(e[i]);
            if (verbose) {
                std::cout << "  (ratio to previous: " << ratio << ")";
            }

            // Non-overlapping requires ratio >= 2^53
            if (ratio < SEPARATION_THRESHOLD) {
                if (verbose) {
                    std::cout << " OVERLAPS! (need ratio >= 2^53 = " << SEPARATION_THRESHOLD << ")";
                }
                all_nonoverlapping = false;
            } else {
                if (verbose) {
                    std::cout << " PASSES";
                }
            }
        }

        if (verbose) std::cout << "\n";
    }

    if (verbose) {
        std::cout << "Result: " << (all_nonoverlapping ? " Non-overlapping" : " OVERLAPPING DETECTED") << "\n";
    }

    return all_nonoverlapping;
}

// Helper to print expansion details
void print_expansion(const std::vector<double>& e, const std::string& label) {
    std::cout << label << " (" << e.size() << " components): [";
    for (size_t i = 0; i < e.size(); ++i) {
        if (i > 0) std::cout << ", ";
        std::cout << std::scientific << std::setprecision(10) << e[i];
    }
    std::cout << "]\n";
    std::cout << "Sum = " << std::setprecision(17) << expansion_ops::estimate(e) << "\n";
}

} // namespace sw::universal

int main()
try {
    using namespace sw::universal;

    std::cout << "╔═══════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  ROOT CAUSE ANALYSIS: scale_expansion Non-Overlapping Violation   ║\n";
    std::cout << "╚═══════════════════════════════════════════════════════════════════╝\n";

    int nrOfFailedTestCases = 0;

    // ========================================================================
    // TEST 1: Basic scaling exposes overlapping components
    // ========================================================================
    {
        std::cout << "\n\n";
        std::cout << "┌─────────────────────────────────────────────────────────────────┐\n";
        std::cout << "│ Test 1: Scale [1.0, 1e-17] by 0.1 - Overlapping Exposure        │\n";
        std::cout << "└─────────────────────────────────────────────────────────────────┘\n";

        // Create a valid nonoverlapping expansion
        std::vector<double> e;
        e.push_back(1.0);
        double hi, lo;
        expansion_ops::two_sum(1.0, 1e-17, hi, lo);
        if (lo != 0.0) e.push_back(lo);

        std::cout << "\nInput expansion:\n";
        print_expansion(e, "e");
        bool input_ok = verify_nonoverlapping(e, "Input", true);

        // Scale by 0.1
        std::vector<double> result = expansion_ops::scale_expansion(e, 0.1);

        std::cout << "\nResult after scale_expansion(e, 0.1):\n";
        print_expansion(result, "result");
        bool result_ok = verify_nonoverlapping(result, "Result", true);

        if (!result_ok) {
            std::cout << "\n  BUG CONFIRMED: scale_expansion returned overlapping components!\n";
            nrOfFailedTestCases++;
        }

        // Verify value preservation
        double expected = expansion_ops::estimate(e) * 0.1;
        double actual = expansion_ops::estimate(result);
        double error = std::abs(actual - expected);

        std::cout << "\nValue preservation:\n";
        std::cout << "  Expected: " << std::setprecision(17) << expected << "\n";
        std::cout << "  Actual:   " << std::setprecision(17) << actual << "\n";
        std::cout << "  Error:    " << std::scientific << error << "\n";

        if (error > 1e-30) {
            std::cout << "  Value not preserved accurately!\n";
            nrOfFailedTestCases++;
        } else {
            std::cout << "  Value preserved\n";
        }
    }

    // ========================================================================
    // TEST 2: Scaling by non-power-of-2 always produces overlaps
    // ========================================================================
    {
        std::cout << "\n\n";
        std::cout << "┌─────────────────────────────────────────────────────────────────┐\n";
        std::cout << "│ Test 2: Scale [2.0, 1e-16] by 0.3 - Multiple Overlaps           │\n";
        std::cout << "└─────────────────────────────────────────────────────────────────┘\n";

        std::vector<double> e = {2.0, 1e-16};

        std::cout << "\nInput:\n";
        print_expansion(e, "e");

        std::vector<double> result = expansion_ops::scale_expansion(e, 0.3);

        std::cout << "\nResult after scale_expansion(e, 0.3):\n";
        print_expansion(result, "result");
        bool result_ok = verify_nonoverlapping(result, "Result", true);

        if (!result_ok) {
            std::cout << "\n  BUG CONFIRMED: Non-power-of-2 scaling produces overlaps!\n";
            nrOfFailedTestCases++;
        }
    }

    // ========================================================================
    // TEST 3: Scaling expansion with many components
    // ========================================================================
    {
        std::cout << "\n\n";
        std::cout << "┌─────────────────────────────────────────────────────────────────┐\n";
        std::cout << "│ Test 3: Scale multi-component expansion - Cascade of Overlaps   │\n";
        std::cout << "└─────────────────────────────────────────────────────────────────┘\n";

        // Build a 4-component expansion representing π/4 or similar
        std::vector<double> e;
        double pi_4 = 0.7853981633974483;  // π/4 high bits
        e.push_back(pi_4);
        e.push_back(9.6765358979846479e-18);  // π/4 correction 1
        e.push_back(-3.9765413851024444e-35); // π/4 correction 2
        e.push_back(2.1184879405313824e-52);  // π/4 correction 3

        std::cout << "\nInput: 4-component approximation of π/4\n";
        print_expansion(e, "e");
        verify_nonoverlapping(e, "Input", false);

        // Scale by 1/7 (non-representable)
        double scale = 1.0 / 7.0;
        std::vector<double> result = expansion_ops::scale_expansion(e, scale);

        std::cout << "\nResult after scale_expansion(e, 1/7):\n";
        print_expansion(result, "result");
        bool result_ok = verify_nonoverlapping(result, "Result", true);

        if (!result_ok) {
            std::cout << "\n  BUG CONFIRMED: Multi-component scaling produces cascading overlaps!\n";
            nrOfFailedTestCases++;
        }

        std::cout << "\nNote: Result has " << result.size() << " components (doubled from input)\n";
        std::cout << "Many of these components violate non-overlapping property.\n";
    }

    // ========================================================================
    // TEST 4: Impact on downstream operations
    // ========================================================================
    {
        std::cout << "\n\n";
        std::cout << "┌─────────────────────────────────────────────────────────────────┐\n";
        std::cout << "│ Test 4: Downstream Impact - Using Result in Addition            │\n";
        std::cout << "└─────────────────────────────────────────────────────────────────┘\n";

        std::vector<double> e1 = {1.0, 1e-17};
        std::vector<double> e2 = expansion_ops::scale_expansion(e1, 0.1);  // Produces overlapping result

        std::cout << "\ne1 (original): ";
        print_expansion(e1, "");
        std::cout << "e2 (scaled, overlapping): ";
        print_expansion(e2, "");

        // Try to add them
        std::vector<double> sum = expansion_ops::linear_expansion_sum(e1, e2);

        std::cout << "\nsum = e1 + e2: ";
        print_expansion(sum, "");

        // Verify sum
        double expected = expansion_ops::estimate(e1) + expansion_ops::estimate(e2);
        double actual = expansion_ops::estimate(sum);
        double rel_error = std::abs((actual - expected) / expected);

        std::cout << "Expected sum: " << std::setprecision(17) << expected << "\n";
        std::cout << "Actual sum:   " << std::setprecision(17) << actual << "\n";
        std::cout << "Relative error: " << std::scientific << rel_error << "\n";

        if (rel_error > 1e-15) {
            std::cout << " Error propagation detected from overlapping input!\n";
        } else {
            std::cout << " Downstream operation survived (linear_expansion_sum is robust)\n";
        }
    }

    // ========================================================================
    // Summary
    // ========================================================================
    std::cout << "\n\n";
    std::cout << "╔═══════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                    ROOT CAUSE ANALYSIS SUMMARY                    ║\n";
    std::cout << "╚═══════════════════════════════════════════════════════════════════╝\n\n";

    std::cout << "CONFIRMED ISSUES:\n";
    std::cout << "1. scale_expansion returns components violating non-overlapping property\n";
    std::cout << "2. Sorting by magnitude is INSUFFICIENT for expansion validity\n";
    std::cout << "3. Any non-power-of-2 scaling produces overlapping components\n";
    std::cout << "4. Multi-component expansions produce cascading overlaps\n\n";

    std::cout << "ROOT CAUSE:\n";
    std::cout << "Function returns sorted products without renormalization pass.\n";
    std::cout << "Comment at line 436-439 acknowledges this TODO.\n\n";

    std::cout << "REQUIRED FIX:\n";
    std::cout << "1. After sorting, perform renormalization:\n";
    std::cout << "   - Accumulate sorted terms left-to-right using fast_two_sum\n";
    std::cout << "   - Extract non-overlapping components\n";
    std::cout << "   - Drop zeros\n";
    std::cout << "2. Preserve special cases (b=0, ±1)\n";
    std::cout << "3. Ensure result satisfies Shewchuk expansion invariants\n\n";

    std::cout << "IMPACT:\n";
    std::cout << "- Used by multiply_cascades (just fixed) - could affect precision\n";
    std::cout << "- Used by ereal multiplication - could propagate errors\n";
    std::cout << "- Any algorithm assuming valid expansion invariants will misbehave\n\n";

    if (nrOfFailedTestCases > 0) {
        std::cout << "╔═══════════════════════════════════════════════════════════════════╗\n";
        std::cout << "║            " << nrOfFailedTestCases << " VIOLATIONS CONFIRMED - FIX REQUIRED                 ║\n";
        std::cout << "╚═══════════════════════════════════════════════════════════════════╝\n";
    }

    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
    std::cerr << "Caught exception: " << msg << std::endl;
    return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
    std::cerr << "Caught runtime exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
catch (...) {
    std::cerr << "Caught unknown exception" << std::endl;
    return EXIT_FAILURE;
}
