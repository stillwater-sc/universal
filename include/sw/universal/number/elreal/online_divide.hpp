// !!! WORK IN PROGRESS -- PARTIAL. NOT INCLUDED BY ANY PRODUCTION CODE. !!!
// Status:
//  * SINGLE-BLOCK DIVISOR: WORKS to full precision. 1/7, 1/3, 22/7, ... match eager
//    div() to the double-host ceiling (~300 digits); exact ratios (6/3) terminate.
//    Resolved en route: (a) the block_two_div CORRECTION-vs-REMAINDER semantic gap
//    (twoDivFCL computes the true remainder x - q*y via block_two_mult + priestRenorm
//    and divides it by y), (b) a leading-zero remainder mis-seeding the infSum bound
//    (removeZeros), (c) non-termination past the host floor (stop on a non-normalised
//    quotient block).
//  * SPARSE MULTI-BLOCK DIVISOR: WORKS to full precision now that the block exponent is
//    wide (integer<256>, #1066). divideHelper recurses with newdiv = g0*divisor each level,
//    so the divisor's exponent grows unboundedly (trace: 17,35,70,...); int32 overflowed
//    after ~11 levels. With the wide exponent, 1/(1+2^-55) reaches the full 19 blocks (host
//    floor) with correct 0-overlap and exact reconstruction q*b == 1 -- the int32 cap is gone.
//  * GENERAL (DENSE) MULTI-BLOCK DIVISOR: STILL OPEN. The wide exponent was necessary but not
//    sufficient. For a divisor whose blocks are NOT powers of two, two problems remain that
//    the int32 overflow used to mask:
//      (a) 0-OVERLAP CORRECTNESS: the quotient stream can emit a non-canonical (0-overlap-
//          violating) adjacent pair (same class as the #1057 add bug) -- value-correct but
//          not the unique DBL_k form.
//      (b) COST EXPLOSION: singleMult(g0, divisor) fills in low-order blocks, so the running
//          divisor's BLOCK COUNT grows ~1/level; for a dense divisor even take(2) does not
//          terminate in reasonable time. The int32 overflow was an accidental cost bound.
//    Needs an algorithm fix (keep the running divisor sparse / bound its block count, and a
//    0-overlap carry-arrest in the fold) -- the hard remaining phase-1 item, NOT a rebase.
//
// online_divide.hpp: McCleeary LFPERA streaming division (dissertation 4.2.6).
//
// Direct translation of FCL.hs Appendix A.4: twoDivFCL / singleDiv / divideHelper /
// div / divide. The quotient is produced on demand as a streaming infSum of partial
// quotients; each block-pair quotient is the exact-ish 2-block block_two_div. Builds on
// the streaming infSum (infsum.hpp) and the streaming product (online_multiply.hpp), now
// that add() holds the 0-overlap canonical form under composition (#1057). Phase 1 of
// #1061.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once

#include <cstdint>
#include <vector>

#include <universal/number/elreal/block.hpp>
#include <universal/number/elreal/block_eft.hpp>     // block_two_div_rn, block_two_mult
#include <universal/number/elreal/zbcl.hpp>
#include <universal/number/elreal/series.hpp>
#include <universal/number/elreal/threeAdd.hpp>      // priestRenorm
#include <universal/number/elreal/sum.hpp>           // zbcl_from_blocks
#include <universal/number/elreal/negate.hpp>        // negate
#include <universal/number/elreal/infsum.hpp>        // infinitesum
#include <universal/number/elreal/online_multiply.hpp> // mul_online, singleMult

