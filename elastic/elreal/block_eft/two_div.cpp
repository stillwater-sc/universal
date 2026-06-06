// two_div.cpp: block_two_div / block_two_div_rn tests.
//
// Verifies:
//   - q + r approximates a/b with error <= 2 ulps of q (the second-order EFT
//     correction recovers the leading residual but not all higher-order bits;
//     this is the same precision contract as Bailey/Hida QD's div).
//   - exp arithmetic: out.exp == a.exp - b.exp.
//   - 0-overlap (q, r) for typical inputs.
//   - Edge cases: division by 1, division yielding exact result, division
//     producing recurring decimal (1/3).
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

// See two_sum.cpp for why we pick double (instead of long double) as the
// reference type for Universal-wrapped FpTypes.
template <typename FpType>
using ref_t_for = std::conditional_t<
    sw::universal::has_universal_fp_api_v<FpType>
        || (std::numeric_limits<FpType>::digits < 53),
    double,
    long double>;

template <typename FpType>
int verify_two_div_one(const sw::universal::block<FpType>& a,
                       const sw::universal::block<FpType>& b,
                       const std::string& tag,
                       double tol_ulps_of_q = 2.0) {
    using namespace sw::universal;
    int nrFailures = 0;
    if (b.is_zero_block()) return 0; // skip division by zero in this generic check

    auto [q, r] = block_two_div(a, b);

    using R = ref_t_for<FpType>;
    R ref = static_cast<R>(a.v) / static_cast<R>(b.v);
    R got = static_cast<R>(q.v) + static_cast<R>(r.v);
    R err = std::fabs(ref - got);
    R tol = static_cast<R>(std::abs(static_cast<double>(q.v)))
          * static_cast<R>(std::ldexp(1.0,
                -std::numeric_limits<FpType>::digits + 1))
          * static_cast<R>(tol_ulps_of_q);
    if (err > tol) {
        std::cout << tag << " value error " << err
                  << " > tol " << tol
                  << " (a=" << static_cast<double>(a.v)
                  << " b=" << static_cast<double>(b.v)
                  << " q=" << static_cast<double>(q.v)
                  << " r=" << static_cast<double>(r.v) << ")\n";
        ++nrFailures;
    }

    std::int32_t expected_off = static_cast<std::int32_t>(a.exp - b.exp);
    if (q.exp != expected_off || r.exp != expected_off) {
        std::cout << tag << " exp mismatch: expected " << expected_off
                  << " got q=" << q.exp << " r=" << r.exp
                  << '\n';
        ++nrFailures;
    }

    // 0-overlap on the 2-block expansion. Zero r (e.g., exact divisions like
    // a/1 or 6/2) trivially 0-overlaps any q; non-zero pairs must satisfy
    // E(q) >= E(r) + k by construction of Bailey's two_div (the correction
    // term is bounded by ulp(q)/2).
    if (!q.is_zero_block() && !r.is_zero_block()
        && !zero_overlap(q, r)) {
        std::cout << tag << " 0-overlap FAILED: q.v=" << static_cast<double>(q.v)
                  << " r.v=" << static_cast<double>(r.v) << '\n';
        ++nrFailures;
    }
    return nrFailures;
}

template <typename FpType>
int verify_two_div(const std::string& tag) {
    using namespace sw::universal;
    using B = block<FpType>;
    int nrFailures = 0;

    // a / 1 = a exactly (r should be zero)
    {
        B a{ FpType{2.5}, 0 }, one{ FpType{1}, 0 };
        auto [q, r] = block_two_div(a, one);
        if (q.v != FpType{2.5}) {
            std::cout << tag << " a/1 wrong quotient\n"; ++nrFailures;
        }
        if (r.v != FpType{0}) {
            std::cout << tag << " a/1 non-zero residual\n"; ++nrFailures;
        }
    }
    // Exact division 6/2 = 3
    {
        B a{ FpType{6}, 0 }, b{ FpType{2}, 0 };
        nrFailures += verify_two_div_one(a, b, tag + " 6/2");
    }
    // Recurring: 1/3
    {
        B a{ FpType{1}, 0 }, b{ FpType{3}, 0 };
        nrFailures += verify_two_div_one(a, b, tag + " 1/3");
    }
    // exp subtraction
    {
        B a{ FpType{4}, 5 }, b{ FpType{2}, 2 };
        nrFailures += verify_two_div_one(a, b, tag + " offset");
        auto [q, r] = block_two_div(a, b);
        if (q.exp != 3) {
            std::cout << tag << " offset subtraction failed: " << q.exp
                      << " expected 3\n"; ++nrFailures;
        }
    }
    // RN variant
    {
        B a{ FpType{6}, 0 }, b{ FpType{2}, 0 };
        B rn = block_two_div_rn(a, b);
        if (rn.v != FpType{3}) {
            std::cout << tag << " RN 6/2 != 3\n"; ++nrFailures;
        }
    }

    // Randomised
    std::mt19937_64 rng(0xdeadbeefULL);
    std::uniform_real_distribution<double> ud(0.5, 50.0);
    int N = 200;
    for (int i = 0; i < N; ++i) {
        double av = ud(rng) * (rng() & 1 ? 1.0 : -1.0);
        double bv = ud(rng) * (rng() & 1 ? 1.0 : -1.0);
        FpType ah = static_cast<FpType>(av);
        FpType bh = static_cast<FpType>(bv);
        if (ah == FpType{0} || bh == FpType{0}) continue;
        B a{ ah, 0 }, b{ bh, 0 };
        nrFailures += verify_two_div_one(a, b, tag + " rand");
    }

    return nrFailures;
}

} // anonymous

int main()
try {
    using namespace sw::universal;
    std::string test_suite = "elreal block_two_div / block_two_div_rn";
    int nrOfFailedTestCases = 0;
    bool reportTestCases = false;
    ReportTestSuiteHeader(test_suite, reportTestCases);

    nrOfFailedTestCases += verify_two_div<float>("block<float>");
    nrOfFailedTestCases += verify_two_div<double>("block<double>");
    nrOfFailedTestCases += verify_two_div<half>("block<half>");
    nrOfFailedTestCases += verify_two_div<bfloat16>("block<bfloat16>");
    nrOfFailedTestCases += verify_two_div<cfloat<24, 5, std::uint16_t, true, false, false>>("block<cfloat<24,5>>");

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (const std::exception& err) {
    std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
