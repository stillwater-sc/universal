// exponent.cpp: Phase 7.3 (#931) tests for exp / log / pow on ZBCL.
//
// exp/log and the general pow (via exp(y*log(x))) are Taylor/artanh series, so
// like the Phase 7.1 series constants they are tested on {double, float}: a
// bfloat16 host (narrow 7-bit significand) drives the series into deep
// expansions that underflow at the default depth (see constants.cpp). pow's
// small-integer fast path is pure multiplication and is tested on all hosts.
//
// Identities checked stream-wise: log(exp(x)) == x, exp(log(x)) == x (x>0),
// exp(a+b) == exp(a)*exp(b), log(x*y) == log(x)+log(y), and value vs std::<fn>,
// to a host-scaled tolerance; 0-overlap on every result.
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
#include <universal/number/elreal/math/exponent.hpp>
#include <universal/number/bfloat16/bfloat16.hpp>
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

// Test depths. exp/log are single-series and fine at depth 2, but verify_explog
// also exercises general pow = exp(y*log(x)) (two stacked series), so it runs at
// depth 3 (~159 bits) for guard bits; still ~(3/4)^4 cheaper than the default 4.
// The integer pow path is a pure multiply and needs no series headroom (depth 2).
static constexpr std::size_t kExpDepth    = 3;
static constexpr std::size_t kPowIntDepth = 2;

// exp/log/general-pow -- {double, float}.
template <typename FpType>
int verify_explog(double tol, const std::string& host, std::size_t depth) {
    using namespace sw::universal;
    int n = 0;

    for (double v : { 0.0, 1.0, 2.0, -1.0, 0.5, 3.5 })
        n += near(exp(from_native<FpType>(v), depth), std::exp(v), tol, host + " exp(" + std::to_string(v) + ")");
    for (double v : { 1.0, 2.0, 5.0, 10.0, 0.5, 100.0 })
        n += near(log(from_native<FpType>(v), depth), std::log(v), tol, host + " log(" + std::to_string(v) + ")");

    // round trips
    for (double v : { 0.5, 2.0, 3.0 }) {
        n += near(log(exp(from_native<FpType>(v), depth), depth), v, tol, host + " log(exp)");
        ZBCL<FpType> pos = from_native<FpType>(std::exp(v));   // x = e^v > 0
        n += near(exp(log(pos, depth), depth), std::exp(v), tol * std::exp(v), host + " exp(log)");
    }
    // exp(a+b) == exp(a)*exp(b)
    {
        ZBCL<FpType> a = from_native<FpType>(1.0), b = from_native<FpType>(0.5);
        double lhs = est::approx(exp(add(a, b), depth));
        double rhs = est::approx(mul(exp(a, depth), exp(b, depth)));
        if (std::abs(lhs - rhs) > tol * std::exp(1.5)) { std::cout << host << " exp(a+b)!=exp(a)exp(b)\n"; ++n; }
    }
    // log(x*y) == log(x)+log(y)
    {
        ZBCL<FpType> x = from_native<FpType>(3.0), y = from_native<FpType>(7.0);
        double lhs = est::approx(log(mul(x, y), depth));
        double rhs = est::approx(add(log(x, depth), log(y, depth)));
        if (std::abs(lhs - rhs) > tol) { std::cout << host << " log(xy)!=log(x)+log(y)\n"; ++n; }
    }
    // general pow via exp(y*log(x))
    n += near(pow(from_native<FpType>(2.0), from_native<FpType>(0.5), depth), std::sqrt(2.0), tol, host + " pow(2,0.5)");
    n += near(pow(from_native<FpType>(9.0), from_native<FpType>(0.5), depth), 3.0, tol, host + " pow(9,0.5)");
    return n;
}

// pow integer fast path (pure multiply) -- all hosts.
template <typename FpType>
int verify_pow_int(double tol, const std::string& host, std::size_t depth) {
    using namespace sw::universal;
    int n = 0;
    struct { double b, e, r; } cases[] = { {2,10,1024}, {3,4,81}, {5,3,125}, {2,0,1}, {7,2,49} };
    for (auto& c : cases) {
        ZBCL<FpType> p = pow(from_native<FpType>(c.b), from_native<FpType>(c.e), depth);
        if (std::abs(est::approx(p) - c.r) > tol * std::max(1.0, c.r)) {
            std::cout << host << " pow(" << c.b << "," << c.e << ") = " << est::approx(p)
                      << " != " << c.r << '\n'; ++n;
        }
        n += est::check_zero_overlap<FpType>(p, 16, host + " pow-int");
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
    std::string test_suite = "elreal Phase 7.3 (#931) exp / log / pow";
    int nrOfFailedTestCases = 0;
    bool reportTestCases = false;
    ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

    // TODO: place hand-run diagnostics here (this branch ignores failures)

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return EXIT_SUCCESS;

#else

#if REGRESSION_LEVEL_1

    nrOfFailedTestCases += verify_explog<double>(1e-10, "explog<double>", kExpDepth);
    nrOfFailedTestCases += verify_explog<float>(1e-5, "explog<float>", kExpDepth);

    nrOfFailedTestCases += verify_pow_int<double>(1e-12, "pow<double>", kPowIntDepth);
    nrOfFailedTestCases += verify_pow_int<float>(1e-5, "pow<float>", kPowIntDepth);
    nrOfFailedTestCases += verify_pow_int<bfloat16>(1e-1, "pow<bfloat16>", kPowIntDepth);

#endif

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (const std::exception& err) {
    std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
