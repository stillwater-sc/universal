// renormalize_improvement.cpp: test improved two-phase renormalization algorithm
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project: https://github.com/stillwater-sc/universal

#include <universal/utility/directives.hpp>
#include <universal/number/qd_cascade/qd_cascade.hpp>
#include <universal/number/td_cascade/td_cascade.hpp>
#include <universal/internal/floatcascade/floatcascade.hpp>
#include <universal/verification/test_suite.hpp>
#include <limits>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <random>

namespace sw {
namespace universal {
namespace internal {

// ===========================================================================
// TWO-PHASE RENORMALIZATION ALGORITHM (based on Hida-Li-Bailey QD library)
// ===========================================================================

// Improved renormalize using two-phase approach from QD library
// Phase 1: Compression pass (bottom-up accumulation)
// Phase 2: Conditional refinement (carry propagation with zero detection)

template<size_t N>
void renormalize_twophase(floatcascade<N>& fc) {
    // Handle infinity early
    if (std::isinf(fc[0])) return;

    // Temporary storage for error terms
    std::array<double, N> s;
    s.fill(0.0);

    // ===== PHASE 1: Compression =====
    // Accumulate from bottom to top using quick_two_sum
    // After this phase: fc[0] + s[1] + s[2] + ... + s[N-1] = original sum

    double sum = fc[N-1];
    for (int i = static_cast<int>(N) - 2; i >= 0; --i) {
        sum = quick_two_sum(fc[i], sum, s[i+1]);
    }
    s[0] = sum;

    // ===== PHASE 2: Conditional Refinement =====
    // Propagate carries with zero detection to ensure non-overlapping property
    // This is the critical step missing from the original single-pass algorithm

    // Implementation strategy for generic N:
    // - Iterate through components
    // - If component is non-zero, propagate next error term through it
    // - If component is zero, try to accumulate error into previous component
    // - Continue until all error terms are distributed

    // Start with s[0] as base
    fc[0] = s[0];

    // Process remaining components with conditional refinement
    for (size_t i = 1; i < N; ++i) {
        if (i == N-1) {
            // Last component: just assign remaining error
            fc[i] = s[i];
        } else {
            // Intermediate component: refine with next error term
            if (s[i] != 0.0) {
                // Propagate s[i+1] through s[i]
                fc[i] = quick_two_sum(s[i], fc[i+1], s[i+1]);
            } else {
                // s[i] is zero, try to absorb into previous component
                double prev = fc[i-1];
                fc[i-1] = quick_two_sum(prev, fc[i+1], fc[i]);
                s[i+1] = 0.0;  // Error absorbed
            }
        }
    }
}

// Specialization for N=4 (quad-double) - matches QD library structure exactly
template<>
void renormalize_twophase<4>(floatcascade<4>& fc) {
    double s0, s1, s2 = 0.0, s3 = 0.0;

    // Handle infinity
    if (std::isinf(fc[0])) return;

    // ===== PHASE 1: Compression =====
    s0 = quick_two_sum(fc[2], fc[3], fc[3]);
    s0 = quick_two_sum(fc[1], s0, fc[2]);
    fc[0] = quick_two_sum(fc[0], s0, fc[1]);

    // ===== PHASE 2: Conditional Refinement =====
    s0 = fc[0];
    s1 = fc[1];

    if (s1 != 0.0) {
        s1 = quick_two_sum(s1, fc[2], s2);
        if (s2 != 0.0)
            s2 = quick_two_sum(s2, fc[3], s3);
        else
            s1 = quick_two_sum(s1, fc[3], s2);
    } else {
        s0 = quick_two_sum(s0, fc[2], s1);
        if (s1 != 0.0)
            s1 = quick_two_sum(s1, fc[3], s2);
        else
            s0 = quick_two_sum(s0, fc[3], s1);
    }

    fc[0] = s0;
    fc[1] = s1;
    fc[2] = s2;
    fc[3] = s3;
}

// Specialization for N=3 (triple-double)
template<>
void renormalize_twophase<3>(floatcascade<3>& fc) {
    double s0, s1, s2 = 0.0;

    // Handle infinity
    if (std::isinf(fc[0])) return;

    // ===== PHASE 1: Compression =====
    s0 = quick_two_sum(fc[1], fc[2], fc[2]);
    fc[0] = quick_two_sum(fc[0], s0, fc[1]);

    // ===== PHASE 2: Conditional Refinement =====
    s0 = fc[0];
    s1 = fc[1];

    if (s1 != 0.0) {
        s1 = quick_two_sum(s1, fc[2], s2);
    } else {
        s0 = quick_two_sum(s0, fc[2], s1);
    }

    fc[0] = s0;
    fc[1] = s1;
    fc[2] = s2;
}

// Specialization for N=2 (double-double)
template<>
void renormalize_twophase<2>(floatcascade<2>& fc) {
    // For N=2, single quick_two_sum is sufficient
    if (std::isinf(fc[0])) return;
    fc[0] = quick_two_sum(fc[0], fc[1], fc[1]);
}

} // namespace internal
} // namespace universal
} // namespace sw

