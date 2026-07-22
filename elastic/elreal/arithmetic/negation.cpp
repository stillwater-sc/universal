// negation.cpp: Phase 6 (#930) tests for negate() (dissertation 4.2.4).
//
// Verifies exact_value(negate(x)) == -exact_value(x), the involution
// -(-x) == x (bit-identical stream), 0-overlap preservation, and the empty
// (zero) co-list special case, parameterised over {double, float, bfloat16}.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <string>
#include <vector>

#include <universal/number/elreal/elreal.hpp>
#include <universal/number/bfloat16/bfloat16.hpp>
#include <universal/verification/dyadic_exact.hpp>
#include <universal/verification/test_suite.hpp>

#include <universal/verification/elreal_oracle.hpp>

namespace {

namespace est = sw::universal::elreal_oracle;

template <typename FpType>
int verify_one(double v, const std::string& tag) {
    using namespace sw::universal;
    int n = 0;
    ZBCL<FpType> x = from_native<FpType>(v);
    ZBCL<FpType> nx = negate(x);

    // exact_value(negate(x)) == -exact_value(x)
    if (!(est::exact_value(nx) == -est::exact_value(x))) {
        std::cout << tag << " value mismatch\n"; ++n;
    }
    // involution: -(-x) == x, block-for-block
    auto xb = x.take(16);
    auto nnb = negate(nx).take(16);
    if (xb.size() != nnb.size()) { std::cout << tag << " -(-x) size mismatch\n"; ++n; }
    else for (std::size_t i = 0; i < xb.size(); ++i) {
        if (xb[i].v != nnb[i].v || xb[i].exp != nnb[i].exp) {
            std::cout << tag << " -(-x) block " << i << " mismatch\n"; ++n; break;
        }
    }
    // 0-overlap preserved
    n += est::check_zero_overlap<FpType>(nx, 16, tag);
    return n;
}

template <typename FpType>
int verify_all(const std::string& host) {
    using namespace sw::universal;
    int n = 0;
    for (double v : { 1.0, -1.0, 6.0, 0.25, -123.5, 1e6, 7.0 })
        n += verify_one<FpType>(v, host + " negate(" + std::to_string(v) + ")");
    // empty (zero) co-list negates to empty
    if (!negate(ZBCL<FpType>{}).is_empty()) { std::cout << host << " negate(0) not empty\n"; ++n; }
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
    std::string test_suite = "elreal Phase 6 (#930) negate()";
    int nrOfFailedTestCases = 0;
    bool reportTestCases = false;
    ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

    // TODO: place hand-run diagnostics here (this branch ignores failures)

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return EXIT_SUCCESS;

#else

#if REGRESSION_LEVEL_1

    nrOfFailedTestCases += verify_all<double>("negate<double>");
    nrOfFailedTestCases += verify_all<float>("negate<float>");
    nrOfFailedTestCases += verify_all<bfloat16>("negate<bfloat16>");

#endif

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (const std::exception& err) {
    std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
