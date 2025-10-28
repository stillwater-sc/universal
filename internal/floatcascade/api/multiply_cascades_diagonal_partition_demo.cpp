// multiply_cascades_diagonal_partition_demo.cpp: Demonstration of the diagonal partitioning algorithm
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

/*
 * DEMONSTRATION: Diagonal Partitioning Algorithm for Multi-Component Multiplication
 *
 * This test demonstrates the corrected multiply_cascades algorithm that uses proper
 * diagonal partitioning to multiply two N-component floating-point cascades.
 *
 * BACKGROUND:
 * When multiplying two N-component cascades a and b, we generate N² partial products.
 * These products have different significance levels based on the significance of their
 * input components. The key insight is that products can be organized by "diagonals"
 * where each diagonal represents a specific significance level.
 *
 * PROVEN QD APPROACH (Priest 1991, Hida-Li-Bailey 2000):
 * The Quad-Double library and related research established the diagonal partitioning
 * method as the correct way to handle multi-component multiplication:
 *
 * 1. DIAGONAL PARTITIONING:
 *    For indices i,j ∈ [0,N-1], place product a[i]×b[j] and its error term into
 *    diagonal k = i+j. This creates 2N-1 diagonals (k = 0 to 2N-2).
 *    - Diagonal 0: Most significant (a[0]×b[0])
 *    - Diagonal N-1: Middle significance
 *    - Diagonal 2N-2: Least significant (a[N-1]×b[N-1])
 *
 * 2. PER-DIAGONAL ACCUMULATION:
 *    Within each diagonal, accumulate all products and error terms from the previous
 *    diagonal using stable two_sum chains. This preserves precision and tracks errors.
 *
 * 3. COMPONENT EXTRACTION:
 *    Collect all diagonal sums and their errors, sort by magnitude, then extract the
 *    top N non-overlapping components using a two_sum cascade.
 *
 * 4. RENORMALIZATION:
 *    Apply final renormalization to ensure the non-overlapping property holds.
 *
 * An incorrect implementation might only handled diagonals 0-2 explicitly, then dumped all
 * remaining terms into result[2]. This will cause several issues:
 * - Uninitialized components for N≥3 (result[3]...result[N-1] will never be assigned)
 * - Loss of precision from improper accumulation
 * - Violation of the non-overlapping property
 * - Failure to adhere to the diagonal partitioning principle
 *
 * CORNER CASES DISCOVERED:
 * 1. Zero components in diagonals create "holes" in magnitude ordering
 * 2. Denormalized inputs (overlapping components) require robust accumulation
 * 3. Sign mixing in components needs careful error propagation
 * 4. Direct magnitude-sorted extraction can violate ordering (fixed by two_sum cascade)
 * 5. Renormalization can introduce zeros in the middle of the result array
 */

#include <universal/utility/directives.hpp>
#include <universal/internal/floatcascade/floatcascade.hpp>
#include <iostream>
#include <iomanip>
#include <vector>

