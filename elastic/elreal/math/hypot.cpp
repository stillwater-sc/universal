// hypot.cpp: Phase 7.2 (#931) tests for hypot() on ZBCL.
//
// Pythagorean triples are checked bit-exactly (the result is an exact integer);
// general arguments are checked against std::hypot to a host-scaled tolerance.
// 0-overlap is verified on every result.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>

#include <universal/number/elreal/elreal.hpp>
#include <universal/number/bfloat16/bfloat16.hpp>
#include <universal/verification/dyadic_exact.hpp>
#include <universal/verification/test_suite.hpp>

#include "../arithmetic/arithmetic_oracle.hpp"

namespace {

namespace est = sw::universal::elreal_arith_test;

template <typename FpType>
int verify_all(double tol, const std::string& host) {
    using namespace sw::universal;
    int n = 0;

    // (1) Pythagorean triples -> integer hypotenuse (value check; hypot goes
    // through the approximate sqrt, and intermediates like 8^2+15^2=289 are not
    // representable in narrow hosts, so this is tolerance-based, not exact).
    struct { double x, y, h; } trip[] = {
        {3.0, 4.0, 5.0}, {5.0, 12.0, 13.0}, {8.0, 15.0, 17.0}, {6.0, 8.0, 10.0}
    };
    for (auto& t : trip) {
        ZBCL<FpType> r = hypot(from_native<FpType>(t.x), from_native<FpType>(t.y));
        if (std::abs(est::approx(r) - t.h) > tol * std::max(1.0, t.h)) {
            std::cout << host << " hypot(" << t.x << "," << t.y << ") = " << est::approx(r)
                      << " != " << t.h << '\n'; ++n;
        }
        n += est::check_zero_overlap<FpType>(r, 16, host + " hypot-triple");
    }

    // (2) General arguments vs std::hypot.
    struct { double x, y; } gen[] = { {1.0, 1.0}, {2.0, 3.0}, {0.0, 4.0}, {7.0, 0.0} };
    for (auto& g : gen) {
        ZBCL<FpType> r = hypot(from_native<FpType>(g.x), from_native<FpType>(g.y));
        if (std::abs(est::approx(r) - std::hypot(g.x, g.y)) > tol) {
            std::cout << host << " hypot(" << g.x << "," << g.y << ") = " << est::approx(r)
                      << " != " << std::hypot(g.x, g.y) << " (tol " << tol << ")\n"; ++n;
        }
        n += est::check_zero_overlap<FpType>(r, 16, host + " hypot-gen");
    }
    return n;
}

} // anonymous

int main()
try {
    using namespace sw::universal;
    std::string test_suite = "elreal Phase 7.2 (#931) hypot()";
    int nrOfFailedTestCases = 0;
    bool reportTestCases = false;
    ReportTestSuiteHeader(test_suite, reportTestCases);

    nrOfFailedTestCases += verify_all<double>(1e-12, "hypot<double>");
    nrOfFailedTestCases += verify_all<float>(1e-6, "hypot<float>");
    nrOfFailedTestCases += verify_all<bfloat16>(1e-4, "hypot<bfloat16>");

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (const std::exception& err) {
    std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
