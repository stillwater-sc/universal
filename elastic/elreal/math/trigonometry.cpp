// trigonometry.cpp: Phase 7.5/7.6 (#931) tests for atan/asin/acos and sin/cos/tan
// on ZBCL.
//
// atan and sin/cos/tan are robust and tested on {double, float}. asin/acos are
// tested on {double} only: they are the deepest compositions in the suite
// (sqrt . atan . div . pi), and a float host's narrow exponent range
// (min_exponent ~ -125) underflows the intermediate expansions for arguments
// toward the domain boundary, where 1 - x^2 also loses bits to cancellation. A
// double host (min_exponent ~ -1022) has ample headroom. See the host-scope note
// in trigonometry.hpp.
//
// Forward trig exercises the octant reduction t = x - n*(pi/2): for x near a
// multiple of pi/2 this cancels, and the reduced argument must renormalise to a
// 0-overlap stream (see #1044) or sin/cos of a tiny t would trip the invariant.
//
// Checks: value vs std::<fn>, sin^2+cos^2==1, tan==sin/cos, parity,
// asin(x)+acos(x)==pi/2, atan(x)+atan(1/x)==pi/2 (x>0), 4*atan(1)==pi, domain
// (|x|>1 -> empty for asin/acos); 0-overlap.
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

// Test depths. The math functions default to depth 4 (~212 bits) for high-precision
// use, but these tests assert only loose host tolerances (1e-10/1e-5) plus
// structural identities and the 0-overlap invariant -- all satisfied with margin at
// far smaller depth (depth d ~ d*53 bits on a double host, already past the host).
// Cost is ~O(depth^4) per call, so this is the dominant lever on the -O0/instrumented
// CI tiers. Single-series functions (atan/sin/cos/tan) use depth 2 (~106 bits); the
// deepest compositions (asin/acos = sqrt.atan.div.pi) keep depth 3 for guard bits.
static constexpr std::size_t kTrigDepth = 2;
static constexpr std::size_t kAsinDepth = 3;

// atan -- {double, float}.
template <typename FpType>
int verify_atan(double tol, const std::string& host, std::size_t depth) {
    using namespace sw::universal;
    int n = 0;
    for (double x : { 0.0, 0.5, 1.0, -1.0, 2.0, -3.0, 0.25, 7.0 })
        n += near(atan(from_native<FpType>(x), depth), std::atan(x), tol, host + " atan(" + std::to_string(x) + ")");
    const double pi = std::acos(-1.0);
    // atan(x) + atan(1/x) == pi/2, x > 0
    for (double x : { 0.5, 2.0, 5.0 }) {
        double s = est::approx(add(atan(from_native<FpType>(x), depth), atan(from_native<FpType>(1.0 / x), depth)));
        if (std::abs(s - pi / 2.0) > tol) { std::cout << host << " atan(x)+atan(1/x)!=pi/2 at " << x << '\n'; ++n; }
    }
    // 4*atan(1) == pi ; parity atan(-x)==-atan(x)
    {
        double q = est::approx(atan(from_native<FpType>(1.0), depth));
        if (std::abs(4.0 * q - pi) > tol) { std::cout << host << " 4*atan(1)!=pi (" << 4.0 * q << ")\n"; ++n; }
        ZBCL<FpType> a = from_native<FpType>(0.7);
        if (std::abs(est::approx(atan(negate(a), depth)) + est::approx(atan(a, depth))) > tol) { std::cout << host << " atan parity\n"; ++n; }
    }
    return n;
}

// asin / acos -- {double} only (see header note).
template <typename FpType>
int verify_asin_acos(double tol, const std::string& host, std::size_t depth) {
    using namespace sw::universal;
    int n = 0;

    for (double x : { 0.0, 0.5, -0.5, 0.8, 0.9, 1.0, -1.0 })
        n += near(asin(from_native<FpType>(x), depth), std::asin(x), tol, host + " asin(" + std::to_string(x) + ")");
    for (double x : { 0.0, 0.5, -0.5, 0.9, 1.0, -0.9 })
        n += near(acos(from_native<FpType>(x), depth), std::acos(x), tol, host + " acos(" + std::to_string(x) + ")");

    const double pi = std::acos(-1.0);
    // asin(x) + acos(x) == pi/2
    for (double x : { 0.3, -0.6, 0.9 }) {
        double s = est::approx(add(asin(from_native<FpType>(x), depth), acos(from_native<FpType>(x), depth)));
        if (std::abs(s - pi / 2.0) > tol) { std::cout << host << " asin+acos!=pi/2 at " << x << " (" << s << ")\n"; ++n; }
    }
    // parity asin(-x) == -asin(x)
    {
        ZBCL<FpType> a = from_native<FpType>(0.7);
        if (std::abs(est::approx(asin(negate(a), depth)) + est::approx(asin(a, depth))) > tol) { std::cout << host << " asin parity\n"; ++n; }
    }
    // domain: asin/acos of |x|>1 -> empty
    if (!asin(from_native<FpType>(1.5), depth).is_empty()) { std::cout << host << " asin(1.5) not empty\n"; ++n; }
    if (!acos(from_native<FpType>(2.0), depth).is_empty()) { std::cout << host << " acos(2.0) not empty\n"; ++n; }
    return n;
}