// ===========================================================================
// TEST SUITE
// ===========================================================================

using namespace sw::universal;
using namespace sw::universal::internal;

// Verify non-overlapping property
template<size_t N>
bool verifyNonOverlapping(const floatcascade<N>& fc, std::string& errorMsg, double& maxViolation) {
    maxViolation = 0.0;
    std::ostringstream oss;

    for (size_t i = 0; i < N - 1; ++i) {
        if (fc[i] == 0.0) continue;

        double component_i = fc[i];
        double component_i_plus_1 = fc[i + 1];

        int exponent = std::ilogb(component_i);
        double ulp = std::ldexp(1.0, exponent - 52);
        double threshold = ulp / 2.0;
        double abs_next = std::fabs(component_i_plus_1);

        if (abs_next > threshold) {
            double violation = abs_next / threshold;
            maxViolation = std::max(maxViolation, violation);

            oss << "Non-overlapping property violated at index " << i << ":\n"
                << "  component[" << i << "] = " << std::scientific << std::setprecision(17) << component_i << "\n"
                << "  |component[" << i+1 << "]| = " << abs_next << "\n"
                << "  ulp(component[" << i << "])/2 = " << threshold << "\n"
                << "  Violation factor: " << std::fixed << std::setprecision(3) << violation << "x\n";
        }
    }

    if (maxViolation > 0.0) {
        oss << "\nMaximum violation: " << std::fixed << std::setprecision(3) << maxViolation << "x";
        errorMsg = oss.str();
        return false;
    }

    return true;
}

// Test 1: Compare old vs new renormalize on multiplication results
int TestMultiplicationRenormalization() {
    using fc4 = floatcascade<4>;
    int nrOfFailedTests = 0;

    std::cout << "Test 1: Multiplication Renormalization Comparison\n";
    std::cout << "===================================================\n\n";

    // Create test cases that trigger renormalization issues
    struct TestCase {
        std::string name;
        double a0, a1, a2, a3;
        double b0, b1, b2, b3;
    };

    std::vector<TestCase> tests = {
        {"Powers of 2", 1.0, 0.5, 0.25, 0.125, 2.0, 1.0, 0.5, 0.25},
        {"Large exponent diff", 1e100, 1e47, 1e-6, 1e-59, 1e50, 1e-3, 1e-56, 1e-109},
        {"Near 1 values", 1.0, 1e-16, 1e-32, 1e-48, 1.0, 2e-16, 3e-32, 4e-48},
    };

    for (const auto& test : tests) {
        std::cout << "Test case: " << test.name << "\n";

        fc4 a(std::array<double, 4>{test.a0, test.a1, test.a2, test.a3});
        fc4 b(std::array<double, 4>{test.b0, test.b1, test.b2, test.b3});

        // Multiply
        fc4 result_old = a;
        result_old *= b;
        // result_old now uses old renormalize()

        fc4 result_new = a;
        result_new *= b;
        // Apply new renormalize
        renormalize_twophase(result_new);

        // Verify non-overlapping property
        std::string errorMsg_old, errorMsg_new;
        double violation_old, violation_new;

        bool old_ok = verifyNonOverlapping(result_old, errorMsg_old, violation_old);
        bool new_ok = verifyNonOverlapping(result_new, errorMsg_new, violation_new);

        std::cout << "  Old renormalize: ";
        if (old_ok) {
            std::cout << "PASS (max violation: 0.0x)\n";
        } else {
            std::cout << "FAIL (max violation: " << std::fixed << std::setprecision(3) << violation_old << "x)\n";
            std::cout << "    " << errorMsg_old << "\n";
        }

        std::cout << "  New renormalize: ";
        if (new_ok) {
            std::cout << "PASS (max violation: 0.0x)\n";
        } else {
            std::cout << "FAIL (max violation: " << std::fixed << std::setprecision(3) << violation_new << "x)\n";
            std::cout << "    " << errorMsg_new << "\n";
            ++nrOfFailedTests;
        }

        // Check that values are still equal (renormalization should not change value)
        double sum_old = 0.0, sum_new = 0.0;
        for (size_t i = 0; i < 4; ++i) {
            sum_old += result_old[i];
            sum_new += result_new[i];
        }
        if (sum_old != sum_new) {
            std::cout << "  ERROR: Sums differ! Old=" << sum_old << ", New=" << sum_new << "\n";
            ++nrOfFailedTests;
        }

        std::cout << "\n";
    }

    return nrOfFailedTests;
}

