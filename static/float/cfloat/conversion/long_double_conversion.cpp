// long_double_conversion.cpp: regression for issue #937.
//
// Prior to the fix, clang -O2 miscompiled the `1.0f / TargetFloat(1ull << N)`
// pattern in `cfloat::to_native<long double>()` for values that require a
// negative unbiased exponent (e.g. 0.125, 0.25, 0.5). The miscompile produced
// NaN or the wrong magnitude. gcc and MSVC were unaffected; the bug slipped
// through because no test in the cfloat conversion suite exercised
// `static_cast<long double>(cfloat<>)` -- MSVC aliases long double to double,
// so the broken path was untested.
//
// This file walks a sweep of representative cfloat<> configurations and asserts
// `static_cast<long double>(cf{v}) == static_cast<double>(cf{v}) (as long double)`
// for every value v in a fixed sample. The equality holds because every v in the
// sample is exactly representable in the cfloat<> configuration under test.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

#define CFLOAT_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_suite.hpp>

#if LONG_DOUBLE_SUPPORT

namespace {

// Verify that cf -> long double matches cf -> double for a fixed sweep of
// exactly-representable values. Any deviation indicates a regression of the
// kind issue #937 was about: the long-double path producing different bits
// than the double path for the same cfloat input.
template <typename CfloatT>
int verify_long_double_path(const std::string& tag) {
    using namespace sw::universal;
    int nrFailures = 0;

    // Exactly-representable test values across positive, negative, and a range
    // of negative-exponent powers of two.
    const double samples[] = {
        0.0, 0.125, 0.25, 0.5, 1.0, 1.5, 2.0, 3.25, -1.5,
        0.0625,   // 2^-4
        0.015625, // 2^-6 (within half's normal range)
    };

    for (double v : samples) {
        CfloatT cf{v};
        // Skip values that aren't exactly representable in this cfloat config
        // (we only want to detect conversion deviation, not rounding).
        double  d_back  = static_cast<double>(cf);
        if (d_back != v) continue;

        long double via_ld = static_cast<long double>(cf);
        long double via_d  = static_cast<long double>(static_cast<double>(cf));
        if (via_ld != via_d) {
            std::cout << tag << " mismatch on v=" << v
                      << ": (long double) cf = " << static_cast<double>(via_ld)
                      << " vs (double) cf as long double = " << static_cast<double>(via_d)
                      << '\n';
            ++nrFailures;
        }
    }
    return nrFailures;
}

} // anonymous

#endif // LONG_DOUBLE_SUPPORT

int main()
try {
    using namespace sw::universal;
    std::string test_suite = "cfloat<> -> long double conversion (issue #937)";
    int nrOfFailedTestCases = 0;
    bool reportTestCases = false;
    ReportTestSuiteHeader(test_suite, reportTestCases);

#if LONG_DOUBLE_SUPPORT
    // Standard small-format aliases
    nrOfFailedTestCases += verify_long_double_path<half>("half");
    // Configurable widths spanning typical custom hardware shapes
    nrOfFailedTestCases += verify_long_double_path<cfloat<16, 5, std::uint16_t, true, false, false>>("cfloat<16,5>");
    nrOfFailedTestCases += verify_long_double_path<cfloat<24, 5, std::uint16_t, true, false, false>>("cfloat<24,5>");
    nrOfFailedTestCases += verify_long_double_path<cfloat<32, 8, std::uint32_t, true, false, false>>("cfloat<32,8>");
    nrOfFailedTestCases += verify_long_double_path<cfloat<64, 11, std::uint64_t, true, false, false>>("cfloat<64,11>");
#else
    std::cout << test_suite << " SKIPPED (LONG_DOUBLE_SUPPORT == 0)\n";
#endif

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (const std::exception& err) {
    std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
