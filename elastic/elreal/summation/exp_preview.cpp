// exp_preview.cpp: Phase 5 (#929) summation test on the exp(1) Taylor series.
//
//     e = sum_{n>=0} 1/n!
//
// A Phase 7 preview: this is exactly how transcendentals will drive sum(). The
// series converges fast (1/n! drops below double eps by n ~ 18), so a small
// depth budget already gives full precision. We check the exact-oracle contract
// on the folded terms, 0-overlap, and the double-precision value against
// std::exp(1).
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include <universal/number/elreal/elreal.hpp>
#include <universal/verification/dyadic_exact.hpp>
#include <universal/verification/test_suite.hpp>

#include <universal/verification/elreal_oracle.hpp>

namespace {

namespace est = sw::universal::elreal_oracle;

// Taylor term n of exp(1): 1/n!.
template <typename FpType>
sw::universal::ZBCL<FpType> recip_factorial(std::size_t n) {
    using namespace sw::universal;
    double fact = 1.0;
    for (std::size_t i = 2; i <= n; ++i) fact *= static_cast<double>(i);
    return from_native<FpType>(1.0 / fact);
}

template <typename FpType>
int verify_exp(std::size_t depth, double tol, const std::string& tag) {
    using namespace sw::universal;
    int nrFailures = 0;

    auto gen = [](std::size_t n) { return recip_factorial<FpType>(n); };
    auto result = sum<FpType>(series_from_generator<FpType>(gen), depth);

    // (1) Exact-oracle contract on the folded terms.
    std::vector<ZBCL<FpType>> folded;
    folded.reserve(depth);
    for (std::size_t n = 0; n < depth; ++n) folded.push_back(gen(n));
    dyadic got = est::exact_value(result);
    dyadic ref = est::exact_series_sum<FpType>(folded);
    if (!(got == ref)) {
        std::cout << tag << " exact mismatch vs folded-term oracle\n";
        ++nrFailures;
    }

    // (2) 0-overlap on the result.
    nrFailures += est::check_zero_overlap<FpType>(result, 16, tag);

    // (3) Value against std::exp(1).
    double approx = to_double_approx(result, 64);
    double e = std::exp(1.0);
    if (std::abs(approx - e) > tol) {
        std::cout << tag << " exp(1) FAILED: got " << approx
                  << " expected ~" << e << " (tol " << tol << ")\n";
        ++nrFailures;
    }
    return nrFailures;
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
    std::string test_suite = "elreal Phase 5 (#929) summation: exp(1) = sum 1/n! (Phase 7 preview)";
    int nrOfFailedTestCases = 0;
    bool reportTestCases = false;
    ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

    // TODO: place hand-run diagnostics here (this branch ignores failures)

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return EXIT_SUCCESS;

#else

#if REGRESSION_LEVEL_1

    // depth=20 -> 1/20! ~ 4e-19 << double eps, so full double precision.
    nrOfFailedTestCases += verify_exp<double>(20, 1e-12, "exp<double>");
    // float host: the represented terms sum to e at float precision; compare in
    // double with a float-eps-scaled tolerance.
    nrOfFailedTestCases += verify_exp<float>(20, 1e-5, "exp<float>");

#endif

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (const std::exception& err) {
    std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
