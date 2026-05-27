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
// int32 `exp`) is  value(b) = v * 2^exp,  and v is exactly representable in a
// double for every elreal FpType (all have <= 24 significand bits), so
//     exact(b) = from_double(double(b.v))  with its dyadic scale shifted by b.exp.
// The exact value of a ZBCL prefix is the (exact) sum of its block dyadics.
//
// What is asserted:
//   * block_two_sum / block_two_mult are error-free transforms, so
//         exact(high) + exact(low) == exact(a) {+,*} exact(b)   EXACTLY,
//     UNLESS the residual `low` is a subnormal the FpType cannot represent --
//     the cfloat<24,5> representability floor identified in #942. We encode that
//     exception structurally (a nonzero dyadic error is tolerated ONLY when the
//     least-significant result block is a nonzero subnormal); a wrong high part,
//     or a wrong NORMAL residual, still fails.
//   * threeAdd and the lazy add() combinator reproduce the exact sum on the
//     wide-exponent RN hosts (double, float, cfloat<32,8>), where the expansion
//     is always representable -- asserted as exact dyadic equality. Narrow-
//     exponent RN hosts (half, cfloat<24,5>) are validated at the EFT level
//     above; their multi-block strict bound needs a rounding oracle (future).
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
#include <cmath>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/bfloat16/bfloat16.hpp>
#include <universal/number/elreal/elreal.hpp>
#include <universal/verification/dyadic_exact.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

using namespace sw::universal;

// exact value of a single block as a dyadic rational. Shares no code with the
// block/EFT/threeAdd/add algorithms under test.
template <typename FpType>
dyadic exact_block(const block<FpType>& b) {
    if (b.is_zero_block()) return dyadic();
    dyadic d = dyadic::from_double(static_cast<double>(b.v));
    d.scale += b.exp;          // multiply by 2^exp exactly (value = v * 2^exp)
    return d;
}

template <typename FpType>
dyadic exact_blocks(const std::vector<block<FpType>>& bs) {
    dyadic acc;                // 0
    for (const auto& b : bs) acc = acc + exact_block(b);
    return acc;
}

template <typename FpType>
dyadic exact_zbcl(const ZBCL<FpType>& z) {
    return exact_blocks(z.take(32));   // finite sums settle well within 32 blocks
}

// value of a block in double. For the round-to-nearest hosts we use the dyadic
// oracle (above); for bfloat16 we use double directly -- it carries ~45 bits of
// headroom over bfloat16's 8-bit significand, so it is a genuinely exact oracle
// for bfloat16 sums/products in this test range (unlike long-double-over-double,
// which was the Phase 1-4 weakness this file removes).
template <typename FpType>
double dval(const block<FpType>& b) {
    if (b.is_zero_block()) return 0.0;
    return std::ldexp(static_cast<double>(b.v), b.exp);
}

// ulp of a block at its own exponent: 2^(E(b) - (k-1)).
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
    dyadic ref = exact_block(a) + exact_block(b);
    dyadic got = exact_block(high) + exact_block(low);
    dyadic err = ref - got;
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
    dyadic ref = exact_block(a) * exact_block(b);
    dyadic got = exact_block(high) + exact_block(low);
    dyadic err = ref - got;
    if (!acceptable(err, low)) {
        std::cout << tag << " two_mult value WRONG: a=" << double(a.v)
                  << " b=" << double(b.v)
                  << " high=" << double(high.v) << " low=" << double(low.v)
                  << " (low normalised=" << low.is_normalised() << ")\n";
        return 1;
    }
    return 0;
}

// ---- threeAdd (wide hosts): exact(out1+out2+out3) == exact(ia+ib+ic) --------
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

// ---- add() (wide hosts): exact(take(N)) == exact(za) + exact(zb) ------------
template <typename FpType>
int check_add_exact(double a, double b, const std::string& tag) {
    auto za = from_native<FpType>(a);
    auto zb = from_native<FpType>(b);
    auto z  = add(za, zb);
    dyadic ref = exact_zbcl(za) + exact_zbcl(zb);   // uses the represented values
    dyadic got = exact_zbcl(z);
    if (ref != got) {
        std::cout << tag << " add value WRONG: a=" << a << " b=" << b << "\n";
        return 1;
    }
    return 0;
}

// EFT sweep: runs for EVERY host (the residual block is directly inspectable, so
// the #942 subnormal-floor exception is exact).
template <typename FpType>
int sweep_eft(const std::string& tag) {
    using B = block<FpType>;
    int fail = 0;
    constexpr int p = std::numeric_limits<FpType>::digits;

    // --- two_sum, shared exp = 0 ---
    // residual at the ulp boundary
    {
        FpType tiny = static_cast<FpType>(std::ldexp(1.0, -p));
        if (tiny != FpType{0})
            fail += check_two_sum<FpType>(B{FpType{1}, 0}, B{tiny, 0}, tag + " 1+2^-p");
    }
    // exact cancellation
    fail += check_two_sum<FpType>(B{FpType{1.25}, 0}, B{FpType{-1.25}, 0}, tag + " 1.25-1.25");
    // wide scale separation within one exp
    fail += check_two_sum<FpType>(B{static_cast<FpType>(1e4), 0},
                                  B{static_cast<FpType>(1.0/1024), 0}, tag + " 1e4+2^-10");
    // nonzero shared exp must not change exactness
    fail += check_two_sum<FpType>(B{FpType{1}, 7}, B{FpType{0.25}, 7}, tag + " offset=7");

    // --- two_mult ---
    fail += check_two_mult<FpType>(B{FpType{1.5}, 0}, B{FpType{1.5}, 0}, tag + " 1.5*1.5");
    fail += check_two_mult<FpType>(B{FpType{3}, 0}, B{FpType{0.5}, 0}, tag + " 3*0.5 exact");
    fail += check_two_mult<FpType>(B{static_cast<FpType>(123.456), 2},
                                   B{static_cast<FpType>(-7.875), -1}, tag + " mixed-exp");

    // --- randomised EFT sweep ---
    std::mt19937_64 rng(0x1022ABCDULL);
    std::uniform_real_distribution<double> ud(-100.0, 100.0);
    for (int i = 0; i < 300; ++i) {
        FpType av = static_cast<FpType>(ud(rng));
        FpType bv = static_cast<FpType>(ud(rng));
        if (av == FpType{0} || bv == FpType{0}) continue;
        fail += check_two_sum<FpType>(B{av, 0}, B{bv, 0}, tag + " rand-sum");
        fail += check_two_mult<FpType>(B{av, 0}, B{bv, 0}, tag + " rand-mult");
    }
    return fail;
}

