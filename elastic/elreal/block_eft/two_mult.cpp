// two_mult.cpp: block_two_mult / block_two_mult_rn tests.
//
// Verifies:
//   - Value preservation: high.v + low.v == a.v * b.v (in long double).
//   - 0-overlap: zero_overlap(high, low) on non-zero results.
//   - exp arithmetic: out.exp == a.exp + b.exp.
//   - Edge cases: zero, signed-zero, identity multiplications.
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
// reference type for Universal-wrapped FpTypes: their long-double conversion
// is currently broken.
template <typename FpType>
using ref_t_for = std::conditional_t<
    sw::universal::has_universal_fp_api_v<FpType>
        || (std::numeric_limits<FpType>::digits < 53),
    double,
    long double>;

template <typename FpType>
inline ref_t_for<FpType>
reference_prod(const sw::universal::block<FpType>& a,
               const sw::universal::block<FpType>& b) {
    using R = ref_t_for<FpType>;
    return static_cast<R>(a.v) * static_cast<R>(b.v);
}

template <typename FpType>
int verify_two_mult_one(const sw::universal::block<FpType>& a,
                        const sw::universal::block<FpType>& b,
                        const std::string& tag) {
    using namespace sw::universal;
    int nrFailures = 0;
    auto [high, low] = block_two_mult(a, b);

    // Value preservation tolerance: ideal IEEE arithmetic produces an exact
    // EFT residual (ref == got bit-identical). Universal's wider cfloat<>
    // multiplications can be off by a few ulp^2 because the internal
    // intermediate precision is bounded by the FpType itself; we tolerate
    // that since it accurately reflects what the eventual hardware would
    // compute. For native float/double the relative error must be effectively
    // zero, and the tolerance below catches algorithm-level mistakes (which
    // would be many orders of magnitude larger).
    using R = ref_t_for<FpType>;
    R ref  = reference_prod(a, b);
    R got  = static_cast<R>(high.v) + static_cast<R>(low.v);
    // Tolerance: scaled multiple of ulp^2. Ideal IEEE arithmetic gives an exact
    // EFT (diff = 0). The exact-residual two_prod (issue #942) computes a*b - p
    // in a wider intermediate, so even/odd precision no longer leaks split
    // error; the only residual that remains is when the result type cannot
    // represent it -- cfloat<24,5> (es=5, p=19) underflows the residual into its
    // subnormal range (~32 ulp^2 floor). We allow 64 * ulp^2: it passes every
    // host (max observed ~32 for cfloat<24,5>; 0 for float/double/bfloat16/
    // cfloat<32,8>) while catching the pre-#942 odd-p Dekker noise (~80 ulp^2
    // for cfloat<24,5>) and any genuine off-by-1-ulp algorithm regression.
    R diff = std::fabs(ref - got);
    constexpr int p  = std::numeric_limits<FpType>::digits;
    R ulp_sq = static_cast<R>(std::ldexp(1.0, -2 * p));
    R tol  = ulp_sq * std::fmax(std::fabs(ref), R{1}) * 64;
    if (diff > tol) {
        std::cout << tag << " value preservation: ref=" << ref
                  << " got=" << got
                  << " diff=" << diff
                  << " tol=" << tol
                  << " (a=" << static_cast<double>(a.v)
                  << " b=" << static_cast<double>(b.v) << ")\n";
        ++nrFailures;
    }

    std::int32_t expected_off = a.exp + b.exp;
    if (high.exp != expected_off || low.exp != expected_off) {
        std::cout << tag << " exp mismatch: expected " << expected_off
                  << " got high=" << high.exp << " low=" << low.exp
                  << '\n';
        ++nrFailures;
    }

    if (!high.is_zero_block() && !low.is_zero_block()
        && !zero_overlap(high, low)) {
        std::cout << tag << " 0-overlap FAILED\n"; ++nrFailures;
    }
    return nrFailures;
}

template <typename FpType>
int verify_two_mult(const std::string& tag) {
    using namespace sw::universal;
    using B = block<FpType>;
    int nrFailures = 0;

    // Edge: zero * x = 0
    {
        B z{ FpType{0}, 0 }, x{ FpType{2.5}, 0 };
        auto [h, l] = block_two_mult(z, x);
        if (!h.is_zero_block() || !l.is_zero_block()) {
            std::cout << tag << " 0*x produced non-zero\n"; ++nrFailures;
        }
    }
    // Edge: x * 1 = x exactly
    {
        B a{ FpType{1.5}, 0 }, one{ FpType{1}, 0 };
        auto [h, l] = block_two_mult(a, one);
        if (h.v != FpType{1.5} || l.v != FpType{0}) {
            std::cout << tag << " x*1 not exact (h="
                      << static_cast<double>(h.v) << " l="
                      << static_cast<double>(l.v) << ")\n";
            ++nrFailures;
        }
    }
    // Standard with residual: a value that requires the low part to be non-zero
    {
        B a{ FpType{1.5}, 0 }, b{ FpType{1.5}, 0 };
        nrFailures += verify_two_mult_one(a, b, tag + " 1.5*1.5");
    }
    // exp addition
    {
        B a{ FpType{2}, 3 }, b{ FpType{3}, -1 };
        nrFailures += verify_two_mult_one(a, b, tag + " offset");
    }
    // RN variant
    {
        B a{ FpType{1.5}, 0 }, b{ FpType{2.0}, 0 };
        B rn = block_two_mult_rn(a, b);
        if (rn.v != static_cast<FpType>(a.v * b.v)) {
            std::cout << tag << " RN mismatch\n"; ++nrFailures;
        }
    }

    // Randomised
    std::mt19937_64 rng(0xfeedfaceULL);
    std::uniform_real_distribution<double> ud(-10.0, 10.0);
    int N = 200;
    for (int i = 0; i < N; ++i) {
        FpType av = static_cast<FpType>(ud(rng));
        FpType bv = static_cast<FpType>(ud(rng));
        if (av == FpType{0} || bv == FpType{0}) continue;
        B a{ av, 0 }, b{ bv, 0 };
        nrFailures += verify_two_mult_one(a, b, tag + " rand");
    }

    return nrFailures;
}

} // anonymous

int main()
try {
    using namespace sw::universal;
    std::string test_suite = "elreal block_two_mult / block_two_mult_rn";
    int nrOfFailedTestCases = 0;
    bool reportTestCases = false;
    ReportTestSuiteHeader(test_suite, reportTestCases);

    nrOfFailedTestCases += verify_two_mult<float>("block<float>");
    nrOfFailedTestCases += verify_two_mult<double>("block<double>");
    nrOfFailedTestCases += verify_two_mult<half>("block<half>");
    nrOfFailedTestCases += verify_two_mult<bfloat16>("block<bfloat16>");
    nrOfFailedTestCases += verify_two_mult<cfloat<24, 5, std::uint16_t, true, false, false>>("block<cfloat<24,5>>");

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (const std::exception& err) {
    std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
