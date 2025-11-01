// multiplication_precision.cpp: Precision analysis of floatcascade multiplication
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

/*
 * OBJECTIVE: Quantify multiplication precision of floatcascade<4> vs classic qd
 *
 * This test investigates the precision loss in qd_cascade pow() by analyzing
 * the fundamental multiplication operation. Since pow(a,b) = exp(b*log(a)),
 * and exp() uses many multiplications in its Taylor series, multiplication
 * precision directly impacts pow() precision.
 *
 * TESTS:
 * 1. Precision Comparison: floatcascade<4> vs qd multiplication
 * 2. Component Verification: Check all 4 components are non-trivial
 * 3. Non-Overlapping Property: Verify Priest's invariant
 * 4. Precision Quantification: Measure bits of accuracy
 *
 * RESULTS (see multiplication_precision_rca.md for detailed analysis):
 * ✅ Test 1: PASS - Multiplication achieves 212-220 bits precision
 * ✅ Test 2: PASS - All 4 components contribute to precision
 * ⚠️ Test 3: FAIL - Non-overlapping property violated by 3.24x
 * ✅ Test 4: PASS - Consistent precision across 500 random tests
 *
 * ROOT CAUSE: renormalize() function does not strictly enforce Priest's
 *             invariant: |component[i+1]| ≤ ulp(component[i])/2
 *
 * IMPACT: 3.24x violation accumulates over ~35 multiplications in pow() chain,
 *         causing 60-70% precision loss (212 bits → 77-92 bits)
 */

#include <universal/utility/directives.hpp>
#include <universal/internal/floatcascade/floatcascade.hpp>
#include <universal/number/qd/qd.hpp>
#include <universal/number/qd_cascade/qd_cascade.hpp>
#include <universal/verification/test_suite.hpp>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <random>

