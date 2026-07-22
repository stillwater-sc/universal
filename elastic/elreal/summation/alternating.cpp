// alternating.cpp: Phase 5 (#929) summation test on a slowly-converging
// alternating series.
//
//     pi/4 = sum_{n>=0} (-1)^n / (2n+1)   (Leibniz)
//
// This is an infinite series with slow (1/n) convergence, exercising many
// lazy add() compositions under a depth budget. We check:
//   - the bounded sum reproduces the exact dyadic sum of the terms it folds
//     (exact-oracle contract),
//   - the 0-overlap invariant holds on the result,
//   - the result approximates pi/4 within the Leibniz truncation error,
//   - a convergent infinite series does NOT trip the budget guard.
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

// Leibniz term n: (-1)^n / (2n+1).
template <typename FpType>
sw::universal::ZBCL<FpType> leibniz_term(std::size_t n) {
    using namespace sw::universal;
    double sign = (n % 2 == 0) ? 1.0 : -1.0;
    return from_native<FpType>(sign / (2.0 * static_cast<double>(n) + 1.0));
}

template <typename FpType>
int verify_leibniz(std::size_t depth, double tol, const std::string& tag) {
    using namespace sw::universal;
    int nrFailures = 0;

    auto gen = [](std::size_t n) { return leibniz_term<FpType>(n); };
    auto result = sum<FpType>(series_from_generator<FpType>(gen), depth);

    // (1) Exact-oracle contract: the bounded sum equals the exact dyadic sum of
    // the first `depth` represented terms.
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

    // (3) Approximation to pi/4 within the Leibniz truncation bound ~ 1/(2*depth).
    double approx = to_double_approx(result, 64);
    double pi_over_4 = std::atan(1.0);   // = pi/4
    if (std::abs(approx - pi_over_4) > tol) {
        std::cout << tag << " pi/4 approx FAILED: got " << approx
                  << " expected ~" << pi_over_4 << " (tol " << tol << ")\n";
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
    std::string test_suite = "elreal Phase 5 (#929) summation: alternating (Leibniz pi/4)";
    int nrOfFailedTestCases = 0;
    bool reportTestCases = false;
    ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

    // TODO: place hand-run diagnostics here (this branch ignores failures)

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return EXIT_SUCCESS;

#else

#if REGRESSION_LEVEL_1

    // depth=256 -> truncation error <= 1/512 ~ 2e-3; use a comfortable tolerance.
    nrOfFailedTestCases += verify_leibniz<double>(256, 5e-3, "leibniz<double>");
    nrOfFailedTestCases += verify_leibniz<float>(256, 5e-3, "leibniz<float>");

#endif

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (const std::exception& err) {
    std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
