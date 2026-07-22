// cons_take.cpp: cons/head/tail/take/is_empty contract tests for ZBCL<FpType>.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <vector>

#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/bfloat16/bfloat16.hpp>
#include <universal/number/elreal/elreal.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

template <typename FpType>
int verify_cons_take(const std::string& tag) {
    using namespace sw::universal;
    using B = block<FpType>;
    using S = ZBCL<FpType>;
    int nrFailures = 0;

    // Empty
    S e = empty<FpType>();
    if (!e.is_empty()) { std::cout << tag << " empty() not is_empty\n"; ++nrFailures; }
    if (static_cast<bool>(e)) { std::cout << tag << " empty() truthy\n"; ++nrFailures; }
    if (!e.take(3).empty()) { std::cout << tag << " empty().take(3) non-empty\n"; ++nrFailures; }

    // Singleton
    B b0{ FpType{1}, 0 };
    S s1 = S::singleton(b0);
    if (s1.is_empty()) { std::cout << tag << " singleton is_empty\n"; ++nrFailures; }
    if (s1.head().v != b0.v) { std::cout << tag << " singleton head wrong\n"; ++nrFailures; }
    if (!s1.tail().is_empty()) {
        std::cout << tag << " singleton.tail() not empty\n"; ++nrFailures;
    }

    // take(n) on a single-element stream returns at most one block
    auto taken = s1.take(5);
    if (taken.size() != 1u) {
        std::cout << tag << " singleton take(5) size=" << taken.size() << "\n";
        ++nrFailures;
    }

    // Two-block finite stream: build a stream containing b0 (scale 0) and
    // b1 (scale -k) where k = B::k. zero_overlap(b0, b1) holds at the boundary
    // by construction (E(b0) = 0, E(b1) = -k, gap = k).
    constexpr int k = B::k;
    FpType b1_val = static_cast<FpType>(std::ldexp(1.0, -k));
    B b1{ b1_val, 0 };
    S s2 = S::cons(b0, S::singleton(b1));
    if (s2.head().v != b0.v) { std::cout << tag << " cons head wrong\n"; ++nrFailures; }
    if (s2.tail().is_empty()) {
        std::cout << tag << " cons.tail() empty when shouldn't be\n"; ++nrFailures;
    }
    if (s2.tail().head().v != b1.v) {
        std::cout << tag << " cons.tail().head() wrong\n"; ++nrFailures;
    }
    if (!s2.tail().tail().is_empty()) {
        std::cout << tag << " cons.tail().tail() not empty\n"; ++nrFailures;
    }

    // take(2) on the two-block stream returns exactly 2 blocks in order.
    auto taken2 = s2.take(2);
    if (taken2.size() != 2u
        || taken2[0].v != b0.v
        || taken2[1].v != b1.v) {
        std::cout << tag << " take(2) wrong size or order\n"; ++nrFailures;
    }

    // take(10) on the two-block stream still returns 2 (no over-read).
    auto taken10 = s2.take(10);
    if (taken10.size() != 2u) {
        std::cout << tag << " take(10) on 2-block stream size="
                  << taken10.size() << "\n";
        ++nrFailures;
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
    std::string test_suite = "elreal ZBCL<FpType> cons/take";
    int nrOfFailedTestCases = 0;
    bool reportTestCases = false;
    ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

    // TODO: place hand-run diagnostics here (this branch ignores failures)

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return EXIT_SUCCESS;

#else

#if REGRESSION_LEVEL_1

    nrOfFailedTestCases += verify_cons_take<float>("ZBCL<float>");
    nrOfFailedTestCases += verify_cons_take<double>("ZBCL<double>");
    nrOfFailedTestCases += verify_cons_take<half>("ZBCL<half>");
    nrOfFailedTestCases += verify_cons_take<bfloat16>("ZBCL<bfloat16>");
    nrOfFailedTestCases += verify_cons_take<cfloat<24, 5, std::uint16_t, true, false, false>>("ZBCL<cfloat<24,5>>");

#endif

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (const std::exception& err) {
    std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
