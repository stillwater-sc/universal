// conversion.cpp: native <-> elreal conversion + non-finite mapping (#1079 Phase 5).
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

// (1) native -> elreal -> native round-trips for exactly-representable values
int verify_roundtrip() {
    int n = 0;
    const double cases[] = { 0.0, 1.0, -1.0, 0.5, -0.25, 3.0, 1024.0, -2048.0,
                             123456.0, 1.0e308, 1.0e-307, 0.125, -7.5 };
    for (double d : cases) {
        Real e = d;                                   // implicit native ctor
        if (static_cast<double>(e) != d) ++n;         // exact double round-trip
        if (static_cast<float>(Real(static_cast<float>(d))) != static_cast<float>(d)) ++n;
    }
    // integer ctors land on the same value as the double ctor
    if (Real(42) != Real(42.0)) ++n;
    if (Real(-7) != Real(-7.0)) ++n;
    if (Real(1000000ULL) != Real(1000000.0)) ++n;
    // long double path keeps at least double fidelity
    if (std::fabs(static_cast<long double>(Real(0.1)) - 0.1L) > 1e-15L) ++n;
    return n;
}

// (2) non-finite construction + conversion mapping
int verify_nonfinite_convert() {
    int n = 0;
    const double dinf = std::numeric_limits<double>::infinity();
    const double dnan = std::numeric_limits<double>::quiet_NaN();

    Real pinf = dinf, ninf = -dinf, nan = dnan;
    if (!pinf.isinf() || !ninf.isinf() || !nan.isnan()) ++n;
    if (pinf.isfinite() || nan.isfinite()) ++n;
    if (pinf.sign() != 1 || ninf.sign() != -1) ++n;
    if (!ninf.isneg()) ++n;
    // conversion back to host reproduces inf / nan
    if (!std::isinf(static_cast<double>(pinf)) || static_cast<double>(pinf) < 0.0) ++n;
    if (!std::isinf(static_cast<double>(ninf)) || static_cast<double>(ninf) > 0.0) ++n;
    if (!std::isnan(static_cast<double>(nan))) ++n;
    if (!std::isnan(static_cast<float>(nan))) ++n;
    // free-function classifiers
    if (!isnan(nan) || !isinf(pinf) || !isfinite(Real(1.0))) ++n;
    if (!signbit(ninf) || signbit(pinf)) ++n;
    // rendering: to_triple carries the sign separately, so the tag is unsigned
    if (to_triple(pinf) != "(+, inf)") ++n;
    if (to_triple(ninf) != "(-, inf)") ++n;
    if (to_triple(nan)  != "(+, nan)") ++n;
    if (to_components(ninf) != "( -inf )") ++n;        // components keeps the sign
    return n;
}

// (3) SpecificValue encodings
int verify_specific_values() {
    int n = 0;
    if (!Real(SpecificValue::infpos).isinf() || Real(SpecificValue::infpos).sign() != 1) ++n;
    if (!Real(SpecificValue::infneg).isinf() || Real(SpecificValue::infneg).sign() != -1) ++n;
    if (!Real(SpecificValue::qnan).isnan()) ++n;
    if (!Real(SpecificValue::snan).isnan()) ++n;          // snan folds to qnan
    if (!Real(SpecificValue::zero).iszero()) ++n;
    if (!(static_cast<double>(Real(SpecificValue::maxpos)) > 0.0)) ++n;
    if (!(static_cast<double>(Real(SpecificValue::maxneg)) < 0.0)) ++n;
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
    std::string test_suite = "elreal conversion: native <-> elreal + non-finite (#1079 Phase 5)";
    int nrOfFailedTestCases = 0;
    bool reportTestCases = false;
    ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

    // TODO: place hand-run diagnostics here (this branch ignores failures)

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return EXIT_SUCCESS;

#else

#if REGRESSION_LEVEL_1

    nrOfFailedTestCases += verify_roundtrip();
    nrOfFailedTestCases += verify_nonfinite_convert();
    nrOfFailedTestCases += verify_specific_values();

#endif

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (const std::exception& err) {
    std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
