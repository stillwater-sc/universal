// dd_cascade_example.cpp: Simple example demonstrating dd_cascade usage
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// Compile: g++ -std=c++20 -I include dd_cascade_example.cpp -o dd_cascade_example

#include <iostream>
#include <iomanip>
#include <universal/number/dd_cascade/dd_cascade.hpp>

int main() {
    using namespace sw::universal;

    std::cout << "dd_cascade Example - Double-Double Arithmetic using floatcascade<2>\n";
    std::cout << std::string(70, '=') << '\n';

    // Basic construction
    dd_cascade a(1.0, 1e-17);
    dd_cascade b(2.0, 2e-17);

    std::cout << "\nConstruction:\n";
    std::cout << "a = " << a << '\n';
    std::cout << "b = " << b << '\n';

    // Component access
    std::cout << "\nComponent access:\n";
    std::cout << "a.high() = " << std::setprecision(17) << a.high() << '\n';
    std::cout << "a.low()  = " << std::setprecision(17) << a.low() << '\n';
    std::cout << "a[0]     = " << std::setprecision(17) << a[0] << '\n';
    std::cout << "a[1]     = " << std::setprecision(17) << a[1] << '\n';

    // Arithmetic operations
    std::cout << "\nArithmetic operations:\n";
    dd_cascade sum = a + b;
    dd_cascade diff = a - b;
    dd_cascade prod = a * b;
    dd_cascade quot = a / b;

    std::cout << "a + b = " << sum << '\n';
    std::cout << "a - b = " << diff << '\n';
    std::cout << "a * b = " << prod << '\n';
    std::cout << "a / b = " << quot << '\n';

    // Comparison operators
    std::cout << "\nComparison operators:\n";
    std::cout << "a < b  : " << (a < b) << '\n';
    std::cout << "a > b  : " << (a > b) << '\n';
    std::cout << "a == a : " << (a == a) << '\n';

    // Special values
    std::cout << "\nSpecial values:\n";
    dd_cascade zero(SpecificValue::zero);
    dd_cascade inf_pos(SpecificValue::infpos);
    dd_cascade nan(SpecificValue::qnan);

    std::cout << "zero    = " << zero << " (iszero: " << zero.iszero() << ")\n";
    std::cout << "inf_pos = " << inf_pos << " (isinf: " << inf_pos.isinf() << ")\n";
    std::cout << "nan     = " << nan << " (isnan: " << nan.isnan() << ")\n";

    // Demonstrate precision
    std::cout << "\nPrecision demonstration:\n";
    dd_cascade pi_approx(3.141592653589793, 1.2246467991473532e-16);
    std::cout << "Pi approximation (dd_cascade): " << std::setprecision(32) << pi_approx << '\n';
    std::cout << "Pi approximation (double):     " << std::setprecision(17) << 3.141592653589793 << '\n';

    // Test that zero + a preserves components (the Windows CI failure case)
    std::cout << "\nWindows CI failure test case:\n";
    dd_cascade test_zero(0.0, 0.0);
    dd_cascade test_a(1.0, 1e-17);
    dd_cascade test_sum = test_zero + test_a;

    std::cout << "0 + a = " << test_sum << '\n';
    std::cout << "Components preserved: "
              << (test_sum.high() == test_a.high() && test_sum.low() == test_a.low()
                  ? "YES ✓" : "NO ✗") << '\n';

    std::cout << "\n" << std::string(70, '=') << '\n';
    std::cout << "Example completed successfully!\n";

    return 0;
}
