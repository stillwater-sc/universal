// large_types.cpp: targeted tests for large cfloat configurations (nbits > 64)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
//
// These tests specifically exercise code paths unique to multi-block cfloats
// that are not covered by exhaustive enumeration of smaller types.
// The tests use carefully chosen values that trigger:
// - Integer conversion with fraction bits at TOP of large fraction fields
// - Multi-block shift operations
// - Arithmetic with carry propagation across blocks
// - The round() function with large shifts

#include <universal/utility/directives.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_reporters.hpp>

namespace sw { namespace universal {

// Test integer assignment for large cfloat types
// These values specifically exercise the convert_signed_integer code path
// for types where fbits >= (64 - es)
template<typename CfloatType>
int VerifyLargeIntegerConversion(bool reportTestCases) {
    int nrOfFailedTests = 0;
    CfloatType a;

    // Test cases chosen to exercise different bit patterns:
    // - Powers of 2: test hidden bit handling
    // - Values with many fraction bits: test fraction placement at TOP
    // - Negative values: test sign handling

    struct TestCase {
        int input;  // Use int to match how constructors are typically called
        const char* description;
    };

    TestCase tests[] = {
        // Powers of 2 - exercise hidden bit, zero fraction
        {1, "2^0 - minimal"},
        {2, "2^1"},
        {64, "2^6"},
        {128, "2^7"},
        {1024, "2^10"},

        // Near powers of 2 - exercise fraction bits
        {3, "2^2-1, 1 fraction bit"},
        {7, "2^3-1, 2 fraction bits"},
        {15, "2^4-1, 3 fraction bits"},
        {63, "2^6-1, 5 fraction bits"},
        {127, "2^7-1, 6 fraction bits"},

        // Values from Muller recurrence - known to trigger bugs
        {111, "Muller constant - 7 bits"},
        {1130, "Muller constant - 11 bits"},
        {3000, "Muller constant - 12 bits"},

        // Negative values
        {-4, "negative power of 2"},
        {-111, "negative Muller constant"},
        {-1130, "negative large value"},

        // Values that fill more bits
        {255, "8 bits all ones"},
        {1023, "10 bits all ones"},
        {4095, "12 bits all ones"},
        {65535, "16 bits all ones"},
    };

    for (const auto& test : tests) {
        a = test.input;
        double result = double(a);
        double expected = static_cast<double>(test.input);

        if (result != expected) {
            ++nrOfFailedTests;
            if (reportTestCases) {
                std::cerr << "FAIL: " << typeid(CfloatType).name()
                          << "(" << test.input << ") = " << result
                          << " expected " << expected
                          << " [" << test.description << "]\n";
            }
        }
    }

    return nrOfFailedTests;
}

// Test unsigned integer assignment for large cfloat types
template<typename CfloatType>
int VerifyLargeUnsignedConversion(bool reportTestCases) {
    int nrOfFailedTests = 0;
    CfloatType a;

    struct TestCase {
        unsigned int input;
        const char* description;
    };

    TestCase tests[] = {
        // Powers of 2
        {1u, "2^0"},
        {64u, "2^6"},
        {1024u, "2^10"},

        // Near powers of 2
        {127u, "2^7-1"},
        {255u, "2^8-1"},

        // Muller constants
        {111u, "Muller constant"},
        {1130u, "Muller constant"},
        {3000u, "Muller constant"},

        // Large values
        {65535u, "16 bits all ones"},
        {100000u, "100k"},
    };

    for (const auto& test : tests) {
        a = test.input;
        double result = double(a);
        double expected = static_cast<double>(test.input);

        if (result != expected) {
            ++nrOfFailedTests;
            if (reportTestCases) {
                std::cerr << "FAIL unsigned: " << typeid(CfloatType).name()
                          << "(" << test.input << ") = " << result
                          << " expected " << expected
                          << " [" << test.description << "]\n";
            }
        }
    }

    return nrOfFailedTests;
}

// Test basic arithmetic that exercises multi-block operations
template<typename CfloatType>
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
    // - Division with non-terminating results
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

        // Values near 1
        {1.0, 1e-10, 1.0 + 1e-10, 1.0 - 1e-10, 1e-10, 1e10, "near unity"},
    };

