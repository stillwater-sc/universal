// transcendentals_highprecision.cpp: identity-driven high-precision validation of
// the elreal transcendentals (exp/log/pow, sin/cos/tan, atan/asin/acos,
// sinh/cosh/tanh) to >= 300 decimal digits (#1049).
//
// Why this exists
// ---------------
// The per-PR transcendental tests (exponent.cpp, hyperbolic.cpp, trigonometry.cpp)
// only validate to host-double tolerance (~1e-10, ~16 digits). For an exact-lazy
// real type that is almost no coverage: the argument-reduction + series + mul/div
// machinery these compose from is untested past ~16 digits, so a high-precision
// operator bug is invisible (#1044 already surfaced two under cancellation). 300-digit
// external transcendental tables mostly do not exist, so the primary mechanism here
// is SELF-CHECKING IDENTITIES validated exactly via the independent dyadic oracle
// (elreal_reference_digits.hpp, per #987): an identity residual is converted to an
// exact dyadic and compared with no shared code path through elreal arithmetic, so
// an operator bug cannot hide inside a loose tolerance. A handful of absolute
// spot-checks against mpmath 320-digit strings (reference_constants.hpp) catch a
// wrong-but-self-consistent series that an identity alone would mask.
//
// Cost / placement
// ----------------
// The 300-digit checks run the functions at depth ~20 (O(depth^4), ~8000x a depth-2
// run). They are gated to REGRESSION_LEVEL_4 (stress / manual) and are a no-op at the
// sanity level CI / sanitizers / coverage use. A small always-on fast-tier identity
// section (depth 6, double tolerance) provides per-PR coverage.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// Regression testing guards: typically set by the cmake configuration, but
// MANUAL_TESTING is an override. A bare manual compile (no -D) runs everything.
#define MANUAL_TESTING 0
#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

// asin(x)+acos(x) == pi/2 exercises add() over an EXACT leading-term cancellation
// (acos = pi/2 - asin), which add() does not renormalize to 0-overlap -- it aborts
// the ZBCL invariant in assert-enabled builds (#1057). The value is still correct;
// only the canonical form is violated. Gated off until #1057 lands; flip to 1 then.
#define ELREAL_ADD_EXACT_CANCEL_FIXED 1

// exp / sin / cos (and tan / sinh / cosh / tanh built on them) cap at host-double
// precision because their Taylor/Maclaurin series scale each term by a host-double
// reciprocal coefficient (#1058, same bug class as e_zbcl #1054). Their 300-digit
// checks are gated off until #1058 lands; flip to 1 then. log/atan/asin/acos use the
// full-precision odd_power_series and already reach 300+, so they stay active.
#define ELREAL_EXP_SERIES_HIGH_PRECISION 1

// sin(asin(x))==x / cos(acos(x))==x cap at ~234 digits: sin/cos lose precision on a
// high-precision multi-block argument near pi/6 (asin itself is 320-digit accurate; the
// loss is in sin_series/cos_series and predates #1061 Ph3b -- eager was identical).
// Gated off until #1076 lands; flip to 1 then.
#define ELREAL_SINCOS_ROUNDTRIP_HIGH_PRECISION 0

#include <cmath>
#include <iostream>
#include <string>
#include <string_view>

#include <universal/number/elreal/elreal.hpp>
#include <universal/number/elreal/math/constants.hpp>
#include <universal/number/elreal/math/exponent.hpp>
#include <universal/number/elreal/math/hyperbolic.hpp>
#include <universal/number/elreal/math/trigonometry.hpp>
#include <universal/verification/elreal_oracle.hpp>
#include <universal/verification/elreal_reference_digits.hpp>
#include <universal/verification/test_suite.hpp>
#include <math/constants/reference_constants.hpp>