// sin / cos / tan -- {double, float}.
//
// nearZeroSquares: test the sin^2+cos^2 identity at points where one component is
// a near-zero (cos(pi/2), sin(pi), cos(3pi/2)). Squaring a ~1e-8 component needs
// ~2k bits below the unit leading term, which underflows a float host's exponent
// floor (~ -126) -- an inherent host-range limit, like asin/acos. So this is
// enabled for double only; on float the identity is still checked at the
// well-conditioned points where both components are O(1).
template <typename FpType>
int verify_forward_trig(double tol, const std::string& host, bool nearZeroSquares, std::size_t depth) {
    using namespace sw::universal;
    int n = 0;
    const double pi = std::acos(-1.0);

    // value vs std over a spread that includes the octant boundaries and the
    // near-zero values cos(pi/2), sin(pi), cos(3pi/2) (the cancellation cases).
    // The direct values are representable on both hosts; only squaring them is not.
    for (double x : { 0.0, 0.3, pi / 6, pi / 4, 1.0, pi / 2, 2.0, pi, 3.0,
                      3 * pi / 2, 2.3, -0.7, -1.1, -pi / 2 }) {
        n += near(sin(from_native<FpType>(x), depth), std::sin(x), tol, host + " sin(" + std::to_string(x) + ")");
        n += near(cos(from_native<FpType>(x), depth), std::cos(x), tol, host + " cos(" + std::to_string(x) + ")");
    }

    // sin^2 + cos^2 == 1 (also forces the reduced-argument 0-overlap path).
    // Well-conditioned points (both components O(1)) -- all hosts.
    for (double x : { 0.4, 1.2, 2.7, -1.1 }) {
        ZBCL<FpType> s = sin(from_native<FpType>(x), depth), c = cos(from_native<FpType>(x), depth);
        double id = est::approx(add(mul(s, s), mul(c, c)));
        if (std::abs(id - 1.0) > tol) { std::cout << host << " sin^2+cos^2!=1 at " << x << " (" << id << ")\n"; ++n; }
    }
    // Near-zero points (one component ~1e-8) -- double host only.
    if (nearZeroSquares) {
        for (double x : { pi / 2, pi, 3 * pi / 2 }) {
            ZBCL<FpType> s = sin(from_native<FpType>(x), depth), c = cos(from_native<FpType>(x), depth);
            double id = est::approx(add(mul(s, s), mul(c, c)));
            if (std::abs(id - 1.0) > tol) { std::cout << host << " sin^2+cos^2!=1 at " << x << " (" << id << ")\n"; ++n; }
        }
    }

    // tan == sin/cos vs std::tan, away from the cos==0 poles (relative test).
    for (double x : { 0.0, 0.3, pi / 6, pi / 4, 1.0, -0.7, -1.1, 2.3 }) {
        double got = est::approx(tan(from_native<FpType>(x), depth));
        double ref = std::tan(x);
        if (std::abs(got - ref) > tol * std::max(1.0, std::abs(ref))) {
            std::cout << host << " tan(" << x << ") = " << got << " != " << ref << '\n'; ++n;
        }
    }

    // parity: sin(-x) == -sin(x), cos(-x) == cos(x).
    {
        ZBCL<FpType> a = from_native<FpType>(0.9);
        if (std::abs(est::approx(sin(negate(a), depth)) + est::approx(sin(a, depth))) > tol) { std::cout << host << " sin parity\n"; ++n; }
        if (std::abs(est::approx(cos(negate(a), depth)) - est::approx(cos(a, depth))) > tol) { std::cout << host << " cos parity\n"; ++n; }
    }
    return n;
}

} // anonymous

int main()
try {
    using namespace sw::universal;
    std::string test_suite = "elreal Phase 7.5/7.6 (#931) atan/asin/acos and sin/cos/tan";
    int nrOfFailedTestCases = 0;
    bool reportTestCases = false;
    ReportTestSuiteHeader(test_suite, reportTestCases);

    nrOfFailedTestCases += verify_atan<double>(1e-10, "atan<double>", kTrigDepth);
    nrOfFailedTestCases += verify_atan<float>(1e-5, "atan<float>", kTrigDepth);
    nrOfFailedTestCases += verify_asin_acos<double>(1e-10, "iasin<double>", kAsinDepth);
    nrOfFailedTestCases += verify_forward_trig<double>(1e-10, "trig<double>", true, kTrigDepth);
    nrOfFailedTestCases += verify_forward_trig<float>(1e-5, "trig<float>", false, kTrigDepth);

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (const std::exception& err) {
    std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