namespace sw { namespace universal {

template <typename FpType> inline ZBCL<FpType> singleDiv(ZBCL<FpType> fs, block<FpType> g);

// zbcl_shift(z, n): multiply z by 2^n, exactly, by shifting every block exponent by n.
// Lazy (FCL.hs shiftUp / shiftDown). Used for the divisor range reduction.
template <typename FpType>
inline ZBCL<FpType> zbcl_shift(ZBCL<FpType> z, typename block<FpType>::exp_t n) {
    if (n == 0 || z.is_empty()) return z;
    block<FpType> h{ z.head().v, z.head().exp + n };
    ZBCL<FpType> rest = z.tail();
    return ZBCL<FpType>::cons(h, [rest, n]() { return zbcl_shift(rest, n); });
}

// twoDivFCL(x, y): block x / block y as a lazy ZBCL (infinite for an irrational ratio).
// Long division: q = round(x/y); remainder rem = x - q*y (exact, multi-block via
// block_two_mult + priestRenorm); the next quotient blocks are rem/y. So
//   x/y = q : (rem / y) = q : singleDiv(rem, y).
// NOTE: the codebase's block_two_div returns the quotient CORRECTION (x/y - q), not the
// FCL `twoDiv` remainder; recursing on that divides by y twice and is wrong (caps at one
// limb). We compute the true remainder x - q*y instead and divide it by y.
template <typename FpType>
inline ZBCL<FpType> twoDivFCL(block<FpType> x, block<FpType> y) {
    if (x.is_zero_block()) {
        return ZBCL<FpType>::singleton(createZero<FpType>(x.exponent() - y.exponent()));
    }
    const block<FpType> q = block_two_div_rn(x, y);   // round(x/y)
    if (!q.is_normalised()) return ZBCL<FpType>{};    // quotient underflowed the host floor; stop
    auto qy = block_two_mult(q, y);                   // q*y = (hi, lo), exact
    // x - q*y: the high parts cancel, so priestRenorm leaves a LEADING ZERO block at x's
    // exponent. removeZeros it -- a leading zero (at a high exponent) would mis-seed the
    // infSum bound in singleDiv and spin the cancellation path.
    std::vector<block<FpType>> rem = removeZeros(priestRenorm(std::vector<block<FpType>>{
        x, block<FpType>{ -qy.first.v, qy.first.exp }, block<FpType>{ -qy.second.v, qy.second.exp } }));
    ZBCL<FpType> remz = zbcl_from_blocks<FpType>(rem);   // x - q*y
    if (remz.is_empty()) return ZBCL<FpType>::singleton(q);   // exact division
    block<FpType> ycopy = y;
    return ZBCL<FpType>::cons(q, [remz, ycopy]() { return singleDiv(remz, ycopy); });
}

// singleDivHelper / singleDiv: a ZBCL divided by a single block g.
template <typename FpType>
inline series<FpType> singleDivHelper(ZBCL<FpType> fs, block<FpType> g) {
    if (fs.is_empty()) return series<FpType>{};
    ZBCL<FpType> term = twoDivFCL(fs.head(), g);      // f_i / g
    ZBCL<FpType> rest = fs.tail();
    block<FpType> gcopy = g;
    return series<FpType>::cons(term, [rest, gcopy]() { return singleDivHelper(rest, gcopy); });
}
template <typename FpType>
inline ZBCL<FpType> singleDiv(ZBCL<FpType> fs, block<FpType> g) {
    return infinitesum(singleDivHelper(std::move(fs), g));
}

// divideHelper: the long division. q ~= fs/g0 (leading divisor block); the residual is
// refined by dividing -(fs * divisorTail) by g0*divisor. FCL.hs:
//   divideHelper fs (g:gs) =
//     if isZero g then [g] : divideHelper fs gs
//     else (singleDiv fs g) : divideHelper (negation (multiply fs gs)) (singleMult g (g:gs))
template <typename FpType>
inline series<FpType> divideHelper(ZBCL<FpType> fs, ZBCL<FpType> divisor) {
    if (fs.is_empty() || divisor.is_empty()) return series<FpType>{};
    const block<FpType> g = divisor.head();
    if (g.is_zero_block()) {
        ZBCL<FpType> zterm = ZBCL<FpType>::singleton(g);
        ZBCL<FpType> drest = divisor.tail();
        return series<FpType>::cons(zterm, [fs, drest]() { return divideHelper(fs, drest); });
    }
    // Faithful FCL.hs translation. The divisor magnitude (g0*divisor) grows every level, so
    // its exponent grows unboundedly -- the dissertation uses arbitrary-precision (Integer)
    // exponents, which this is correct against. On the codebase's int32 block exponent it
    // overflows after ~11 levels (capping multi-block precision). The fix is to follow the
    // dissertation and widen block<FpType>::exp (int64 / arbitrary precision), NOT to
    // reformulate the algorithm -- see the WIP banner.
    ZBCL<FpType> qterm  = singleDiv(fs, g);                       // fs / g0
    ZBCL<FpType> newfs  = negate(mul_online(fs, divisor.tail())); // -(fs * divisorTail)
    ZBCL<FpType> newdiv = singleMult(g, divisor);                // g0 * divisor
    return series<FpType>::cons(qterm, [newfs, newdiv]() { return divideHelper(newfs, newdiv); });
}

// div_raw(fs, gs) = infiniteSum(divideHelper fs gs). Operands assumed range-reduced.
template <typename FpType>
inline ZBCL<FpType> div_raw(ZBCL<FpType> fs, ZBCL<FpType> gs) {
    if (fs.is_empty()) return ZBCL<FpType>{};
    return infinitesum(divideHelper(std::move(fs), std::move(gs)));
}

// div_online(fs, gs): fs / gs. shiftUpToTwo normalises the divisor's leading exponent
// to >= 1 (scaling BOTH operands, so the quotient is unchanged) for convergence of the
// long-division series. 0/gs = 0; gs == 0 is the caller's precondition (empty divisor).
template <typename FpType>
inline ZBCL<FpType> div_online(ZBCL<FpType> fs, ZBCL<FpType> gs) {
    if (gs.is_empty()) return ZBCL<FpType>{};   // divide-by-zero: caller's precondition
    if (fs.is_empty()) return ZBCL<FpType>{};   // 0 / gs = 0
    const typename block<FpType>::exp_t lead = gs.head().exponent();
    const typename block<FpType>::exp_t shift = (lead < 1) ? (1 - lead) : 0;   // shiftUpToTwo
    return div_raw(zbcl_shift(std::move(fs), shift), zbcl_shift(std::move(gs), shift));
}

}} // namespace sw::universal
