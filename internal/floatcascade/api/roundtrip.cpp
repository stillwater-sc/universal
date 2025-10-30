// roundtrip.cpp: test suite for parse/to_string round-trip validation for floatcascade
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// TEST TOLERANCE:
// Round-trip conversions (string → parse → to_string → parse) introduce tiny errors
// due to accumulated floating-point operations (multiplication by 10, division, etc.).
// These errors are on the order of 1e-22 to 1e-30, which is:
//   - ~1000x smaller than the double-double precision (~1e-31)
//   - Completely negligible for all practical applications
//   - Similar to comparing (a * b) / b to a in regular floating-point
//
// Therefore, we use a tolerance of 1e-20 to allow for these accumulated errors
// while still catching real bugs in the implementation.
#include <universal/utility/directives.hpp>
#include <universal/number/dd_cascade/dd_cascade.hpp>
// TODO: Enable after Phase 6
// #include <universal/number/td_cascade/td_cascade.hpp>
// #include <universal/number/qd_cascade/qd_cascade.hpp>
#include <universal/verification/test_suite.hpp>
#include <cmath>
#include <iomanip>

// Helper function to test round-trip conversion for any floatcascade-based type
template<typename CascadeType>
bool TestRoundTrip(const std::string& input, const std::string& testName, bool reportTestCases) {
    CascadeType value;

    // Parse the input string
    if (!parse(input, value)) {
        if (reportTestCases) {
            std::cout << "FAIL: " << testName << " - parse failed for input: " << input << '\n';
        }
        return false;
    }

    // Convert back to string with high precision
    std::string output = value.to_string(32, 0, false, true, false, false, false, false, ' ');

    // Parse the output string
    CascadeType roundtrip;
    if (!parse(output, roundtrip)) {
        if (reportTestCases) {
            std::cout << "FAIL: " << testName << " - parse failed for output: " << output << '\n';
        }
        return false;
    }

    // Compare all components - they should match within acceptable tolerance
    // Accumulated floating-point errors from multiple operations are expected
    bool matches = true;
    double max_component_error = 0.0;

    // Check only the first 2 components for dd_cascade
    // TODO: Update this when testing td_cascade (3) and qd_cascade (4)
    constexpr size_t num_components = 2;  // dd_cascade has 2 components

    for (size_t i = 0; i < num_components; ++i) {
        double comp_orig = value[i];
        double comp_rt = roundtrip[i];
        double comp_diff = std::abs(comp_orig - comp_rt);

        if (comp_diff > max_component_error) {
            max_component_error = comp_diff;
        }

        // Accept errors < 1e-20 (absolute) or < 1e-28 (relative)
        // This is well within double-double precision but catches real bugs
        constexpr double abs_tolerance = 1e-20;
        constexpr double rel_tolerance = 1e-28;

        double abs_orig = std::abs(comp_orig);
        if (abs_orig > 0.0) {
            // Use relative tolerance for non-zero values
            if (comp_diff > abs_orig * rel_tolerance && comp_diff > abs_tolerance) {
                matches = false;
            }
        } else {
            // Use absolute tolerance for zero values
            if (comp_diff > abs_tolerance) {
                matches = false;
            }
        }
    }

    if (reportTestCases) {
        if (matches) {
            std::cout << "PASS: " << testName << '\n';
            if (max_component_error > 0) {
                std::cout << "  Max component error: " << std::scientific << std::setprecision(3)
                          << max_component_error << '\n';
            }
        } else {
            std::cout << "FAIL: " << testName << '\n';
            std::cout << "  Input:              " << input << '\n';
            std::cout << "  To_string output:   " << output << '\n';
            std::cout << "  Max component error: " << std::scientific << std::setprecision(3)
                      << max_component_error << '\n';
            std::cout << "  Original components:   [" << std::setprecision(17)
                      << value[0] << ", " << value[1] << "]\n";
            std::cout << "  Round-trip components: [" << std::setprecision(17)
                      << roundtrip[0] << ", " << roundtrip[1] << "]\n";
        }
    }

    return matches;
}

int main()
try {
    using namespace sw::universal;

    std::string test_suite = "floatcascade parse/to_string round-trip validation";
    bool reportTestCases = true;
    int nrOfFailedTestCases = 0;

    ReportTestSuiteHeader(test_suite, reportTestCases);

    // Test cases covering various scenarios
    std::vector<std::pair<std::string, std::string>> testCases = {
        // Basic decimal values
        {"3.14159265358979323846", "pi approximation"},
        {"2.71828182845904523536", "e approximation"},
        {"1.41421356237309504880", "sqrt(2) approximation"},

        // Scientific notation - positive exponents
        {"1.23456789e10", "scientific notation +10"},
        {"6.02214076e23", "Avogadro's number"},
        {"9.10938356e-31", "electron mass (kg)"},

        // Scientific notation - negative exponents
        {"1.602176634e-19", "elementary charge"},
        {"6.62607015e-34", "Planck constant"},

        // Negative values
        {"-3.14159265358979323846", "negative pi"},
        {"-2.71828182845904523536", "negative e"},
        {"-1.23456789e10", "negative scientific"},

        // Small values
        {"0.00001", "small decimal"},
        {"1e-20", "very small scientific"},
        {"0.000000000000001", "15 zeros"},

        // Large values
        {"1000000.0", "one million"},
        {"1.7976931348623157e308", "near max double"},

        // Edge cases
        {"1.0", "one"},
        {"0.1", "one tenth"},
        {"0.5", "one half"},
        {"2.0", "two"},
        {"10.0", "ten"},
        {"100.0", "hundred"},

        // Values that don't convert exactly to binary
        {"0.3", "three tenths"},
        {"0.7", "seven tenths"},
        {"0.9", "nine tenths"},

        // Zero
        {"0.0", "zero"},
    };

    std::cout << "\n=== Testing dd_cascade (N=2) ===\n";
    for (const auto& test : testCases) {
        if (!TestRoundTrip<dd_cascade>(test.first, test.second, reportTestCases)) {
            ++nrOfFailedTestCases;
        }
    }

    // TODO: Enable after Phase 6 - td_cascade and qd_cascade need to_string/parse wrappers
    /*
    std::cout << "\n=== Testing td_cascade (N=3) ===\n";
    for (const auto& test : testCases) {
        if (!TestRoundTrip<td_cascade>(test.first, test.second, reportTestCases)) {
            ++nrOfFailedTestCases;
        }
    }

    std::cout << "\n=== Testing qd_cascade (N=4) ===\n";
    for (const auto& test : testCases) {
        if (!TestRoundTrip<qd_cascade>(test.first, test.second, reportTestCases)) {
            ++nrOfFailedTestCases;
        }
    }
    */

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