namespace {

using sw::universal::ZBCL;
using ER = ZBCL<double>;

// exact subtraction a - b via the elreal EFT add (no dedicated sub).
ER sub(const ER& a, const ER& b) { return sw::universal::add(a, sw::universal::negate(b)); }

// ---- fast tier (always on): identities to host-double tolerance ----------------
// Mirrors the per-PR coverage in exponent/hyperbolic/trigonometry.cpp so the new
// file still exercises the identities when REGRESSION_LEVEL_4 is off (CI). Uses the
// double approximation of each result (elreal_oracle::approx).
int fast_identities(bool reportTestCases) {
    using namespace sw::universal;
    namespace est = sw::universal::elreal_oracle;
    constexpr std::size_t D = 6;
    const double tol = 1e-12;
    const double pi  = std::acos(-1.0);
    int n = 0;

    auto close = [&](const char* name, double got, double want) {
        if (std::abs(got - want) > tol) { std::cout << "  FAIL fast " << name << ": " << got << " != " << want << '\n'; ++n; }
        else if (reportTestCases)       { std::cout << "  ok   fast " << name << '\n'; }
    };

    ER half = from_native<double>(0.5);
    close("log(exp(0.5))==0.5", est::approx(log(exp(half, D), D)), 0.5);
    close("exp(log(2))==2",     est::approx(exp(log(from_native<double>(2.0), D), D)), 2.0);

    ER s = sin(half, D), c = cos(half, D);
    close("sin^2+cos^2==1", est::approx(add(mul(s, s, D), mul(c, c, D))), 1.0);

    ER sh = sinh(half, D), ch = cosh(half, D);
    close("cosh^2-sinh^2==1", est::approx(sub(mul(ch, ch, D), mul(sh, sh, D))), 1.0);

#if ELREAL_ADD_EXACT_CANCEL_FIXED
    close("asin+acos==pi/2", est::approx(add(asin(half, D), acos(half, D))), pi / 2.0);
#endif
    close("4*atan(1)==pi",   4.0 * est::approx(atan(from_native<double>(1.0), D)), pi);

    // addition theorem exp(a+b) == exp(a)*exp(b)
    ER a = from_native<double>(0.5), b = from_native<double>(0.25);
    close("exp(a+b)==exp(a)exp(b)",
          est::approx(exp(add(a, b), D)) - est::approx(mul(exp(a, D), exp(b, D), D)) + 1.0, 1.0);

    // 0-overlap invariant on a representative result
    n += est::check_zero_overlap(sin(half, D), D, "fast sin(0.5)");
    return n;
}

#if REGRESSION_LEVEL_4
using sw::universal::agreed_decimal_digits;

constexpr std::size_t kDepth = 20;   // function-evaluation depth (double-host ceiling)
constexpr std::size_t kWork  = 26;   // working depth for identity mul/div (kDepth + guard)
constexpr int kMinDigits = 300;

// Absolute spot-check: z agrees with the decimal reference to >= kMinDigits digits.
int check_value(const char* name, const ER& z, std::string_view ref, bool reportTestCases) {
    int got = agreed_decimal_digits(z, ref);
    if (got < kMinDigits) {
        std::cout << "  FAIL " << name << ": agreed " << got << " digits, want >= " << kMinDigits << '\n';
        return 1;
    }
    if (reportTestCases) std::cout << "  ok   " << name << ": " << got << " digits\n";
    return 0;
}

// Identity check: lhs agrees with rhs (relative) to >= kMinDigits digits. Neither
// side needs a closed-form decimal string; both are compared as exact dyadics.
int check_identity(const char* name, const ER& lhs, const ER& rhs, bool reportTestCases) {
    int got = agreed_decimal_digits(lhs, rhs);
    if (got < kMinDigits) {
        std::cout << "  FAIL " << name << ": identity holds to only " << got << " digits, want >= " << kMinDigits << '\n';
        return 1;
    }
    if (reportTestCases) std::cout << "  ok   " << name << ": " << got << " digits\n";
    return 0;
}
#endif

} // anonymous

