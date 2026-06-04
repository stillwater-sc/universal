// exact_value_oracle.cpp: independent EXACT oracle for the elreal building blocks (#1022)
//
// The Phase 1-4 elreal tests check value preservation against `long double`.
// That is not an exact oracle: on x86_64 it carries only ~11 mantissa bits over
// double (enough to catch gross errors, not to certify bit-exactness), and on
// MSVC / most ARM / RISC-V `long double` aliases `double`, giving ZERO headroom
// over the type under test. This file removes that dependency for the building
// blocks, exactly as the ereal/Priest effort did (#987, dyadic_exact.hpp).
//
// The reference is exact dyadic-rational arithmetic (dyadic_exact.hpp, backed by
// einteger). It shares NO code with the block / EFT / threeAdd / add algorithms,
// so it catches a result that is structurally well-formed (0-overlap, right
// exponents) but encodes the WRONG value -- the failure mode `long double`
// cannot see.
//
// Exact value of a McCleeary block (sign,exp,bv packed into FpType `v` plus an
// int32 `exp`) is  value(b) = v * 2^exp, where exact_real(v) is the exact dyadic
// of v at ANY significand width: for <= 53-bit hosts double is already exact; for
// wider hosts (quad and up) it extracts the full integer significand directly,
// never capping at double -- verified here at 113 bits (cfloat<128,15>). See
// exact_real() for the two regimes and why the split is necessary (a narrow-
// exponent wide host cannot represent its own scaled-up significand). The exact
// value of a ZBCL prefix is the (exact) sum of its block dyadics.
//
// What is asserted:
//   * block_two_sum: exact(high) + exact(low) == exact(a) + exact(b) EXACTLY,
//     on every round-to-nearest host INCLUDING quad precision -- UNLESS the
//     residual `low` is a subnormal the FpType cannot represent (the cfloat<24,5>
//     #942 floor). That exception is encoded structurally (a nonzero dyadic error
//     is tolerated ONLY when the least-significant result block is a nonzero
//     subnormal); a wrong high part, or a wrong NORMAL residual, still fails.
//   * block_two_mult: same, but NOT exercised at quad precision. two_prod_host
//     computes the residual of an odd-precision product in a `double`
//     intermediate (#942); that holds for p <= 26 but cannot represent the
//     residual of a 113-bit product -- a genuine limitation of the multiply EFT,
//     not of this oracle (filed separately). Exercised on the p <= 24 hosts.
//   * threeAdd and the lazy add() combinator are twoSum-based (no two_prod), so
//     they are exact on the RN hosts including quad -- asserted as exact dyadic
//     equality. Narrow-exponent RN hosts (half, cfloat<24,5>) are validated at
//     the EFT level above; their multi-block strict bound needs a rounding
//     oracle (future).
//
// bfloat16 is NOT an exact-EFT host. It rounds toward zero
// (numeric_limits<bfloat16>::round_style == round_toward_zero; the float->bf16
// cast is `bits >> 16`, which discards the low bits with no rounding). Knuth /
// Dekker error-free transforms are exact only under round-to-nearest, so on
// bfloat16 they leak up to ~1 residual-ulp -- an inherent property of the host,
// like the cfloat<24,5> subnormal floor (#942) but caused by the rounding mode
// rather than the exponent range. We therefore pin bfloat16 with a truncation
// BOUND (sweep_eft_truncating) instead of exactness, and use double as its exact
// reference (double has ~45 bits of headroom over bfloat16's 8-bit significand).
// This is the latent inexactness the Phase 1-4 long-double / 128-ulp^2
// tolerances were masking; see epic #923.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cassert>
#include <cmath>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/bfloat16/bfloat16.hpp>
#include <universal/number/elreal/elreal.hpp>
#include <universal/verification/elreal_oracle.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

using namespace sw::universal;
// exact_real / exact_block / exact_blocks / exact_value + ZBCL_EXACT_WINDOW: the
// shared exact-dyadic oracle this file pioneered (#1022), now consolidated (#1035).
using namespace sw::universal::elreal_oracle;