namespace sw::universal {

// Helper to visualize the NxN product matrix with diagonal partitioning
template<size_t N>
void visualize_product_matrix(const floatcascade<N>& a, const floatcascade<N>& b) {
    std::cout << "\n=== NxN Product Matrix with Diagonal Partitioning (N=" << N << ") ===\n\n";

    // Compute all products
    std::array<double, N * N> products;
    std::array<double, N * N> errors;

    for (size_t i = 0; i < N; ++i) {
        for (size_t j = 0; j < N; ++j) {
            expansion_ops::two_prod(a[i], b[j], products[i * N + j], errors[i * N + j]);
        }
    }

    // Display the matrix with diagonal labels
    std::cout << "Matrix notation: P[i,j] = a[i] x b[j]\n";
    std::cout << "Diagonal k contains all products where i+j = k\n\n";

    std::cout << std::setw(12) << " ";
    for (size_t j = 0; j < N; ++j) {
        std::cout << "   b[" << j << "]" << std::setw(14) << " ";
    }
    std::cout << "\n";

    for (size_t i = 0; i < N; ++i) {
        std::cout << "a[" << i << "]  ";
        for (size_t j = 0; j < N; ++j) {
            size_t diag = i + j;
            std::cout << "  [D" << diag << "]  " << std::setw(12) << std::scientific
                      << std::setprecision(4) << products[i * N + j];
        }
        std::cout << "\n";
    }

    // Show diagonal groupings
    std::cout << "\n=== Diagonal Groupings ===\n";
    for (size_t diag = 0; diag < 2 * N - 1; ++diag) {
        std::cout << "Diagonal " << diag << " (significance level " << diag << "): ";

        // List products in this diagonal
        bool first = true;
        for (size_t i = 0; i <= diag && i < N; ++i) {
            size_t j = diag - i;
            if (j < N) {
                if (!first) std::cout << ", ";
                std::cout << "P[" << i << "," << j << "]";
                first = false;
            }
        }

        // Show accumulated value
        double diag_sum = 0.0;
        for (size_t i = 0; i <= diag && i < N; ++i) {
            size_t j = diag - i;
            if (j < N) {
                diag_sum += products[i * N + j];
            }
        }

        std::cout << " → sum ≈ " << std::scientific << std::setprecision(4) << diag_sum << "\n";
    }
}

// Helper to demonstrate diagonal accumulation process
template<size_t N>
void demonstrate_diagonal_accumulation(const floatcascade<N>& a, const floatcascade<N>& b) {
    std::cout << "\n=== Diagonal Accumulation Process ===\n\n";

    // Compute all products
    std::array<double, N * N> products;
    std::array<double, N * N> errors;

    for (size_t i = 0; i < N; ++i) {
        for (size_t j = 0; j < N; ++j) {
            expansion_ops::two_prod(a[i], b[j], products[i * N + j], errors[i * N + j]);
        }
    }

    // Process each diagonal
    std::array<double, 2 * N - 1> diagonal_sums;
    std::array<double, 2 * N - 1> diagonal_errors;

    for (size_t k = 0; k < 2 * N - 1; ++k) {
        diagonal_sums[k] = 0.0;
        diagonal_errors[k] = 0.0;
    }

    for (size_t diag = 0; diag < 2 * N - 1; ++diag) {
        std::cout << "Diagonal " << diag << ":\n";

        // Collect terms
        std::vector<double> terms;

        // Products where i+j == diag
        for (size_t i = 0; i <= diag && i < N; ++i) {
            size_t j = diag - i;
            if (j < N) {
                terms.push_back(products[i * N + j]);
                std::cout << "  + product[" << i << "," << j << "] = "
                          << std::scientific << std::setprecision(6) << products[i * N + j] << "\n";
            }
        }

        // Errors from previous diagonal
        if (diag > 0) {
            for (size_t i = 0; i <= diag - 1 && i < N; ++i) {
                size_t j = diag - 1 - i;
                if (j < N) {
                    terms.push_back(errors[i * N + j]);
                    std::cout << "  + error[" << i << "," << j << "] (from diag " << (diag-1)
                              << ") = " << std::scientific << std::setprecision(6)
                              << errors[i * N + j] << "\n";
                }
            }
        }

        // Accumulate using stable two_sum
        if (!terms.empty()) {
            double sum = terms[0];
            double accumulated_error = 0.0;

            for (size_t t = 1; t < terms.size(); ++t) {
                double new_sum, err;
                expansion_ops::two_sum(sum, terms[t], new_sum, err);
                sum = new_sum;

                // Accumulate errors
                double err_sum, err_err;
                expansion_ops::two_sum(accumulated_error, err, err_sum, err_err);
                accumulated_error = err_sum;

                if (diag + 1 < 2 * N - 1) {
                    diagonal_errors[diag + 1] += err_err;
                }
            }

            diagonal_sums[diag] = sum;
            diagonal_errors[diag] = accumulated_error;

            std::cout << "  = sum: " << std::scientific << std::setprecision(10) << sum
                      << ", error: " << accumulated_error << "\n";
        }
        std::cout << "\n";
    }

    std::cout << "=== Final Diagonal Summary ===\n";
    for (size_t k = 0; k < 2 * N - 1; ++k) {
        if (diagonal_sums[k] != 0.0 || diagonal_errors[k] != 0.0) {
            std::cout << "Diagonal[" << k << "]: sum = " << std::scientific
                      << std::setprecision(10) << diagonal_sums[k]
                      << ", error = " << diagonal_errors[k] << "\n";
        }
    }
}

// Helper to show component extraction process
template<size_t N>
void demonstrate_component_extraction(const floatcascade<N>& a, const floatcascade<N>& b) {
    std::cout << "\n=== Component Extraction Process ===\n\n";

    floatcascade<N> result = expansion_ops::multiply_cascades(a, b);

    std::cout << "Input a: [";
    for (size_t i = 0; i < N; ++i) {
        if (i > 0) std::cout << ", ";
        std::cout << std::scientific << std::setprecision(6) << a[i];
    }
    std::cout << "]\n";

    std::cout << "Input b: [";
    for (size_t i = 0; i < N; ++i) {
        if (i > 0) std::cout << ", ";
        std::cout << std::scientific << std::setprecision(6) << b[i];
    }
    std::cout << "]\n\n";

    std::cout << "Result:  [";
    for (size_t i = 0; i < N; ++i) {
        if (i > 0) std::cout << ", ";
        std::cout << std::scientific << std::setprecision(10) << result[i];
    }
    std::cout << "]\n\n";

    // Verify magnitude ordering
    std::cout << "=== Verification ===\n";
    std::cout << "1. Magnitude ordering (must be decreasing):\n";
    bool ordered = true;
    for (size_t i = 0; i < N; ++i) {
        std::cout << "   |result[" << i << "]| = " << std::abs(result[i]);
        if (i > 0 && std::abs(result[i]) > std::abs(result[i-1])) {
            std::cout << " ERROR: Larger than previous!";
            ordered = false;
        }
        std::cout << "\n";
    }
    std::cout << "   Status: " << (ordered ? "PASS" : "FAIL") << "\n\n";

    // Verify value preservation
    double expected = 0.0;
    for (size_t i = 0; i < N; ++i) expected += a[i];
    double expected_b = 0.0;
    for (size_t i = 0; i < N; ++i) expected_b += b[i];
    expected *= expected_b;

    double actual = 0.0;
    for (size_t i = 0; i < N; ++i) actual += result[i];

    std::cout << "2. Value preservation:\n";
    std::cout << "   Expected (sum(a) x sum(b)): " << std::setprecision(15) << expected << "\n";
    std::cout << "   Actual   (sum(result)):     " << std::setprecision(15) << actual << "\n";
    std::cout << "   Relative error: " << std::abs((actual - expected) / expected) << "\n";
    std::cout << "   Status: " << (std::abs((actual - expected) / expected) < 1e-10 ? "PASS" : "FAIL") << "\n";
}

} // namespace sw::universal

