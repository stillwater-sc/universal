// large_types.cpp: targeted tests for large areal configurations (nbits > 64)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
//
// These tests specifically exercise code paths unique to multi-block areals
// that are not covered by exhaustive enumeration of smaller types.
// The tests use carefully chosen values that trigger:
// - Double-to-areal conversion with fraction bits at TOP of large fields
// - Multi-block shift operations
// - Arithmetic with carry propagation across blocks
// - The ubit uncertainty tracking
//
// IMPORTANT: Multi-block areal requires BlockType <= 32 bits for portable
// carry propagation. Using uint64_t blocks with multi-block configurations
// will trigger a static_assert.

#include <universal/utility/directives.hpp>
#include <universal/number/areal/areal.hpp>
#include <universal/verification/test_reporters.hpp>

namespace sw { namespace universal {

// Test integer/double assignment for large areal types
// These values specifically exercise the double-to-areal conversion
// for types where fbits > 52 (double's precision)
template<typename ArealType>
int VerifyLargeConversion(bool reportTestCases) {
    int nrOfFailedTests = 0;
    ArealType a;

    // Test cases chosen to exercise different bit patterns:
    // - Powers of 2: test hidden bit handling
    // - Values with many fraction bits: test fraction placement at TOP
    // - Negative values: test sign handling
    // - Fractional values: test subnormal handling

    struct TestCase {
        double input;
        const char* description;
    };

    TestCase tests[] = {
        // Powers of 2 - exercise hidden bit, zero fraction
        {1.0, "2^0 - minimal"},
        {2.0, "2^1"},
        {64.0, "2^6"},
        {128.0, "2^7"},
        {1024.0, "2^10"},

        // Near powers of 2 - exercise fraction bits
        {3.0, "2^2-1, 1 fraction bit"},
        {7.0, "2^3-1, 2 fraction bits"},
        {15.0, "2^4-1, 3 fraction bits"},
        {63.0, "2^6-1, 5 fraction bits"},
        {127.0, "2^7-1, 6 fraction bits"},

        // Values from Muller recurrence - known to trigger bugs
        {111.0, "Muller constant - 7 bits"},
        {1130.0, "Muller constant - 11 bits"},
        {3000.0, "Muller constant - 12 bits"},

        // Negative values
        {-4.0, "negative power of 2"},
        {-111.0, "negative Muller constant"},
        {-1130.0, "negative large value"},

        // Fractional values - test precision
        {0.5, "1/2"},
        {0.25, "1/4"},
        {0.125, "1/8"},
        {1.5, "3/2"},
        {2.5, "5/2"},

        // Values that fill more bits
        {255.0, "8 bits all ones"},
        {1023.0, "10 bits all ones"},
        {4095.0, "12 bits all ones"},
        {65535.0, "16 bits all ones"},
    };

    for (const auto& test : tests) {
        a = test.input;
        double result = double(a);

        if (result != test.input) {
            ++nrOfFailedTests;
            if (reportTestCases) {
                std::cerr << "FAIL: " << typeid(ArealType).name()
                          << "(" << test.input << ") = " << result
                          << " expected " << test.input
                          << " [" << test.description << "]\n";
            }
        }
    }

    return nrOfFailedTests;
}

// Test basic arithmetic that exercises multi-block operations
template<typename ArealType>
int VerifyLargeArithmetic(bool reportTestCases) {
    int nrOfFailedTests = 0;

    struct ArithmeticTest {
        double a, b;
        double sum, diff, prod, quot;
        const char* description;
    };

    // Test cases chosen to exercise:
    // - Addition/subtraction with different exponents (alignment shifts)
    // - Multiplication producing results that span blocks
    // - Division with exact results (to avoid rounding comparison issues)
    ArithmeticTest tests[] = {
        // Basic integer arithmetic
        {2.0, -4.0, -2.0, 6.0, -8.0, -0.5, "small integers"},
        {111.0, 1130.0, 1241.0, -1019.0, 125430.0, 111.0/1130.0, "Muller constants"},

        // Values that test fraction alignment
        {1.5, 0.25, 1.75, 1.25, 0.375, 6.0, "fractional values"},
        {100.5, 0.125, 100.625, 100.375, 12.5625, 804.0, "mixed magnitude"},

        // Large values
        {1024.0, 512.0, 1536.0, 512.0, 524288.0, 2.0, "powers of 2"},
        {3000.0, -8.0, 2992.0, 3008.0, -24000.0, -375.0, "Muller division"},
    };

    ArealType a, b, result;

    for (const auto& test : tests) {
        a = test.a;
        b = test.b;

        // Test addition
        result = a + b;
        if (double(result) != test.sum) {
            ++nrOfFailedTests;
            if (reportTestCases) {
                std::cerr << "FAIL: " << test.a << " + " << test.b
                          << " = " << double(result) << " expected " << test.sum
                          << " [" << test.description << "]\n";
            }
        }

        // Test subtraction
        result = a - b;
        if (double(result) != test.diff) {
            ++nrOfFailedTests;
            if (reportTestCases) {
                std::cerr << "FAIL: " << test.a << " - " << test.b
                          << " = " << double(result) << " expected " << test.diff
                          << " [" << test.description << "]\n";
            }
        }

        // Test multiplication
        result = a * b;
        if (double(result) != test.prod) {
            ++nrOfFailedTests;
            if (reportTestCases) {
                std::cerr << "FAIL: " << test.a << " * " << test.b
                          << " = " << double(result) << " expected " << test.prod
                          << " [" << test.description << "]\n";
            }
        }

        // Test division (use epsilon comparison for non-terminating results)
        result = a / b;
        double quot_result = double(result);
        double quot_error = std::abs(quot_result - test.quot);
        double quot_tolerance = std::abs(test.quot) * 1e-14 + 1e-15;
        if (quot_error > quot_tolerance) {
            ++nrOfFailedTests;
            if (reportTestCases) {
                std::cerr << "FAIL: " << test.a << " / " << test.b
                          << " = " << quot_result << " expected " << test.quot
                          << " error=" << quot_error
                          << " [" << test.description << "]\n";
            }
        }
    }

    return nrOfFailedTests;
}

// The Muller recurrence step - a compound test that exercises
// multiple operations in sequence, known to expose rounding issues
template<typename ArealType>
int VerifyMullerStep(bool reportTestCases) {
    int nrOfFailedTests = 0;

    // v[3] = 111 - 1130/v[2] + 3000/(v[2]*v[1])
    // where v[1] = 2, v[2] = -4
    // Expected: v[3] = 18.5

    ArealType v1(2), v2(-4);
    ArealType c111(111), c1130(1130), c3000(3000);

    ArealType v3 = c111 - c1130 / v2 + c3000 / (v2 * v1);

    double result = double(v3);
    double expected = 18.5;

    // Allow small epsilon for floating-point comparison
    if (std::abs(result - expected) > 1e-10) {
        ++nrOfFailedTests;
        if (reportTestCases) {
            std::cerr << "FAIL: Muller step v[3] = " << result
                      << " expected " << expected << "\n";
            std::cerr << "  v1 = " << double(v1) << ", v2 = " << double(v2) << "\n";
            std::cerr << "  111 = " << double(c111) << "\n";
            std::cerr << "  1130 = " << double(c1130) << "\n";
            std::cerr << "  3000 = " << double(c3000) << "\n";
        }
    }

    return nrOfFailedTests;
}

// Test that ubit is correctly tracked through operations
template<typename ArealType>
int VerifyUbitTracking(bool reportTestCases) {
    int nrOfFailedTests = 0;

    // Division that produces non-terminating result should set ubit
    ArealType a(1), b(3);
    ArealType result = a / b;  // 1/3 is non-terminating

    // The ubit should be set (bit 0)
    if (!result.at(0)) {
        ++nrOfFailedTests;
        if (reportTestCases) {
            std::cerr << "FAIL: 1/3 should have ubit set\n";
        }
    }

    // Exact operations should not set ubit
    ArealType c(4), d(2);
    result = c / d;  // 4/2 = 2 exactly

    if (result.at(0)) {
        ++nrOfFailedTests;
        if (reportTestCases) {
            std::cerr << "FAIL: 4/2 should not have ubit set\n";
        }
    }

    return nrOfFailedTests;
}

}} // namespace sw::universal