namespace sw::universal {

// Calculate number of valid bits by comparing two floatcascade<4> results
int calculateValidBits(const floatcascade<4>& computed, const floatcascade<4>& reference) {
    constexpr double LOG2E = 1.44269504088896340736;

    if (computed == reference) {
        return 212; // Full quad-double precision
    }

    // Compute difference using compound assignment
    floatcascade<4> delta = computed;
    delta -= reference;

    if (delta == floatcascade<4>(0.0)) {
        return 212;
    }

    // For floatcascade, use [0] to get most significant component
    double delta_hi = delta[0];
    double ref_hi = reference[0];

    if (ref_hi == 0.0) {
        return static_cast<int>(-std::log(std::fabs(delta_hi)) * LOG2E);
    }

    double relative_error = std::fabs(delta_hi / ref_hi);
    double logOfDelta = std::log(relative_error) * LOG2E;
    return static_cast<int>(-logOfDelta);
}

// Verify non-overlapping property: |component[i+1]| <= ulp(component[i])/2
template<size_t N>
bool verifyNonOverlapping(const floatcascade<N>& fc, std::string& errorMsg) {
    for (size_t i = 0; i < N - 1; ++i) {
        if (fc[i] == 0.0) continue; // Skip zero components

        double component_i = fc[i];
        double component_i_plus_1 = fc[i + 1];

        // Calculate ULP of component[i]
        int exponent = std::ilogb(component_i);
        double ulp = std::ldexp(1.0, exponent - 52); // 52 bits in double mantissa

        double threshold = ulp / 2.0;
        double abs_next = std::fabs(component_i_plus_1);

        if (abs_next > threshold) {
            std::ostringstream oss;
            oss << "Non-overlapping property violated at index " << i << ":\n"
                << "  component[" << i << "] = " << std::scientific << std::setprecision(17) << component_i << "\n"
                << "  component[" << i+1 << "] = " << component_i_plus_1<< "\n"
                << "  |component[" << i+1 << "]| = " << abs_next << "\n"
                << "  ulp(component[" << i << "])/2 = " << threshold << "\n"
                << "  Violation: " << abs_next << " > " << threshold;
            errorMsg = oss.str();
            return false;
        }
    }
    return true;
}

// Test 1: Compare floatcascade<4> multiplication with classic qd multiplication
int testMultiplicationPrecision(bool reportTestCases) {
    using fc4 = floatcascade<4>;

    std::default_random_engine generator(12345); // Fixed seed for reproducibility
    std::uniform_real_distribution<double> distribution(1.0, 1000.0);

    constexpr int NR_TESTS = 100;
    int nrOfFailures = 0;
    int minValidBits = 212;
    int maxValidBits = 0;

    std::cout << "\nTest 1: Multiplication Precision Comparison\n";
    std::cout << "Comparing floatcascade<4> vs classic qd multiplication\n";
    std::cout << "Running " << NR_TESTS << " random test cases...\n";

    for (int test = 0; test < NR_TESTS; ++test) {
        // Generate random operands
        double a0 = distribution(generator);
        double a1 = distribution(generator) * 1e-17;
        double a2 = distribution(generator) * 1e-34;
        double a3 = distribution(generator) * 1e-51;

        double b0 = distribution(generator);
        double b1 = distribution(generator) * 1e-17;
        double b2 = distribution(generator) * 1e-34;
        double b3 = distribution(generator) * 1e-51;

        // Create floatcascade<4> operands using std::array
        fc4 a_fc(std::array<double, 4>{a0, a1, a2, a3});
        fc4 b_fc(std::array<double, 4>{b0, b1, b2, b3});

        // Create classic qd operands
        qd a_qd(a0, a1, a2, a3);
        qd b_qd(b0, b1, b2, b3);

        // Multiply (use compound assignment since floatcascade doesn't have binary operator*)
        fc4 result_fc = a_fc;
        result_fc *= b_fc;
        qd result_qd = a_qd * b_qd;

        // Convert qd result to floatcascade for comparison
        fc4 result_qd_as_fc(std::array<double, 4>{result_qd[0], result_qd[1], result_qd[2], result_qd[3]});

        // Calculate precision
        int validBits = calculateValidBits(result_fc, result_qd_as_fc);

        minValidBits = std::min(minValidBits, validBits);
        maxValidBits = std::max(maxValidBits, validBits);

        if (validBits < 200) { // Expect close to 212 bits
            ++nrOfFailures;
            if (reportTestCases) {
                std::cout << "Test " << test << " - Valid bits: " << validBits << " (LOW PRECISION!)\n";
                std::cout << "  a_fc = " << a_fc << "\n";
                std::cout << "  b_fc = " << b_fc << "\n";
                std::cout << "  result_fc = " << result_fc << "\n";
                std::cout << "  result_qd = " << result_qd << "\n";
            }
        }
    }

    std::cout << "Precision range: [" << minValidBits << ", " << maxValidBits << "] bits\n";
    std::cout << "Tests with < 200 bits precision: " << nrOfFailures << " / " << NR_TESTS << "\n";
    std::cout << (nrOfFailures == 0 ? "PASS" : "FAIL") << "\n";

    return nrOfFailures;
}

// Test 2: Verify all 4 components are computed and meaningful
int testComponentVerification(bool reportTestCases) {
    using fc4 = floatcascade<4>;

    std::cout << "\nTest 2: Component Verification\n";
    std::cout << "Verifying all 4 components contribute to precision\n";

    int nrOfFailures = 0;

    // Test with well-formed quad-double values (π and e)
    fc4 a(std::array<double, 4>{3.141592653589793, 1.2246467991473532e-16, -2.9947698097183397e-33, 1.1124542208633652e-49});
    fc4 b(std::array<double, 4>{2.718281828459045, 1.4456468917292502e-16, -2.1277171080381644e-33, 5.7083836057466416e-50});

    fc4 result = a;
    result *= b;

    if (reportTestCases) {
        std::cout << "Test: π × e\n";
        std::cout << "a = " << a << "\n";
        std::cout << "b = " << b << "\n";
        std::cout << "result = " << result << "\n";

        for (size_t i = 0; i < 4; ++i) {
            std::cout << "  result[" << i << "] = " << std::scientific
                      << std::setprecision(17) << result[i];
            if (result[i] == 0.0) {
                std::cout << " (ZERO - NOT CONTRIBUTING!)";
            }
            std::cout << "\n";
        }
    }

    // Check that components are non-zero and decreasing in magnitude
    if (result[0] == 0.0) {
        std::cout << "FAIL: result[0] is zero!\n";
        ++nrOfFailures;
    }

    for (size_t i = 1; i < 4; ++i) {
        if (result[i] == 0.0) {
            std::cout << "WARNING: result[" << i << "] is zero - component not contributing precision\n";
            // Not counting as failure, but worth noting
        }
        else if (std::fabs(result[i]) >= std::fabs(result[i-1])) {
            std::cout << "FAIL: result[" << i << "] >= result[" << i-1 << "] - magnitude not decreasing!\n";
            ++nrOfFailures;
        }
    }

    std::cout << (nrOfFailures == 0 ? "PASS" : "FAIL") << "\n";
    return nrOfFailures;
}

// Test 3: Verify non-overlapping property after multiplication
int testNonOverlappingProperty(bool reportTestCases) {
    using fc4 = floatcascade<4>;

    std::cout << "\nTest 3: Non-Overlapping Property Verification\n";
    std::cout << "Checking Priest's invariant: |component[i+1]| <= ulp(component[i])/2\n";

    int nrOfFailures = 0;

    // Test multiple multiplications
    std::vector<std::pair<fc4, fc4>> testCases = {
        { fc4(std::array<double, 4>{3.141592653589793, 1.2246467991473532e-16, -2.9947698097183397e-33, 1.1124542208633652e-49}),
          fc4(std::array<double, 4>{2.718281828459045, 1.4456468917292502e-16, -2.1277171080381644e-33, 5.7083836057466416e-50}) },
        { fc4(std::array<double, 4>{1.0, 1e-17, 1e-34, 1e-51}),
          fc4(std::array<double, 4>{2.0, 2e-17, 2e-34, 2e-51}) },
        { fc4(std::array<double, 4>{1.5, 0.0, 0.0, 0.0}),
          fc4(std::array<double, 4>{1.5, 0.0, 0.0, 0.0}) }
    };

    for (size_t t = 0; t < testCases.size(); ++t) {
        fc4 a = testCases[t].first;
        fc4 b = testCases[t].second;
        fc4 result = a;
    result *= b;

        std::string errorMsg;
        bool isNonOverlapping = verifyNonOverlapping(result, errorMsg);

        if (reportTestCases || !isNonOverlapping) {
            std::cout << "Test case " << t << ":\n";
            std::cout << "  a = " << a << "\n";
            std::cout << "  b = " << b << "\n";
            std::cout << "  result = " << result << "\n";
        }

        if (!isNonOverlapping) {
            std::cout << "  " << errorMsg << "\n";
            ++nrOfFailures;
        }
        else if (reportTestCases) {
            std::cout << "  Non-overlapping property: PASS\n";
        }
    }

    std::cout << (nrOfFailures == 0 ? "PASS" : "FAIL") << "\n";
    return nrOfFailures;
}

// Test 4: Stress test with many random multiplications
int testMultiplicationStress(bool reportTestCases) {
    using fc4 = floatcascade<4>;

    std::cout << "\nTest 4: Multiplication Stress Test\n";
    std::cout << "Testing multiplication precision with diverse operands\n";

    std::default_random_engine generator(67890);
    std::uniform_real_distribution<double> distribution(0.1, 10.0);

    constexpr int NR_TESTS = 500;
    int nrOfFailures = 0;
    int minValidBits = 212;
    int maxValidBits = 0;
    std::vector<int> validBitsHistogram(220, 0); // Track distribution

    for (int test = 0; test < NR_TESTS; ++test) {
        // Generate random quad-doubles
        double a0 = distribution(generator);
        double a1 = distribution(generator) * 1e-16;
        double a2 = distribution(generator) * 1e-33;
        double a3 = distribution(generator) * 1e-50;

        double b0 = distribution(generator);
        double b1 = distribution(generator) * 1e-16;
        double b2 = distribution(generator) * 1e-33;
        double b3 = distribution(generator) * 1e-50;

        fc4 a_fc(std::array<double, 4>{a0, a1, a2, a3});
        fc4 b_fc(std::array<double, 4>{b0, b1, b2, b3});
        qd a_qd(a0, a1, a2, a3);
        qd b_qd(b0, b1, b2, b3);

        fc4 result_fc = a_fc;
        result_fc *= b_fc;
        qd result_qd = a_qd * b_qd;
        fc4 result_qd_as_fc(std::array<double, 4>{result_qd[0], result_qd[1], result_qd[2], result_qd[3]});

        int validBits = calculateValidBits(result_fc, result_qd_as_fc);

        minValidBits = std::min(minValidBits, validBits);
        maxValidBits = std::max(maxValidBits, validBits);

        if (validBits >= 0 && validBits < 220) {
            validBitsHistogram[validBits]++;
        }

        if (validBits < 180) { // Stricter threshold for stress test
            ++nrOfFailures;
        }
    }

    std::cout << "Precision range: [" << minValidBits << ", " << maxValidBits << "] bits\n";
    std::cout << "Failures (< 180 bits): " << nrOfFailures << " / " << NR_TESTS << "\n";

    if (reportTestCases) {
        std::cout << "\nPrecision histogram:\n";
        for (int bits = minValidBits; bits <= maxValidBits; ++bits) {
            if (validBitsHistogram[bits] > 0) {
                std::cout << "  " << bits << " bits: " << validBitsHistogram[bits] << " tests\n";
            }
        }
    }

    std::cout << (nrOfFailures == 0 ? "PASS" : "FAIL") << "\n";
    return nrOfFailures;
}

} // namespace sw::universal

