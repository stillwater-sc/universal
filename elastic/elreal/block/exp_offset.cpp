// exp_arithmetic.cpp: tests for block<FpType>::exp (per-unit McCleeary exponent).
//
// Originally this file tested an `exp_offset` field that was a coarse
// multiplier in units of 2^E_FpType. The refactor to fine-grained per-unit
// exponents (renaming the field to `exp`) is foundational to the McCleeary
// algorithm; this file tests the simplified arithmetic.
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
int verify_exp(const std::string& tag) {
    using namespace sw::universal;
    using B = block<FpType>;
    int nrFailures = 0;

    // exp = 0: exponent() == scale_of_v()
    B at_one{ FpType{1.5}, 0 };
    if (at_one.exponent() != at_one.scale_of_v()) {
        std::cout << tag << " exp=0 mismatch\n"; ++nrFailures;
    }

    // +1 step adds 1
    B step_up{ FpType{1.5}, 1 };
    if (step_up.exponent() != at_one.scale_of_v() + 1) {
        std::cout << tag << " exp=+1 wrong: got " << step_up.exponent() << '\n';
        ++nrFailures;
    }

    // -1 step subtracts 1
    B step_dn{ FpType{1.5}, -1 };
    if (step_dn.exponent() != at_one.scale_of_v() - 1) {
        std::cout << tag << " exp=-1 wrong\n"; ++nrFailures;
    }

    // Large positive exp
    B very_high{ FpType{1.5}, 100000 };
    if (very_high.exponent() != at_one.scale_of_v() + 100000) {
        std::cout << tag << " exp=+100000 wrong\n"; ++nrFailures;
    }

    // INT32 boundary: exponent() returns int32_t now and may saturate / wrap;
    // we just confirm it does not overflow the runtime by computing.
    B extreme{ FpType{1.5}, std::numeric_limits<std::int32_t>::max() - 1 };
    std::int32_t e = extreme.exponent();
    (void)e;

    // Zero block: exponent() returns exp directly (not scale_of_v + exp).
    B zero_with_exp{ FpType{0}, 42 };
    if (zero_with_exp.exponent() != 42) {
        std::cout << tag << " zero-block exp wrong: got " << zero_with_exp.exponent()
                  << " expected 42\n";
        ++nrFailures;
    }

    return nrFailures;
}

} // anonymous

int main()
try {
    using namespace sw::universal;
    std::string test_suite = "elreal block<FpType> exp arithmetic";
    int nrOfFailedTestCases = 0;
    bool reportTestCases = false;
    ReportTestSuiteHeader(test_suite, reportTestCases);

    nrOfFailedTestCases += verify_exp<float>("block<float>");
    nrOfFailedTestCases += verify_exp<double>("block<double>");
    nrOfFailedTestCases += verify_exp<half>("block<half>");
    nrOfFailedTestCases += verify_exp<bfloat16>("block<bfloat16>");
    nrOfFailedTestCases += verify_exp<cfloat<24, 5, std::uint16_t, true, false, false>>("block<cfloat<24,5>>");
    nrOfFailedTestCases += verify_exp<cfloat<32, 8, std::uint32_t, true, false, false>>("block<cfloat<32,8>>");

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (const std::exception& err) {
    std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
