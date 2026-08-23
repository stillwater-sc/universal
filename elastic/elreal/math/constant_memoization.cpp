// constant_memoization.cpp: the transcendental constants are cached, and cached in a
// way that does not make results depend on call history (#1383).
//
// pi and ln2 were recomputed from scratch on every call, and that -- not the series
// each wraps -- was essentially the whole cost of sin/cos/tan and log. Measured at
// depth 8 on a double host before the fix: pi_zbcl alone 371.9 ms against sin(0.5)
// 371.8 ms in total, while exp and sqrt, needing no constant, cost 9.6 and 4.0 ms.
// After: sin 5.7 ms in steady state.
//
// Two properties are guarded here, and the second is the one that constrains the
// design. Caching the DEEPEST value and truncating it for shallower requests would be
// cheaper, and it is wrong: these constants are not prefix-stable -- pi at depth 2
// differs from take(2) of pi at depth 8 after ~32 digits, because the deeper
// evaluation refines the last block rather than merely extending it. Under that
// scheme pi_zbcl(2) would return one value or another depending on whether some
// unrelated earlier call had asked for more.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <new>
#include <string>

#include <universal/number/elreal/elreal.hpp>
#include <universal/number/elreal/math/constants.hpp>
#include <universal/verification/elreal_reference_digits.hpp>
#include <universal/verification/test_suite.hpp>

// ---- cumulative allocation counter ---------------------------------------------
// A cached call should do essentially no work. Counting allocations says so
// deterministically, where a wall-clock comparison would be flaky on a shared runner.
namespace {
long g_allocations = 0;
}
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmismatched-new-delete"
#endif
void* operator new(std::size_t n) {
    void* p = std::malloc(n ? n : 1);
    if (!p) throw std::bad_alloc();
    ++g_allocations;
    return p;
}
void operator delete(void* p) noexcept { std::free(p); }
void operator delete(void* p, std::size_t) noexcept { std::free(p); }
void* operator new[](std::size_t n) { return operator new(n); }
void operator delete[](void* p) noexcept { operator delete(p); }
void operator delete[](void* p, std::size_t) noexcept { operator delete(p); }
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

namespace {

using namespace sw::universal;

// (1) a repeat call must be served from the cache, i.e. cost essentially nothing
template <typename FpType>
int verify_constant_is_cached(const char* host, bool reportTestCases) {
    int n = 0;
    const std::size_t D = 6;

    const long a0 = g_allocations;
    (void)pi_zbcl<FpType>(D).take(D).size();          // first: computes
    const long firstCost = g_allocations - a0;

    const long a1 = g_allocations;
    (void)pi_zbcl<FpType>(D).take(D).size();          // second: should be a lookup
    const long repeatCost = g_allocations - a1;

    // The first evaluation runs two odd_power_series; a cache hit is a vector index
    // and a shared_ptr copy. Demanding a 100x gap keeps this immune to how the
    // expansion happens to be shaped while still failing loudly if the memo is gone.
    if (repeatCost * 100 > firstCost) {
        std::cout << "  FAIL [" << host << "] pi_zbcl repeat cost " << repeatCost
                  << " allocations against " << firstCost
                  << " for the first call -- the constant is not being cached (#1383)\n";
        ++n;
    }
    else if (reportTestCases) {
        std::cout << "  ok   [" << host << "] pi_zbcl cached: " << firstCost
                  << " allocations to compute, " << repeatCost << " to re-read\n";
    }
    return n;
}

// (2) the cached value must be the one that depth would have produced on its own --
//     NOT a truncation of some deeper value cached earlier. This is what stops the
//     memo from making results depend on call history.
template <typename FpType>
int verify_shallow_is_unaffected_by_deeper(const char* host, bool reportTestCases) {
    int n = 0;
    struct Case { const char* name; ZBCL<FpType> memoized; ZBCL<FpType> computed; };

    // ask for a deep value FIRST, so a truncating cache would be primed to answer the
    // shallow request from it
    (void)pi_zbcl<FpType>(16).take(16).size();
    (void)ln2_zbcl<FpType>(16).take(16).size();
    (void)e_zbcl<FpType>(16).take(16).size();

    const Case cases[] = {
        { "pi",  pi_zbcl<FpType>(2),  pi_zbcl_compute<FpType>(2)  },
        { "ln2", ln2_zbcl<FpType>(2), ln2_zbcl_compute<FpType>(2) },
        { "e",   e_zbcl<FpType>(2),   e_zbcl_compute<FpType>(2)   },
    };
    for (const auto& c : cases) {
        if (agreed_decimal_digits(c.memoized, c.computed, 4000) < 4000) {
            std::cout << "  FAIL [" << host << "] " << c.name
                      << " at depth 2 differs from a direct depth-2 evaluation after a deeper"
                      << " call -- is the cache truncating a deeper value? (#1383)\n";
            ++n;
        }
        else if (reportTestCases) {
            std::cout << "  ok   [" << host << "] " << c.name << " at depth 2 is unaffected by the deeper call\n";
        }
    }
    return n;
}

} // anonymous

#define MANUAL_TESTING 0
#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
    using namespace sw::universal;
    std::string test_suite = "elreal transcendental constants are memoized (#1383)";
    int nrOfFailedTestCases = 0;
    bool reportTestCases = false;
    ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

    // TODO: place hand-run diagnostics here (this branch ignores failures)

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return EXIT_SUCCESS;

#else

    nrOfFailedTestCases += verify_constant_is_cached<double>("double", reportTestCases);
    nrOfFailedTestCases += verify_shallow_is_unaffected_by_deeper<double>("double", reportTestCases);

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (const std::exception& err) {
    std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