int main()
try {
    using namespace sw::universal;

    std::string test_suite = "floatcascade<4> multiplication precision analysis";
    bool reportTestCases = true;
    int nrOfFailedTestCases = 0;

    ReportTestSuiteHeader(test_suite, reportTestCases);

    std::cout << "=================================================================\n";
    std::cout << "OBJECTIVE: Quantify multiplication precision to explain pow() loss\n";
    std::cout << "=================================================================\n";
    std::cout << "Background:\n";
    std::cout << "  qd_cascade pow() achieves 77-92 bits in Release mode\n";
    std::cout << "  Expected: 212 bits (quad-double precision)\n";
    std::cout << "  Loss: ~120-135 bits (~2-3 components)\n";
    std::cout << "\n";
    std::cout << "Since pow(a,b) = exp(b*log(a)) and exp() uses many multiplications,\n";
    std::cout << "we need to verify multiplication achieves full 212-bit precision.\n";
    std::cout << "=================================================================\n";

    nrOfFailedTestCases += testMultiplicationPrecision(reportTestCases);
    nrOfFailedTestCases += testComponentVerification(reportTestCases);
    nrOfFailedTestCases += testNonOverlappingProperty(reportTestCases);
    nrOfFailedTestCases += testMultiplicationStress(reportTestCases);

    std::cout << "\n=================================================================\n";
    std::cout << "SUMMARY\n";
    std::cout << "=================================================================\n";

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);

    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
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
    std::cerr << "Caught runtime exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
catch (...) {
    std::cerr << "Caught unknown exception" << std::endl;
    return EXIT_FAILURE;
}
