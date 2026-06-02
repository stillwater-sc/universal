// division_exceptions.cpp: Phase 6 (#930) -- div() divide-by-zero THROWING path.
//
// With ELREAL_THROW_ARITHMETIC_EXCEPTION == 1, dividing by the empty (zero)
// co-list must throw elreal_divide_by_zero. (The default non-throwing behaviour
// -- returning the empty co-list -- is covered in division.cpp.)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// Opt in to throwing BEFORE the umbrella header is included.
#define ELREAL_THROW_ARITHMETIC_EXCEPTION 1

#include <iostream>
#include <string>

#include <universal/number/elreal/elreal.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

template <typename FpType>
int verify(const std::string& host) {
    using namespace sw::universal;
    int n = 0;
    try {
        ZBCL<FpType> r = div(from_native<FpType>(5.0), ZBCL<FpType>{});
        (void)r.take(1);
        std::cout << host << " expected elreal_divide_by_zero, none thrown\n"; ++n;
    }
    catch (const elreal_divide_by_zero&) { /* expected */ }
    catch (const std::exception& e) {
        std::cout << host << " wrong exception: " << e.what() << '\n'; ++n;
    }
    return n;
}

} // anonymous

int main()
try {
    using namespace sw::universal;
    std::string test_suite = "elreal Phase 6 (#930) div() divide-by-zero throw";
    int nrOfFailedTestCases = 0;
    bool reportTestCases = false;
    ReportTestSuiteHeader(test_suite, reportTestCases);

    nrOfFailedTestCases += verify<double>("div<double>");
    nrOfFailedTestCases += verify<float>("div<float>");

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (const std::exception& err) {
    std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
