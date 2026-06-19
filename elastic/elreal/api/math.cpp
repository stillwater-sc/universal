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
// elreal_e() and elreal_euler_gamma() are exercised by the math/constants suite,
// not here: they still route through the eager e_zbcl / euler_gamma_zbcl factories
// (the online conversion is #1061 Phase 3b), and at ~10-16 s each they would
// dominate this fast-tier wiring test. The wrappers are one-liners identical to
// the ones below, so these checks already cover the constant-wrapping pattern;
// the e value itself is verified through exp(1) in verify_unary().
int verify_constants() {
    int n = 0;
    if (!near(elreal_pi(),      3.14159265358979324))  ++n;
    if (!near(elreal_ln2(),     0.69314718055994531))  ++n;
    if (!near(elreal_ln10(),    2.30258509299404568))  ++n;
    if (!near(elreal_sqrt2(),   1.41421356237309505))  ++n;
    if (!near(elreal_sqrt3(),   1.73205080756887729))  ++n;
    if (!near(elreal_sqrt5(),   2.23606797749978969))  ++n;
    if (!near(elreal_phi(),     1.61803398874989485))  ++n;
    if (!near(elreal_log2_10(), 3.32192809488736235))  ++n;
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

int main()
try {
    using namespace sw::universal;
    std::string test_suite = "elreal math facade: functions + constants (#1079 Phase 4)";
    int nrOfFailedTestCases = 0;
    bool reportTestCases = false;
    ReportTestSuiteHeader(test_suite, reportTestCases);

    // bound the working precision so the transcendental series stay quick while
    // still resolving past host-double (this is the facade wiring test, not a
    // precision sweep -- the ZBCL math is validated to hundreds of digits by the
    // math/* suites).
    elreal_precision_guard g(2);

    nrOfFailedTestCases += verify_constants();
    nrOfFailedTestCases += verify_unary();
    nrOfFailedTestCases += verify_binary();
    nrOfFailedTestCases += verify_precision_threading();

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (const std::exception& err) {
    std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
