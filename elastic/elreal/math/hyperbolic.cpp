// hyperbolic.cpp: Phase 7.4 (#931) tests for sinh / cosh / tanh on ZBCL.
//
// These are direct identities on exp() (Phase 7.3), so like exp they are tested
// on {double, float} (bfloat16's narrow significand drives the underlying series
// past its range -- see exponent.cpp / constants.cpp). Checks: value vs std::<fn>,
// the identity cosh^2 - sinh^2 == 1, tanh == sinh/cosh, parity sinh(-x)==-sinh(x)
// and cosh(-x)==cosh(x), and 0-overlap on every result.
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
#include <universal/number/elreal/math/hyperbolic.hpp>
#include <universal/verification/dyadic_exact.hpp>
#include <universal/verification/test_suite.hpp>

#include <universal/verification/elreal_oracle.hpp>

namespace {

namespace est = sw::universal::elreal_oracle;

template <typename FpType>
int near(const sw::universal::ZBCL<FpType>& z, double ref, double tol, const std::string& tag) {
    using namespace sw::universal;
    int n = 0;
    if (std::abs(est::approx(z) - ref) > tol) {
        std::cout << tag << " = " << est::approx(z) << " != " << ref << " (tol " << tol << ")\n"; ++n;
    }
    n += est::check_zero_overlap<FpType>(z, 16, tag);
    return n;
}

// Test depth. sinh/cosh/tanh are single-series (exp-based) functions; depth 2
// (~106 bits on a double host) clears the 1e-10/1e-5 tolerances and the
// cosh^2-sinh^2 identity with wide margin, at ~O(depth^4) lower cost than the
// default depth 4 -- the dominant lever on the instrumented CI tiers.
static constexpr std::size_t kHypDepth = 2;

template <typename FpType>
int verify_all(double tol, const std::string& host, std::size_t depth) {
    using namespace sw::universal;
    int n = 0;

    for (double x : { 0.0, 0.5, 1.0, -1.5, 2.0, -0.25 }) {
        const std::string sx = std::to_string(x);
        n += near(sinh(from_native<FpType>(x), depth), std::sinh(x), tol, host + " sinh(" + sx + ")");
        n += near(cosh(from_native<FpType>(x), depth), std::cosh(x), tol, host + " cosh(" + sx + ")");
        n += near(tanh(from_native<FpType>(x), depth), std::tanh(x), tol, host + " tanh(" + sx + ")");
    }

    // cosh^2 - sinh^2 == 1
    for (double x : { 0.5, 1.3, 2.0 }) {
        ZBCL<FpType> c = cosh(from_native<FpType>(x), depth), s = sinh(from_native<FpType>(x), depth);
        double id = est::approx(add(mul(c, c), negate(mul(s, s))));
        if (std::abs(id - 1.0) > tol) { std::cout << host << " cosh^2-sinh^2 != 1 at " << x << " (" << id << ")\n"; ++n; }
    }
    // tanh == sinh/cosh
    for (double x : { 0.7, 1.5 }) {
        ZBCL<FpType> th = tanh(from_native<FpType>(x), depth);
        ZBCL<FpType> sc = div(sinh(from_native<FpType>(x), depth), cosh(from_native<FpType>(x), depth));
        if (std::abs(est::approx(th) - est::approx(sc)) > tol) { std::cout << host << " tanh!=sinh/cosh at " << x << '\n'; ++n; }
    }
    // parity
    {
        ZBCL<FpType> a = from_native<FpType>(1.1);
        if (std::abs(est::approx(sinh(negate(a), depth)) + est::approx(sinh(a, depth))) > tol) { std::cout << host << " sinh parity\n"; ++n; }
        if (std::abs(est::approx(cosh(negate(a), depth)) - est::approx(cosh(a, depth))) > tol) { std::cout << host << " cosh parity\n"; ++n; }
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
    std::string test_suite = "elreal Phase 7.4 (#931) sinh / cosh / tanh";
    int nrOfFailedTestCases = 0;
    bool reportTestCases = false;
    ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

    // TODO: place hand-run diagnostics here (this branch ignores failures)

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return EXIT_SUCCESS;

#else

#if REGRESSION_LEVEL_1

    nrOfFailedTestCases += verify_all<double>(1e-10, "hyp<double>", kHypDepth);
    nrOfFailedTestCases += verify_all<float>(1e-5, "hyp<float>", kHypDepth);

#endif

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (const std::exception& err) {
    std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
