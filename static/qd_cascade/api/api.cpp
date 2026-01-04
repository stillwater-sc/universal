// qd_cascade_example.cpp: Simple example demonstrating qd_cascade usage
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// Compile: g++ -std=c++20 -I include/sw qd_cascade_example.cpp -o qd_cascade_example

#include <iostream>
#include <iomanip>
#include <universal/number/qd_cascade/qd_cascade.hpp>

int main() {
    using namespace sw::universal;

    std::cout << "qd_cascade Example - Quad-Double Arithmetic using floatcascade<4>\n";
    std::cout << std::string(70, '=') << '\n';

    // Basic construction
    qd_cascade a(1.0, 1e-17, 1e-34, 1e-51);
    qd_cascade b(2.0, 2e-17, 2e-34, 2e-51);

    std::cout << "\nConstruction:\n";
    std::cout << "a = " << a << '\n';
    std::cout << "b = " << b << '\n';

    // Component access
    std::cout << "\nComponent access:\n";
    std::cout << "a[0] = " << std::setprecision(17) << a[0] << '\n';
    std::cout << "a[1] = " << std::setprecision(17) << a[1] << '\n';
    std::cout << "a[2] = " << std::setprecision(17) << a[2] << '\n';
    std::cout << "a[3] = " << std::setprecision(17) << a[3] << '\n';

    // Arithmetic operations
    std::cout << "\nArithmetic operations:\n";
    qd_cascade sum = a + b;
    qd_cascade diff = a - b;
    qd_cascade prod = a * b;
    qd_cascade quot = a / b;

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
    qd_cascade zero(SpecificValue::zero);
    qd_cascade inf_pos(SpecificValue::infpos);
    qd_cascade nan(SpecificValue::qnan);

    std::cout << "zero    = " << zero << " (iszero: " << zero.iszero() << ")\n";
    std::cout << "inf_pos = " << inf_pos << " (isinf: " << inf_pos.isinf() << ")\n";
    std::cout << "nan     = " << nan << " (isnan: " << nan.isnan() << ")\n";

    // Demonstrate precision
    std::cout << "\nPrecision demonstration:\n";
    qd_cascade pi_approx(3.141592653589793,
                         1.2246467991473532e-16,
                         -2.9947698097183397e-33,
                         1.1124542208633652e-49);
    std::cout << "Pi approximation (qd_cascade): " << pi_approx << '\n';
    std::cout << "Pi approximation (double):     " << std::setprecision(17) << 3.141592653589793 << '\n';

    // Test that zero + a preserves components
    std::cout << "\nZero addition test (Windows CI failure case for td):\n";
    qd_cascade test_zero(0.0, 0.0, 0.0, 0.0);
    qd_cascade test_a(1.0, 1e-17, 1e-34, 1e-51);
    qd_cascade test_sum = test_zero + test_a;

    std::cout << "0 + a = " << test_sum << '\n';
    std::cout << "Components preserved: "
              << (test_sum[0] == test_a[0] && test_sum[1] == test_a[1] &&
                  test_sum[2] == test_a[2] && test_sum[3] == test_a[3]
                  ? "YES ✓" : "NO ✗") << '\n';

    // Demonstrate quad-double precision advantage
    std::cout << "\nQuad-double precision advantage:\n";
    std::cout << "Double precision:      ~16 decimal digits\n";
    std::cout << "Double-double (dd):    ~32 decimal digits\n";
    std::cout << "Triple-double (td):    ~48 decimal digits\n";
    std::cout << "Quad-double (qd):      ~64 decimal digits\n";
    std::cout << "\nqd_cascade ULP: " << std::numeric_limits<qd_cascade>::epsilon() << '\n';

    std::cout << "\n" << std::string(70, '=') << '\n';
    std::cout << "Example completed successfully!\n";

    return 0;
}
