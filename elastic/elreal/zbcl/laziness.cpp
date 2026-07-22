// laziness.cpp: verify that ZBCL thunks are NOT invoked until the corresponding
// position in the stream is forced via tail() or take(n).
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <atomic>
#include <iostream>

#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/elreal/elreal.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

// Build a 3-block stream where each tail thunk increments a counter so we can
// observe exactly how many thunks have been forced.
template <typename FpType>
int verify_laziness(const std::string& tag) {
    using namespace sw::universal;
    using B = block<FpType>;
    using S = ZBCL<FpType>;
    int nrFailures = 0;

    constexpr int k = B::k;
    B b0{ FpType{1},                                    0 };
    B b1{ static_cast<FpType>(std::ldexp(1.0, -k)),     0 };
    B b2{ static_cast<FpType>(std::ldexp(1.0, -2 * k)), 0 };

    // Shared mutable counters captured by the thunks. Using shared_ptr keeps
    // the value alive across copies of the stream.
    auto tail1_calls = std::make_shared<std::atomic<int>>(0);
    auto tail2_calls = std::make_shared<std::atomic<int>>(0);

    // Build inside-out: tail-most singleton first, then layer thunks above.
    S s_inner = S::singleton(b2);
    auto tail_of_b1_thunk = [tail2_calls, s_inner]() -> S {
        tail2_calls->fetch_add(1);
        return s_inner;
    };
    S s_middle = S::cons(b1, std::move(tail_of_b1_thunk));

    auto tail_of_b0_thunk = [tail1_calls, s_middle]() -> S {
        tail1_calls->fetch_add(1);
        return s_middle;
    };
    S s = S::cons(b0, std::move(tail_of_b0_thunk));

    // Step 0: just constructing the stream forces nothing.
    if (tail1_calls->load() != 0) {
        std::cout << tag << " tail1 forced at construction\n"; ++nrFailures;
    }
    if (tail2_calls->load() != 0) {
        std::cout << tag << " tail2 forced at construction\n"; ++nrFailures;
    }

    // Step 1: accessing head() forces nothing past head.
    (void) s.head();
    if (tail1_calls->load() != 0) {
        std::cout << tag << " tail1 forced by head()\n"; ++nrFailures;
    }

    // Step 2: take(1) forces nothing past block 0.
    auto t1 = s.take(1);
    if (t1.size() != 1u) { std::cout << tag << " take(1) size wrong\n"; ++nrFailures; }
    if (tail1_calls->load() != 0) {
        std::cout << tag << " tail1 forced by take(1)\n"; ++nrFailures;
    }

    // Step 3: take(2) forces tail1 exactly once.
    auto t2 = s.take(2);
    if (t2.size() != 2u) { std::cout << tag << " take(2) size wrong\n"; ++nrFailures; }
    if (tail1_calls->load() != 1) {
        std::cout << tag << " take(2) forced tail1 " << tail1_calls->load()
                  << " times (expected 1)\n"; ++nrFailures;
    }
    if (tail2_calls->load() != 0) {
        std::cout << tag << " take(2) forced tail2\n"; ++nrFailures;
    }

    // Step 4: take(3) forces tail2 (memoised tail1 not re-invoked).
    auto t3 = s.take(3);
    if (t3.size() != 3u) { std::cout << tag << " take(3) size wrong\n"; ++nrFailures; }
    if (tail1_calls->load() != 1) {
        std::cout << tag << " tail1 re-invoked (no memoisation): "
                  << tail1_calls->load() << "\n"; ++nrFailures;
    }
    if (tail2_calls->load() != 1) {
        std::cout << tag << " take(3) forced tail2 " << tail2_calls->load()
                  << " times (expected 1)\n"; ++nrFailures;
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
    std::string test_suite = "elreal ZBCL<FpType> laziness";
    int nrOfFailedTestCases = 0;
    bool reportTestCases = false;
    ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

    // TODO: place hand-run diagnostics here (this branch ignores failures)

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return EXIT_SUCCESS;

#else

#if REGRESSION_LEVEL_1

    nrOfFailedTestCases += verify_laziness<double>("ZBCL<double>");
    nrOfFailedTestCases += verify_laziness<float>("ZBCL<float>");
    nrOfFailedTestCases += verify_laziness<half>("ZBCL<half>");

#endif

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (const std::exception& err) {
    std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
