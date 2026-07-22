// lazy.cpp: hardening of the elreal lazy state-machine API (#1079 Phase 2).
//
// Verifies, at the class-facade level, that elreal is genuinely lazy and that
// refining precision REUSES already-pulled work (the memoisation edge over the
// eager elastic types): a counter-instrumented stream wrapped in an elreal must
// see each tail thunk fire at most once across limbs()/approx()/refine() calls.
// Also covers prefix stability, convergence, refine/precision semantics, the
// stream() round-trip, and zero/empty edge cases.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <atomic>
#include <cmath>
#include <iostream>
#include <memory>
#include <string>

#include <universal/number/elreal/elreal.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

using namespace sw::universal;
using FpType = double;
using B = block<FpType>;
using S = ZBCL<FpType>;

// (1) Memoisation / reuse: each tail thunk must fire AT MOST ONCE across the
// elreal facade's pull operations -- refining never recomputes earlier limbs.
int verify_memoisation() {
    int n = 0;
    constexpr int k = B::k;
    B b0{ FpType{1},                                    0 };
    B b1{ static_cast<FpType>(std::ldexp(1.0, -k)),     0 };
    B b2{ static_cast<FpType>(std::ldexp(1.0, -2 * k)), 0 };

    auto tail1 = std::make_shared<std::atomic<int>>(0);
    auto tail2 = std::make_shared<std::atomic<int>>(0);
    S inner = S::singleton(b2);
    S middle = S::cons(b1, [tail2, inner]() -> S { tail2->fetch_add(1); return inner; });
    S s      = S::cons(b0, [tail1, middle]() -> S { tail1->fetch_add(1); return middle; });

    elreal<FpType> e(s, 8);   // wrap the instrumented stream

    auto chk = [&](const char* where, int t1, int t2) {
        if (tail1->load() != t1) { std::cout << where << ": tail1=" << tail1->load() << " expected " << t1 << '\n'; ++n; }
        if (tail2->load() != t2) { std::cout << where << ": tail2=" << tail2->load() << " expected " << t2 << '\n'; ++n; }
    };

    chk("construction", 0, 0);                  // wrapping forces nothing
    (void) e.precision();                        // metadata access forces nothing
    chk("precision()", 0, 0);
    if (e.limbs(1).size() != 1u) ++n;  chk("limbs(1)", 0, 0);     // head only
    if (e.limbs(2).size() != 2u) ++n;  chk("limbs(2)", 1, 0);     // tail1 once
    if (e.limbs(2).size() != 2u) ++n;  chk("limbs(2) again", 1, 0); // REUSE: no re-fire
    (void) e.approx<double>(2);        chk("approx(2)", 1, 0);    // REUSE across methods
    if (e.limbs(3).size() != 3u) ++n;  chk("limbs(3)", 1, 1);     // refine pulls only tail2
    e.refine(5);                       chk("refine(5) (no pull)", 1, 1);  // refine alone forces nothing
    (void) e.approx<double>(3);        chk("approx(3)", 1, 1);    // already pulled
    return n;
}

// (2) Prefix stability: limbs(d) is a prefix of limbs(d') for d < d'.
int verify_prefix_stability() {
    int n = 0;
    elreal<FpType> q = elreal<FpType>(1.0) / elreal<FpType>(7.0);   // 1/7, non-terminating
    auto a = q.limbs(4);
    auto b = q.limbs(8);
    if (b.size() < a.size()) { std::cout << "limbs(8) shorter than limbs(4)\n"; ++n; }
    for (std::size_t i = 0; i < a.size() && i < b.size(); ++i) {
        if (a[i].exponent() != b[i].exponent() || a[i].v != b[i].v) {
            std::cout << "prefix mismatch at limb " << i << '\n'; ++n; break;
        }
    }
    return n;
}

// (3) Convergence: approx() tends to the true value as depth rises.
int verify_convergence() {
    int n = 0;
    elreal<FpType> q = elreal<FpType>(1.0) / elreal<FpType>(7.0);
    if (std::fabs(q.approx<double>(8) - 1.0 / 7.0) > 1e-15) { std::cout << "1/7 approx<double> wrong\n"; ++n; }
    // approx<T> sums in T -- a wider host keeps more precision than double would
    if (std::fabs(static_cast<double>(q.approx<long double>(8)) - 1.0 / 7.0) > 1e-15) {
        std::cout << "1/7 approx<long double> wrong\n"; ++n;
    }
    // shallower never beats deeper in component count for a non-terminating value
    if (q.limbs(2).size() > q.limbs(6).size()) { std::cout << "limb count not monotone\n"; ++n; }
    return n;
}

// (4) refine() raises precision monotonically; precision() sets exactly.
int verify_precision_control() {
    int n = 0;
    elreal<FpType> a = 1.0;
    a.precision(4);                 if (a.precision() != 4) ++n;
    a.refine(2);                    if (a.precision() != 4) ++n;   // refine never lowers
    a.refine(9);                    if (a.precision() != 9) ++n;   // refine raises
    a.precision(3);                 if (a.precision() != 3) ++n;   // precision() sets exactly
    return n;
}

// (5) stream() round-trip: reconstructing from the raw stream preserves the value.
int verify_stream_roundtrip() {
    int n = 0;
    elreal<FpType> a = elreal<FpType>(22.0) / elreal<FpType>(7.0);
    elreal<FpType> b(a.stream(), a.precision());
    if (!(a == b)) { std::cout << "stream round-trip != original\n"; ++n; }
    if (std::fabs(double(a) - double(b)) > 1e-15) { std::cout << "stream round-trip value drift\n"; ++n; }
    return n;
}

// (6) zero / empty edge cases.
int verify_edge_cases() {
    int n = 0;
    elreal<FpType> z;                                  // default = 0
    if (!z.iszero()) ++n;
    if (z.limbs(5).size() != 0u) ++n;                  // empty stream -> no limbs
    if (z.limbs(0).size() != 0u) ++n;                  // limbs(0) -> empty
    if (z.approx<double>(8) != 0.0) ++n;
    if (double(z) != 0.0) ++n;
    elreal<FpType> a = 3.0;
    if (a.limbs(0).size() != 0u) ++n;                  // limbs(0) on nonzero -> empty
    return n;
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
    std::string test_suite = "elreal lazy state-machine API hardening (#1079 Phase 2)";
    int nrOfFailedTestCases = 0;
    bool reportTestCases = false;
    ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

    // TODO: place hand-run diagnostics here (this branch ignores failures)

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return EXIT_SUCCESS;

#else

#if REGRESSION_LEVEL_1

    nrOfFailedTestCases += verify_memoisation();
    nrOfFailedTestCases += verify_prefix_stability();
    nrOfFailedTestCases += verify_convergence();
    nrOfFailedTestCases += verify_precision_control();
    nrOfFailedTestCases += verify_stream_roundtrip();
    nrOfFailedTestCases += verify_edge_cases();

#endif

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (const std::exception& err) {
    std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