// The exact-dyadic helpers (exact_real / exact_block / exact_blocks / exact_value)
// and ZBCL_EXACT_WINDOW that this oracle pioneered (#1022) now live in the shared
// header <universal/verification/elreal_oracle.hpp> (#1035) and are brought into
// scope by the using-directive above. The EFT/threeAdd/add reference checks below
// build on them but share no code with the algorithms under test.

// value of a block in double, and its ulp -- used only for the bfloat16
// truncation bound (double has ~45 bits of headroom over bfloat16's 8).
template <typename FpType>
double dval(const block<FpType>& b) {
    if (b.is_zero_block()) return 0.0;
    return std::ldexp(static_cast<double>(b.v), b.exp);
}

template <typename FpType>
double block_ulp(const block<FpType>& b) {
    const int E = b.is_zero_block() ? b.exp : b.exponent();
    return std::ldexp(1.0, E - (block<FpType>::k - 1));
}

// A nonzero dyadic error is acceptable ONLY when the least-significant result
// block is a nonzero subnormal -- i.e. the format genuinely cannot represent the
// residual (the #942 floor). err == 0 is always acceptable. A wrong high part
// (least block zero, err != 0) or a wrong normal residual (least block normal,
// err != 0) is NOT acceptable.
template <typename FpType>
bool acceptable(const dyadic& err, const block<FpType>& least) {
    if (err.iszero()) return true;
    return !least.is_zero_block() && !least.is_normalised();
}

// ---- block_two_sum: exact(high)+exact(low) == exact(a)+exact(b) -------------
template <typename FpType>
int check_two_sum(const block<FpType>& a, const block<FpType>& b,
                  const std::string& tag) {
    auto [high, low] = block_two_sum(a, b);          // precondition: a.exp == b.exp
    dyadic err = (exact_block(a) + exact_block(b)) - (exact_block(high) + exact_block(low));
    if (!acceptable(err, low)) {
        std::cout << tag << " two_sum value WRONG: a=" << double(a.v)
                  << " b=" << double(b.v)
                  << " high=" << double(high.v) << " low=" << double(low.v)
                  << " (low normalised=" << low.is_normalised() << ")\n";
        return 1;
    }
    return 0;
}

// ---- block_two_mult: exact(high)+exact(low) == exact(a)*exact(b) ------------
template <typename FpType>
int check_two_mult(const block<FpType>& a, const block<FpType>& b,
                   const std::string& tag) {
    auto [high, low] = block_two_mult(a, b);
    dyadic err = (exact_block(a) * exact_block(b)) - (exact_block(high) + exact_block(low));
    if (!acceptable(err, low)) {
        std::cout << tag << " two_mult value WRONG: a=" << double(a.v)
                  << " b=" << double(b.v)
                  << " high=" << double(high.v) << " low=" << double(low.v)
                  << " (low normalised=" << low.is_normalised() << ")\n";
        return 1;
    }
    return 0;
}

// ---- threeAdd: exact(out1+out2+out3) == exact(ia+ib+ic) ---------------------
template <typename FpType>
int check_threeAdd_exact(const block<FpType>& ia, const block<FpType>& ib,
                         const block<FpType>& ic, const std::string& tag) {
    auto r = threeAdd(ia, ib, ic);
    dyadic ref = exact_block(ia) + exact_block(ib) + exact_block(ic);
    dyadic got = exact_block(r.out1) + exact_block(r.out2) + exact_block(r.out3);
    if (ref != got) {
        std::cout << tag << " threeAdd value WRONG: ia=" << double(ia.v)
                  << " ib=" << double(ib.v) << " ic=" << double(ic.v) << "\n";
        return 1;
    }
    return 0;
}

// ---- add() : exact(take(N)) == exact(za) + exact(zb) ------------------------
template <typename FpType>
int check_add_exact(double a, double b, const std::string& tag) {
    auto za = from_native<FpType>(a);
    auto zb = from_native<FpType>(b);
    auto z  = add(za, zb);
    if ((exact_value(za) + exact_value(zb)) != exact_value(z)) {
        std::cout << tag << " add value WRONG: a=" << a << " b=" << b << "\n";
        return 1;
    }
    return 0;
}

