// from_native.cpp: tests for empty<FpType>(), from_native, to_double_approx.
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
int verify_from_native(const std::string& tag) {
    using namespace sw::universal;
    using S = ZBCL<FpType>;
    int nrFailures = 0;

    // empty() yields the canonical 0 stream.
    S e = empty<FpType>();
    if (!e.is_empty()) {
        std::cout << tag << " empty() not is_empty\n"; ++nrFailures;
    }
    if (to_double_approx(e, 4) != 0.0) {
        std::cout << tag << " to_double_approx(empty) != 0\n"; ++nrFailures;
    }

    // from_native(0.0) yields an empty stream.
    S z = from_native<FpType>(0.0);
    if (!z.is_empty()) {
        std::cout << tag << " from_native(0.0) not empty\n"; ++nrFailures;
    }

    // from_native(1.0) round-trips exactly through the host FpType.
    S one = from_native<FpType>(1.0);
    if (one.is_empty()) { std::cout << tag << " from_native(1.0) empty\n"; ++nrFailures; }
    double recovered = to_double_approx(one, 4);
    double host_one  = static_cast<double>(FpType{1});
    if (recovered != host_one) {
        std::cout << tag << " from_native(1.0) round-trip: got " << recovered
                  << " expected " << host_one << '\n';
        ++nrFailures;
    }

    // from_native(-2.5) round-trips through the host FpType.
    S neg = from_native<FpType>(-2.5);
    double neg_recovered = to_double_approx(neg, 4);
    double neg_expected  = static_cast<double>(FpType{-2.5});
    if (neg_recovered != neg_expected) {
        std::cout << tag << " from_native(-2.5) round-trip: got " << neg_recovered
                  << " expected " << neg_expected << '\n';
        ++nrFailures;
    }

    return nrFailures;
}

} // anonymous

int main()
try {
    using namespace sw::universal;
    std::string test_suite = "elreal ZBCL<FpType> from_native";
    int nrOfFailedTestCases = 0;
    bool reportTestCases = false;
    ReportTestSuiteHeader(test_suite, reportTestCases);

    nrOfFailedTestCases += verify_from_native<float>("ZBCL<float>");
    nrOfFailedTestCases += verify_from_native<double>("ZBCL<double>");
    nrOfFailedTestCases += verify_from_native<half>("ZBCL<half>");
    nrOfFailedTestCases += verify_from_native<bfloat16>("ZBCL<bfloat16>");
    nrOfFailedTestCases += verify_from_native<cfloat<24, 5, std::uint16_t, true, false, false>>("ZBCL<cfloat<24,5>>");

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (const std::exception& err) {
    std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
