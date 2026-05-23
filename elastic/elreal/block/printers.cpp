// printers.cpp: smoke tests for elreal block<FpType> pretty-printers
// (to_binary, to_hex, color_print).
//
// Phase 1 (#925) doesn't pin printer output to a golden format -- subsequent
// phases will refine the rendering. This file just confirms the printers
// produce non-empty output for every supported FpType and don't crash.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>

#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/bfloat16/bfloat16.hpp>
#include <universal/number/elreal/elreal.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

template <typename FpType>
int verify_printers(const std::string& tag) {
    using namespace sw::universal;
    using B = block<FpType>;
    int nrFailures = 0;

    B b1{ FpType{1.5}, 0 };
    B b2{ FpType{-3.0}, 7 };
    B z{ FpType{0}, 0 };

    auto tb1 = to_binary(b1);
    auto tb2 = to_binary(b2);
    auto tbz = to_binary(z);
    if (tb1.empty()) { std::cout << tag << " to_binary(b1) empty\n"; ++nrFailures; }
    if (tb2.empty()) { std::cout << tag << " to_binary(b2) empty\n"; ++nrFailures; }
    if (tbz.empty()) { std::cout << tag << " to_binary(zero) empty\n"; ++nrFailures; }

    auto th1 = to_hex(b1);
    auto th2 = to_hex(b2);
    if (th1.empty()) { std::cout << tag << " to_hex(b1) empty\n"; ++nrFailures; }
    if (th2.empty()) { std::cout << tag << " to_hex(b2) empty\n"; ++nrFailures; }

    auto cp1 = color_print(b1);
    if (cp1.empty()) { std::cout << tag << " color_print(b1) empty\n"; ++nrFailures; }

    // Print one example per type for human inspection
    std::cout << tag << "  to_binary: " << tb1 << '\n';
    std::cout << tag << "  to_binary(off): " << tb2 << '\n';
    std::cout << tag << "  to_hex:    " << th1 << '\n';

    return nrFailures;
}

} // anonymous

int main()
try {
    using namespace sw::universal;
    std::string test_suite = "elreal block<FpType> pretty-printers";
    int nrOfFailedTestCases = 0;
    bool reportTestCases = false;
    ReportTestSuiteHeader(test_suite, reportTestCases);

    nrOfFailedTestCases += verify_printers<float>("block<float>");
    nrOfFailedTestCases += verify_printers<double>("block<double>");
    nrOfFailedTestCases += verify_printers<half>("block<half>");
    nrOfFailedTestCases += verify_printers<bfloat16>("block<bfloat16>");
    nrOfFailedTestCases += verify_printers<cfloat<24, 5, std::uint16_t, true, false, false>>("block<cfloat<24,5>>");
    nrOfFailedTestCases += verify_printers<cfloat<32, 8, std::uint32_t, true, false, false>>("block<cfloat<32,8>>");

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (const std::exception& err) {
    std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
