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
#include <universal/verification/test_suite.hpp>

// ---- global allocation counter (cumulative; never decremented) -------------------
namespace {
long g_totalAllocations = 0;
}
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

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (const std::exception& err) {
    std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
