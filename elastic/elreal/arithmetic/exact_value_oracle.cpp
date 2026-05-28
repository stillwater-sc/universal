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
#include <universal/verification/dyadic_exact.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

using namespace sw::universal;

// Exact value of a binary floating-point value v as a dyadic rational, with NO
// precision loss at any significand width.
//
// Two regimes, chosen at compile time by the significand width p = digits:
//
//   * p <= 53: double represents v exactly, so from_double is exact and cheap.
//     This covers float(24), double(53), half(11), bfloat16(8), cfloat<24,5>(19),
//     cfloat<32,8>(24) -- every <= 53-bit host, including the narrow-exponent
//     ones whose integer significand would OVERFLOW the type's own range.
//
//   * p > 53: too wide for double. Extract the full significand directly:
//     v = m * 2^e (frexp); the integer significand m*2^p is peeled into the
//     bigint 24 bits at a time using only exact operations (power-of-two shifts,
//     bit-exact floor, sub-2^24 chunks that convert through double->int64
//     losslessly). This needs 2^p (and the 2^24 chunk) to be representable in T,
//     i.e. the exponent range must exceed the significand width -- which holds
//     for every real quad/extended host (long double, cfloat<128,15>, ...); the
//     static_assert documents the requirement. Verified here at 113 bits.
//
// Unqualified frexp/ldexp/floor resolve to std:: (native) or sw::universal::
// (Universal types) by overload resolution.
template <typename T>
dyadic exact_real(T v) {
    if (v == T(0)) return dyadic();
    if constexpr (std::numeric_limits<T>::digits <= 53) {
        return dyadic::from_double(static_cast<double>(v));
    } else {
        static_assert(std::numeric_limits<T>::max_exponent
                          > std::numeric_limits<T>::digits,
            "exact_real's wide-significand path scales the significand up to an "
            "integer; that requires the exponent range to exceed the significand "
            "width. A host with > 53 significand bits but a small exponent range "
            "cannot even represent its own integer significand.");
        using std::frexp; using std::ldexp; using std::floor;
        int e = 0;
        T m = frexp(v, &e);                          // v = m * 2^e, |m| in [0.5,1)
        constexpr int p = std::numeric_limits<T>::digits;
        T scaled = ldexp(m, p);                      // exact integer, |scaled| < 2^p
        bool neg = scaled < T(0);
        if (neg) scaled = -scaled;
        dyadic::bigint M(0);
        const T CHUNK = ldexp(T(1), 24);             // 2^24 (representable: p > 53 > 24)
        int shift = 0;
        while (scaled > T(0)) {
            T hi = floor(scaled / CHUNK);            // scaled = hi*2^24 + lo (both exact)
            T lo = scaled - hi * CHUNK;              // lo in [0, 2^24), exact integer
            assert(lo >= T(0) && lo < CHUNK && "exact_real chunk out of [0, 2^24)");
            dyadic::bigint chunk(static_cast<long long>(static_cast<double>(lo)));
            chunk <<= shift;
            M = M + chunk;
            scaled = hi;
            shift += 24;
        }
        if (neg) M = -M;
        return dyadic(M, e - p);
    }
}

// exact value of a single block as a dyadic rational (value(b) = v * 2^exp).
// Shares no code with the block/EFT/threeAdd/add algorithms under test.
template <typename FpType>
dyadic exact_block(const block<FpType>& b) {
    if (b.is_zero_block()) return dyadic();
    dyadic d = exact_real(b.v);
    d.scale += b.exp;          // multiply by 2^exp exactly (value = v * 2^exp)
    return d;
}

template <typename FpType>
dyadic exact_blocks(const std::vector<block<FpType>>& bs) {
    dyadic acc;                // 0
    for (const auto& b : bs) acc = acc + exact_block(b);
    return acc;
}

// Number of ZBCL blocks forced when computing an exact value. Finite sums of the
// test inputs settle well within this; kept identical to addition.cpp's window
// so the two files agree on the "exact value" of the same stream.
constexpr std::size_t ZBCL_EXACT_WINDOW = 32;

template <typename FpType>
dyadic exact_zbcl(const ZBCL<FpType>& z) {
    return exact_blocks(z.take(ZBCL_EXACT_WINDOW));
}

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
    if ((exact_zbcl(za) + exact_zbcl(zb)) != exact_zbcl(z)) {
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

// two_mult sweep: RN hosts whose two_prod_host is exact. The generic cfloat
// two_prod uses a double intermediate for odd p, limiting it to p <= 26; the
// p <= 24 hosts here are within that bound. double itself (sweep_two_mult<double>,
// p = 53) is exact too -- intentional, not a bug: it has a dedicated two_prod
// specialisation (error_free_ops two_prod / hardware FMA), not the double
// intermediate. NOT quad: the odd-p double intermediate cannot hold a 113-bit
// product residual (separate elreal limitation, issue #1024).
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
    // two_mult: exact on the p<=24 RN hosts. NOT quad -- two_prod_host's odd-p
    // double intermediate cannot hold a 113-bit product residual (separate issue).
    nrOfFailedTestCases += sweep_two_mult<float>("float");
    nrOfFailedTestCases += sweep_two_mult<double>("double");
    nrOfFailedTestCases += sweep_two_mult<half>("half");
    nrOfFailedTestCases += sweep_two_mult<cf24>("cfloat<24,5>");
    nrOfFailedTestCases += sweep_two_mult<cf32>("cfloat<32,8>");
    // threeAdd + add(): twoSum-based -> exact on every RN host including quad.
    nrOfFailedTestCases += sweep_combinators_exact<float>("float");
    nrOfFailedTestCases += sweep_combinators_exact<double>("double");
    nrOfFailedTestCases += sweep_combinators_exact<cf32>("cfloat<32,8>");
    nrOfFailedTestCases += sweep_combinators_exact<q128>("cfloat<128,15> (quad)");
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
