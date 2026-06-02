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

#include "../arithmetic/arithmetic_oracle.hpp"

namespace {

namespace est = sw::universal::elreal_arith_test;

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

template <typename FpType>
int verify_all(double tol, const std::string& host) {
    using namespace sw::universal;
    int n = 0;

    for (double x : { 0.0, 0.5, 1.0, -1.5, 2.0, -0.25 }) {
        const std::string sx = std::to_string(x);
        n += near(sinh(from_native<FpType>(x)), std::sinh(x), tol, host + " sinh(" + sx + ")");
        n += near(cosh(from_native<FpType>(x)), std::cosh(x), tol, host + " cosh(" + sx + ")");
        n += near(tanh(from_native<FpType>(x)), std::tanh(x), tol, host + " tanh(" + sx + ")");
    }

    // cosh^2 - sinh^2 == 1
    for (double x : { 0.5, 1.3, 2.0 }) {
        ZBCL<FpType> c = cosh(from_native<FpType>(x)), s = sinh(from_native<FpType>(x));
        double id = est::approx(add(mul(c, c), negate(mul(s, s))));
        if (std::abs(id - 1.0) > tol) { std::cout << host << " cosh^2-sinh^2 != 1 at " << x << " (" << id << ")\n"; ++n; }
    }
    // tanh == sinh/cosh
    for (double x : { 0.7, 1.5 }) {
        ZBCL<FpType> th = tanh(from_native<FpType>(x));
        ZBCL<FpType> sc = div(sinh(from_native<FpType>(x)), cosh(from_native<FpType>(x)));
        if (std::abs(est::approx(th) - est::approx(sc)) > tol) { std::cout << host << " tanh!=sinh/cosh at " << x << '\n'; ++n; }
    }
    // parity
    {
        ZBCL<FpType> a = from_native<FpType>(1.1);
        if (std::abs(est::approx(sinh(negate(a))) + est::approx(sinh(a))) > tol) { std::cout << host << " sinh parity\n"; ++n; }
        if (std::abs(est::approx(cosh(negate(a))) - est::approx(cosh(a))) > tol) { std::cout << host << " cosh parity\n"; ++n; }
    }
    return n;
}

} // anonymous

int main()
try {
    using namespace sw::universal;
    std::string test_suite = "elreal Phase 7.4 (#931) sinh / cosh / tanh";
    int nrOfFailedTestCases = 0;
    bool reportTestCases = false;
    ReportTestSuiteHeader(test_suite, reportTestCases);

    nrOfFailedTestCases += verify_all<double>(1e-10, "hyp<double>");
    nrOfFailedTestCases += verify_all<float>(1e-5, "hyp<float>");

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (const std::exception& err) {
    std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
