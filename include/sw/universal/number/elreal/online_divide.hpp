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
//  * GENERAL (DENSE) MULTI-BLOCK DIVISOR: now via NEWTON-RAPHSON reciprocal (#1068).
//    - The faithful long division (divideHelper) is correct for dense divisors but
//      COST-EXPLODES: newfs = mul_online(fs, divisorTail) grows fs every level and
//      infsumRec's cancellation region pulls an unbounded prefix of that growing stream
//      per emitted block (measured: correct through depth 7 ~28ms, then non-terminating
//      at depth 8, at exponent ~ -340 -- far above any host floor). So div_online routes
//      a dense divisor to a/b = a*(1/b) with a Newton reciprocal (recip_newton): r_{n+1}
//      = r_n(2 - b r_n), error squaring per step, seeded from 1/leading-block. Reuses the
//      working mul_online + add; terminates; 0-overlap; reconstructs q*b == a exactly.
//      This is a DELIBERATE DEVIATION from McCleeary 4.2.6 for the dense case (#1068).
//    - REMAINING LIMIT (caps dense depth at ~8 blocks / ~118 digits): mul_online emits a
//      0-overlap-violating pair once an operand exceeds ~9-10 blocks. This is a real
//      canonicalisation limit in the STREAMING MULTIPLY, not a host underflow (it bites at
//      ~ -550, far above min_exponent). Earlier drafts of this banner mis-attributed it to
//      the host floor. Fixing mul_online's canonicalisation (the real "div floor rework")
//      lifts the dense cap toward the host ceiling -- tracked in #1068.
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
#include <universal/number/elreal/zbcl_helpers.hpp>  // from_native
#include <universal/number/elreal/series.hpp>
#include <universal/number/elreal/threeAdd.hpp>      // priestRenorm, add
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

// zbcl_truncate(z, m): the finite m-block prefix of z, rebuilt as a 0-overlap ZBCL.
// (z is already 0-overlap, so take(m) is a canonical prefix; zbcl_from_blocks drops
// any trailing zero blocks.) Used to keep Newton's intermediate products finite.
template <typename FpType>
inline ZBCL<FpType> zbcl_truncate(const ZBCL<FpType>& z, std::size_t m) {
    return zbcl_from_blocks<FpType>(z.take(m));
}

// recip_newton(b, depth): the reciprocal 1/b to `depth` blocks via Newton-Raphson
//
//     r_{n+1} = r_n * (2 - b * r_n)
//
// Newton's reciprocal iteration squares the error each step (b*r_{n+1} = 1 - e_n^2,
// where e_n = 1 - b*r_n), so the number of correct blocks doubles per iteration.
// Seeded with the reciprocal of b's leading block -- already ~k bits (one block)
// correct -- it reaches the host's ~19-component ceiling in ~5 iterations.
//
// r is truncated to depth+guard blocks after each iteration: Newton self-corrects,
// so dropping the not-yet-significant tail is harmless, and it keeps EVERY
// intermediate product (b*r, r*s) finite. That bounded cost is exactly what the
// dissertation's long-division fan-out (divideHelper) fails to provide for a dense
// divisor -- see the file banner. This is a DELIBERATE DEVIATION from McCleeary's
// 4.2.6 long division for the dense case; tracked in #1068.
template <typename FpType>
inline ZBCL<FpType> recip_newton(const ZBCL<FpType>& b, std::size_t depth) {
    const block<FpType> b0 = b.head();
    // r0 = 1 / value(b0): significand 1/v0 (in (0.5,1] for v0 in [1,2), always
    // normal), scale -b0.exp. Built directly -- from_native cannot carry the wide
    // (integer<256>) exponent of a deep leading block.
    block<FpType> r0blk{ FpType(1) / b0.v, -b0.exp };
    ZBCL<FpType> r   = ZBCL<FpType>::singleton(r0blk);
    ZBCL<FpType> two = from_native<FpType>(2.0);

    const std::size_t guard = depth + 1;
    std::size_t iters = 1;                       // r0 is ~1 block correct ...
    for (std::size_t p = 1; p < depth; p <<= 1) ++iters;   // ... double to >= depth
    for (std::size_t i = 0; i < iters; ++i) {
        ZBCL<FpType> br = mul_online(b, r);                  // b*r ~ 1
        ZBCL<FpType> s  = add(two, negate(std::move(br)));   // 2 - b*r
        r = zbcl_truncate(mul_online(r, std::move(s)), guard);
    }
    return zbcl_truncate(r, depth);
}

