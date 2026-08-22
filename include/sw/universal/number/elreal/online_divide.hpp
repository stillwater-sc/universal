// !!! WORK IN PROGRESS -- PARTIAL. NOT INCLUDED BY ANY PRODUCTION CODE. !!!
// Status:
//  * SINGLE-BLOCK DIVISOR: WORKS. 1/7, 1/3, 22/7, ... match eager div() block-for-block;
//    6/3 terminates. The lazy quotient refines to the host's natural ~19-component ceiling
//    on demand: the min_exp+2k floor is gone entirely (universal#1051) -- blocks are
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
//    - DEPTH: the dense quotient refines to the `depth` the CALLER asks for (default
//      kDenseQuotientDepth, matching the eager div()). It used to stop at a depth
//      DERIVED FROM THE HOST'S EXPONENT RANGE -- 17 components / ~265 digits on double,
//      and only 3 / ~22 digits on float -- which capped every dense division no matter
//      how deep the caller pulled (universal#1371). That was the same mistaken premise
//      #1362 removed elsewhere: a block carries its scale in a wide integer<256>
//      exponent, so min_exponent bounds nothing about how deep an expansion may go.
//      This required the
//      streaming-multiply host-floor arrest (#1068): mul_online USED to emit subnormal
//      blocks once a product reached ~2^-1022, and a subnormal block cannot hold its k
//      bits, so two of them landed ~22 apart and broke 0-overlap (an earlier draft of
//      this banner wrongly blamed a "~ -550 canonicalisation limit"; the violation is at
//      ~ -1052, i.e. the host floor). singleMultHelper now drops genuinely subnormal
//      blocks at the source, so no 0-overlap-violating block is ever produced and the
//      dense quotient is no longer capped at ~8 blocks.
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
#include <functional>
#include <limits>
#include <memory>
#include <optional>
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
    // Normalise the OPERANDS, not the results. block_two_div_rem computes the
    // remainder e = x - s*y in host arithmetic at the operands' natural scale; near
    // a narrow host's subnormal wall that intermediate loses bits, so e is wrong
    // before anything sees it and twoDiv's "e is >= k below s" guarantee fails.
    // Normalising the outputs cannot restore what the subnormal arithmetic
    // destroyed. Rescaled first, the division runs at scale ~1 (universal#1051).
    x.normalise();  x.bias_for_eft();
    y.normalise();  y.bias_for_eft();
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
    const block<FpType> s = se.first;
    if (!s.is_normalised()) return ZBCL<FpType>{};    // zero quotient block: done
    const block<FpType> e = se.second;                // single remainder block x - s*y
    block<FpType> ycopy = y;
    return ZBCL<FpType>::cons(s, [e, ycopy]() { return twoDivZBCL(e, ycopy); });
}

// singleDivHelper / singleDiv: a ZBCL divided by a single block g.
//
// FCL.hs computes this as infSum over [f_0/g, f_1/g, ...] -- each dividend block
// divided INDEPENDENTLY to a full stream, then summed. That is correct but costs
// O(D^2): term i must be carried down to the output frontier, i.e. D-i blocks, so
// D terms cost D^2/2 block divisions. Measured directly: 2*D^2 twoDivZBCL calls to
// produce D quotient blocks.
//
// Dividing an N-digit number by a ONE-digit divisor is a linear operation -- the
// schoolbook carry. The observation that makes it work here is that the residual of
// f_i/g lands at the scale of f_{i+1}, so the two add and one running remainder
// suffices. Each emitted quotient block then costs O(1) instead of O(D):
//
//     rem  += next dividend block
//     (s,e) = twoDiv(rem.head(), g)      -- one quotient block, exact residual
//     rem   = rem.tail() + e             -- carry
//
// The remainder stays small (1 block on a double host, up to ~6 on float), and is
// capped so a pathological carry cannot grow it without bound.
//
// PENDING BUFFER. The raw quotient blocks are not always 0-overlap: on a double host
// they come out with gaps of k+1 and need nothing, but on float roughly 2 in 60 land
// one bit too close. Rather than renormalising the whole expansion at the end -- a
// pass that is itself superlinear and would dominate what we just made linear -- the
// blocks are folded through a small canonical buffer and emitted with a lookahead, so
// a late carry can still reach a block that has not been handed out yet. That keeps
// the per-block cost O(1) and the output canonical.
//
// This stays a LAZY producer: one quotient block per pull, so an infinite quotient
// (1/3) still streams for as long as the caller asks.
template <typename FpType>
struct singleDivState {
    ZBCL<FpType>   cur;              // dividend blocks not yet consumed
    ZBCL<FpType>   rem;              // running remainder (small, capped)
    ZBCL<FpType>   pending;          // quotient blocks not yet settled (small, canonical)
    block<FpType>  gN;               // normalised divisor
    bool           exhausted{false}; // no further raw quotient blocks
};

