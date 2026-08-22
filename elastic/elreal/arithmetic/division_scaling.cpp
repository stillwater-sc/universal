// division_scaling.cpp: division by a single block must stay LINEAR in depth (#1061).
//
// singleDiv once followed FCL.hs literally -- divide EACH dividend block independently
// to a full stream, then infSum the D of them. Correct, but O(D^2): term i has to be
// carried down to the output frontier, so D quotient blocks cost D^2/2 block divisions.
// The schoolbook carry (one running remainder) makes it O(1) per emitted block.
//
// WHY THIS MEASURES ALLOCATIONS AND NOT TIME. The first version of this guard compared
// wall-clock across a 4x depth span. It passed locally at x4.2 against a threshold of
// 8, and then failed on two CI runners at x11.0 and x12.4 -- and passed on rerun. A
// shared runner can stretch either measurement, and a ratio of two noisy numbers is
// noisier than each, so no threshold both catches the regression and survives
// contention. Allocation counts are deterministic: identical on every run and every
// platform, because they are a property of the algorithm rather than of the machine.
//
// The assertion is dimensionless -- allocations PER EMITTED BLOCK must not grow with
// depth. Linear work is flat; quadratic work grows with D. Measured:
//
//     implementation        allocs/block @D=40   @D=160   growth
//     per-block-then-sum          3060            23559    x7.7
//     schoolbook carry              48               49    x1.02
//
// so a threshold of 3 clears the current form by ~3x and catches the old one by ~2.5x,
// with no dependence on machine speed at all.
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
#include <universal/verification/elreal_reference_digits.hpp>
#include <universal/verification/test_suite.hpp>

// ---- global allocation counter (cumulative; never decremented) -------------------
namespace {
long g_totalAllocations = 0;
}
// GCC cannot see that these replacements are paired (each malloc has its free) and
// warns about a mismatched allocation function. They are paired; the warning is a
// false positive for a global operator replacement.
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmismatched-new-delete"
#endif
void* operator new(std::size_t n) {
    void* p = std::malloc(n ? n : 1);
    if (!p) throw std::bad_alloc();
    ++g_totalAllocations;
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

// allocations consumed producing D quotient blocks of (1/7 at D blocks) / 5
template <typename FpType>
double allocations_per_block(std::size_t D) {
    ZBCL<FpType> big = zbcl_from_blocks<FpType>(
        div_online(from_native<FpType>(1.0), from_native<FpType>(7.0)).take(D));
    const long before = g_totalAllocations;
    const std::size_t produced =
        zbcl_from_blocks<FpType>(div_online(big, from_native<FpType>(5.0)).take(D)).take(D).size();
    const long used = g_totalAllocations - before;
    if (produced == 0) return 0.0;
    return double(used) / double(produced);
}

template <typename FpType>
int verify_division_scaling(const char* host, bool reportTestCases) {
    const double shallow = allocations_per_block<FpType>(40);
    const double deep    = allocations_per_block<FpType>(160);
    if (shallow <= 0.0) {
        std::cout << "  FAIL [" << host << "] the shallow division produced no blocks\n";
        return 1;
    }
    const double growth = deep / shallow;
    constexpr double kMaxGrowth = 3.0;
    if (growth > kMaxGrowth) {
        std::cout << "  FAIL [" << host << "] per-block cost grew x" << growth
                  << " over a 4x depth span (" << shallow << " -> " << deep
                  << " allocations per block; want <= " << kMaxGrowth
                  << ") -- division by a single block is no longer linear\n";
        return 1;
    }
    if (reportTestCases) {
        std::cout << "  ok   [" << host << "] per-block cost flat: " << shallow << " -> " << deep
                  << " allocations per block (x" << growth << ")\n";
    }
    return 0;
}

// The FaithfulLongDivision policy must actually reach the long division. Value alone
// cannot show this: both policies produce bit-identical quotients, so a comparison of
// results would pass even if the policy were ignored. Allocation count distinguishes
// them -- the long division does materially more work (measured ~2.4x at depth 32) --
// and is deterministic, unlike timing.
template <typename FpType>
int verify_dense_policy_dispatches(const char* host, bool reportTestCases) {
    const std::size_t D = 24;
    ZBCL<FpType> b = sqrt(from_native<FpType>(2.0), D);
    ZBCL<FpType> a = from_native<FpType>(2.0);

    const long n0 = g_totalAllocations;
    ZBCL<FpType> qn = zbcl_from_blocks<FpType>(div_online(a, b, D).take(D));
    const long nNewton = g_totalAllocations - n0;

    const long l0 = g_totalAllocations;
    ZBCL<FpType> ql = zbcl_from_blocks<FpType>(
        div_online(a, b, D, DenseDivision::FaithfulLongDivision).take(D));
    const long nLong = g_totalAllocations - l0;

    int n = 0;
    // same answer -- that is the point of offering the alternative at all
    if (agreed_decimal_digits(qn, ql, 4000) < 4000) {
        std::cout << "  FAIL [" << host << "] the two dense-division policies disagree\n";
        ++n;
    }
    // ... reached by a DIFFERENT amount of work, which is how we know the policy was
    // honoured. Deliberately not asserting a direction: which path is cheaper depends
    // on the divisor's width, and long division is actually ahead once the divisor is
    // about as wide as the depth (0.92x at 24 blocks). Asserting "long division costs
    // more" would encode a fact that is only true for narrow divisors.
    if (nLong == nNewton) {
        std::cout << "  FAIL [" << host << "] both policies used " << nLong
                  << " allocations -- the policy argument is not reaching the dense path\n";
        ++n;
    }
    else if (reportTestCases) {
        std::cout << "  ok   [" << host << "] policies agree in value, differ in work ("
                  << (double(nLong) / double(nNewton)) << "x allocations)\n";
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
    std::string test_suite = "elreal single-block division scales linearly (#1061)";
    int nrOfFailedTestCases = 0;
    bool reportTestCases = false;
    ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

    // TODO: place hand-run diagnostics here (this branch ignores failures)

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return EXIT_SUCCESS;

#else

    nrOfFailedTestCases += verify_division_scaling<double>("double", reportTestCases);
    nrOfFailedTestCases += verify_division_scaling<float>("float", reportTestCases);
    nrOfFailedTestCases += verify_dense_policy_dispatches<double>("double", reportTestCases);
    nrOfFailedTestCases += verify_dense_policy_dispatches<float>("float", reportTestCases);

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (const std::exception& err) {
    std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
