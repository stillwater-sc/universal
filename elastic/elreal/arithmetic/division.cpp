// division.cpp: Phase 6 (#930) tests for div() (dissertation 4.2.6).
//
// Verifies exact divisors (6/2 = 3) bit-exactly, the identities x/1 = x and
// x/x = 1, the round-trip (x/y)*y ~= x to a host-scaled tolerance, the
// default divide-by-zero behaviour (empty co-list), and 0-overlap, parameterised
// over {double, float, bfloat16}. The throwing path is covered by
// division_exceptions.cpp.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <string>

#include <universal/number/elreal/elreal.hpp>
#include <universal/number/bfloat16/bfloat16.hpp>
#include <universal/verification/dyadic_exact.hpp>
#include <universal/verification/test_suite.hpp>

#include <universal/verification/elreal_oracle.hpp>

namespace {

namespace est = sw::universal::elreal_oracle;

template <typename FpType>
int verify_all(const std::string& host) {
    using namespace sw::universal;
    int n = 0;
    const double host_eps = static_cast<double>(std::numeric_limits<FpType>::epsilon());

    // (1) Exact divisors: result is bit-exact.
    struct { double a, b, q; } exact[] = {
        {6.0, 2.0, 3.0}, {12.0, 4.0, 3.0}, {-10.0, 2.0, -5.0}, {7.0, 1.0, 7.0}, {9.0, -3.0, -3.0}
    };
    for (auto& t : exact) {
        ZBCL<FpType> r = div(from_native<FpType>(t.a), from_native<FpType>(t.b));
        if (!(est::exact_value(r) == est::exact_value(from_native<FpType>(t.q)))) {
            std::cout << host << " " << t.a << "/" << t.b << " != " << t.q << '\n'; ++n;
        }
        n += est::check_zero_overlap<FpType>(r, 16, host + " div-exact");
    }

    // (2) x / 1 = x (exact).
    {
        ZBCL<FpType> x = from_native<FpType>(123.5);
        if (!(est::exact_value(div(x, from_native<FpType>(1.0))) == est::exact_value(x))) {
            std::cout << host << " x/1 != x\n"; ++n;
        }
    }

    // (3) x / x = 1 (exact).
    {
        ZBCL<FpType> x = from_native<FpType>(42.0);
        if (!(est::exact_value(div(x, x)) == est::exact_value(from_native<FpType>(1.0)))) {
            std::cout << host << " x/x != 1\n"; ++n;
        }
    }

    // (4) Round-trip (x/y)*y ~= x to a host-scaled tolerance (div is iterative).
    struct { double a, b; } rt[] = { {1.0, 3.0}, {2.0, 7.0}, {22.0, 7.0}, {-5.0, 6.0} };
    for (auto& t : rt) {
        ZBCL<FpType> x = from_native<FpType>(t.a), y = from_native<FpType>(t.b);
        ZBCL<FpType> back = mul(div(x, y), y);
        double tol = 16.0 * host_eps * std::max(1.0, std::abs(t.a));
        if (std::abs(est::approx(back) - t.a) > tol) {
            std::cout << host << " (" << t.a << "/" << t.b << ")*" << t.b
                      << " = " << est::approx(back) << " != " << t.a
                      << " (tol " << tol << ")\n"; ++n;
        }
        n += est::check_zero_overlap<FpType>(div(x, y), 16, host + " div-rt");
    }

    // (5) Default divide-by-zero (ELREAL_THROW_ARITHMETIC_EXCEPTION == 0): empty.
    {
        ZBCL<FpType> x = from_native<FpType>(5.0);
        if (!div(x, ZBCL<FpType>{}).is_empty()) { std::cout << host << " x/0 not empty\n"; ++n; }
    }
    // 0 / y = 0.
    {
        if (!div(ZBCL<FpType>{}, from_native<FpType>(3.0)).is_empty()) {
            std::cout << host << " 0/y not empty\n"; ++n;
        }
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
    std::string test_suite = "elreal Phase 6 (#930) div()";
    int nrOfFailedTestCases = 0;
    bool reportTestCases = false;
    ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

    // TODO: place hand-run diagnostics here (this branch ignores failures)

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return EXIT_SUCCESS;

#else

#if REGRESSION_LEVEL_1

    nrOfFailedTestCases += verify_all<double>("div<double>");
    nrOfFailedTestCases += verify_all<float>("div<float>");
    nrOfFailedTestCases += verify_all<bfloat16>("div<bfloat16>");

#endif

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (const std::exception& err) {
    std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