// Blocks of lookahead kept before a quotient block is handed out, and the cap on the
// running remainder. Both are small constants: their only job is to bound the work per
// emitted block, and the structures they bound are O(1) in practice.
inline constexpr std::size_t kSingleDivLookahead = 3;
inline constexpr std::size_t kSingleDivCarryCap  = 8;

template <typename FpType>
inline std::optional<block<FpType>> singleDiv_step(singleDivState<FpType>& st) {
    using Z = ZBCL<FpType>;
    for (;;) {
        if (!st.exhausted) {
            if (!st.cur.is_empty()) {
                st.rem = zbcl_from_blocks<FpType>(
                    add(st.rem, Z::singleton(st.cur.head())).take(kSingleDivCarryCap));
                st.cur = st.cur.tail();
            }
            if (st.rem.is_empty()) { st.exhausted = true; continue; }
            block<FpType> x = st.rem.head();
            x.normalise();                              // operands, not results (#1051)
            auto se = block_two_div_rem(x, st.gN);      // (s, e): x/g = s + e/g
            if (!se.first.is_normalised()) { st.exhausted = true; continue; }
            st.rem = zbcl_from_blocks<FpType>(
                add(st.rem.tail(), Z::singleton(se.second)).take(kSingleDivCarryCap));
            st.pending = zbcl_from_blocks<FpType>(
                add(st.pending, Z::singleton(se.first)).take(kSingleDivCarryCap));
            if (st.pending.take(kSingleDivLookahead + 1).size() > kSingleDivLookahead) {
                block<FpType> out = st.pending.head();
                st.pending = st.pending.tail();
                return out;
            }
            continue;
        }
        if (st.pending.is_empty()) return std::nullopt;   // drained
        block<FpType> out = st.pending.head();
        st.pending = st.pending.tail();
        return out;
    }
}