    CfloatType a, b, result;

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
template<typename CfloatType>
int VerifyMullerStep(bool reportTestCases) {
    int nrOfFailedTests = 0;

    // v[3] = 111 - 1130/v[2] + 3000/(v[2]*v[1])
    // where v[1] = 2, v[2] = -4
    // Expected: v[3] = 18.5

    CfloatType v1(2), v2(-4);
    CfloatType c111(111), c1130(1130), c3000(3000);

    CfloatType v3 = c111 - c1130 / v2 + c3000 / (v2 * v1);

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

}} // namespace sw::universal

// Regression testing guards
#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main()
try {
    using namespace sw::universal;

    std::string test_suite = "cfloat large type arithmetic";
    std::string test_tag = "large_types";
    bool reportTestCases = true;
    int nrOfFailedTestCases = 0;

    ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

    // Manual testing for debugging
    {
        using Cfloat128 = cfloat<128, 15, uint32_t, true, false, false>;
        Cfloat128 a(111);
        std::cout << "cfloat<128,15>(111) = " << double(a) << "\n";
        std::cout << "binary: " << to_binary(a) << "\n";
    }

#else // !MANUAL_TESTING

#if REGRESSION_LEVEL_1

    // Large cfloat configurations that exercise multi-block code paths
    // Using uint32_t blocks for portable carry propagation

    std::cout << "\nTesting cfloat<80,11> (IEEE extended precision equivalent)\n";
    using Cfloat80 = cfloat<80, 11, uint32_t, true, false, false>;
    nrOfFailedTestCases += ReportTestResult(
        VerifyLargeIntegerConversion<Cfloat80>(reportTestCases),
        "cfloat<80,11>", "signed integer conversion");
    nrOfFailedTestCases += ReportTestResult(
        VerifyLargeUnsignedConversion<Cfloat80>(reportTestCases),
        "cfloat<80,11>", "unsigned integer conversion");
    nrOfFailedTestCases += ReportTestResult(
        VerifyLargeArithmetic<Cfloat80>(reportTestCases),
        "cfloat<80,11>", "arithmetic");
    nrOfFailedTestCases += ReportTestResult(
        VerifyMullerStep<Cfloat80>(reportTestCases),
        "cfloat<80,11>", "Muller step");

    std::cout << "\nTesting cfloat<128,15> (IEEE quad precision equivalent)\n";
    using Cfloat128 = cfloat<128, 15, uint32_t, true, false, false>;
    nrOfFailedTestCases += ReportTestResult(
        VerifyLargeIntegerConversion<Cfloat128>(reportTestCases),
        "cfloat<128,15>", "signed integer conversion");
    nrOfFailedTestCases += ReportTestResult(
        VerifyLargeUnsignedConversion<Cfloat128>(reportTestCases),
        "cfloat<128,15>", "unsigned integer conversion");
    nrOfFailedTestCases += ReportTestResult(
        VerifyLargeArithmetic<Cfloat128>(reportTestCases),
        "cfloat<128,15>", "arithmetic");
    nrOfFailedTestCases += ReportTestResult(
        VerifyMullerStep<Cfloat128>(reportTestCases),
        "cfloat<128,15>", "Muller step");

    std::cout << "\nTesting cfloat<256,19> (octuple precision)\n";
    using Cfloat256 = cfloat<256, 19, uint32_t, true, false, false>;
    nrOfFailedTestCases += ReportTestResult(
        VerifyLargeIntegerConversion<Cfloat256>(reportTestCases),
        "cfloat<256,19>", "signed integer conversion");
    nrOfFailedTestCases += ReportTestResult(
        VerifyLargeUnsignedConversion<Cfloat256>(reportTestCases),
        "cfloat<256,19>", "unsigned integer conversion");
    nrOfFailedTestCases += ReportTestResult(
        VerifyLargeArithmetic<Cfloat256>(reportTestCases),
        "cfloat<256,19>", "arithmetic");
    nrOfFailedTestCases += ReportTestResult(
        VerifyMullerStep<Cfloat256>(reportTestCases),
        "cfloat<256,19>", "Muller step");

#endif

#if REGRESSION_LEVEL_2
    // Additional large configurations
    std::cout << "\nTesting cfloat<160,15>\n";
    using Cfloat160 = cfloat<160, 15, uint32_t, true, false, false>;
    nrOfFailedTestCases += ReportTestResult(
        VerifyLargeIntegerConversion<Cfloat160>(reportTestCases),
        "cfloat<160,15>", "integer conversion");
    nrOfFailedTestCases += ReportTestResult(
        VerifyMullerStep<Cfloat160>(reportTestCases),
        "cfloat<160,15>", "Muller step");
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
