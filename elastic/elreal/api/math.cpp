// math.cpp: the elreal-class math facade -- functions and constants (#1079 Phase 4).
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <cstdlib>      // EXIT_SUCCESS / EXIT_FAILURE
#include <exception>    // std::exception
#include <iostream>     // std::cerr / std::endl
#include <string>

#include <universal/number/elreal/elreal.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

using namespace sw::universal;
using Real = elreal<double>;

// close enough at host-double resolution: the facade refines well past 53 bits,
// so a correctly-wired function must agree with the std:: reference to ~1 ulp.
bool near(const Real& v, double ref, double tol = 1.0e-12) {
    return std::fabs(static_cast<double>(v) - ref) <= tol * (1.0 + std::fabs(ref));
}

// (1) named constants resolve to their reference values.
// elreal_e() is now online (#1061 Phase 3b: ~16 s -> ~0.1 s) so it is checked here
// directly. elreal_euler_gamma() stays out of this fast-tier wiring test: it is the
// Brent-McMillan algorithm (no clean Taylor form), still seconds at this depth, and
// is validated to 320 digits by the math/constants suite.
int verify_constants() {
    int n = 0;
    // Checked against double literals, so ~17 digits is the whole question and
    // kConstDepth supplies it with room to spare. Taking each generator's default
    // instead (16, and 32 for e) buys hundreds of digits nobody here looks at --
    // cheap while the refinement floors capped it early, and 3x the runtime once
    // they were removed (universal#1051), which pushed this test past the
    // sanitizer build's 300s ctest limit.
    constexpr std::size_t kConstDepth = 3;
    if (!near(elreal_pi(kConstDepth),      3.14159265358979324))  ++n;
    if (!near(elreal_e(kConstDepth),       2.71828182845904524))  ++n;
    if (!near(elreal_ln2(kConstDepth),     0.69314718055994531))  ++n;
    if (!near(elreal_ln10(kConstDepth),    2.30258509299404568))  ++n;
    if (!near(elreal_sqrt2(kConstDepth),   1.41421356237309505))  ++n;
    if (!near(elreal_sqrt3(kConstDepth),   1.73205080756887729))  ++n;
    if (!near(elreal_sqrt5(kConstDepth),   2.23606797749978969))  ++n;
    if (!near(elreal_phi(kConstDepth),     1.61803398874989485))  ++n;
    if (!near(elreal_log2_10(kConstDepth), 3.32192809488736235))  ++n;

    // depth 0 must still mean "the generator's own default" -- the back-compat
    // promise of the optional depth parameter. The dispatch is a single shared
    // expression across all ten accessors, so exercising it on the two cheapest
    // constants covers the same path pi and e would, for 0.25s instead of 31s at
    // -O0 (elreal_e() alone is 18.8s, which is what put this test over the
    // sanitizer build's 300s limit in the first place).
    {
        auto same = [](ZBCL<double> a, ZBCL<double> b) {
            auto va = a.take(256), vb = b.take(256);
            if (va.size() != vb.size()) return false;
            for (std::size_t i = 0; i < va.size(); ++i)
                if (va[i].v != vb[i].v || va[i].exp != vb[i].exp) return false;
            return true;
        };
        if (!same(elreal_sqrt2().stream(), sqrt2_zbcl<double>())) ++n;
        if (!same(elreal_phi().stream(),   phi_zbcl<double>()))   ++n;
    }
    return n;
}

// (2) unary functions wrap the ZBCL versions and thread precision
int verify_unary() {
    int n = 0;
    // sqrt: value and the squaring identity
    Real two = Real(1.0) + Real(1.0);
    if (!near(sqrt(two), 1.41421356237309505)) ++n;
    if (!near(sqrt(two) * sqrt(two), 2.0))     ++n;
    // exp / log are inverses; exp(1) == e; log(e) == 1
    Real one = two / two;
    if (!near(log(exp(one)), 1.0))            ++n;
    if (!near(exp(one), 2.71828182845904524)) ++n;   // exp(1) == e (fast online path)
    // trig: Pythagorean identity and a known angle (pi/6 -> 1/2)
    Real x = one / (one + two);                 // 1/3, an unremarkable interior angle
    if (!near(sin(x) * sin(x) + cos(x) * cos(x), 1.0)) ++n;
    Real pi6 = elreal_pi() / (two + two + two); // pi/6
    if (!near(sin(pi6), 0.5)) ++n;
    // inverse trig: 4 atan(1) == pi
    if (!near(atan(one) * (two + two), 3.14159265358979324)) ++n;
    // hyperbolic identity: cosh^2 - sinh^2 == 1
    if (!near(cosh(x) * cosh(x) - sinh(x) * sinh(x), 1.0)) ++n;
    if (!near(tanh(x), std::tanh(1.0 / 3.0)))               ++n;
    return n;
}

// (3) binary functions
int verify_binary() {
    int n = 0;
    Real two = Real(1.0) + Real(1.0);
    // pow(2, 10) == 1024 (exact integer power)
    if (!near(pow(two, two * (two + two + (two / two))), 1024.0)) ++n;   // 2^10
    // pow(x, 1/2) == sqrt(x)
    Real x = two + two + two;                   // 6
    if (!near(pow(x, Real(1.0) / two), std::sqrt(6.0))) ++n;
    // hypot(3, 4) == 5
    Real three = two + (two / two);
    Real four  = two + two;
    if (!near(hypot(three, four), 5.0)) ++n;
    return n;
}

// (4) precision threads through the facade: a deeper operand yields a deeper result
int verify_precision_threading() {
    int n = 0;
    elreal_precision_guard g(6);
    Real two = Real(1.0) + Real(1.0);
    Real r = sqrt(two);
    if (r.precision() != 6) ++n;                  // result carries the operand's depth
    if (r.limbs(r.precision()).size() < 2) ++n;   // sqrt(2) is irrational -> multi-block
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
    std::string test_suite = "elreal math facade: functions + constants (#1079 Phase 4)";
    int nrOfFailedTestCases = 0;
    bool reportTestCases = false;
    ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

    // TODO: place hand-run diagnostics here (this branch ignores failures)

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return EXIT_SUCCESS;

#else

#if REGRESSION_LEVEL_1

    // bound the working precision so the transcendental series stay quick while
    // still resolving past host-double (this is the facade wiring test, not a
    // precision sweep -- the ZBCL math is validated to hundreds of digits by the
    // math/* suites).
    elreal_precision_guard g(2);

    nrOfFailedTestCases += verify_constants();
    nrOfFailedTestCases += verify_unary();
    nrOfFailedTestCases += verify_binary();
    nrOfFailedTestCases += verify_precision_threading();

#endif

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (const std::exception& err) {
    std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
