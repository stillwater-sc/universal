// zero_overlap.cpp: the McCleeary 0-overlap predicate, E(b1) >= E(b2) + k.
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
int verify_zero_overlap(const std::string& tag) {
    using namespace sw::universal;
    using B = block<FpType>;
    int nrFailures = 0;

    constexpr int k = B::k;

    // Boundary: b1 at 2^0, b2 at 2^(-k) -- exactly satisfies E(b1) >= E(b2) + k.
    B b1{ FpType{1}, 0 };
    // For b2, build a value at scale -k. ldexp(1, -k) may underflow narrow types,
    // so we synthesise via the block constructor with appropriate exp for
    // very small k targets. For our supported types k is at least 7 (bfloat16),
    // so ldexp(1, -k) is representable in all of them.
    FpType b2_val = static_cast<FpType>(std::ldexp(1.0, -k));
    B b2{ b2_val, 0 };
    if (!zero_overlap(b1, b2)) {
        std::cout << tag << " 0-overlap boundary E(b1)=0, E(b2)=-k FAILED\n"; ++nrFailures;
    }

    // Just inside the gap: b2 at 2^(-k+1) violates the predicate.
    FpType b2_inside_val = static_cast<FpType>(std::ldexp(1.0, -k + 1));
    B b2_inside{ b2_inside_val, 0 };
    if (zero_overlap(b1, b2_inside)) {
        std::cout << tag << " 0-overlap mis-allows E(b2)=-k+1\n"; ++nrFailures;
    }

    // Wide gap: b2 at 2^(-2k) is well outside k.
    FpType b2_wide_val = static_cast<FpType>(std::ldexp(1.0, -2 * k));
    B b2_wide{ b2_wide_val, 0 };
    if (!zero_overlap(b1, b2_wide)) {
        std::cout << tag << " 0-overlap rejects E(b2)=-2k\n"; ++nrFailures;
    }

    // Zero block on either side: trivially 0-overlaps.
    B zero{ FpType{0}, 0 };
    if (!zero_overlap(b1, zero))   { std::cout << tag << " zero on rhs not trivial\n"; ++nrFailures; }
    if (!zero_overlap(zero, b1))   { std::cout << tag << " zero on lhs not trivial\n"; ++nrFailures; }
    if (!zero_overlap(zero, zero)) { std::cout << tag << " zero/zero not trivial\n"; ++nrFailures; }

    // Cross-exp gap: b1 with exp=+k, b2 with exp=0. With v=1 in both (scale=0),
    // E(b1) = k, E(b2) = 0, so the gap = k satisfies the >= k bound exactly.
    B b1_off{ FpType{1}, k };
    B b2_off{ FpType{1}, 0 };
    if (!zero_overlap(b1_off, b2_off)) {
        std::cout << tag << " 0-overlap rejects exp=+k vs exp=0 (gap exactly k)\n";
        ++nrFailures;
    }

    // Reverse direction must FAIL the predicate (b1 has smaller exponent).
    if (zero_overlap(b2_off, b1_off)) {
        std::cout << tag << " 0-overlap mis-allows reversed pair\n"; ++nrFailures;
    }

    return nrFailures;
}

} // anonymous

int main()
try {
    using namespace sw::universal;
    std::string test_suite = "elreal block<FpType> zero_overlap";
    int nrOfFailedTestCases = 0;
    bool reportTestCases = false;
    ReportTestSuiteHeader(test_suite, reportTestCases);

    nrOfFailedTestCases += verify_zero_overlap<float>("block<float>");
    nrOfFailedTestCases += verify_zero_overlap<double>("block<double>");
    nrOfFailedTestCases += verify_zero_overlap<half>("block<half>");
    nrOfFailedTestCases += verify_zero_overlap<bfloat16>("block<bfloat16>");
    nrOfFailedTestCases += verify_zero_overlap<cfloat<24, 5, std::uint16_t, true, false, false>>("block<cfloat<24,5>>");
    nrOfFailedTestCases += verify_zero_overlap<cfloat<32, 8, std::uint32_t, true, false, false>>("block<cfloat<32,8>>");

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (const std::exception& err) {
    std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