template <typename FpType>
inline ZBCL<FpType> singleDiv(ZBCL<FpType> fs, block<FpType> g) {
    using Z = ZBCL<FpType>;
    if (fs.is_empty() || g.is_zero_block()) return Z{};
    block<FpType> gN = g; gN.normalise();
    auto st = std::make_shared<singleDivState<FpType>>(
        singleDivState<FpType>{ std::move(fs), Z{}, Z{}, gN, false });
    auto loop = std::make_shared<std::function<Z()>>();
    *loop = [st, loop]() -> Z {
        auto nxt = singleDiv_step(*st);
        if (!nxt) return Z{};
        return Z::cons(*nxt, [loop]() { return (*loop)(); });
    };
    auto first = singleDiv_step(*st);
    if (!first) return Z{};
    return Z::cons(*first, [loop]() { return (*loop)(); });
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

    // `depth` is caller-controlled since universal#1371 (it used to be a small
    // host-derived constant), so both of these must survive an absurd argument: at
    // SIZE_MAX, depth + 1 wraps to 0 and would truncate every iterate to nothing, and
    // p <<= 1 wraps to 0 and leaves `p < depth` true forever. Saturating keeps a
    // nonsense depth to a terminating (if useless) call instead of a hang, and is
    // exact for every depth a caller can actually afford to materialise.
    constexpr std::size_t kSizeMax = std::numeric_limits<std::size_t>::max();
    const std::size_t guard = (depth < kSizeMax) ? depth + 1 : kSizeMax;
    std::size_t iters = 1;                       // r0 is ~1 block correct ...
    for (std::size_t p = 1; p < depth; ) {       // ... double to >= depth
        ++iters;
        if (p > kSizeMax / 2) break;             // one more doubling would wrap
        p <<= 1;
    }
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
// Default working depth for a DENSE-divisor quotient, in blocks. Matches the eager
// div()'s default so the streaming and eager APIs agree, and is deliberately NOT
// derived from FpType: the host's exponent range does not bound an expansion's depth
// (universal#1371). Dense division costs roughly linearly in this, so callers with a
// known working precision should pass it explicitly rather than pay for the default.
inline constexpr std::size_t kDenseQuotientDepth = 64;

template <typename FpType>
inline bool is_dense_divisor(const ZBCL<FpType>& gs) {
    const std::vector<block<FpType>> bl = gs.take(4);
    if (bl.size() < 2) return false;                       // single block: long division
    for (const auto& b : bl) if (!b.is_zero_block() && !singleBit(b)) return true;
    return false;                                          // all power-of-two: sparse
}

// div_online(fs, gs, depth): fs / gs. Single-block and sparse divisors use the faithful
// McCleeary long division (div_raw); shiftUpToTwo normalises the divisor's leading
// exponent to >= 1 (scaling BOTH operands, so the quotient is unchanged) for the
// long-division series' convergence. A DENSE multi-block divisor instead uses
// a/b = a * (1/b) with a Newton-Raphson reciprocal (recip_newton): the long-division
// fan-out is correct but cost-explodes for dense divisors (#1061), so we deviate from
// McCleeary 4.2.6 there -- tracked in #1068. 0/gs = 0; gs == 0 is the caller's
// precondition (empty divisor).
//
// `depth` bounds the DENSE path only: the long-division path is genuinely streaming
// and produces blocks for as long as the caller pulls, so passing a depth there would
// be meaningless. The dense path cannot be lazy in the same way -- Newton recomputes
// the whole reciprocal at each depth rather than extending it, so a deeper pull would
// not agree block-for-block with a shallower one -- which is why it takes a budget up
// front. Callers that know their working precision should pass it.
template <typename FpType>
inline ZBCL<FpType> div_online(ZBCL<FpType> fs, ZBCL<FpType> gs,
                               std::size_t depth = kDenseQuotientDepth) {
    if (gs.is_empty()) return ZBCL<FpType>{};   // divide-by-zero: caller's precondition
    if (fs.is_empty()) return ZBCL<FpType>{};   // 0 / gs = 0

    if (is_dense_divisor(gs)) {
        // Refine the Newton reciprocal to the depth the CALLER asked for. This used to
        // be a constant derived from the host's exponent range -- 17 blocks on double,
        // 3 on float -- which silently capped every dense quotient regardless of how
        // many blocks the caller pulled, stalling e.g. a Newton sqrt at ~282 digits
        // (universal#1371). Depth is the caller's budget, not a property of FpType.
        ZBCL<FpType> r = recip_newton(gs, depth);
        return zbcl_truncate(mul_online(std::move(fs), std::move(r)), depth);
    }

    const typename block<FpType>::exp_t lead = gs.head().exponent();
    const typename block<FpType>::exp_t shift = (lead < 1) ? (1 - lead) : 0;   // shiftUpToTwo
    return div_raw(zbcl_shift(std::move(fs), shift), zbcl_shift(std::move(gs), shift));
}

}} // namespace sw::universal