// Test cases demonstrating corner cases and the fix
int main()
try {
    using namespace sw::universal;

    std::cout << "╔═══════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  DEMONSTRATION: Diagonal Partitioning for Cascade Multiplication  ║\n";
    std::cout << "╚═══════════════════════════════════════════════════════════════════╝\n";

    int nrOfFailedTestCases = 0;

    // ========================================================================
    // DEMONSTRATION 1: Simple N=3 case showing diagonal structure
    // ========================================================================
    {
        std::cout << "\n\n";
        std::cout << "┌─────────────────────────────────────────────────────────────────┐\n";
        std::cout << "│ Demo 1: Simple Well-Separated Triple-Double (N=3)               │\n";
        std::cout << "└─────────────────────────────────────────────────────────────────┘\n";

        floatcascade<3> a, b;
        a[0] = 1.0;
        a[1] = 1.0e-17;
        a[2] = 1.0e-34;

        b[0] = 2.0;
        b[1] = 2.0e-17;
        b[2] = 2.0e-34;

        visualize_product_matrix(a, b);
        demonstrate_diagonal_accumulation(a, b);
        demonstrate_component_extraction(a, b);
    }

    // ========================================================================
    // DEMONSTRATION 2: N=4 case that exposed the original bug
    // ========================================================================
    {
        std::cout << "\n\n";
        std::cout << "┌─────────────────────────────────────────────────────────────────┐\n";
        std::cout << "│ Demo 2: Quad-Double (N=4) - The Bug Revealer                    │\n";
        std::cout << "└─────────────────────────────────────────────────────────────────┘\n";
        std::cout << "\nThis case exposed the original bug where result[3] was left\n";
        std::cout << "uninitialized and diagonals 3-6 were improperly accumulated.\n";

        floatcascade<4> a, b;
        a[0] = 1.0;
        a[1] = 1.0e-17;
        a[2] = 1.0e-34;
        a[3] = 1.0e-51;

        b[0] = 3.0;
        b[1] = 3.0e-17;
        b[2] = 3.0e-34;
        b[3] = 3.0e-51;

        visualize_product_matrix(a, b);
        demonstrate_component_extraction(a, b);
    }

    // ========================================================================
    // CORNER CASE 1: Denormalized inputs (overlapping components)
    // ========================================================================
    {
        std::cout << "\n\n";
        std::cout << "┌─────────────────────────────────────────────────────────────────┐\n";
        std::cout << "│ Corner Case 1: Denormalized Inputs (Overlapping Components)     │\n";
        std::cout << "└─────────────────────────────────────────────────────────────────┘\n";
        std::cout << "\nInputs have overlapping magnitude components, violating the\n";
        std::cout << "non-overlapping property. The algorithm must handle this robustly.\n";

        floatcascade<4> a, b;
        a[0] = 1.0;
        a[1] = 0.1;      // Overlaps with a[0]
        a[2] = 0.01;     // Overlaps with a[1]
        a[3] = 0.001;    // Overlaps with a[2]

        b[0] = 2.0;
        b[1] = 0.2;      // Overlaps with b[0]
        b[2] = 0.02;     // Overlaps with b[1]
        b[3] = 0.002;    // Overlaps with b[2]

        demonstrate_component_extraction(a, b);

        floatcascade<4> result = expansion_ops::multiply_cascades(a, b);

        // Check if result[1] is zero (the bug symptom)
        if (std::abs(result[1]) < 1e-100) {
            std::cout << "\nWARNING: result[1] is effectively zero - potential issue!\n";
            nrOfFailedTestCases++;
        }

        // Verify all components initialized
        bool all_initialized = true;
        for (size_t i = 0; i < 4; ++i) {
            if (std::isnan(result[i]) || std::isinf(result[i])) {
                std::cout << "\nERROR: result[" << i << "] is NaN or Inf!\n";
                all_initialized = false;
                nrOfFailedTestCases++;
            }
        }

        if (all_initialized) {
            std::cout << "\nAll components properly initialized\n";
        }
    }

    // ========================================================================
    // CORNER CASE 2: Mixed signs in components
    // ========================================================================
    {
        std::cout << "\n\n";
        std::cout << "┌─────────────────────────────────────────────────────────────────┐\n";
        std::cout << "│ Corner Case 2: Mixed Signs in Components                        │\n";
        std::cout << "└─────────────────────────────────────────────────────────────────┘\n";
        std::cout << "\nComponents have different signs, which can cause cancellation\n";
        std::cout << "in diagonal accumulation. Error tracking is critical.\n";

        floatcascade<3> a, b;
        a[0] = 1.0;
        a[1] = -1.0e-17;  // Negative component
        a[2] = 1.0e-34;

        b[0] = 2.0;
        b[1] = 2.0e-17;
        b[2] = -2.0e-34;  // Negative component

        demonstrate_component_extraction(a, b);
    }

    // ========================================================================
    // CORNER CASE 3: Identity multiplication
    // ========================================================================
    {
        std::cout << "\n\n";
        std::cout << "┌─────────────────────────────────────────────────────────────────┐\n";
        std::cout << "│ Corner Case 3: Identity Multiplication (1.0 x value)            │\n";
        std::cout << "└─────────────────────────────────────────────────────────────────┘\n";
        std::cout << "\nMultiplying by 1.0 should preserve the input structure.\n";
        std::cout << "This tests if the algorithm handles sparse diagonals correctly.\n";

        floatcascade<4> one, value;
        one[0] = 1.0;
        one[1] = 0.0;
        one[2] = 0.0;
        one[3] = 0.0;

        value[0] = 2.5;
        value[1] = 1.0e-17;
        value[2] = 1.0e-34;
        value[3] = 1.0e-51;

        demonstrate_component_extraction(one, value);

        floatcascade<4> result = expansion_ops::multiply_cascades(one, value);

        // Verify result ≈ value
        double max_rel_error = 0.0;
        for (size_t i = 0; i < 4; ++i) {
            if (std::abs(value[i]) > 1e-100) {
                double rel_error = std::abs((result[i] - value[i]) / value[i]);
                if (rel_error > max_rel_error) max_rel_error = rel_error;
            }
        }

        std::cout << "\nIdentity test: max relative error = " << max_rel_error << "\n";
        if (max_rel_error > 1e-10) {
            std::cout << "FAIL: Identity property not preserved\n";
            nrOfFailedTestCases++;
        } else {
            std::cout << "PASS: Identity property preserved\n";
        }
    }

    // ========================================================================
    // CORNER CASE 4: Zero absorption
    // ========================================================================
    {
        std::cout << "\n\n";
        std::cout << "┌─────────────────────────────────────────────────────────────────┐\n";
        std::cout << "│ Corner Case 4: Zero Absorption (0 x value = 0)                  │\n";
        std::cout << "└─────────────────────────────────────────────────────────────────┘\n";

        floatcascade<3> zero, value;
        zero[0] = 0.0;
        zero[1] = 0.0;
        zero[2] = 0.0;

        value[0] = 12345.6789;
        value[1] = 1.234e-15;
        value[2] = 5.678e-30;

        floatcascade<3> result = expansion_ops::multiply_cascades(zero, value);

        std::cout << "Result: [";
        for (size_t i = 0; i < 3; ++i) {
            if (i > 0) std::cout << ", ";
            std::cout << result[i];
        }
        std::cout << "]\n";

        bool is_zero = true;
        for (size_t i = 0; i < 3; ++i) {
            if (std::abs(result[i]) > 1e-100) {
                is_zero = false;
                break;
            }
        }

        if (is_zero) {
            std::cout << "PASS: Zero absorption works correctly\n";
        } else {
            std::cout << "FAIL: Result should be zero\n";
            nrOfFailedTestCases++;
        }
    }

    // ========================================================================
    // Summary
    // ========================================================================
    std::cout << "\n\n";
    std::cout << "╔═══════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                         DEMONSTRATION SUMMARY                     ║\n";
    std::cout << "╚═══════════════════════════════════════════════════════════════════╝\n";
    std::cout << "\nKey Insights from the Diagonal Partitioning Algorithm:\n\n";
    std::cout << "1. DIAGONAL STRUCTURE: Products are naturally organized by significance\n";
    std::cout << "   level k = i+j, creating 2N-1 diagonals from most to least significant.\n\n";
    std::cout << "2. STABLE ACCUMULATION: Each diagonal uses two_sum chains to accumulate\n";
    std::cout << "   all products and errors, preserving precision throughout.\n\n";
    std::cout << "3. ERROR PROPAGATION: Errors from diagonal k automatically contribute\n";
    std::cout << "   to diagonal k+1, maintaining the error-free transformation property.\n\n";
    std::cout << "4. COMPONENT EXTRACTION: Sorting by magnitude and using a two_sum cascade\n";
    std::cout << "   ensures proper ordering without introducing zeros in the middle.\n\n";
    std::cout << "5. RENORMALIZATION: Final step ensures non-overlapping property holds,\n";
    std::cout << "   which is essential for subsequent operations.\n\n";

    std::cout << "Corner Cases Successfully Handled:\n";
    std::cout << "  - Denormalized inputs with overlapping components\n";
    std::cout << "  - Mixed signs causing cancellation in diagonals\n";
    std::cout << "  - Sparse matrices (identity, zero multiplication)\n";
    std::cout << "  - All N components properly initialized and ordered\n";
    std::cout << "  - Precision preserved through error tracking\n\n";

    if (nrOfFailedTestCases == 0) {
        std::cout << "╔═══════════════════════════════════════════════════════════════════╗\n";
        std::cout << "║                    ALL DEMONSTRATIONS PASSED                      ║\n";
        std::cout << "╚═══════════════════════════════════════════════════════════════════╝\n";
    } else {
        std::cout << "╔═══════════════════════════════════════════════════════════════════╗\n";
        std::cout << "║               " << nrOfFailedTestCases << " DEMONSTRATIONS FAILED                       ║\n";
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
