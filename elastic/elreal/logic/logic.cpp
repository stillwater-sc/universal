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

// (1) finite ordering (computed irrational/rational operands -- not literals, so
// cppcheck cannot fold the elreal comparisons to a known truth value).
int verify_finite_order() {
    int n = 0;
    Real third = Real(1.0) / Real(3.0);
    Real half  = Real(1.0) / Real(2.0);
    Real one   = third + third + third;            // == 1 to precision
    if (!(third < half)) ++n;
    if (!(half > third)) ++n;
    if (!(third <= half)) ++n;
    if (!(half >= third)) ++n;
    if (third == half) ++n;
    if (!(third != half)) ++n;
    if (!(one == Real(1.0))) ++n;                  // 3*(1/3) == 1
    if (!(one <= Real(1.0)) || !(one >= Real(1.0))) ++n;
    // negation flips ordering
    if (!((-half) < (-third))) ++n;
    // mixed with double
    if (!(third < 0.5)) ++n;
    if (!(half == 0.5)) ++n;
    if (!(half > 0.25)) ++n;
    return n;
}

// (2) NaN is unordered: every relation but != is false
int verify_nan_unordered() {
    int n = 0;
    Real nan = std::numeric_limits<double>::quiet_NaN();
    Real x   = Real(1.0) / Real(3.0);
    if (nan == nan) ++n;
    if (nan == x)   ++n;
    if (nan <  x)   ++n;
    if (nan >  x)   ++n;
    if (nan <= x)   ++n;
    if (nan >= x)   ++n;
    if (!(nan != nan)) ++n;                        // the one true relation
    if (!(nan != x))   ++n;
    return n;
}

// (3) infinities: -inf < finite < +inf; like-signed infinities compare equal
int verify_inf_order() {
    int n = 0;
    const double dinf = std::numeric_limits<double>::infinity();
    Real pinf = dinf, ninf = -dinf;
    Real x = Real(1.0) / Real(3.0);
    if (!(pinf > x))    ++n;
    if (!(ninf < x))    ++n;
    if (!(ninf < pinf)) ++n;
    if (!(pinf > ninf)) ++n;
    if (!(pinf == pinf)) ++n;
    if (!(ninf == ninf)) ++n;
    if (pinf == ninf)    ++n;
    if (!(pinf >= pinf) || !(ninf <= ninf)) ++n;
    if (!(x < pinf) || !(x > ninf)) ++n;
    return n;
}

} // anonymous

int main()
try {
    using namespace sw::universal;
    std::string test_suite = "elreal logic: ordering incl. non-finite (#1079 Phase 5)";
    int nrOfFailedTestCases = 0;
    bool reportTestCases = false;
    ReportTestSuiteHeader(test_suite, reportTestCases);

    nrOfFailedTestCases += verify_finite_order();
    nrOfFailedTestCases += verify_nan_unordered();
    nrOfFailedTestCases += verify_inf_order();

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (const std::exception& err) {
    std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
