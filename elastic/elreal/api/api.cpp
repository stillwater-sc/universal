// api.cpp: usage patterns for the elreal class facade (#1079 Phase 1).
//
// elreal<FpType> is a plug-in arithmetic number system over the McCleeary LFPERA
// lazy block co-list (ZBCL): standard native ctors / conversions / operators, plus
// a lazy state-machine extension (runtime precision, incremental refine, raw stream).
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
#include <universal/traits/elreal_traits.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

using namespace sw::universal;

// a generic kernel: elreal must drop into templated numeric code like any other type
template <typename Real>
Real poly(const Real& x) { return x * x * x - Real(2) * x + Real(1); }   // x^3 - 2x + 1

int check(const char* tag, double got, double want, double tol) {
    if (std::fabs(got - want) > tol) {
        std::cout << "  FAIL " << tag << ": " << got << " != " << want << '\n';
        return 1;
    }
    return 0;
}

// (1) construction + native conversion
int verify_construction() {
    int n = 0;
    elreal<double> a;            n += (a.iszero() ? 0 : 1);          // default = 0
    elreal<double> b = 3.0;      n += check("ctor(3.0)", double(b), 3.0, 1e-12);
    elreal<double> c = 42;       n += check("ctor(int)", double(c), 42.0, 1e-12);
    elreal<double> d(b);         n += check("copy", double(d), 3.0, 1e-12);
    n += (is_elreal<elreal<double>> ? 0 : 1);
    return n;
}

// (2) lazy arithmetic + boundary conversion
int verify_arithmetic() {
    int n = 0;
    elreal<double> a = 3.0, b = 7.0;
    n += check("3+7",  double(a + b), 10.0, 1e-12);
    n += check("3-7",  double(a - b), -4.0, 1e-12);
    n += check("3*7",  double(a * b), 21.0, 1e-12);
    n += check("3/7",  double(a / b), 3.0 / 7.0, 1e-12);
    n += check("-3",   double(-a), -3.0, 1e-12);
    n += check("abs",  double(abs(-a)), 3.0, 1e-12);
    n += check("poly(2)", double(poly(elreal<double>(2.0))), 5.0, 1e-12);   // plug-in kernel
    return n;
}

// (3) depth-bounded comparison. The operands are COMPUTED (irrational quotients /
// exact ratios) rather than literal-initialised, so each comparison is a genuine
// runtime exercise of the operator -- ordering on the nonzero leading limb, and
// exact equality on a terminating ratio.
int verify_logic() {
    int n = 0;
    elreal<double> third = elreal<double>(1.0) / elreal<double>(3.0);   // 0.3333...
    elreal<double> half  = elreal<double>(1.0) / elreal<double>(2.0);   // 0.5
    elreal<double> one   = 1.0;
    n += (third <  half  ? 0 : 1);    // 1/3 < 1/2
    n += (half  >  third ? 0 : 1);    // 1/2 > 1/3
    n += (third != half  ? 0 : 1);
    n += (third <= half  ? 0 : 1);
    n += (half  >= third ? 0 : 1);
    n += (third <  one   ? 0 : 1);    // 1/3 < 1
    // exact equality on a terminating ratio: 7/7 == 1 to any precision.
    elreal<double> seven = elreal<double>(7.0);
    n += ((seven / seven) == one ? 0 : 1);
    return n;
}

// (4) lazy API / state-machine extension
int verify_lazy_api() {
    int n = 0;
    elreal<double> q = elreal<double>(1.0) / elreal<double>(3.0);    // 1/3, irrational stream
    n += (q.limbs(3).size() == 3 ? 0 : 1);                          // pull 3 limbs
    q.refine(16);
    n += (q.precision() == 16 ? 0 : 1);                            // incremental refine
    n += check("1/3 approx@16", q.approx<double>(16), 1.0 / 3.0, 1e-12);
    n += (q.stream().is_empty() ? 1 : 0);                          // raw state machine handle
    {   // scoped precision override
        elreal_precision_guard g(20);
        elreal<double> z;
        n += (z.precision() == 20 ? 0 : 1);
    }
    elreal<double> z2;
    n += (z2.precision() == 8 ? 0 : 1);                            // restored
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
    std::string test_suite = "elreal class facade (plug-in + lazy API) (#1079)";
    int nrOfFailedTestCases = 0;
    bool reportTestCases = false;
    ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

    // TODO: place hand-run diagnostics here (this branch ignores failures)

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return EXIT_SUCCESS;

#else

#if REGRESSION_LEVEL_1

    nrOfFailedTestCases += verify_construction();
    nrOfFailedTestCases += verify_arithmetic();
    nrOfFailedTestCases += verify_logic();
    nrOfFailedTestCases += verify_lazy_api();

#endif

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (const std::exception& err) {
    std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
