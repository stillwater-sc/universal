// !!! WORK IN PROGRESS -- PARTIAL. NOT INCLUDED BY ANY PRODUCTION CODE. !!!
// Status:
//  * SINGLE-BLOCK DIVISOR: WORKS. 1/7, 1/3, 22/7, ... match eager div() block-for-block;
//    6/3 terminates. The lazy quotient refines to the host's natural ~19-component ceiling
//    on demand: the min_exp+2k floor is now gated to narrow hosts only (#1061), so on a
//    wide host div_online(1,3) reaches ~293 digits / 19 blocks -- the same depth as eager
//    div() -- instead of stopping ~33 digits short. Guarded by verify_div_deep_reach.
//  * SPARSE (power-of-two) MULTI-BLOCK DIVISOR: WORKS to the host floor with the wide
//    block exponent (integer<256>, #1066): 1/(1+2^-55) -> full depth, 0-overlap, exact
//    reconstruction q*b == 1. (divideHelper recurses with newdiv = g0*divisor, doubling
//    the divisor exponent each level; int32 overflowed after ~11 levels, integer<256>
//    does not.)
//  * GENERAL (DENSE) MULTI-BLOCK DIVISOR: cost explosion FIXED; one host-floor issue left.
//    - FIXED (cost explosion): twoDivZBCL was fanning a single-block division out into a
//      MULTI-block remainder (x - q*y via priestRenorm) + singleDiv, which is NOT what the
//      dissertation does. Def 4.2.8 keeps it single-block: twoDiv returns the SINGLE
//      remainder block e (x/y = s + e/y, block_two_div_rem), and twoDivZBCL recurses
//      single-block-by-single-block. With that, a dense divisor produces correct canonical
//      blocks in milliseconds (was: did not terminate). Also matched the dissertation on the
//      zero case ([x]) and on using plain infSum (not a drop-leading-zero "addition") in
//      infsumRec.
//    - REMAINING (host floor, DENSE only): twoDivZBCL's own floor is gated to narrow hosts
//      (#1061), so the single-block and sparse paths refine to the host ceiling. But a DENSE
//      divisor's INTERNAL streaming products newfs = fs*divisorTail and newdiv = g0*divisor
//      (mul_online / singleMult) are NOT yet gated: near ~2k above the host's smallest normal
//      exponent the EFTs there can no longer keep blocks k apart (the slot k below is
//      subnormal), so they emit subnormal residuals that break k-spacing when infSum consumes
//      them, and 0-overlap breaks. The eager div() survives by re-running priestRenorm +
//      keep_normalised each step; a streaming producer cannot. Fix: uniform host-floor
//      handling across the streaming multiply path (the "div floor rework"). Until then a
//      DENSE divisor is correct only down to ~the floor margin (tests keep it shallow).
//
// online_divide.hpp: McCleeary LFPERA streaming division (dissertation 4.2.6).
//
// Direct translation of FCL.hs Appendix A.4: twoDivZBCL / singleDiv / divideHelper /
// div / divide. The quotient is produced on demand as a streaming infSum of partial
// quotients; twoDivZBCL is single-block long division using block_two_div_rem (twoDiv,
// Def 4.1.12). Builds on the streaming infSum (infsum.hpp) and the streaming product
// (online_multiply.hpp). Phase 1 of #1061.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once

#include <cstdint>
#include <limits>
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

