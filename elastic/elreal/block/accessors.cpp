// accessors.cpp: sign(), scale_of_v(), exponent(), value_as<>() tests for
// elreal block<FpType>.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <cstdint>
#include <iostream>

#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/bfloat16/bfloat16.hpp>
#include <universal/number/elreal/elreal.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

template <typename FpType>
int verify_accessors(const std::string& tag) {
    using namespace sw::universal;
    using B = block<FpType>;
    int nrFailures = 0;

    // sign on positive, negative, +0, -0
    B pos{ FpType{1.5}, 0 };
    B neg{ FpType{-1.5}, 0 };
    if (pos.sign() != +1) { std::cout << tag << " pos.sign() != +1\n"; ++nrFailures; }
    if (neg.sign() != -1) { std::cout << tag << " neg.sign() != -1\n"; ++nrFailures; }

    // scale_of_v: powers of two yield integer scales
    B p2_0{ FpType{1}, 0 };       // 2^0
    B p2_1{ FpType{2}, 0 };       // 2^1
    B p2_n1{ FpType{0.5}, 0 };    // 2^-1
    if (p2_0.scale_of_v() != 0)   { std::cout << tag << " scale(1.0) != 0\n"; ++nrFailures; }
    if (p2_1.scale_of_v() != 1)   { std::cout << tag << " scale(2.0) != 1\n"; ++nrFailures; }
    if (p2_n1.scale_of_v() != -1) { std::cout << tag << " scale(0.5) != -1\n"; ++nrFailures; }

    // exponent() = scale_of_v + exp (per-unit, McCleeary's getExp)
    B off2{ FpType{1.5}, 2 };
    std::int32_t expected = static_cast<std::int32_t>(off2.scale_of_v()) + 2;
    if (off2.exponent() != expected) {
        std::cout << tag << " exponent() with exp=2 wrong: got " << off2.exponent()
                  << " expected " << expected << '\n';
        ++nrFailures;
    }

    // value_as<double> with offset=0: the recovered value must bit-match the
    // host FpType's stored value (no rounding -- value_as is just a cast).
    B v{ FpType{3.25}, 0 };
    double recovered = v.template value_as<double>();
    double host_stored = static_cast<double>(FpType{3.25});
    if (recovered != host_stored) {
        std::cout << tag << " value_as<double>(off=0) mismatch: "
                  << recovered << " vs host_stored " << host_stored << '\n';
        ++nrFailures;
    }

    return nrFailures;
}

} // anonymous

int main()
try {
    using namespace sw::universal;
    std::string test_suite = "elreal block<FpType> accessors";
    int nrOfFailedTestCases = 0;
    bool reportTestCases = false;
    ReportTestSuiteHeader(test_suite, reportTestCases);

    nrOfFailedTestCases += verify_accessors<float>("block<float>");
    nrOfFailedTestCases += verify_accessors<double>("block<double>");
    nrOfFailedTestCases += verify_accessors<half>("block<half>");
    nrOfFailedTestCases += verify_accessors<bfloat16>("block<bfloat16>");
    nrOfFailedTestCases += verify_accessors<cfloat<24, 5, std::uint16_t, true, false, false>>("block<cfloat<24,5>>");
    nrOfFailedTestCases += verify_accessors<cfloat<32, 8, std::uint32_t, true, false, false>>("block<cfloat<32,8>>");

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (const std::exception& err) {
    std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