// two_sum sweep: every round-to-nearest host, INCLUDING quad precision. The
// residual block is directly inspectable, so the #942 subnormal exception is
// exact.
template <typename FpType>
int sweep_two_sum(const std::string& tag) {
    using B = block<FpType>;
    int fail = 0;
    constexpr int p = std::numeric_limits<FpType>::digits;
    {
        FpType tiny = static_cast<FpType>(std::ldexp(1.0, -p));
        if (tiny != FpType{0})
            fail += check_two_sum<FpType>(B{FpType{1}, 0}, B{tiny, 0}, tag + " 1+2^-p");
    }
    fail += check_two_sum<FpType>(B{FpType{1.25}, 0}, B{FpType{-1.25}, 0}, tag + " 1.25-1.25");
    fail += check_two_sum<FpType>(B{static_cast<FpType>(1e4), 0},
                                  B{static_cast<FpType>(1.0/1024), 0}, tag + " 1e4+2^-10");
    fail += check_two_sum<FpType>(B{FpType{1}, 7}, B{FpType{0.25}, 7}, tag + " offset=7");

    std::mt19937_64 rng(0x1022ABCDULL);
    std::uniform_real_distribution<double> ud(-100.0, 100.0);
    for (int i = 0; i < 300; ++i) {
        FpType av = static_cast<FpType>(ud(rng));
        FpType bv = static_cast<FpType>(ud(rng));
        if (av == FpType{0} || bv == FpType{0}) continue;
        fail += check_two_sum<FpType>(B{av, 0}, B{bv, 0}, tag + " rand-sum");
    }
    return fail;
}

// two_mult sweep: exact on every RN host. two_prod_host picks the right residual
// path per width -- even p: Dekker; odd p with 2p <= 53: double intermediate
// (half, cfloat<24,5>); odd p with 2p > 53: fused fma (quad, #1024). double uses
// its dedicated specialisation. (Pre-#1024 the odd-p wide path used a double
// intermediate and could not hold a 113-bit product residual; now fixed.)
template <typename FpType>
int sweep_two_mult(const std::string& tag) {
    using B = block<FpType>;
    int fail = 0;
    fail += check_two_mult<FpType>(B{FpType{1.5}, 0}, B{FpType{1.5}, 0}, tag + " 1.5*1.5");
    fail += check_two_mult<FpType>(B{FpType{3}, 0}, B{FpType{0.5}, 0}, tag + " 3*0.5 exact");
    fail += check_two_mult<FpType>(B{static_cast<FpType>(123.456), 2},
                                   B{static_cast<FpType>(-7.875), -1}, tag + " mixed-exp");
    std::mt19937_64 rng(0x4242ULL);
    std::uniform_real_distribution<double> ud(-100.0, 100.0);
    for (int i = 0; i < 300; ++i) {
        FpType av = static_cast<FpType>(ud(rng));
        FpType bv = static_cast<FpType>(ud(rng));
        if (av == FpType{0} || bv == FpType{0}) continue;
        fail += check_two_mult<FpType>(B{av, 0}, B{bv, 0}, tag + " rand-mult");
    }
    return fail;
}

// threeAdd + add(): strict exact equality. twoSum-based, so exact on every RN
// host including quad precision.
template <typename FpType>
int sweep_combinators_exact(const std::string& tag) {
    using B = block<FpType>;
    int fail = 0;
    fail += check_threeAdd_exact<FpType>(B{FpType{1}, 0}, B{FpType{0.25}, 0}, B{FpType{0.0625}, 0}, tag + " 3add nice");
    fail += check_threeAdd_exact<FpType>(B{static_cast<FpType>(1e6), 0}, B{FpType{1}, 0}, B{static_cast<FpType>(1.0/4096), 0}, tag + " 3add spread");
    fail += check_threeAdd_exact<FpType>(B{FpType{5}, 0}, B{FpType{-5}, 0}, B{FpType{2.5}, 0}, tag + " 3add cancel");

    fail += check_add_exact<FpType>(1.0, 0.25, tag + " add 1+0.25");
    fail += check_add_exact<FpType>(1e3, 1.0, tag + " add 1e3+1");
    fail += check_add_exact<FpType>(0.1, 0.2, tag + " add 0.1+0.2");
    fail += check_add_exact<FpType>(-2.5, -7.5, tag + " add -2.5-7.5");
    fail += check_add_exact<FpType>(1.0e8, 3.0e-3, tag + " add 1e8+3e-3");

    std::mt19937_64 rng(0xC0FFEEULL);
    std::uniform_real_distribution<double> ud(-1000.0, 1000.0);
    for (int i = 0; i < 200; ++i) {
        double a = ud(rng), b = ud(rng);
        fail += check_add_exact<FpType>(a, b, tag + " add rand");
        FpType av = static_cast<FpType>(a), bv = static_cast<FpType>(b), cv = static_cast<FpType>(ud(rng));
        if (av != FpType{0} && bv != FpType{0} && cv != FpType{0})
            fail += check_threeAdd_exact<FpType>(B{av, 0}, B{bv, 0}, B{cv, 0}, tag + " 3add rand");
    }
    return fail;
}

