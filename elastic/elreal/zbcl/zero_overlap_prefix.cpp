// zero_overlap_prefix.cpp: verify that every adjacent pair of blocks in a
// constructed ZBCL satisfies the 0-overlap predicate.
//
// In Phase 2 the ZBCL builder is the test code itself (no arithmetic operations
// yet), so this test exists to lock in the invariant before Phase 3+ starts
// producing streams algorithmically. The debug-assertion check in
// `ZBCL::tail()` will catch violations at runtime; this test exercises the
// happy path and a few sentinel cases.
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
int verify_prefix_zero_overlap(const std::string& tag) {
    using namespace sw::universal;
    using B = block<FpType>;
    using S = ZBCL<FpType>;
    int nrFailures = 0;

    constexpr int k = B::k;

    // Build a five-block stream with each block at scale -i*k. By construction
    // every adjacent pair satisfies zero_overlap: E(b_n) - E(b_{n+1}) = k.
    B b0{ FpType{1},                                    0 };
    B b1{ static_cast<FpType>(std::ldexp(1.0, -1 * k)), 0 };
    B b2{ static_cast<FpType>(std::ldexp(1.0, -2 * k)), 0 };
    B b3{ static_cast<FpType>(std::ldexp(1.0, -3 * k)), 0 };

    // Skip blocks where ldexp underflows in FpType (happens for narrow
    // bfloat16/half at depth where -3*k pushes past representable normals).
    auto is_usable = [](const B& b) { return !b.is_zero_block(); };
    if (!is_usable(b3)) {
        std::cout << tag << " skipping depth-3 (FpType underflow)\n";
    }

    S tail = is_usable(b3) ? S::singleton(b3) : S{};
    if (is_usable(b2)) tail = S::cons(b2, tail);
    if (is_usable(b1)) tail = S::cons(b1, tail);
    S stream = S::cons(b0, tail);

    // Walk the stream and confirm zero_overlap on every adjacent pair.
    auto blocks = stream.take(10);
    if (blocks.size() < 2u) {
        std::cout << tag << " stream collapsed to <2 blocks\n"; ++nrFailures;
        return nrFailures;
    }
    for (std::size_t i = 0; i + 1 < blocks.size(); ++i) {
        if (!zero_overlap(blocks[i], blocks[i + 1])) {
            std::cout << tag << " 0-overlap FAILED between block " << i
                      << " and block " << (i + 1) << '\n';
            ++nrFailures;
        }
    }

    // Empty co-list: trivially 0-overlap-preserving (no adjacent pairs to check).
    S e;
    if (!e.take(5).empty()) { std::cout << tag << " empty leaked blocks\n"; ++nrFailures; }

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
    std::string test_suite = "elreal ZBCL<FpType> 0-overlap on prefixes";
    int nrOfFailedTestCases = 0;
    bool reportTestCases = false;
    ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

    // TODO: place hand-run diagnostics here (this branch ignores failures)

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return EXIT_SUCCESS;

#else

#if REGRESSION_LEVEL_1

    nrOfFailedTestCases += verify_prefix_zero_overlap<float>("ZBCL<float>");
    nrOfFailedTestCases += verify_prefix_zero_overlap<double>("ZBCL<double>");
    nrOfFailedTestCases += verify_prefix_zero_overlap<half>("ZBCL<half>");
    nrOfFailedTestCases += verify_prefix_zero_overlap<bfloat16>("ZBCL<bfloat16>");
    nrOfFailedTestCases += verify_prefix_zero_overlap<cfloat<24, 5, std::uint16_t, true, false, false>>("ZBCL<cfloat<24,5>>");

#endif

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (const std::exception& err) {
    std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
