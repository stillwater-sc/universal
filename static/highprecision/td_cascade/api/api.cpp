// td_cascade_example.cpp: Simple example demonstrating td_cascade usage
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// Compile: g++ -std=c++20 -I include/sw td_cascade_example.cpp -o td_cascade_example

#include <iostream>
#include <iomanip>
#include <universal/number/td_cascade/td_cascade.hpp>

int main() {
    using namespace sw::universal;

    std::cout << "td_cascade Example - Triple-Double Arithmetic using floatcascade<3>\n";
    std::cout << std::string(70, '=') << '\n';

    // Basic construction
    td_cascade a(1.0, 1e-17, 1e-34);
    td_cascade b(2.0, 2e-17, 2e-34);

    std::cout << "\nConstruction:\n";
    std::cout << "a = " << a << '\n';
    std::cout << "b = " << b << '\n';

    // Component access
    std::cout << "\nComponent access:\n";
    std::cout << "a[0] = " << std::setprecision(17) << a[0] << '\n';
    std::cout << "a[1] = " << std::setprecision(17) << a[1] << '\n';
    std::cout << "a[2] = " << std::setprecision(17) << a[2] << '\n';

    // Arithmetic operations
    std::cout << "\nArithmetic operations:\n";
    td_cascade sum = a + b;
    td_cascade diff = a - b;
    td_cascade prod = a * b;
    td_cascade quot = a / b;

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
    td_cascade zero(SpecificValue::zero);
    td_cascade inf_pos(SpecificValue::infpos);
    td_cascade nan(SpecificValue::qnan);

    std::cout << "zero    = " << zero << " (iszero: " << zero.iszero() << ")\n";
    std::cout << "inf_pos = " << inf_pos << " (isinf: " << inf_pos.isinf() << ")\n";
    std::cout << "nan     = " << nan << " (isnan: " << nan.isnan() << ")\n";

    // Demonstrate precision
    std::cout << "\nPrecision demonstration:\n";
    td_cascade pi_approx(3.141592653589793,
                         1.2246467991473532e-16,
                         -2.9947698097183397e-33);
    std::cout << "Pi approximation (td_cascade): " << pi_approx << '\n';
    std::cout << "Pi approximation (double):     " << std::setprecision(17) << 3.141592653589793 << '\n';

    // Test that zero + a preserves components (Windows CI failure case)
    std::cout << "\nZero addition test (Windows CI failure case):\n";
    td_cascade test_zero(0.0, 0.0, 0.0);
    td_cascade test_a(1.0, 1e-17, 1e-34);
    td_cascade test_sum = test_zero + test_a;

    std::cout << "0 + a = " << test_sum << '\n';
    std::cout << "Components preserved: "
              << (test_sum[0] == test_a[0] && test_sum[1] == test_a[1] && test_sum[2] == test_a[2]
                  ? "YES ✓" : "NO ✗") << '\n';

    // Demonstrate triple-double precision advantage
    std::cout << "\nTriple-double precision advantage:\n";
    std::cout << "Double precision:      ~16 decimal digits\n";
    std::cout << "Double-double (dd):    ~32 decimal digits\n";
    std::cout << "Triple-double (td):    ~48 decimal digits\n";
    std::cout << "\ntd_cascade ULP: " << std::numeric_limits<td_cascade>::epsilon() << '\n';

    std::cout << "\n" << std::string(70, '=') << '\n';
    std::cout << "Example completed successfully!\n";

    return 0;
}
