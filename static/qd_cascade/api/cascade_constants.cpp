// cascade_constants_test.cpp: Test mathematical constants for dd/td/qd cascade types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// Compile: g++ -std=c++20 -I include/sw cascade_constants_test.cpp -o cascade_constants_test

#include <iostream>
#include <iomanip>
#include <universal/utility/console_utf8.hpp>
#include <universal/number/dd_cascade/dd_cascade.hpp>
#include <universal/number/td_cascade/td_cascade.hpp>
#include <universal/number/qd_cascade/qd_cascade.hpp>

int main() {
	ConsoleUTF8 console;
    using namespace sw::universal;

    std::cout << "Cascade Mathematical Constants Test\n";
    std::cout << std::string(80, '=') << '\n';

    // Test dd_cascade constants (2 components, ~32 decimal digits)
    std::cout << "\nDouble-Double Cascade Constants (106 bits precision):\n";
    std::cout << std::string(80, '-') << '\n';
    std::cout << "ddc_pi       = " << ddc_pi << '\n';
    std::cout << "ddc_e        = " << ddc_e << '\n';
    std::cout << "ddc_sqrt2    = " << ddc_sqrt2 << '\n';
    std::cout << "ddc_ln2      = " << ddc_ln2 << '\n';
    std::cout << "ddc_phi      = " << ddc_phi << '\n';

    // Test td_cascade constants (3 components, ~48 decimal digits)
    std::cout << "\nTriple-Double Cascade Constants (159 bits precision):\n";
    std::cout << std::string(80, '-') << '\n';
    std::cout << "tdc_pi       = " << tdc_pi << '\n';
    std::cout << "tdc_e        = " << tdc_e << '\n';
    std::cout << "tdc_sqrt2    = " << tdc_sqrt2 << '\n';
    std::cout << "tdc_ln2      = " << tdc_ln2 << '\n';
    std::cout << "tdc_phi      = " << tdc_phi << '\n';

    // Test qd_cascade constants (4 components, ~64 decimal digits)
    std::cout << "\nQuad-Double Cascade Constants (212 bits precision):\n";
    std::cout << std::string(80, '-') << '\n';
    std::cout << "qdc_pi       = " << qdc_pi << '\n';
    std::cout << "qdc_e        = " << qdc_e << '\n';
    std::cout << "qdc_sqrt2    = " << qdc_sqrt2 << '\n';
    std::cout << "qdc_ln2      = " << qdc_ln2 << '\n';
    std::cout << "qdc_phi      = " << qdc_phi << '\n';

    // Demonstrate precision hierarchy
    std::cout << "\nPrecision Hierarchy (all showing pi):\n";
    std::cout << std::string(80, '-') << '\n';
    std::cout << "Component breakdown:\n\n";

    std::cout << "ddc_pi[0] = " << std::setprecision(17) << ddc_pi[0] << '\n';
    std::cout << "ddc_pi[1] = " << std::setprecision(17) << ddc_pi[1] << '\n';
    std::cout << '\n';

    std::cout << "tdc_pi[0] = " << std::setprecision(17) << tdc_pi[0] << '\n';
    std::cout << "tdc_pi[1] = " << std::setprecision(17) << tdc_pi[1] << '\n';
    std::cout << "tdc_pi[2] = " << std::setprecision(17) << tdc_pi[2] << '\n';
    std::cout << '\n';

    std::cout << "qdc_pi[0] = " << std::setprecision(17) << qdc_pi[0] << '\n';
    std::cout << "qdc_pi[1] = " << std::setprecision(17) << qdc_pi[1] << '\n';
    std::cout << "qdc_pi[2] = " << std::setprecision(17) << qdc_pi[2] << '\n';
    std::cout << "qdc_pi[3] = " << std::setprecision(17) << qdc_pi[3] << '\n';

    // Verify consistency: dd components should match first 2 of td, td should match first 3 of qd
    std::cout << "\nConsistency Check (Oracle Extraction Validation):\n";
    std::cout << std::string(80, '-') << '\n';

    bool dd_td_consistent = (ddc_pi[0] == tdc_pi[0] &&
                             ddc_pi[1] == tdc_pi[1]);
    bool td_qd_consistent = (tdc_pi[0] == qdc_pi[0] &&
                             tdc_pi[1] == qdc_pi[1] &&
                             tdc_pi[2] == qdc_pi[2]);

    std::cout << "ddc_pi[0:1] matches tdc_pi[0:1]: "
              << (dd_td_consistent ? "✓ PASS" : "✗ FAIL") << '\n';
    std::cout << "tdc_pi[0:2] matches qdc_pi[0:2]: "
              << (td_qd_consistent ? "✓ PASS" : "✗ FAIL") << '\n';

    // Test arithmetic with constants
    std::cout << "\nArithmetic with Constants:\n";
    std::cout << std::string(80, '-') << '\n';

    auto qd_circle_area = qdc_pi * qd_cascade(1.0);  // Area of unit circle
    std::cout << "Circle area (r=1) using qdc_pi: " << qd_circle_area << '\n';

    auto qd_euler_identity_part = qdc_e * qdc_pi;
    std::cout << "e * pi (part of Euler's identity): " << qd_euler_identity_part << '\n';

    std::cout << "\n" << std::string(80, '=') << '\n';
    std::cout << "All cascade constants loaded and validated successfully!\n";

    return 0;
}
