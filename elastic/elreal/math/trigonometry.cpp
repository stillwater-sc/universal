// trigonometry.cpp: Phase 7.5 (#931) tests for atan / asin / acos on ZBCL.
//
// atan is robust and tested on {double, float}. asin/acos are tested on {double}
// only: they are the deepest compositions in the suite (sqrt . atan . div . pi),
// and a float host's narrow exponent range (min_exponent ~ -125) underflows the
// intermediate expansions for arguments toward the domain boundary, where
// 1 - x^2 also loses bits to cancellation. A double host (min_exponent ~ -1022)
// has ample headroom. See the host-scope note in trigonometry.hpp.
//
// Checks: value vs std::<fn>, asin(x)+acos(x)==pi/2, atan(x)+atan(1/x)==pi/2
// (x>0), 4*atan(1)==pi, parity, domain (|x|>1 -> empty for asin/acos); 0-overlap.
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
#include <universal/number/elreal/math/trigonometry.hpp>
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

// atan -- {double, float}.
template <typename FpType>
int verify_atan(double tol, const std::string& host) {
    using namespace sw::universal;
    int n = 0;
    for (double x : { 0.0, 0.5, 1.0, -1.0, 2.0, -3.0, 0.25, 7.0 })
        n += near(atan(from_native<FpType>(x)), std::atan(x), tol, host + " atan(" + std::to_string(x) + ")");
    const double pi = std::acos(-1.0);
    // atan(x) + atan(1/x) == pi/2, x > 0
    for (double x : { 0.5, 2.0, 5.0 }) {
        double s = est::approx(add(atan(from_native<FpType>(x)), atan(from_native<FpType>(1.0 / x))));
        if (std::abs(s - pi / 2.0) > tol) { std::cout << host << " atan(x)+atan(1/x)!=pi/2 at " << x << '\n'; ++n; }
    }
    // 4*atan(1) == pi ; parity atan(-x)==-atan(x)
    {
        double q = est::approx(atan(from_native<FpType>(1.0)));
        if (std::abs(4.0 * q - pi) > tol) { std::cout << host << " 4*atan(1)!=pi (" << 4.0 * q << ")\n"; ++n; }
        ZBCL<FpType> a = from_native<FpType>(0.7);
        if (std::abs(est::approx(atan(negate(a))) + est::approx(atan(a))) > tol) { std::cout << host << " atan parity\n"; ++n; }
    }
    return n;
}

// asin / acos -- {double} only (see header note).
template <typename FpType>
int verify_asin_acos(double tol, const std::string& host) {
    using namespace sw::universal;
    int n = 0;

    for (double x : { 0.0, 0.5, -0.5, 0.8, 0.9, 1.0, -1.0 })
        n += near(asin(from_native<FpType>(x)), std::asin(x), tol, host + " asin(" + std::to_string(x) + ")");
    for (double x : { 0.0, 0.5, -0.5, 0.9, 1.0, -0.9 })
        n += near(acos(from_native<FpType>(x)), std::acos(x), tol, host + " acos(" + std::to_string(x) + ")");

    const double pi = std::acos(-1.0);
    // asin(x) + acos(x) == pi/2
    for (double x : { 0.3, -0.6, 0.9 }) {
        double s = est::approx(add(asin(from_native<FpType>(x)), acos(from_native<FpType>(x))));
        if (std::abs(s - pi / 2.0) > tol) { std::cout << host << " asin+acos!=pi/2 at " << x << " (" << s << ")\n"; ++n; }
    }
    // parity asin(-x) == -asin(x)
    {
        ZBCL<FpType> a = from_native<FpType>(0.7);
        if (std::abs(est::approx(asin(negate(a))) + est::approx(asin(a))) > tol) { std::cout << host << " asin parity\n"; ++n; }
    }
    // domain: asin/acos of |x|>1 -> empty
    if (!asin(from_native<FpType>(1.5)).is_empty()) { std::cout << host << " asin(1.5) not empty\n"; ++n; }
    if (!acos(from_native<FpType>(2.0)).is_empty()) { std::cout << host << " acos(2.0) not empty\n"; ++n; }
    return n;
}

} // anonymous

int main()
try {
    using namespace sw::universal;
    std::string test_suite = "elreal Phase 7.5 (#931) atan / asin / acos";
    int nrOfFailedTestCases = 0;
    bool reportTestCases = false;
    ReportTestSuiteHeader(test_suite, reportTestCases);

    nrOfFailedTestCases += verify_atan<double>(1e-10, "atan<double>");
    nrOfFailedTestCases += verify_atan<float>(1e-5, "atan<float>");
    nrOfFailedTestCases += verify_asin_acos<double>(1e-10, "iasin<double>");

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (const std::exception& err) {
    std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
