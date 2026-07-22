// attributes.cpp: numeric_limits, attributes, and manipulators for elreal (#1079 Phase 3).
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <limits>
#include <sstream>
#include <string>

#include <universal/number/elreal/elreal.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

using namespace sw::universal;
using Real = elreal<double>;

// (1) numeric_limits specialisation
//
// The compile-time-constant fields are checked with static_assert (the idiomatic
// test for constexpr members; it also avoids cppcheck knownConditionTrueFalse on
// the folded comparisons). Only the runtime value factories are checked at runtime.
int verify_numeric_limits() {
    using lim = std::numeric_limits<Real>;
    static_assert(lim::is_specialized,  "elreal numeric_limits must be specialized");
    static_assert(lim::is_signed,       "elreal is signed");
    static_assert(!lim::is_integer,     "elreal is not an integer type");
    static_assert(!lim::is_exact,       "elreal materialises to host blocks (not exact)");
    static_assert(lim::radix == 2,      "elreal radix is 2");
    static_assert(lim::has_infinity,    "elreal has an inf state (#1079 Phase 5)");
    static_assert(lim::has_quiet_NaN,   "elreal has a qnan state (#1079 Phase 5)");
    static_assert(!lim::has_signaling_NaN, "elreal folds snan to qnan");
    static_assert(!lim::is_bounded,     "elreal has an unbounded exponent");
    // precision reported against the nominal default (kElrealDefaultPrecision blocks).
    static_assert(lim::digits == static_cast<int>(kElrealDefaultPrecision) * std::numeric_limits<double>::digits,
                  "elreal digits = default precision * host digits");
    static_assert(lim::digits10 > 0, "elreal digits10 is positive");

    int n = 0;
    // value factories produce sane magnitudes
    if (!(double(lim::max()) > 0.0)) ++n;
    if (!(double(lim::min()) > 0.0)) ++n;
    if (!(double(lim::lowest()) < 0.0)) ++n;
    if (!(double(lim::epsilon()) > 0.0 && double(lim::epsilon()) < 1.0)) ++n;
    // non-finite factories now return real states (#1079 Phase 5)
    if (!std::isinf(double(lim::infinity()))) ++n;
    if (!(double(lim::infinity()) > 0.0)) ++n;       // +inf
    if (!std::isnan(double(lim::quiet_NaN()))) ++n;
    // denorm_absent contract: denorm_min() == min()
    if (double(lim::denorm_min()) != double(lim::min())) ++n;
    return n;
}

// (2) attributes: sign / scale / significand
int verify_attributes() {
    int n = 0;
    Real pos = 6.0, neg = -6.0, zero;
    if (sign(pos) != 1) ++n;
    if (sign(neg) != -1) ++n;
    if (sign(zero) != 1) ++n;                       // +1 for zero
    // 6.0 = 1.5 * 2^2 -> scale 2, significand ~1.5
    if (scale(pos) != 2) ++n;
    if (std::fabs(significand<double>(pos) - 1.5) > 1e-12) ++n;
    if (scale(zero) != 0) ++n;
    if (significand<double>(zero) != 0.0) ++n;
    // sign attribute matches isneg()
    if (neg.isneg() != true || pos.isneg() != false) ++n;
    return n;
}

// (3) manipulators: type_tag / to_components / to_binary / to_triple / I/O
int verify_manipulators() {
    int n = 0;
    if (type_tag(Real{}) != "elreal<double>") ++n;
    if (type_tag(elreal<float>{}) != "elreal<float>") ++n;

    Real q = Real(1.0) / Real(8.0);                 // 0.125 = exactly one block
    std::string comps = to_components(q);
    if (comps.find("0.125") == std::string::npos) ++n;
    std::string bin = to_binary(q);
    if (bin.empty() || bin == "0") ++n;
    // 0.125 = 1 * 2^-3  ->  triple (sign, scale, significand) = (+, -3, 1)
    std::string tri = to_triple(q);
    if (tri != "(+, -3, 1)") ++n;

    // operator<< then operator>> round-trips a double-representable value
    std::stringstream ss;
    ss << Real(0.25);
    Real back;
    ss >> back;
    if (std::fabs(double(back) - 0.25) > 1e-15) ++n;

    // zero renders cleanly
    if (to_components(Real{}) != "( 0 )") ++n;
    if (to_binary(Real{}) != "0") ++n;
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
    std::string test_suite = "elreal numeric_limits / attributes / manipulators (#1079 Phase 3)";
    int nrOfFailedTestCases = 0;
    bool reportTestCases = false;
    ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

    // TODO: place hand-run diagnostics here (this branch ignores failures)

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return EXIT_SUCCESS;

#else

#if REGRESSION_LEVEL_1

    nrOfFailedTestCases += verify_numeric_limits();
    nrOfFailedTestCases += verify_attributes();
    nrOfFailedTestCases += verify_manipulators();

#endif

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (const std::exception& err) {
    std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