int main()
try {
    using namespace sw::universal;
    std::string test_suite = "elreal high-precision transcendental hardening (identity-driven, #1049)";
    int nrOfFailedTestCases = 0;
    bool reportTestCases = true;
    ReportTestSuiteHeader(test_suite, reportTestCases);

    nrOfFailedTestCases += fast_identities(reportTestCases);

#if REGRESSION_LEVEL_4
    const std::size_t D = kDepth;
    const std::size_t W = kWork;
    ER half = from_native<double>(0.5);

    // === full-precision group (log / atan / asin / acos): reaches 300+ digits =====
    // These compose div/mul/add/sum/odd_power_series, whose series divides by the
    // integer denominator at FULL precision, so they hit the double-host ceiling.
    // Absolute spot-checks against mpmath 320-digit references; arguments are exact
    // doubles (1, 1/2) so the reference equals what elreal sees.
    nrOfFailedTestCases += check_value("log(2)",    log(from_native<double>(2.0), D),  s_ln2,  reportTestCases);
    nrOfFailedTestCases += check_value("atan(1)",   atan(from_native<double>(1.0), D), s_pi_4, reportTestCases);
    nrOfFailedTestCases += check_value("asin(0.5)", asin(from_native<double>(0.5), D), s_pi_6, reportTestCases);
    nrOfFailedTestCases += check_value("acos(0.5)", acos(from_native<double>(0.5), D), s_pi_3, reportTestCases);
    // identities within the full-precision group
    nrOfFailedTestCases += check_value("atan(0.5)+atan(2)==pi/2",
                                       add(atan(half, D), atan(from_native<double>(2.0), D)), s_pi_2, reportTestCases);
    nrOfFailedTestCases += check_value("4*atan(1)==pi",
                                       mul(atan(from_native<double>(1.0), D), from_native<double>(4.0), W), s_pi, reportTestCases);
    ER x = from_native<double>(2.0), y = from_native<double>(3.0);
    nrOfFailedTestCases += check_identity("log(xy)==log(x)+log(y)",
                                          log(mul(x, y, W), D), add(log(x, D), log(y, D)), reportTestCases);

    // asin(x)+acos(x)==pi/2 exercises add() over an EXACT leading-term cancellation;
    // add() does not renormalize it to 0-overlap (aborts the ZBCL invariant). #1057.
#if ELREAL_ADD_EXACT_CANCEL_FIXED
    nrOfFailedTestCases += check_value("asin(0.5)+acos(0.5)==pi/2",
                                       add(asin(half, D), acos(half, D)), s_pi_2, reportTestCases);
#endif

    // === exp / sin / cos family: currently capped at ~17 digits (#1058) ===========
    // exp(), sin_series/cos_series multiply each Taylor term by a HOST-DOUBLE
    // reciprocal coefficient (1/n, 1/(2n+1)!), capping the sum at host-double
    // precision; sinh/cosh/tanh/tan compose them. Same bug class as e_zbcl (#1054)
    // but in the FUNCTIONS, not the constant. Gated off until #1058 lands; flip to 1
    // then -- the references and identities below already pass at the fixed precision.
#if ELREAL_EXP_SERIES_HIGH_PRECISION
    ER a = from_native<double>(0.5), b = from_native<double>(0.25);
    // absolute spot-checks
    nrOfFailedTestCases += check_value("exp(1)",    exp(from_native<double>(1.0), D),  s_e,         reportTestCases);
    nrOfFailedTestCases += check_value("sin(0.5)",  sin(from_native<double>(0.5), D),  s_sin_half,  reportTestCases);
    nrOfFailedTestCases += check_value("cos(0.5)",  cos(from_native<double>(0.5), D),  s_cos_half,  reportTestCases);
    nrOfFailedTestCases += check_value("tan(0.5)",  tan(from_native<double>(0.5), D),  s_tan_half,  reportTestCases);
    nrOfFailedTestCases += check_value("sinh(0.5)", sinh(from_native<double>(0.5), D), s_sinh_half, reportTestCases);
    nrOfFailedTestCases += check_value("cosh(0.5)", cosh(from_native<double>(0.5), D), s_cosh_half, reportTestCases);
    nrOfFailedTestCases += check_value("tanh(0.5)", tanh(from_native<double>(0.5), D), s_tanh_half, reportTestCases);
    // inverse round-trips
    nrOfFailedTestCases += check_value("log(exp(0.5))==0.5",  log(exp(half, D), D),                     "0.5", reportTestCases);
    nrOfFailedTestCases += check_value("exp(log(2))==2",      exp(log(from_native<double>(2.0), D), D), "2",   reportTestCases);
    // sin(asin)/cos(acos) cap at ~234 digits: sin/cos lose precision on a high-precision
    // MULTI-BLOCK argument near pi/6 (asin is itself 320-digit accurate; the loss is in
    // sin_series/cos_series, and it predates the #1061 Ph3b online conversion -- eager was
    // identical at ~239). Gated off until #1076 lands; flip to 1 then.
#if ELREAL_SINCOS_ROUNDTRIP_HIGH_PRECISION
    nrOfFailedTestCases += check_value("sin(asin(0.5))==0.5", sin(asin(half, D), D),                    "0.5", reportTestCases);
    nrOfFailedTestCases += check_value("cos(acos(0.5))==0.5", cos(acos(half, D), D),                    "0.5", reportTestCases);
#endif
    nrOfFailedTestCases += check_value("tan(atan(0.5))==0.5", tan(atan(half, D), D),                    "0.5", reportTestCases);
    // Pythagorean / hyperbolic
    ER s = sin(half, D), c = cos(half, D);
    nrOfFailedTestCases += check_value("sin^2+cos^2==1", add(mul(s, s, W), mul(c, c, W)), "1", reportTestCases);
    ER sh = sinh(half, D), ch = cosh(half, D);
    nrOfFailedTestCases += check_value("cosh^2-sinh^2==1", sub(mul(ch, ch, W), mul(sh, sh, W)), "1", reportTestCases);
    // addition theorems (compared side-to-side as exact dyadics)
    nrOfFailedTestCases += check_identity("exp(a+b)==exp(a)*exp(b)",
                                          exp(add(a, b), D), mul(exp(a, D), exp(b, D), W), reportTestCases);
    ER sa = sin(a, D), ca = cos(a, D), sb = sin(b, D), cb = cos(b, D);
    nrOfFailedTestCases += check_identity("sin(a+b)==sin a cos b+cos a sin b",
                                          sin(add(a, b), D), add(mul(sa, cb, W), mul(ca, sb, W)), reportTestCases);
    ER sha = sinh(a, D), cha = cosh(a, D), shb = sinh(b, D), chb = cosh(b, D);
    nrOfFailedTestCases += check_identity("cosh(a+b)==cosh a cosh b+sinh a sinh b",
                                          cosh(add(a, b), D), add(mul(cha, chb, W), mul(sha, shb, W)), reportTestCases);
#endif
#else
    std::cout << "  high-precision transcendental validation runs at REGRESSION_LEVEL_4 (stress) only\n";
#endif

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (const std::exception& err) {
    std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
