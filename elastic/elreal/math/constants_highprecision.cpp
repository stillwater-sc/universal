// constants_highprecision.cpp: exact high-precision validation of the elreal
// constant generators against the 320-digit decimal reference strings (#1048).
//
// Why this exists
// ---------------
// The per-PR constants test (constants.cpp) only compares against std::numbers
// (~16 digits). For an exact-lazy real type that validates almost nothing: the
// mul/div/priestRenorm/series machinery the generators are built from is untested
// past ~16 digits, so high-precision operator bugs are invisible. The exact-dyadic
// oracle here (verification/elreal_reference_digits.hpp) compares a generated
// constant to the mpmath 320-digit reference to hundreds of digits. It already
// caught two bugs the double-tolerance test missed:
//   - e_zbcl summed host-double 1/n! terms -> only ~17 correct digits (now fixed);
//   - euler_gamma_zbcl returned a bare double literal (~17 digits) -- now a real
//     Brent-McMillan B1 generator reaching the same digit ceiling (#1053).
//
// Ceiling
// -------
// A double-hosted ZBCL near 1.0 tops out at ~19 components (~308 decimal digits):
// the smallest non-overlapping component sits near 2^-1022, the host's normal-range
// floor. #1052 lifted the artificial pipeline floors (divide.hpp exp_floor; the
// series-accumulation guard in odd_power_series / e_zbcl) so the generators now
// reach that architectural ceiling rather than stopping ~30 digits short. This test
// asserts >= 300 digits; going beyond ~308-312 would require a host with a wider
// exponent range and is out of scope here.
//
// Cost / placement
// ----------------
// Generating a constant to its ceiling is O(depth^4): ~10s each at -O2 (depth 20),
// far more under -O0/instrumented builds. This test is gated to REGRESSION_LEVEL_4
// (stress / manual) and is a no-op at the sanity level CI / sanitizers / coverage
// use, so it never re-bloats those tiers.
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

#include <iostream>
#include <string>
#include <string_view>

#include <universal/number/elreal/elreal.hpp>
#include <universal/number/elreal/math/constants.hpp>
#include <universal/verification/elreal_reference_digits.hpp>
#include <universal/verification/test_suite.hpp>
#include <math/constants/reference_constants.hpp>

namespace {

#if REGRESSION_LEVEL_4
// Generation depth. The double-host ceiling is reached by ~depth 18-20 (each
// 53-bit block ~ 16 digits); 20 clears the >= 300-digit threshold with margin.
constexpr std::size_t kDepth = 20;
constexpr int kMinDigits = 300;

int check(const char* name, const sw::universal::ZBCL<double>& z,
          std::string_view ref, bool reportTestCases) {
    using namespace sw::universal;
    int got = agreed_decimal_digits(z, ref);
    if (got < kMinDigits) {
        std::cout << "  FAIL " << name << ": agreed " << got
                  << " digits, want >= " << kMinDigits << '\n';
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
    std::string test_suite = "elreal high-precision constants vs 320-digit reference (#1048)";
    int nrOfFailedTestCases = 0;
    bool reportTestCases = true;
    ReportTestSuiteHeader(test_suite, reportTestCases);

#if REGRESSION_LEVEL_4
    // The ten constants with both a high-precision generator and a reference string.
    nrOfFailedTestCases += check("pi",          pi_zbcl<double>(kDepth),          s_pi,          reportTestCases);
    nrOfFailedTestCases += check("e",           e_zbcl<double>(kDepth),           s_e,           reportTestCases);
    nrOfFailedTestCases += check("ln2",         ln2_zbcl<double>(kDepth),         s_ln2,         reportTestCases);
    nrOfFailedTestCases += check("ln10",        ln10_zbcl<double>(kDepth),        s_ln10,        reportTestCases);
    nrOfFailedTestCases += check("log2_10",     log2_10_zbcl<double>(kDepth),     s_log2_10,     reportTestCases);
    nrOfFailedTestCases += check("phi",         phi_zbcl<double>(kDepth),         s_phi,         reportTestCases);
    nrOfFailedTestCases += check("sqrt2",       sqrt2_zbcl<double>(kDepth),       s_sqrt2,       reportTestCases);
    nrOfFailedTestCases += check("sqrt3",       sqrt3_zbcl<double>(kDepth),       s_sqrt3,       reportTestCases);
    nrOfFailedTestCases += check("sqrt5",       sqrt5_zbcl<double>(kDepth),       s_sqrt5,       reportTestCases);
    // euler_gamma via Brent-McMillan B1 (#1053). Like the other constants it is an
    // eager-batch generator; it is the slowest here (no elementary series + a slow
    // shared ln2) -- the online-ops speedup for the whole series layer is #1061 Ph3.
    nrOfFailedTestCases += check("euler_gamma", euler_gamma_zbcl<double>(kDepth), s_euler_gamma, reportTestCases);
#else
    std::cout << "  high-precision constant validation runs at REGRESSION_LEVEL_4 (stress) only\n";
#endif

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (const std::exception& err) {
    std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
