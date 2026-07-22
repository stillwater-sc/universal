// logic.cpp: elreal comparison operators incl. IEEE non-finite ordering (#1079 Phase 5).
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <limits>
#include <string>

#include <universal/number/elreal/elreal.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

using namespace sw::universal;
using Real = elreal<double>;

// opaque(x): pass x through a volatile so static analysis (cppcheck/Codacy) cannot
// constant-fold the operands. Without this the comparison operators get folded to
// known truth values and adjacent checks read as duplicateCondition -- and, worse,
// the operators would not actually be exercised at analysis time.
double opaque(double x) { volatile double v = x; return v; }

// Each relation is accumulated with `n += !(...)` rather than `if (...) ++n;`:
// the comparison operators are genuinely exercised on opaque operands, while the
// checks are plain statements (not if-conditions), so the mirror operators (a<b
// vs b>a, a<=b vs b>=a) do not read as duplicate if-conditions to static analysis.

// (1) finite ordering
int verify_finite_order() {
    int n = 0;
    Real third = Real(opaque(1.0)) / Real(opaque(3.0));
    Real half  = Real(opaque(1.0)) / Real(opaque(2.0));
    Real one   = third + third + third;            // == 1 to precision
    n += !(third <  half);
    n += !(half  >  third);
    n += !(third <= half);
    n += !(half  >= third);
    n +=  (third == half);
    n += !(third != half);
    n += !(one == Real(1.0));                       // 3*(1/3) == 1
    n += !(one <= Real(1.0));
    n += !(one >= Real(1.0));
    n += !((-half) < (-third));                     // negation flips ordering
    // mixed with double
    n += !(third < 0.5);
    n += !(half == 0.5);
    n += !(half > 0.25);
    return n;
}

// (2) NaN is unordered: every relation but != is false
int verify_nan_unordered() {
    int n = 0;
    Real nan = Real(opaque(std::numeric_limits<double>::quiet_NaN()));
    Real x   = Real(opaque(1.0)) / Real(opaque(3.0));
    n += (nan == nan);
    n += (nan == x);
    n += (nan <  x);
    n += (nan >  x);
    n += (nan <= x);
    n += (nan >= x);
    n += !(nan != nan);                             // the one true relation
    n += !(nan != x);
    return n;
}

// (3) infinities: -inf < finite < +inf; like-signed infinities compare equal
int verify_inf_order() {
    int n = 0;
    const double dinf = opaque(std::numeric_limits<double>::infinity());
    Real pinf = dinf, ninf = -dinf;
    Real pinf2 = Real(opaque(dinf)), ninf2 = -pinf2;   // distinct instances for reflexive ==
    Real x = Real(opaque(1.0)) / Real(opaque(3.0));
    n += !(pinf >  x);
    n += !(ninf <  x);
    n += !(ninf <  pinf);
    n += !(pinf >  ninf);
    n += !(pinf == pinf2);                          // +inf == +inf
    n += !(ninf == ninf2);                          // -inf == -inf
    n +=  (pinf == ninf);
    n += !(pinf >= pinf2);
    n += !(ninf <= ninf2);
    n += !(x < pinf);
    n += !(x > ninf);
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
    std::string test_suite = "elreal logic: ordering incl. non-finite (#1079 Phase 5)";
    int nrOfFailedTestCases = 0;
    bool reportTestCases = false;
    ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

    // TODO: place hand-run diagnostics here (this branch ignores failures)

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return EXIT_SUCCESS;

#else

#if REGRESSION_LEVEL_1

    nrOfFailedTestCases += verify_finite_order();
    nrOfFailedTestCases += verify_nan_unordered();
    nrOfFailedTestCases += verify_inf_order();

#endif

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (const std::exception& err) {
    std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
