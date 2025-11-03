// sqrt_precision_test.cpp: Test sqrt precision with Karp's trick vs Newton-Raphson
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/dd_cascade/dd_cascade.hpp>
#include <universal/number/td_cascade/td_cascade.hpp>
#include <universal/number/qd_cascade/qd_cascade.hpp>
#include <universal/verification/test_suite.hpp>
#include <limits>
#include <cmath>

namespace sw { namespace universal {

// Newton-Raphson sqrt implementation for comparison
template<typename CascadeType>
CascadeType sqrt_newton(const CascadeType& a, int iterations = 3) {
    if (a.iszero()) return CascadeType(0.0);
    if (a.isneg()) {
        std::cerr << "sqrt_newton: negative argument" << std::endl;
        return CascadeType(SpecificValue::snan);
    }

    // Initial approximation from high component
    CascadeType x = std::sqrt(a[0]);

    // Newton iterations: x' = (x + a/x) / 2
    for (int i = 0; i < iterations; ++i) {
        x = (x + a / x) * 0.5;
    }

    return x;
}

// Test sqrt precision for a specific value
template<typename CascadeType>
void test_sqrt_precision(const CascadeType& a, const std::string& label) {
    std::cout << "\n" << label << ":\n";
    std::cout << "Input a = " << std::setprecision(17) << a << "\n";

    // Test current Karp implementation
    CascadeType sqrt_karp = sqrt(a);
    std::cout << "Karp sqrt(a) = " << sqrt_karp << "\n";

    // Test Newton-Raphson
    CascadeType sqrt_newt = sqrt_newton(a, 4);
    std::cout << "Newton sqrt(a) = " << sqrt_newt << "\n";

    // Round-trip test: (sqrt(a))^2 should equal a
    CascadeType karp_squared = sqrt_karp * sqrt_karp;
    CascadeType newt_squared = sqrt_newt * sqrt_newt;

    std::cout << "Karp: (sqrt(a))^2 = " << karp_squared << "\n";
    std::cout << "Newton: (sqrt(a))^2 = " << newt_squared << "\n";

    // Compute errors
    CascadeType karp_error = abs(karp_squared - a);
    CascadeType newt_error = abs(newt_squared - a);

    std::cout << "Karp round-trip error: " << karp_error << "\n";
    std::cout << "Newton round-trip error: " << newt_error << "\n";

    // Relative errors
    double karp_rel = double(karp_error / a);
    double newt_rel = double(newt_error / a);

    std::cout << "Karp relative error: " << karp_rel << "\n";
    std::cout << "Newton relative error: " << newt_rel << "\n";

    if (newt_rel > 0 && karp_rel > 0) {
        double improvement = karp_rel / newt_rel;
        std::cout << "Improvement factor: " << improvement << "x\n";
    }
}

// Test overflow scenarios
template<typename CascadeType>
void test_overflow_range() {
    constexpr double DBL_MAX_VAL = std::numeric_limits<double>::max();
    constexpr double DBL_MIN_VAL = std::numeric_limits<double>::min();

    std::cout << "\n=== Overflow/Range Tests ===\n";

    // Test 1: Near DBL_MAX
    test_sqrt_precision(CascadeType(DBL_MAX_VAL * 0.99), "Near DBL_MAX (0.99 * max)");

    // Test 2: Exactly DBL_MAX
    test_sqrt_precision(CascadeType(DBL_MAX_VAL), "Exactly DBL_MAX");

    // Test 3: Near DBL_MIN
    test_sqrt_precision(CascadeType(DBL_MIN_VAL * 2.0), "Near DBL_MIN (2 * min)");

    // Test 4: Large value with multiple components
    CascadeType large_multi;
    if constexpr (std::is_same_v<CascadeType, qd_cascade>) {
        large_multi = CascadeType(1e308, 1e292, 1e276, 1e260);
    } else if constexpr (std::is_same_v<CascadeType, td_cascade>) {
        large_multi = CascadeType(1e308, 1e292, 1e276);
    } else {
        large_multi = CascadeType(1e308, 1e292);
    }
    test_sqrt_precision(large_multi, "Large multi-component value");
}

// Test precision across range
template<typename CascadeType>
int test_precision_sweep(int num_tests = 100) {
    int failures = 0;
    constexpr double eps_threshold = 1e-25; // Very tight tolerance for cascades

    std::cout << "\n=== Precision Sweep Test (n=" << num_tests << ") ===\n";

    for (int i = 0; i < num_tests; ++i) {
        // Logarithmic sweep from 1e-300 to 1e+300
        double exponent = -300.0 + (600.0 * i) / num_tests;
        double val = std::pow(10.0, exponent);

        CascadeType a(val);
        CascadeType sqrt_karp = sqrt(a);
        CascadeType sqrt_newt = sqrt_newton(a, 4);

        // Round-trip test
        CascadeType karp_squared = sqrt_karp * sqrt_karp;
        CascadeType newt_squared = sqrt_newt * sqrt_newt;

        double karp_rel = double(abs(karp_squared - a) / a);
        double newt_rel = double(abs(newt_squared - a) / a);

        if (karp_rel > eps_threshold) {
            std::cout << "Karp FAIL at 10^" << exponent << ": rel_error = " << karp_rel << "\n";
            failures++;
        }

        if (newt_rel > eps_threshold) {
            std::cout << "Newton FAIL at 10^" << exponent << ": rel_error = " << newt_rel << "\n";
        }
    }

    std::cout << "Karp failures: " << failures << " / " << num_tests << "\n";
    return failures;
}

}} // namespace sw::universal

int main()
try {
    using namespace sw::universal;

    std::string test_suite = "sqrt precision analysis: Karp vs Newton-Raphson";
    std::cout << test_suite << "\n";
    std::cout << std::string(60, '=') << "\n";

    // Test dd_cascade
    std::cout << "\n### DD_CASCADE ###\n";
    test_overflow_range<dd_cascade>();
    test_precision_sweep<dd_cascade>(50);

    // Test td_cascade
    std::cout << "\n### TD_CASCADE ###\n";
    test_overflow_range<td_cascade>();
    test_precision_sweep<td_cascade>(50);

    // Test qd_cascade
    std::cout << "\n### QD_CASCADE ###\n";
    test_overflow_range<qd_cascade>();
    test_precision_sweep<qd_cascade>(50);

    return EXIT_SUCCESS;
}
catch (char const* msg) {
    std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
    return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
    std::cerr << "Caught unexpected universal arithmetic exception : " << err.what() << std::endl;
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