// Test 2: Verify all N values (2, 3, 4, 8)
int TestMultipleSizes() {
    int nrOfFailedTests = 0;

    std::cout << "Test 2: Verify Renormalization for N ∈ {2, 3, 4, 8}\n";
    std::cout << "====================================================\n\n";

    // Test N=2
    {
        floatcascade<2> fc2(std::array<double, 2>{1.0, 1e-16});
        renormalize_twophase(fc2);
        std::string err;
        double viol;
        if (!verifyNonOverlapping(fc2, err, viol)) {
            std::cout << "FAIL: N=2 renormalization violated property (violation: " << viol << "x)\n";
            ++nrOfFailedTests;
        } else {
            std::cout << "PASS: N=2 renormalization\n";
        }
    }

    // Test N=3
    {
        floatcascade<3> fc3(std::array<double, 3>{1.0, 1e-16, 1e-32});
        renormalize_twophase(fc3);
        std::string err;
        double viol;
        if (!verifyNonOverlapping(fc3, err, viol)) {
            std::cout << "FAIL: N=3 renormalization violated property (violation: " << viol << "x)\n";
            ++nrOfFailedTests;
        } else {
            std::cout << "PASS: N=3 renormalization\n";
        }
    }

    // Test N=4
    {
        floatcascade<4> fc4(std::array<double, 4>{1.0, 1e-16, 1e-32, 1e-48});
        renormalize_twophase(fc4);
        std::string err;
        double viol;
        if (!verifyNonOverlapping(fc4, err, viol)) {
            std::cout << "FAIL: N=4 renormalization violated property (violation: " << viol << "x)\n";
            ++nrOfFailedTests;
        } else {
            std::cout << "PASS: N=4 renormalization\n";
        }
    }

    // Test N=8 (octo-double)
    {
        floatcascade<8> fc8(std::array<double, 8>{1.0, 1e-16, 1e-32, 1e-48, 1e-64, 1e-80, 1e-96, 1e-112});
        renormalize_twophase(fc8);
        std::string err;
        double viol;
        if (!verifyNonOverlapping(fc8, err, viol)) {
            std::cout << "FAIL: N=8 renormalization violated property (violation: " << viol << "x)\n";
            ++nrOfFailedTests;
        } else {
            std::cout << "PASS: N=8 renormalization\n";
        }
    }

    std::cout << "\n";
    return nrOfFailedTests;
}

// Test 3: Stress test with random multiplications
int TestRandomMultiplications() {
    using fc4 = floatcascade<4>;
    int nrOfFailedTests = 0;
    constexpr int NUM_TESTS = 1000;

    std::cout << "Test 3: Random Multiplication Stress Test (" << NUM_TESTS << " cases)\n";
    std::cout << "================================================================\n\n";

    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_real_distribution<double> dist(1.0, 1048576.0);

    int violations = 0;
    double max_violation = 0.0;

    for (int i = 0; i < NUM_TESTS; ++i) {
        // Create random quad-double values
        double a = dist(gen);
        double b = dist(gen);

        fc4 fc_a(a);
        fc4 fc_b(b);

        fc4 result = fc_a;
        result *= fc_b;

        // Apply new renormalize
        renormalize_twophase(result);

        // Verify
        std::string err;
        double viol;
        if (!verifyNonOverlapping(result, err, viol)) {
            violations++;
            max_violation = std::max(max_violation, viol);
        }
    }

    std::cout << "Results:\n";
    std::cout << "  Tests run: " << NUM_TESTS << "\n";
    std::cout << "  Violations: " << violations << "\n";
    std::cout << "  Max violation: " << std::fixed << std::setprecision(3) << max_violation << "x\n";

    if (violations > 0) {
        std::cout << "  FAIL: " << violations << " violations detected\n";
        nrOfFailedTests = violations;
    } else {
        std::cout << "  PASS: No violations detected\n";
    }

    std::cout << "\n";
    return nrOfFailedTests;
}

int main()
try {
    using namespace sw::universal;

    std::cout << "Improved Renormalization Algorithm Test Suite\n";
    std::cout << "Based on Hida-Li-Bailey QD Library Two-Phase Approach\n";
    std::cout << "=======================================================\n\n";

    int nrOfFailedTests = 0;

    nrOfFailedTests += TestMultiplicationRenormalization();
    nrOfFailedTests += TestMultipleSizes();
    nrOfFailedTests += TestRandomMultiplications();

    std::cout << "\n=======================================================\n";
    std::cout << "Final Results: ";
    if (nrOfFailedTests == 0) {
        std::cout << "ALL TESTS PASSED\n";
    } else {
        std::cout << nrOfFailedTests << " TESTS FAILED\n";
    }

    return (nrOfFailedTests > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
    std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
    return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
    std::cerr << "Caught unexpected universal arithmetic exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
    std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
    std::cerr << "Caught unexpected runtime exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
catch (...) {
    std::cerr << "Caught unknown exception" << std::endl;
    return EXIT_FAILURE;
}
