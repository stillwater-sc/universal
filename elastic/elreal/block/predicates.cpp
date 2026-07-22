// predicates.cpp: is_zero_block() and is_normalised() tests for elreal block<FpType>.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>

#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/bfloat16/bfloat16.hpp>
#include <universal/number/elreal/elreal.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

template <typename FpType>
int verify_predicates(const std::string& tag) {
    using namespace sw::universal;
    using B = block<FpType>;
    int nrFailures = 0;

    // is_zero_block
    B zero{ FpType{0}, 0 };
    B nonzero{ FpType{1.5}, 0 };
    if (!zero.is_zero_block())    { std::cout << tag << " zero block not detected\n"; ++nrFailures; }
    if (nonzero.is_zero_block())  { std::cout << tag << " nonzero detected as zero\n"; ++nrFailures; }

    // zero with non-zero exp is still a zero block (value is zero
    // regardless of offset scaling)
    B zero_offset{ FpType{0}, 42 };
    if (!zero_offset.is_zero_block()) {
        std::cout << tag << " zero with offset not detected as zero\n"; ++nrFailures;
    }

    // is_normalised on normal values across the IEEE-normal range. Note that
    // McCleeary normalisation is about the hidden bit being set (i.e., the
    // IEEE classification is FP_NORMAL), NOT about |v| in [1,2). Both 0.75 and
    // 3.0 are normal IEEE values and should pass.
    B norm{ FpType{1.5}, 0 };
    B below{ FpType{0.75}, 0 };
    B above{ FpType{3.0}, 0 };
    if (!norm.is_normalised())  { std::cout << tag << " 1.5 not normalised\n";  ++nrFailures; }
    if (!below.is_normalised()) { std::cout << tag << " 0.75 not normalised\n"; ++nrFailures; }
    if (!above.is_normalised()) { std::cout << tag << " 3.0 not normalised\n";  ++nrFailures; }

    // is_normalised on zero -> false
    if (zero.is_normalised()) { std::cout << tag << " zero claimed normalised\n"; ++nrFailures; }

    // is_normalised on a subnormal value -> false. Subnormals exist whenever
    // numeric_limits reports denorm support; otherwise skip silently.
    if constexpr (std::numeric_limits<FpType>::has_denorm == std::denorm_present) {
        FpType subnormal_v = std::numeric_limits<FpType>::denorm_min();
        B sub{ subnormal_v, 0 };
        if (sub.is_normalised()) {
            std::cout << tag << " subnormal claimed normalised\n"; ++nrFailures;
        }
    }

    return nrFailures;
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
    std::string test_suite = "elreal block<FpType> predicates";
    int nrOfFailedTestCases = 0;
    bool reportTestCases = false;
    ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

    // TODO: place hand-run diagnostics here (this branch ignores failures)

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return EXIT_SUCCESS;

#else

#if REGRESSION_LEVEL_1

    nrOfFailedTestCases += verify_predicates<float>("block<float>");
    nrOfFailedTestCases += verify_predicates<double>("block<double>");
    nrOfFailedTestCases += verify_predicates<half>("block<half>");
    nrOfFailedTestCases += verify_predicates<bfloat16>("block<bfloat16>");
    nrOfFailedTestCases += verify_predicates<cfloat<24, 5, std::uint16_t, true, false, false>>("block<cfloat<24,5>>");
    nrOfFailedTestCases += verify_predicates<cfloat<32, 8, std::uint32_t, true, false, false>>("block<cfloat<32,8>>");

#endif

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (const std::exception& err) {
    std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