// twoDivZBCL(x, y): block x / block y as a lazy ZBCL (infinite for an irrational
// ratio). Faithful FCL.hs (dissertation Def 4.2.8):
//   twoDivZBCL x y = if isZero x then [x]
//                    else let (s,e) = twoDiv x y in s : twoDivZBCL e y
// twoDiv (block_two_div_rem) returns the quotient digit s = round(x/y) AND the
// SINGLE remainder block e = x - s*y, with x/y = s + e/y. We then divide that
// single remainder block by y again -- single-block by single-block, the
// remainder exponent dropping by >= k each step (lemma 4.2.23). NO multi-block
// remainder and NO singleDiv fan-out (the earlier divergence that made the dense
// multi-block long division explode and break 0-overlap).
template <typename FpType>
inline ZBCL<FpType> twoDivZBCL(block<FpType> x, block<FpType> y) {
    if (x.is_zero_block()) {
        // FCL.hs: `if isZero x then [x]` -- emit the zero block AT ITS OWN
        // exponent (E(x)), not the quotient scale. Lowering it would break the
        // strictly-decreasing-leading-exponent precondition of the enclosing
        // infSum (singleDivHelper feeds it blocks of strictly decreasing E).
        return ZBCL<FpType>::singleton(x);
    }
    auto se = block_two_div_rem(x, y);                // (s, e): x/y = s + e/y
    // Host-floor guard, gated to narrow hosts -- mirrors divide.hpp's exp_floor.
    //
    // block_two_div_rem is scale-invariant: the quotient/remainder significands
    // it produces stay in [1,2) (always a normal host value) and the scale is
    // carried symbolically in the wide block exponent (integer<256>, #1066), so a
    // block at an arbitrarily negative combined exponent is still well-formed and
    // 0-overlap accounting holds. On a WIDE host (double/float, k>=24) the floor
    // is therefore defensive, not load-bearing: lifting it lets the lazy quotient
    // refine to the host's natural ~19-component ceiling and beyond on demand
    // (e.g. div_online(1,3) reaches the eager div()'s depth instead of stopping
    // ~33 digits short). This is McCleeary's unbounded-exponent stream, the reason
    // the block exponent was widened in the first place.
    //
    // A NARROW host (bfloat16 k=8, fp16 k=11) genuinely denormalises a couple of
    // block-widths above min_exponent -- there block_two_div_rem can no longer
    // place the remainder a full k below the quotient, so twoDiv's "e is >= k
    // below s" guarantee fails and the stream stops being 0-overlap. A streaming
    // producer cannot post-renormalise (the eager div() re-runs priestRenorm +
    // keep_normalised every step), so a narrow host keeps the min_exp+2k floor
    // (the same denormal floor #1044 respects).
    constexpr int k = block<FpType>::k;
    constexpr int host_exp_floor = (k >= 24)
        ? (std::numeric_limits<int>::min() / 2)                       // wide host: no floor
        : (std::numeric_limits<FpType>::min_exponent + 2 * k);        // narrow host: denormal floor
    const block<FpType> s = se.first;
    if (!s.is_normalised() || s.exponent() < host_exp_floor) return ZBCL<FpType>{};
    const block<FpType> e = se.second;                // single remainder block x - s*y
    block<FpType> ycopy = y;
    return ZBCL<FpType>::cons(s, [e, ycopy]() { return twoDivZBCL(e, ycopy); });
}

// singleDivHelper / singleDiv: a ZBCL divided by a single block g.
template <typename FpType>
inline series<FpType> singleDivHelper(ZBCL<FpType> fs, block<FpType> g) {
    if (fs.is_empty()) return series<FpType>{};
    ZBCL<FpType> term = twoDivZBCL(fs.head(), g);     // f_i / g
    ZBCL<FpType> rest = fs.tail();
    block<FpType> gcopy = g;
    return series<FpType>::cons(term, [rest, gcopy]() { return singleDivHelper(rest, gcopy); });
}
template <typename FpType>
inline ZBCL<FpType> singleDiv(ZBCL<FpType> fs, block<FpType> g) {
    return infsum(singleDivHelper(std::move(fs), g));
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
    // Faithful FCL.hs translation. newdiv = g0*divisor doubles the divisor exponent each
    // level; the wide block exponent (integer<256>, #1066) carries that without overflow.
    ZBCL<FpType> qterm  = singleDiv(fs, g);                       // fs / g0
    ZBCL<FpType> newfs  = negate(mul_online(fs, divisor.tail())); // -(fs * divisorTail)
    ZBCL<FpType> newdiv = singleMult(g, divisor);                // g0 * divisor
    return series<FpType>::cons(qterm, [newfs, newdiv]() { return divideHelper(newfs, newdiv); });
}

// div_raw(fs, gs) = infiniteSum(divideHelper fs gs). Operands assumed range-reduced.
template <typename FpType>
inline ZBCL<FpType> div_raw(ZBCL<FpType> fs, ZBCL<FpType> gs) {
    if (fs.is_empty()) return ZBCL<FpType>{};
    return infsum(divideHelper(std::move(fs), std::move(gs)));
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
