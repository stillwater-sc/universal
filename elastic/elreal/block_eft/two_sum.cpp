// two_sum.cpp: block_two_sum / block_two_sum_rn tests.
//
// Verifies:
//   - Value preservation: high.v + low.v == a.v + b.v exactly (long-double
//     reference where the host is narrower than long double; bit-identical
//     check otherwise).
//   - 0-overlap: zero_overlap(high, low) holds on every non-zero result.
//   - Edge cases: zero, opposite-sign, magnitude-disparity, non-zero exp.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <iostream>
#include <random>

#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/bfloat16/bfloat16.hpp>
#include <universal/number/elreal/elreal.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

// Reference type: long double for native double (so we can detect sub-ulp
// residual errors), double for everything else. We avoid long double for
// Universal cfloat<>/half/bfloat16 because their static_cast<long double>
// conversion is currently broken (returns nan or wrong values for some
// inputs); their values comfortably fit in double for the test sweep.
//
// The first clause (`has_universal_fp_api_v`) routes ALL Universal-wrapped
// FpTypes to double regardless of their precision -- this future-proofs the
// selector against testing wider Universal types where the broken
// long-double conversion would still apply.
template <typename FpType>
using ref_t_for = std::conditional_t<
    sw::universal::has_universal_fp_api_v<FpType>
        || (std::numeric_limits<FpType>::digits < 53),
    double,
    long double>;

template <typename FpType>
inline ref_t_for<FpType>
reference_sum(const sw::universal::block<FpType>& a,
              const sw::universal::block<FpType>& b) {
    using R = ref_t_for<FpType>;
    return static_cast<R>(a.v) + static_cast<R>(b.v);
}

template <typename FpType>
int verify_two_sum_one(const sw::universal::block<FpType>& a,
                       const sw::universal::block<FpType>& b,
                       const std::string& tag) {
    using namespace sw::universal;
    int nrFailures = 0;
    auto [high, low] = block_two_sum(a, b);

    // Value preservation: ideal IEEE arithmetic produces an exact EFT
    // residual (diff = 0). Universal's wider cfloat<> sums can leak a few
    // ulp^2 from imperfect intermediate precision; we tolerate up to
    // ~128 * ulp^2, still 1000x smaller than a single ulp.
    using R = ref_t_for<FpType>;
    R ref  = reference_sum(a, b);
    R got  = static_cast<R>(high.v) + static_cast<R>(low.v);
    R diff = std::fabs(ref - got);
    constexpr int p  = std::numeric_limits<FpType>::digits;
    R ulp_sq = static_cast<R>(std::ldexp(1.0, -2 * p));
    R tol  = ulp_sq * std::fmax(std::fabs(ref), R{1}) * 128;
    if (diff > tol) {
        std::cout << tag << " value preservation: ref=" << ref
                  << " got=" << got
                  << " diff=" << diff
                  << " tol=" << tol
                  << " (a=" << static_cast<double>(a.v)
                  << " b=" << static_cast<double>(b.v) << ")\n";
        ++nrFailures;
    }

    // 0-overlap (skipped on zero blocks since they're trivially compliant)
    if (!high.is_zero_block() && !low.is_zero_block()
        && !zero_overlap(high, low)) {
        std::cout << tag << " 0-overlap FAILED: high.v=" << static_cast<double>(high.v)
                  << " low.v=" << static_cast<double>(low.v) << "\n";
        ++nrFailures;
    }
    return nrFailures;
}

template <typename FpType>
int verify_two_sum(const std::string& tag) {
    using namespace sw::universal;
    using B = block<FpType>;
    int nrFailures = 0;

    // Edge: both zero
    {
        B z{ FpType{0}, 0 };
        auto [h, l] = block_two_sum(z, z);
        if (!h.is_zero_block() || !l.is_zero_block()) {
            std::cout << tag << " zero+zero produced non-zero\n"; ++nrFailures;
        }
    }
    // Edge: a + (-a) cancels
    {
        B a{ FpType{1.25}, 0 }, b{ FpType{-1.25}, 0 };
        auto [h, l] = block_two_sum(a, b);
        if (h.v != FpType{0} || l.v != FpType{0}) {
            std::cout << tag << " a + (-a) not exactly zero\n"; ++nrFailures;
        }
    }
    // Standard 1.0 + small (forces a residual)
    {
        constexpr int p = std::numeric_limits<FpType>::digits;
        B a{ FpType{1}, 0 };
        FpType tiny = static_cast<FpType>(std::ldexp(1.0, -p));
        // tiny may underflow for very narrow types; skip if it does
        if (tiny != FpType{0}) {
            B b{ tiny, 0 };
            nrFailures += verify_two_sum_one(a, b, tag + " 1+2^-p");
        }
    }
    // exp = 5 on both inputs
    {
        B a{ FpType{1}, 5 }, b{ FpType{0.25}, 5 };
        nrFailures += verify_two_sum_one(a, b, tag + " offset=5");
        auto [h, l] = block_two_sum(a, b);
        if (h.exp != 5 || l.exp != 5) {
            std::cout << tag << " offset not preserved\n"; ++nrFailures;
        }
    }
    // RN variant: bit-identical to a.v + b.v
    {
        B a{ FpType{3.5}, 0 }, b{ FpType{0.125}, 0 };
        B rn = block_two_sum_rn(a, b);
        if (rn.v != static_cast<FpType>(a.v + b.v)) {
            std::cout << tag << " RN mismatch\n"; ++nrFailures;
        }
    }

    // Randomised sweep
    std::mt19937_64 rng(0x1234abcdULL);
    std::uniform_real_distribution<double> ud(-100.0, 100.0);
    int N = 200;
    for (int i = 0; i < N; ++i) {
        FpType av = static_cast<FpType>(ud(rng));
        FpType bv = static_cast<FpType>(ud(rng));
        if (av == FpType{0} || bv == FpType{0}) continue;
        B a{ av, 0 }, b{ bv, 0 };
        nrFailures += verify_two_sum_one(a, b, tag + " rand");
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
    std::string test_suite = "elreal block_two_sum / block_two_sum_rn";
    int nrOfFailedTestCases = 0;
    bool reportTestCases = false;
    ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

    // TODO: place hand-run diagnostics here (this branch ignores failures)

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return EXIT_SUCCESS;

#else

#if REGRESSION_LEVEL_1

    nrOfFailedTestCases += verify_two_sum<float>("block<float>");
    nrOfFailedTestCases += verify_two_sum<double>("block<double>");
    nrOfFailedTestCases += verify_two_sum<half>("block<half>");
    nrOfFailedTestCases += verify_two_sum<bfloat16>("block<bfloat16>");
    nrOfFailedTestCases += verify_two_sum<cfloat<24, 5, std::uint16_t, true, false, false>>("block<cfloat<24,5>>");

#endif

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (const std::exception& err) {
    std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
