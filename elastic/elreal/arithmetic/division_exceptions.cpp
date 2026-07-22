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

#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 0
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
    using namespace sw::universal;
    std::string test_suite = "elreal Phase 6 (#930) div() divide-by-zero throw";
    int nrOfFailedTestCases = 0;
    bool reportTestCases = false;
    ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

    // TODO: place hand-run diagnostics here (this branch ignores failures)

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return EXIT_SUCCESS;

#else

#if REGRESSION_LEVEL_1

    nrOfFailedTestCases += verify<double>("div<double>");
    nrOfFailedTestCases += verify<float>("div<float>");

#endif

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (const std::exception& err) {
    std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