// Multi-block combinators: strict exact equality, wide-exponent hosts only.
template <typename FpType>
int sweep_combinators_exact(const std::string& tag) {
    using B = block<FpType>;
    int fail = 0;

    // threeAdd hand-picked
    fail += check_threeAdd_exact<FpType>(B{FpType{1}, 0}, B{FpType{0.25}, 0}, B{FpType{0.0625}, 0}, tag + " 3add nice");
    fail += check_threeAdd_exact<FpType>(B{static_cast<FpType>(1e6), 0}, B{FpType{1}, 0}, B{static_cast<FpType>(1.0/4096), 0}, tag + " 3add spread");
    fail += check_threeAdd_exact<FpType>(B{FpType{5}, 0}, B{FpType{-5}, 0}, B{FpType{2.5}, 0}, tag + " 3add cancel");

    // add() hand-picked, incl. classic non-representable decimals (the cast
    // values are what we reference, so equality is well-defined)
    fail += check_add_exact<FpType>(1.0, 0.25, tag + " add 1+0.25");
    fail += check_add_exact<FpType>(1e3, 1.0, tag + " add 1e3+1");
    fail += check_add_exact<FpType>(0.1, 0.2, tag + " add 0.1+0.2");
    fail += check_add_exact<FpType>(-2.5, -7.5, tag + " add -2.5-7.5");
    fail += check_add_exact<FpType>(1.0e8, 3.0e-3, tag + " add 1e8+3e-3");

    // randomised
    std::mt19937_64 rng(0xC0FFEEULL);
    std::uniform_real_distribution<double> ud(-1000.0, 1000.0);
    for (int i = 0; i < 200; ++i) {
        double a = ud(rng), b = ud(rng);
        fail += check_add_exact<FpType>(a, b, tag + " add rand");
        // threeAdd over randomised blocks (shared exp 0)
        FpType av = static_cast<FpType>(a), bv = static_cast<FpType>(b), cv = static_cast<FpType>(ud(rng));
        if (av != FpType{0} && bv != FpType{0} && cv != FpType{0})
            fail += check_threeAdd_exact<FpType>(B{av, 0}, B{bv, 0}, B{cv, 0}, tag + " 3add rand");
    }
    return fail;
}

// Truncating host (bfloat16): EFTs cannot be exact because bfloat16 rounds
// toward zero (numeric_limits::round_style == round_toward_zero; its float->bf16
// cast is `bits >> 16`, discarding the low bits). Knuth/Dekker EFTs require
// round-to-nearest. We therefore PIN the behaviour with a truncation bound
// rather than exactness: the recovered (high + low) must agree with the exact
// product/sum to within ~1 ulp of the residual block (or 1 ulp of the high block
// when the residual truncates away to zero). This still catches a wholesale-wrong
// result while documenting that bfloat16 is an approximate-only elreal host.
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

} // anonymous

int main()
try {
    using namespace sw::universal;
    std::string test_suite = "elreal exact dyadic-rational oracle (#1022)";
    int nrOfFailedTestCases = 0;
    bool reportTestCases = false;
    ReportTestSuiteHeader(test_suite, reportTestCases);

    using cf24 = cfloat<24, 5, std::uint16_t, true, false, false>;
    using cf32 = cfloat<32, 8, std::uint32_t, true, false, false>;

    // EFT building blocks: exact (modulo #942 subnormal floor) on the
    // round-to-nearest hosts. bfloat16 is EXCLUDED: it truncates toward zero,
    // so Knuth/Dekker EFTs are not exact on it (see header note).
    nrOfFailedTestCases += sweep_eft<float>("float");
    nrOfFailedTestCases += sweep_eft<double>("double");
    nrOfFailedTestCases += sweep_eft<half>("half");
    nrOfFailedTestCases += sweep_eft<cf24>("cfloat<24,5>");
    nrOfFailedTestCases += sweep_eft<cf32>("cfloat<32,8>");

    // threeAdd + add(): strict exact equality on the wide-exponent RN hosts.
    nrOfFailedTestCases += sweep_combinators_exact<float>("float");
    nrOfFailedTestCases += sweep_combinators_exact<double>("double");
    nrOfFailedTestCases += sweep_combinators_exact<cf32>("cfloat<32,8>");

    // bfloat16 truncates (round_toward_zero): not an exact-EFT host. Pin its
    // approximate behaviour with a truncation bound so a regression is caught
    // and the limitation is documented in an executable form.
    nrOfFailedTestCases += sweep_eft_truncating<bfloat16>("bfloat16");

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (const std::exception& err) {
    std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
