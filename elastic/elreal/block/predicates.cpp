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

    // zero with non-zero exp_offset is still a zero block (value is zero
    // regardless of offset scaling)
    B zero_offset{ FpType{0}, 42 };
    if (!zero_offset.is_zero_block()) {
        std::cout << tag << " zero with offset not detected as zero\n"; ++nrFailures;
    }

    // is_normalised on normal values
    B norm{ FpType{1.5}, 0 };
    if (!norm.is_normalised()) { std::cout << tag << " 1.5 not normalised\n"; ++nrFailures; }

    // is_normalised on zero -> false
    if (zero.is_normalised()) { std::cout << tag << " zero claimed normalised\n"; ++nrFailures; }

    return nrFailures;
}

} // anonymous

int main()
try {
    using namespace sw::universal;
    std::string test_suite = "elreal block<FpType> predicates";
    int nrOfFailedTestCases = 0;
    bool reportTestCases = false;
    ReportTestSuiteHeader(test_suite, reportTestCases);

    nrOfFailedTestCases += verify_predicates<float>("block<float>");
    nrOfFailedTestCases += verify_predicates<double>("block<double>");
    nrOfFailedTestCases += verify_predicates<half>("block<half>");
    nrOfFailedTestCases += verify_predicates<bfloat16>("block<bfloat16>");
    nrOfFailedTestCases += verify_predicates<cfloat<24, 5, std::uint16_t, true, false, false>>("block<cfloat<24,5>>");
    nrOfFailedTestCases += verify_predicates<cfloat<32, 8, std::uint32_t, true, false, false>>("block<cfloat<32,8>>");

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (const std::exception& err) {
    std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
