// lazy_lifetime.cpp: elreal's lazy producers must release their state (#1378).
//
// add(), infsum() and singleDiv() each hand back a ZBCL whose tail is a thunk over a
// shared state. If that thunk is built as a std::function that captures a shared_ptr
// to the control block owning it, the function object owns itself: dropping the
// returned ZBCL can never bring the count to zero, and every single call strands its
// state for the life of the process. All three used to do exactly that. Measured
// before the fix, RSS grew 0.29 kB per add(), 6.33 kB per singleDiv() and 41.28 kB
// per mul_online() -- 80,000 dropped mul_online streams took a process from 3.5 MB
// to 2.3 GB.
//
// The guard here counts live heap allocations rather than reading RSS, because RSS is
// not portable across the CI matrix (no /proc on macOS or Windows) and an allocator
// counter is exact rather than sampled. A leaking producer shows up as a live count
// that never returns to its pre-loop level.
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

// ---- global allocation counter -------------------------------------------------
// Deliberately simple: a signed live-block count, incremented in operator new and
// decremented in operator delete. It only ever has to answer "did the count come back
// down", so it needs no size tracking and no thread safety (the test is single
// threaded).
namespace {
long g_liveAllocations = 0;
}
void* operator new(std::size_t n) {
    void* p = std::malloc(n ? n : 1);
    if (!p) throw std::bad_alloc();
    ++g_liveAllocations;
    return p;
}
void operator delete(void* p) noexcept { if (p) { --g_liveAllocations; std::free(p); } }
void operator delete(void* p, std::size_t) noexcept { if (p) { --g_liveAllocations; std::free(p); } }
void* operator new[](std::size_t n) { return operator new(n); }
void operator delete[](void* p) noexcept { operator delete(p); }
void operator delete[](void* p, std::size_t) noexcept { operator delete(p); }

namespace {

using namespace sw::universal;

// Run `body` `iterations` times, each creating a lazy stream, pulling a few blocks and
// dropping it. Reports the growth in live allocations across the loop.
template <typename Body>
long leaked_over(Body body, int iterations) {
    body();                                   // warm: the first call may populate statics
    const long before = g_liveAllocations;
    for (int i = 0; i < iterations; ++i) body();
    return g_liveAllocations - before;
}

template <typename FpType>
int verify_lazy_producers_release(const char* host, bool reportTestCases) {
    int n = 0;
    const int iterations = 500;
    ZBCL<FpType> a = zbcl_from_blocks<FpType>(
        div_online(from_native<FpType>(1.0), from_native<FpType>(7.0)).take(8));
    ZBCL<FpType> b = zbcl_from_blocks<FpType>(
        div_online(from_native<FpType>(1.0), from_native<FpType>(11.0)).take(8));

    const long addLeak = leaked_over([&]{ (void)add(a, b).take(5).size(); }, iterations);
    const long mulLeak = leaked_over([&]{ (void)mul_online(a, b).take(5).size(); }, iterations);
    const long divLeak = leaked_over([&]{ (void)div_online(a, from_native<FpType>(3.0)).take(5).size(); }, iterations);

    const char* names[]  = { "add", "mul_online", "div_online" };
    const long  leaks[]  = { addLeak, mulLeak, divLeak };
    for (int i = 0; i < 3; ++i) {
        if (leaks[i] != 0) {
            std::cout << "  FAIL [" << host << "] " << names[i] << ": " << leaks[i]
                      << " allocations still live after " << iterations
                      << " streams were created and dropped ("
                      << (double(leaks[i]) / iterations)
                      << " per call) -- a lazy producer is not releasing its state (#1378)\n";
            ++n;
        }
        else if (reportTestCases) {
            std::cout << "  ok   [" << host << "] " << names[i] << ": all state released\n";
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
    std::string test_suite = "elreal lazy producers release their state (#1378)";
    int nrOfFailedTestCases = 0;
    bool reportTestCases = false;
    ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

    // TODO: place hand-run diagnostics here (this branch ignores failures)

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return EXIT_SUCCESS;

#else

    nrOfFailedTestCases += verify_lazy_producers_release<double>("double", reportTestCases);
    nrOfFailedTestCases += verify_lazy_producers_release<float>("float", reportTestCases);

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (const std::exception& err) {
    std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