// Truncating host (bfloat16): EFTs cannot be exact because bfloat16 rounds
// toward zero (numeric_limits::round_style == round_toward_zero; its float->bf16
// cast is `bits >> 16`, discarding the low bits). Knuth/Dekker EFTs require
// round-to-nearest. We PIN the behaviour with a truncation bound rather than
// exactness: the recovered (high + low) must agree with the exact product/sum to
// within ~1 ulp of the residual block (or 1 ulp of the high block when the
// residual truncates to zero). Documents that bfloat16 is approximate-only.
template <typename FpType>
int check_two_sum_bounded(const block<FpType>& a, const block<FpType>& b,
                          const std::string& tag) {
    auto [high, low] = block_two_sum(a, b);
    double err   = std::fabs((dval(a) + dval(b)) - (dval(high) + dval(low)));
    double bound = 2.0 * (low.is_zero_block() ? block_ulp(high) : block_ulp(low));
    if (err > bound) {
        std::cout << tag << " two_sum exceeds truncation bound: err=" << err
                  << " bound=" << bound << " (a=" << double(a.v) << " b=" << double(b.v) << ")\n";
        return 1;
    }
    return 0;
}

template <typename FpType>
int check_two_mult_bounded(const block<FpType>& a, const block<FpType>& b,
                           const std::string& tag) {
    auto [high, low] = block_two_mult(a, b);
    double err   = std::fabs((dval(a) * dval(b)) - (dval(high) + dval(low)));
    double bound = 2.0 * (low.is_zero_block() ? block_ulp(high) : block_ulp(low));
    if (err > bound) {
        std::cout << tag << " two_mult exceeds truncation bound: err=" << err
                  << " bound=" << bound << " (a=" << double(a.v) << " b=" << double(b.v) << ")\n";
        return 1;
    }
    return 0;
}

template <typename FpType>
int sweep_eft_truncating(const std::string& tag) {
    using B = block<FpType>;
    int fail = 0;
    fail += check_two_sum_bounded<FpType>(B{FpType{1.25}, 0}, B{FpType{-1.25}, 0}, tag + " cancel");
    fail += check_two_sum_bounded<FpType>(B{FpType{1}, 7}, B{FpType{0.25}, 7}, tag + " offset");
    std::mt19937_64 rng(0xB16ULL);
    std::uniform_real_distribution<double> ud(-100.0, 100.0);
    for (int i = 0; i < 300; ++i) {
        FpType av = static_cast<FpType>(ud(rng)), bv = static_cast<FpType>(ud(rng));
        if (av == FpType{0} || bv == FpType{0}) continue;
        fail += check_two_sum_bounded<FpType>(B{av, 0}, B{bv, 0}, tag + " rand-sum");
        fail += check_two_mult_bounded<FpType>(B{av, 0}, B{bv, 0}, tag + " rand-mult");
    }
    return fail;
}