// is_dense_divisor(gs): true iff gs is a multi-block divisor with a non-power-of-two
// block in its leading prefix. A SPARSE (all power-of-two) multi-block divisor stays
// on the faithful long-division path (it reaches the host floor there); only the
// genuinely DENSE case -- where divideHelper's fan-out cost-explodes past ~7 blocks
// -- routes to Newton. A single-block divisor (e.g. 1/3) is never dense.
template <typename FpType>
inline bool is_dense_divisor(const ZBCL<FpType>& gs) {
    const std::vector<block<FpType>> bl = gs.take(4);
    if (bl.size() < 2) return false;                       // single block: long division
    for (const auto& b : bl) if (!b.is_zero_block() && !singleBit(b)) return true;
    return false;                                          // all power-of-two: sparse
}

// div_online(fs, gs): fs / gs. Single-block and sparse divisors use the faithful
// McCleeary long division (div_raw); shiftUpToTwo normalises the divisor's leading
// exponent to >= 1 (scaling BOTH operands, so the quotient is unchanged) for the
// long-division series' convergence. A DENSE multi-block divisor instead uses
// a/b = a * (1/b) with a Newton-Raphson reciprocal (recip_newton): the long-division
// fan-out is correct but cost-explodes for dense divisors (#1061), so we deviate from
// McCleeary 4.2.6 there -- tracked in #1068. 0/gs = 0; gs == 0 is the caller's
// precondition (empty divisor).
template <typename FpType>
inline ZBCL<FpType> div_online(ZBCL<FpType> fs, ZBCL<FpType> gs) {
    if (gs.is_empty()) return ZBCL<FpType>{};   // divide-by-zero: caller's precondition
    if (fs.is_empty()) return ZBCL<FpType>{};   // 0 / gs = 0

    if (is_dense_divisor(gs)) {
        // Two ceilings bound the dense quotient depth; take the smaller:
        //  (1) host floor: stay a 2k margin above min_exponent so Newton's
        //      block_two_mult residuals (a further k below each block) do not
        //      denormalise -- the same floor the single-block path respects.
        //  (2) mul_online's canonicalisation limit: the streaming product emits a
        //      0-overlap-violating pair once an operand exceeds ~9-10 blocks
        //      (a real limit in the streaming multiply, NOT a host underflow; it
        //      bites at ~ -550, far above the floor). Until that is reworked we
        //      keep both Newton operands under it. Empirically 8 blocks is clean
        //      across diverse divisors; 9+ trips the ZBCL 0-overlap assert.
        // Both are the streaming-multiply "div floor rework" -- tracked in #1068.
        constexpr std::size_t host_floor_depth = static_cast<std::size_t>(
            (-(std::numeric_limits<FpType>::min_exponent) - 2 * block<FpType>::k)
            / block<FpType>::k);
        constexpr std::size_t mul_canonical_cap = 8;
        constexpr std::size_t target =
            host_floor_depth < mul_canonical_cap ? host_floor_depth : mul_canonical_cap;
        ZBCL<FpType> r = recip_newton(gs, target);
        return zbcl_truncate(mul_online(std::move(fs), std::move(r)), target);
    }

    const typename block<FpType>::exp_t lead = gs.head().exponent();
    const typename block<FpType>::exp_t shift = (lead < 1) ? (1 - lead) : 0;   // shiftUpToTwo
    return div_raw(zbcl_shift(std::move(fs), shift), zbcl_shift(std::move(gs), shift));
}

}} // namespace sw::universal
