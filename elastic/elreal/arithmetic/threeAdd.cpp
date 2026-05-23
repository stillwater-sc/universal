// threeAdd.cpp: unit tests for the threeAdd helper.
//
// Verifies:
//   - Value preservation: o1 + o2 + o3 == a + b + w (exact, in long double).
//   - Gap preservation: zero_overlap(o1, o2) and zero_overlap(o2, o3).
//   - All outputs share the input exp_offset.
//   - Edge cases: zero inputs, opposite-sign cancellation, residual-producing
//     combinations, non-zero exp_offset.
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

template <typename FpType>
int verify_one(const sw::universal::block<FpType>& a,
               const sw::universal::block<FpType>& b,
               const sw::universal::block<FpType>& w,
               const std::string& tag) {
    using namespace sw::universal;
    int nrFailures = 0;

    auto res = threeAdd(a, b, w);

    // Value preservation
    long double ref = static_cast<long double>(a.v)
                    + static_cast<long double>(b.v)
                    + static_cast<long double>(w.v);
    long double got = static_cast<long double>(res.out1.v)
                    + static_cast<long double>(res.out2.v)
                    + static_cast<long double>(res.out3.v);
    long double diff = std::fabs(ref - got);
    constexpr int p = std::numeric_limits<FpType>::digits;
    long double ulp_sq = std::ldexp(1.0L, -2 * p);
    long double tol = ulp_sq * std::fmax(std::fabs(ref), 1.0L) * 256;
    if (diff > tol) {
        std::cout << tag << " value preservation: ref=" << ref
                  << " got=" << got << " diff=" << diff << " tol=" << tol
                  << " (a=" << static_cast<double>(a.v)
                  << " b=" << static_cast<double>(b.v)
                  << " w=" << static_cast<double>(w.v) << ")\n";
        ++nrFailures;
    }

    // exp_offset preserved
    std::int32_t off = a.exp_offset;
    if (res.out1.exp_offset != off
        || res.out2.exp_offset != off
        || res.out3.exp_offset != off) {
        std::cout << tag << " exp_offset not preserved\n";
        ++nrFailures;
    }

    // 0-overlap chain (skip zero blocks: they trivially overlap)
    if (!res.out1.is_zero_block() && !res.out2.is_zero_block()
        && !zero_overlap(res.out1, res.out2)) {
        std::cout << tag << " 0-overlap(out1, out2) FAILED\n"; ++nrFailures;
    }
    if (!res.out2.is_zero_block() && !res.out3.is_zero_block()
        && !zero_overlap(res.out2, res.out3)) {
        std::cout << tag << " 0-overlap(out2, out3) FAILED\n"; ++nrFailures;
    }
    return nrFailures;
}

template <typename FpType>
int verify_threeAdd(const std::string& tag) {
    using namespace sw::universal;
    using B = block<FpType>;
    int nrFailures = 0;

    // All zero -> all zero output
    {
        B z{ FpType{0}, 0 };
        nrFailures += verify_one(z, z, z, tag + " all-zero");
        auto res = threeAdd(z, z, z);
        if (!res.out1.is_zero_block() || !res.out2.is_zero_block() || !res.out3.is_zero_block()) {
            std::cout << tag << " all-zero produced non-zero outputs\n";
            ++nrFailures;
        }
    }
    // Two blocks cancel
    {
        B a{ FpType{1.5},  0 };
        B b{ FpType{-1.5}, 0 };
        B w{ FpType{1.25}, 0 };
        nrFailures += verify_one(a, b, w, tag + " a+(-a)+w");
    }
    // Residual-producing sum
    {
        constexpr int p = std::numeric_limits<FpType>::digits;
        B a{ FpType{1}, 0 };
        FpType tiny = static_cast<FpType>(std::ldexp(1.0, -p));
        if (tiny != FpType{0}) {
            B b{ tiny, 0 };
            B w{ FpType{0.5}, 0 };
            nrFailures += verify_one(a, b, w, tag + " 1+2^-p+0.5");
        }
    }
    // Non-zero exp_offset
    {
        B a{ FpType{1.5}, 5 };
        B b{ FpType{2.5}, 5 };
        B w{ FpType{0.5}, 5 };
        nrFailures += verify_one(a, b, w, tag + " offset=5");
    }
    // Random sweep
    std::mt19937_64 rng(0xc0ffeecafeULL);
    std::uniform_real_distribution<double> ud(-20.0, 20.0);
    int N = 100;
    for (int i = 0; i < N; ++i) {
        FpType av = static_cast<FpType>(ud(rng));
        FpType bv = static_cast<FpType>(ud(rng));
        FpType wv = static_cast<FpType>(ud(rng));
        if (av == FpType{0} || bv == FpType{0} || wv == FpType{0}) continue;
        B a{ av, 0 }, b{ bv, 0 }, w{ wv, 0 };
        nrFailures += verify_one(a, b, w, tag + " rand");
    }

    return nrFailures;
}

} // anonymous

int main()
try {
    using namespace sw::universal;
    std::string test_suite = "elreal threeAdd helper";
    int nrOfFailedTestCases = 0;
    bool reportTestCases = false;
    ReportTestSuiteHeader(test_suite, reportTestCases);

    nrOfFailedTestCases += verify_threeAdd<double>("threeAdd<double>");
    nrOfFailedTestCases += verify_threeAdd<float>("threeAdd<float>");
    nrOfFailedTestCases += verify_threeAdd<bfloat16>("threeAdd<bfloat16>");

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (const std::exception& err) {
    std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
