// threeAdd.cpp: tests for the dissertation's Definition 4.2.1 threeAdd.
//
// Validates the four properties enumerated in the dissertation:
//   1. Value preservation: JaK + JbK + JcK = Jo1K + Jo2K + Jo3K
//   2. dominate o1 o2 AND dominate o2 o3
//   3. (getExp a) - k <= getExp o1 <= (getExp a) + 2  (where a is the
//      largest-exponent input after sorting)
//   4. No-shrink: if a's exp >= b's exp + k + 3 AND a's exp >= c's exp + k + 3,
//      then getExp o1 == getExp a.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <random>

#include <universal/number/elreal/elreal.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

template <typename FpType>
int verify_one(const sw::universal::block<FpType>& ia,
               const sw::universal::block<FpType>& ib,
               const sw::universal::block<FpType>& ic,
               const std::string& tag) {
    using namespace sw::universal;
    int nrFailures = 0;
    auto r = threeAdd(ia, ib, ic);

    // Property 1: value preservation in long double.
    auto block_value = [](const block<FpType>& b) -> long double {
        if (b.is_zero_block()) return 0.0L;
        return static_cast<long double>(b.v) * std::ldexp(1.0L, b.exp);
    };
    long double ref = block_value(ia) + block_value(ib) + block_value(ic);
    long double got = block_value(r.out1) + block_value(r.out2) + block_value(r.out3);
    long double diff = std::fabs(ref - got);
    // Tolerance: long-double ulp of |ref|. The dissertation's value-preservation
    // claim is bit-exact at FpType precision, but our reference is computed in
    // long double (64-bit mantissa on x86_64), so we can only detect deviation
    // up to long-double precision -- a few ld-ulps at the magnitude of ref.
    constexpr int ld_digits = std::numeric_limits<long double>::digits;
    long double scale = std::fmax(std::fabs(ref), 1.0L);
    long double tol = std::ldexp(1.0L, -ld_digits + 4) * scale; // 16 ld-ulps
    if (diff > tol) {
        std::cout << tag << " value-preservation FAILED: ref=" << ref
                  << " got=" << got << " diff=" << diff << " tol=" << tol << '\n';
        ++nrFailures;
    }

    // Property 2: dominate o1 o2 AND dominate o2 o3 (zero blocks pass trivially).
    if (!dominate(r.out1, r.out2)) {
        std::cout << tag << " dominate(out1, out2) FAILED\n"; ++nrFailures;
    }
    if (!dominate(r.out2, r.out3)) {
        std::cout << tag << " dominate(out2, out3) FAILED\n"; ++nrFailures;
    }

    // Property 3: (getExp a_sorted) - k <= getExp out1 <= (getExp a_sorted) + 2
    block<FpType> arr[3] = { ia, ib, ic };
    std::sort(arr, arr + 3, [](const block<FpType>& x, const block<FpType>& y) {
        return x.exponent() > y.exponent();
    });
    const auto& a_sorted = arr[0];
    if (!a_sorted.is_zero_block() && !r.out1.is_zero_block()) {
        constexpr int k = block<FpType>::k;
        std::int32_t a_exp = a_sorted.exponent();
        std::int32_t o1_exp = r.out1.exponent();
        if (o1_exp > a_exp + 2 || o1_exp < a_exp - k) {
            std::cout << tag << " exp-bound FAILED: o1_exp=" << o1_exp
                      << " a_exp=" << a_exp << " k=" << k << '\n';
            ++nrFailures;
        }
    }

    // Property 4: no-shrink.
    if (!a_sorted.is_zero_block()) {
        constexpr int k = block<FpType>::k;
        bool no_shrink_premise = true;
        for (int i = 1; i < 3; ++i) {
            if (!arr[i].is_zero_block()
                && a_sorted.exponent() < arr[i].exponent() + k + 3) {
                no_shrink_premise = false;
                break;
            }
        }
        if (no_shrink_premise && !r.out1.is_zero_block()) {
            if (r.out1.exponent() != a_sorted.exponent()) {
                std::cout << tag << " no-shrink FAILED: o1_exp=" << r.out1.exponent()
                          << " a_exp=" << a_sorted.exponent() << '\n';
                ++nrFailures;
            }
        }
    }

    return nrFailures;
}

template <typename FpType>
int verify_threeAdd(const std::string& tag) {
    using namespace sw::universal;
    using B = block<FpType>;
    int nrFailures = 0;

    // All zero
    {
        B z{ FpType{0}, 0 };
        nrFailures += verify_one(z, z, z, tag + " all-zero");
    }
    // a + (-a) + small
    {
        B a{ FpType{1.5}, 0 };
        B b{ FpType{-1.5}, 0 };
        B c{ FpType{1.0}, 0 };
        nrFailures += verify_one(a, b, c, tag + " a+(-a)+c");
    }
    // Spread exponents, triggering the no-shrink case
    {
        constexpr int k = B::k;
        B a{ FpType{1.0}, 100 };
        B b{ FpType{1.5}, 100 - k - 5 };
        B c{ FpType{1.25}, 100 - 2 * k - 10 };
        nrFailures += verify_one(a, b, c, tag + " spread");
    }
    // Random sweep
    std::mt19937_64 rng(0xC0FFEEULL);
    std::uniform_real_distribution<double> ud(-10.0, 10.0);
    std::uniform_int_distribution<int> exp_d(-30, 30);
    for (int i = 0; i < 200; ++i) {
        FpType va = static_cast<FpType>(ud(rng));
        FpType vb = static_cast<FpType>(ud(rng));
        FpType vc = static_cast<FpType>(ud(rng));
        std::int32_t ea = exp_d(rng);
        std::int32_t eb = exp_d(rng);
        std::int32_t ec = exp_d(rng);
        B a{ va, ea }, b{ vb, eb }, c{ vc, ec };
        nrFailures += verify_one(a, b, c, tag + " rand");
    }
    return nrFailures;
}

} // anonymous

int main()
try {
    using namespace sw::universal;
    std::string test_suite = "elreal threeAdd (dissertation Def 4.2.1)";
    int nrOfFailedTestCases = 0;
    bool reportTestCases = false;
    ReportTestSuiteHeader(test_suite, reportTestCases);

    nrOfFailedTestCases += verify_threeAdd<double>("threeAdd<double>");
    nrOfFailedTestCases += verify_threeAdd<float>("threeAdd<float>");

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (const std::exception& err) {
    std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
