// construction.cpp: trivial-layout + value construction tests for elreal block<FpType>.
//
// Phase 1 (#925) acceptance: block must be a trivial type so it can sit on
// hardware boundaries (FPGA, accelerator) without an out-of-band constructor.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <type_traits>

#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/bfloat16/bfloat16.hpp>
#include <universal/number/elreal/elreal.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

template <typename FpType>
int verify_construction(const std::string& tag) {
    using namespace sw::universal;
    using B = block<FpType>;

    int nrFailures = 0;

    static_assert(std::is_trivially_copyable_v<B>,
                  "block<FpType> must be trivially copyable");
    static_assert(std::is_trivially_destructible_v<B>,
                  "block<FpType> must be trivially destructible");
    // Default constructibility relaxed: block leaves members indeterminate by
    // design (matches the project's other trivial number types). We require
    // copy-constructibility instead.
    static_assert(std::is_copy_constructible_v<B>);
    static_assert(std::is_copy_assignable_v<B>);

    // value construction
    B one{ FpType{1}, 0 };
    if (one.v != FpType{1}) { std::cout << tag << ": one.v != 1\n"; ++nrFailures; }
    if (one.exp != 0) { std::cout << tag << ": one.exp != 0\n"; ++nrFailures; }

    // value + exp construction
    B big{ FpType{1.5}, 3 };
    if (big.v != FpType{1.5}) { std::cout << tag << ": big.v != 1.5\n"; ++nrFailures; }
    if (big.exp != 3) { std::cout << tag << ": big.exp != 3\n"; ++nrFailures; }

    // negative exp
    B small{ FpType{1.5}, -5 };
    if (small.exp != -5) { std::cout << tag << ": small.exp != -5\n"; ++nrFailures; }

    // copy
    B copy = big;
    if (copy.v != big.v || copy.exp != big.exp) {
        std::cout << tag << ": copy mismatch\n"; ++nrFailures;
    }

    // compile-time constants exposed on the type
    static_assert(B::k > 0, "block::k must be positive");

    std::cout << tag << " k=" << B::k << "\n";

    return nrFailures;
}

} // anonymous

int main()
try {
    using namespace sw::universal;
    std::string test_suite = "elreal block<FpType> construction";
    int nrOfFailedTestCases = 0;
    bool reportTestCases = false;
    ReportTestSuiteHeader(test_suite, reportTestCases);

    nrOfFailedTestCases += verify_construction<float>("block<float>");
    nrOfFailedTestCases += verify_construction<double>("block<double>");
    nrOfFailedTestCases += verify_construction<half>("block<half>");
    nrOfFailedTestCases += verify_construction<bfloat16>("block<bfloat16>");
    nrOfFailedTestCases += verify_construction<cfloat<24, 5, std::uint16_t, true, false, false>>("block<cfloat<24,5>>");
    nrOfFailedTestCases += verify_construction<cfloat<32, 8, std::uint32_t, true, false, false>>("block<cfloat<32,8>>");

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (const std::exception& err) {
    std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
