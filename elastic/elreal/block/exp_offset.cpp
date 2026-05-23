// exp_offset.cpp: int32_t exp_offset boundary tests for elreal block<FpType>.
//
// The block's exp_offset multiplies in units of 2^E. Its purpose is to escape
// the host FpType's hardware exponent range, which matters for transcendentals
// at high depth in later phases.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cstdint>
#include <iostream>
#include <limits>

#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/bfloat16/bfloat16.hpp>
#include <universal/number/elreal/elreal.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

template <typename FpType>
int verify_exp_offset(const std::string& tag) {
    using namespace sw::universal;
    using B = block<FpType>;
    int nrFailures = 0;

    // exp_offset = 0: exponent() == scale_of_v()
    B at_one{ FpType{1.5}, 0 };
    if (at_one.exponent() != at_one.scale_of_v()) {
        std::cout << tag << " offset=0 mismatch\n"; ++nrFailures;
    }

    // Positive offset levering: each +1 step adds exp_step.
    B step_up{ FpType{1.5}, 1 };
    std::int64_t expected = at_one.scale_of_v() + B::exp_step;
    if (step_up.exponent() != expected) {
        std::cout << tag << " offset=+1 wrong: got " << step_up.exponent()
                  << " expected " << expected << '\n';
        ++nrFailures;
    }

    // Negative offset levering: each -1 step subtracts exp_step.
    B step_down{ FpType{1.5}, -1 };
    std::int64_t expected_dn = at_one.scale_of_v() - B::exp_step;
    if (step_down.exponent() != expected_dn) {
        std::cout << tag << " offset=-1 wrong: got " << step_down.exponent()
                  << " expected " << expected_dn << '\n';
        ++nrFailures;
    }

    // Large positive offset: enough to push the block's exponent far beyond
    // FpType's hardware exponent range -- this is the whole point.
    B very_high{ FpType{1.5}, 100 };
    std::int64_t expected_high = at_one.scale_of_v() + std::int64_t(100) * B::exp_step;
    if (very_high.exponent() != expected_high) {
        std::cout << tag << " offset=+100 wrong: got " << very_high.exponent()
                  << " expected " << expected_high << '\n';
        ++nrFailures;
    }

    // Extreme int32 boundary: exp_offset = INT32_MAX. The exponent() computation
    // must not overflow int (it returns int64).
    B extreme{ FpType{1.5}, std::numeric_limits<std::int32_t>::max() };
    std::int64_t expected_ext = at_one.scale_of_v()
        + static_cast<std::int64_t>(std::numeric_limits<std::int32_t>::max()) * B::exp_step;
    if (extreme.exponent() != expected_ext) {
        std::cout << tag << " offset=INT32_MAX wrong\n"; ++nrFailures;
    }

    return nrFailures;
}

} // anonymous

int main()
try {
    using namespace sw::universal;
    std::string test_suite = "elreal block<FpType> exp_offset boundary";
    int nrOfFailedTestCases = 0;
    bool reportTestCases = false;
    ReportTestSuiteHeader(test_suite, reportTestCases);

    nrOfFailedTestCases += verify_exp_offset<float>("block<float>");
    nrOfFailedTestCases += verify_exp_offset<double>("block<double>");
    nrOfFailedTestCases += verify_exp_offset<half>("block<half>");
    nrOfFailedTestCases += verify_exp_offset<bfloat16>("block<bfloat16>");
    nrOfFailedTestCases += verify_exp_offset<cfloat<24, 5, std::uint16_t, true, false, false>>("block<cfloat<24,5>>");
    nrOfFailedTestCases += verify_exp_offset<cfloat<32, 8, std::uint32_t, true, false, false>>("block<cfloat<32,8>>");

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (const std::exception& err) {
    std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