// Full-width quad: genuine 113-bit-significand operands (produced by division, so
// the mantissa is filled, not double-derived). This is the case that exercises
// the bit-based exact_real and the fused-fma two_prod (#1024). Both the pre-fix
// frexp/floor oracle and the pre-#1024 double-intermediate two_mult failed here;
// the double-derived q128 sweeps elsewhere did not catch it (short significands).
template <typename Q>
int sweep_quad_fullwidth(const std::string& tag) {
    using B = block<Q>;
    int fail = 0;
    // hand-picked irrational-in-binary products/sums
    fail += check_two_mult<Q>(B{Q(1)/Q(3), 0}, B{Q(1)/Q(7), 0},  tag + " 1/3 * 1/7");
    fail += check_two_sum<Q> (B{Q(1)/Q(3), 0}, B{Q(1)/Q(7), 0},  tag + " 1/3 + 1/7");
    fail += check_threeAdd_exact<Q>(B{Q(1)/Q(3),0}, B{Q(1)/Q(7),0}, B{Q(1)/Q(11),0}, tag + " 3add 1/3+1/7+1/11");

    std::mt19937_64 rng(0x91D7ULL);
    std::uniform_real_distribution<double> ud(-1000.0, 1000.0);
    for (int i = 0; i < 300; ++i) {
        Q av = Q(ud(rng)) / Q(3) / Q(7);     // fills the 113-bit significand
        Q bv = Q(ud(rng)) / Q(11) / Q(13);
        if (av == Q(0) || bv == Q(0)) continue;
        fail += check_two_sum<Q> (B{av, 0}, B{bv, 0}, tag + " fw two_sum");
        fail += check_two_mult<Q>(B{av, 0}, B{bv, 0}, tag + " fw two_mult");
    }
    return fail;
}

} // anonymous

int main()
try {
    using namespace sw::universal;
    std::string test_suite = "elreal exact dyadic-rational oracle (#1022)";
    int nrOfFailedTestCases = 0;
    bool reportTestCases = false;
    ReportTestSuiteHeader(test_suite, reportTestCases);

    using cf24  = cfloat<24, 5, std::uint16_t, true, false, false>;
    using cf32  = cfloat<32, 8, std::uint32_t, true, false, false>;
    using q128  = cfloat<128, 15, std::uint32_t, true, false, false>;  // IEEE-quad-width host (113-bit significand)

    // two_sum: exact on every RN host -- including quad precision (cf<128,15>).
    // The dyadic oracle extracts the full significand, so this would be
    // impossible to certify through a double reference.
    nrOfFailedTestCases += sweep_two_sum<float>("float");
    nrOfFailedTestCases += sweep_two_sum<double>("double");
    nrOfFailedTestCases += sweep_two_sum<half>("half");
    nrOfFailedTestCases += sweep_two_sum<cf24>("cfloat<24,5>");
    nrOfFailedTestCases += sweep_two_sum<cf32>("cfloat<32,8>");
    nrOfFailedTestCases += sweep_two_sum<q128>("cfloat<128,15> (quad)");
    // two_mult: exact on every RN host -- including quad (fused-fma path, #1024).
    nrOfFailedTestCases += sweep_two_mult<float>("float");
    nrOfFailedTestCases += sweep_two_mult<double>("double");
    nrOfFailedTestCases += sweep_two_mult<half>("half");
    nrOfFailedTestCases += sweep_two_mult<cf24>("cfloat<24,5>");
    nrOfFailedTestCases += sweep_two_mult<cf32>("cfloat<32,8>");
    nrOfFailedTestCases += sweep_two_mult<q128>("cfloat<128,15> (quad)");
    // threeAdd + add(): twoSum-based -> exact on every RN host including quad.
    nrOfFailedTestCases += sweep_combinators_exact<float>("float");
    nrOfFailedTestCases += sweep_combinators_exact<double>("double");
    nrOfFailedTestCases += sweep_combinators_exact<cf32>("cfloat<32,8>");
    nrOfFailedTestCases += sweep_combinators_exact<q128>("cfloat<128,15> (quad)");
    // Full-width quad: genuine 113-bit operands (the case earlier sweeps missed).
    nrOfFailedTestCases += sweep_quad_fullwidth<q128>("cfloat<128,15> full-width");
    // bfloat16 truncates (round_toward_zero): not an exact-EFT host. Pin its
    // approximate behaviour with a truncation bound.
    nrOfFailedTestCases += sweep_eft_truncating<bfloat16>("bfloat16");

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (const std::exception& err) {
    std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