// Regression testing guards
#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main()
try {
    using namespace sw::universal;

    std::string test_suite = "areal large type arithmetic";
    std::string test_tag = "large_types";
    bool reportTestCases = true;
    int nrOfFailedTestCases = 0;

    ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

    // Manual testing for debugging
    {
        using Areal128 = areal<128, 15, uint32_t>;
        Areal128 a(111);
        std::cout << "areal<128,15>(111) = " << double(a) << "\n";
        std::cout << "binary: " << to_binary(a) << "\n";
    }

#else // !MANUAL_TESTING

#if REGRESSION_LEVEL_1

    // Large areal configurations that exercise multi-block code paths
    // MUST use uint32_t blocks for portable carry propagation

    std::cout << "\nTesting areal<80,11> (IEEE extended precision equivalent)\n";
    using Areal80 = areal<80, 11, uint32_t>;
    nrOfFailedTestCases += ReportTestResult(
        VerifyLargeConversion<Areal80>(reportTestCases),
        "areal<80,11>", "conversion");
    nrOfFailedTestCases += ReportTestResult(
        VerifyLargeArithmetic<Areal80>(reportTestCases),
        "areal<80,11>", "arithmetic");
    nrOfFailedTestCases += ReportTestResult(
        VerifyMullerStep<Areal80>(reportTestCases),
        "areal<80,11>", "Muller step");
    nrOfFailedTestCases += ReportTestResult(
        VerifyUbitTracking<Areal80>(reportTestCases),
        "areal<80,11>", "ubit tracking");

    std::cout << "\nTesting areal<128,15> (IEEE quad precision equivalent)\n";
    using Areal128 = areal<128, 15, uint32_t>;
    nrOfFailedTestCases += ReportTestResult(
        VerifyLargeConversion<Areal128>(reportTestCases),
        "areal<128,15>", "conversion");
    nrOfFailedTestCases += ReportTestResult(
        VerifyLargeArithmetic<Areal128>(reportTestCases),
        "areal<128,15>", "arithmetic");
    nrOfFailedTestCases += ReportTestResult(
        VerifyMullerStep<Areal128>(reportTestCases),
        "areal<128,15>", "Muller step");
    nrOfFailedTestCases += ReportTestResult(
        VerifyUbitTracking<Areal128>(reportTestCases),
        "areal<128,15>", "ubit tracking");

    std::cout << "\nTesting areal<256,19> (octuple precision)\n";
    using Areal256 = areal<256, 19, uint32_t>;
    nrOfFailedTestCases += ReportTestResult(
        VerifyLargeConversion<Areal256>(reportTestCases),
        "areal<256,19>", "conversion");
    nrOfFailedTestCases += ReportTestResult(
        VerifyLargeArithmetic<Areal256>(reportTestCases),
        "areal<256,19>", "arithmetic");
    nrOfFailedTestCases += ReportTestResult(
        VerifyMullerStep<Areal256>(reportTestCases),
        "areal<256,19>", "Muller step");
    nrOfFailedTestCases += ReportTestResult(
        VerifyUbitTracking<Areal256>(reportTestCases),
        "areal<256,19>", "ubit tracking");

#endif

#if REGRESSION_LEVEL_2
    // Additional large configurations
    std::cout << "\nTesting areal<160,15>\n";
    using Areal160 = areal<160, 15, uint32_t>;
    nrOfFailedTestCases += ReportTestResult(
        VerifyLargeConversion<Areal160>(reportTestCases),
        "areal<160,15>", "conversion");
    nrOfFailedTestCases += ReportTestResult(
        VerifyMullerStep<Areal160>(reportTestCases),
        "areal<160,15>", "Muller step");
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
#endif

#endif // MANUAL_TESTING

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
    std::cerr << "Caught unexpected runtime exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
catch (...) {
    std::cerr << "Caught unknown exception" << std::endl;
    return EXIT_FAILURE;
}
