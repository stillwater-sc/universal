// depth_budget.cpp: Phase 5 (#929) summation depth-budget / convergence guard.
//
// The dissertation defines summation only for Cauchy series. sum() trusts the
// caller for convergent series but guards against ill-formed (divergent) input:
// if the budget is reached before natural termination AND the term magnitudes
// are not decreasing, it throws elreal_sum_budget_exceeded.
//
// Verifies:
//   - divergent constant series (sum 1) aborts within budget,
//   - divergent growing series (sum (n+1)) aborts,
//   - max_depth == 0 aborts,
//   - a convergent infinite series (sum (1/2)^n) does NOT abort (it truncates).
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <iostream>
#include <string>

#include <universal/number/elreal/elreal.hpp>
#include <universal/verification/dyadic_exact.hpp>
#include <universal/verification/test_suite.hpp>

#include <universal/verification/elreal_oracle.hpp>

namespace {

// Expect sum() to throw elreal_sum_budget_exceeded.
template <typename FpType, typename SeriesFactory>
int expect_abort(SeriesFactory make_series, std::size_t max_depth, const std::string& tag) {
    using namespace sw::universal;
    try {
        auto result = sum<FpType>(make_series(), max_depth);
        // Force a block so any lazy work is realised; should not get here.
        (void)result.take(1);
        std::cout << tag << " FAILED: expected elreal_sum_budget_exceeded, none thrown\n";
        return 1;
    }
    catch (const elreal_sum_budget_exceeded&) {
        return 0;   // expected
    }
}

// Expect sum() to succeed (no throw) and produce a non-empty result.
template <typename FpType, typename SeriesFactory>
int expect_ok(SeriesFactory make_series, std::size_t max_depth, const std::string& tag) {
    using namespace sw::universal;
    try {
        auto result = sum<FpType>(make_series(), max_depth);
        if (result.is_empty()) {
            std::cout << tag << " FAILED: convergent series produced empty sum\n";
            return 1;
        }
        return 0;
    }
    catch (const elreal_sum_budget_exceeded& e) {
        std::cout << tag << " FAILED: convergent series wrongly aborted: " << e.what() << '\n';
        return 1;
    }
}

} // anonymous

int main()
try {
    using namespace sw::universal;
    std::string test_suite = "elreal Phase 5 (#929) summation: depth-budget guard";
    int nrOfFailedTestCases = 0;
    bool reportTestCases = false;
    ReportTestSuiteHeader(test_suite, reportTestCases);

    // Divergent: constant series 1 + 1 + 1 + ... (magnitudes do not decrease).
    auto constant = []() {
        return series_from_generator<double>([](std::size_t) { return from_native<double>(1.0); });
    };
    nrOfFailedTestCases += expect_abort<double>(constant, 8, "sum 1 (divergent)");

    // Divergent: growing series 1 + 2 + 3 + ... (magnitudes increase).
    auto growing = []() {
        return series_from_generator<double>(
            [](std::size_t n) { return from_native<double>(static_cast<double>(n) + 1.0); });
    };
    nrOfFailedTestCases += expect_abort<double>(growing, 8, "sum (n+1) (divergent)");

    // Divergent: oscillating non-decaying series 1 - 1/2 + 1 - 1/2 + ... (leading
    // exponents 0,-1,0,-1,...). The endpoint heuristic would miss this; the
    // window-based guard must still reject it.
    auto oscillating = []() {
        return series_from_generator<double>(
            [](std::size_t n) { return from_native<double>(n % 2 == 0 ? 1.0 : -0.5); });
    };
    nrOfFailedTestCases += expect_abort<double>(oscillating, 16, "sum (osc 1,-1/2) (divergent)");

    // Budget of 0 is always an error.
    auto any_series = []() {
        return series_from_generator<double>([](std::size_t) { return from_native<double>(1.0); });
    };
    nrOfFailedTestCases += expect_abort<double>(any_series, 0, "max_depth == 0");

    // Convergent infinite series must NOT abort -- it truncates at the budget.
    auto geometric = []() {
        return series_from_generator<double>([](std::size_t n) {
            return from_native<double>(std::ldexp(1.0, -static_cast<int>(n)));  // (1/2)^n
        });
    };
    nrOfFailedTestCases += expect_ok<double>(geometric, 8, "sum (1/2)^n (convergent)");

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (const std::exception& err) {
    std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
