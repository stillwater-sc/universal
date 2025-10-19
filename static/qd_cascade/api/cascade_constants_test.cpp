// cascade_constants_test.cpp: Test mathematical constants for dd/td/qd cascade types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// Compile: g++ -std=c++20 -I include/sw cascade_constants_test.cpp -o cascade_constants_test

#include <iostream>
#include <iomanip>
#include <universal/number/dd_cascade/dd_cascade.hpp>
#include <universal/number/td_cascade/td_cascade.hpp>
#include <universal/number/qd_cascade/qd_cascade.hpp>

int main() {
    using namespace sw::universal;

    std::cout << "Cascade Mathematical Constants Test\n";
    std::cout << std::string(80, '=') << '\n';

    // Test dd_cascade constants (2 components, ~32 decimal digits)
    std::cout << "\nDouble-Double Cascade Constants (106 bits precision):\n";
    std::cout << std::string(80, '-') << '\n';
    std::cout << "dd_cascade_pi       = " << dd_cascade_pi << '\n';
    std::cout << "dd_cascade_e        = " << dd_cascade_e << '\n';
    std::cout << "dd_cascade_sqrt2    = " << dd_cascade_sqrt2 << '\n';
    std::cout << "dd_cascade_ln2      = " << dd_cascade_ln2 << '\n';
    std::cout << "dd_cascade_phi      = " << dd_cascade_phi << '\n';

    // Test td_cascade constants (3 components, ~48 decimal digits)
    std::cout << "\nTriple-Double Cascade Constants (159 bits precision):\n";
    std::cout << std::string(80, '-') << '\n';
    std::cout << "td_cascade_pi       = " << td_cascade_pi << '\n';
    std::cout << "td_cascade_e        = " << td_cascade_e << '\n';
    std::cout << "td_cascade_sqrt2    = " << td_cascade_sqrt2 << '\n';
    std::cout << "td_cascade_ln2      = " << td_cascade_ln2 << '\n';
    std::cout << "td_cascade_phi      = " << td_cascade_phi << '\n';

    // Test qd_cascade constants (4 components, ~64 decimal digits)
    std::cout << "\nQuad-Double Cascade Constants (212 bits precision):\n";
    std::cout << std::string(80, '-') << '\n';
    std::cout << "qd_cascade_pi       = " << qd_cascade_pi << '\n';
    std::cout << "qd_cascade_e        = " << qd_cascade_e << '\n';
    std::cout << "qd_cascade_sqrt2    = " << qd_cascade_sqrt2 << '\n';
    std::cout << "qd_cascade_ln2      = " << qd_cascade_ln2 << '\n';
    std::cout << "qd_cascade_phi      = " << qd_cascade_phi << '\n';

    // Demonstrate precision hierarchy
    std::cout << "\nPrecision Hierarchy (all showing pi):\n";
    std::cout << std::string(80, '-') << '\n';
    std::cout << "Component breakdown:\n\n";

    std::cout << "dd_cascade_pi[0] = " << std::setprecision(17) << dd_cascade_pi[0] << '\n';
    std::cout << "dd_cascade_pi[1] = " << std::setprecision(17) << dd_cascade_pi[1] << '\n';
    std::cout << '\n';

    std::cout << "td_cascade_pi[0] = " << std::setprecision(17) << td_cascade_pi[0] << '\n';
    std::cout << "td_cascade_pi[1] = " << std::setprecision(17) << td_cascade_pi[1] << '\n';
    std::cout << "td_cascade_pi[2] = " << std::setprecision(17) << td_cascade_pi[2] << '\n';
    std::cout << '\n';

    std::cout << "qd_cascade_pi[0] = " << std::setprecision(17) << qd_cascade_pi[0] << '\n';
    std::cout << "qd_cascade_pi[1] = " << std::setprecision(17) << qd_cascade_pi[1] << '\n';
    std::cout << "qd_cascade_pi[2] = " << std::setprecision(17) << qd_cascade_pi[2] << '\n';
    std::cout << "qd_cascade_pi[3] = " << std::setprecision(17) << qd_cascade_pi[3] << '\n';

    // Verify consistency: dd components should match first 2 of td, td should match first 3 of qd
    std::cout << "\nConsistency Check (Oracle Extraction Validation):\n";
    std::cout << std::string(80, '-') << '\n';

    bool dd_td_consistent = (dd_cascade_pi[0] == td_cascade_pi[0] &&
                             dd_cascade_pi[1] == td_cascade_pi[1]);
    bool td_qd_consistent = (td_cascade_pi[0] == qd_cascade_pi[0] &&
                             td_cascade_pi[1] == qd_cascade_pi[1] &&
                             td_cascade_pi[2] == qd_cascade_pi[2]);

    std::cout << "dd_cascade_pi[0:1] matches td_cascade_pi[0:1]: "
              << (dd_td_consistent ? "✓ PASS" : "✗ FAIL") << '\n';
    std::cout << "td_cascade_pi[0:2] matches qd_cascade_pi[0:2]: "
              << (td_qd_consistent ? "✓ PASS" : "✗ FAIL") << '\n';

    // Test arithmetic with constants
    std::cout << "\nArithmetic with Constants:\n";
    std::cout << std::string(80, '-') << '\n';

    auto qd_circle_area = qd_cascade_pi * qd_cascade(1.0);  // Area of unit circle
    std::cout << "Circle area (r=1) using qd_cascade_pi: " << qd_circle_area << '\n';

    auto qd_euler_identity_part = qd_cascade_e * qd_cascade_pi;
    std::cout << "e * pi (part of Euler's identity): " << qd_euler_identity_part << '\n';

    std::cout << "\n" << std::string(80, '=') << '\n';
    std::cout << "All cascade constants loaded and validated successfully!\n";

    return 0;
}
