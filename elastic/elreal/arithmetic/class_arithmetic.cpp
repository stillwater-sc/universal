// class_arithmetic.cpp: class-level elreal arithmetic incl. IEEE non-finite rules
// (#1079 Phase 5). The ZBCL-level arithmetic is covered by the sibling tests in
// this folder; this exercises the class facade's operators and non-finite policy.
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

bool near(const Real& v, double ref, double tol = 1.0e-12) {
    return std::fabs(static_cast<double>(v) - ref) <= tol * (1.0 + std::fabs(ref));
}

// (1) finite arithmetic at the class level
int verify_finite() {
    int n = 0;
    Real a = Real(1.0) / Real(3.0);                // 1/3
    Real b = Real(1.0) / Real(7.0);                // 1/7
    if (!near(a + b, 1.0/3.0 + 1.0/7.0)) ++n;
    if (!near(a - b, 1.0/3.0 - 1.0/7.0)) ++n;
    if (!near(a * b, (1.0/3.0) * (1.0/7.0))) ++n;
    if (!near(a / b, (1.0/3.0) / (1.0/7.0))) ++n;
    // identities. NOTE: a + (-a) is value-zero but a LAZY zero stream, not the
    // structurally-empty canonical zero -- iszero() (a cheap structural test, the
    // right semantics for a semi-decidable lazy real) is therefore false here.
    if (!near(a + (-a), 0.0)) ++n;                 // a + (-a) == 0 (by value)
    if (!near(a * Real(1.0), 1.0/3.0)) ++n;
    if (!near((a + b) - b, 1.0/3.0)) ++n;          // exact round trip
    // mixed-double operators
    if (!near(a * 3.0, 1.0)) ++n;
    if (!near(2.0 + a, 2.0 + 1.0/3.0)) ++n;
    return n;
}

// (2) IEEE non-finite arithmetic
int verify_nonfinite() {
    int n = 0;
    const double dinf = std::numeric_limits<double>::infinity();
    Real pinf = dinf, ninf = -dinf;
    Real nan  = std::numeric_limits<double>::quiet_NaN();
    Real one = Real(1.0), zero = Real(0.0), two = Real(2.0);

    // infinity arithmetic
    if (!(pinf + one).isinf())              ++n;   // inf + finite = inf
    if (!(pinf + pinf).isinf())             ++n;   // inf + inf = inf
    if (!(pinf - pinf).isnan())             ++n;   // inf - inf = nan
    if (!(pinf + ninf).isnan())             ++n;
    if (!(pinf * two).isinf() || (pinf * two).sign() != 1) ++n;
    if (!(ninf * two).isinf() || (ninf * two).sign() != -1) ++n;
    if (!(pinf * ninf).isinf() || (pinf * ninf).sign() != -1) ++n;  // sign product
    if (!(pinf * zero).isnan())             ++n;   // inf * 0 = nan
    if (!(pinf / pinf).isnan())             ++n;   // inf / inf = nan
    if (!(one / pinf).iszero())             ++n;   // finite / inf = 0

    // division by zero
    if (!(one / zero).isinf() || (one / zero).sign() != 1) ++n;    // 1/0 = +inf
    if (!((-one) / zero).isinf() || ((-one) / zero).sign() != -1) ++n; // -1/0 = -inf
    if (!(zero / zero).isnan())             ++n;   // 0/0 = nan
    if (!(pinf / zero).isinf())             ++n;   // inf/0 = inf

    // nan propagation
    if (!(nan + one).isnan())  ++n;
    if (!(nan * pinf).isnan()) ++n;
    if (!(nan / two).isnan())  ++n;
    if (!(-nan).isnan())       ++n;

    // abs of non-finite
    if (!abs(ninf).isinf() || abs(ninf).sign() != 1) ++n;          // |-inf| = +inf
    if (!abs(nan).isnan())  ++n;
    if (!near(abs(-(Real(1.0)/Real(3.0))), 1.0/3.0)) ++n;
    return n;
}

} // anonymous

int main()
try {
    using namespace sw::universal;
    std::string test_suite = "elreal class arithmetic incl. non-finite (#1079 Phase 5)";
    int nrOfFailedTestCases = 0;
    bool reportTestCases = false;
    ReportTestSuiteHeader(test_suite, reportTestCases);

    nrOfFailedTestCases += verify_finite();
    nrOfFailedTestCases += verify_nonfinite();

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (const std::exception& err) {
    std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
