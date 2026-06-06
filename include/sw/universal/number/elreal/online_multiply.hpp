// online_multiply.hpp: McCleeary LFPERA streaming multiplication (dissertation 4.2.5).
//
// x * y = sum_{i,j} (x_i * y_j). Each block-pair product is the exact 2-block
// block_two_mult (the dissertation's twoMult). Distribute into a lazy list of 2-block
// ZBCLs and fold with the streaming infSum (infsum.hpp). Direct translation of FCL.hs
// Appendix A.4: singleMult / singleMultHelper / mult / infSumMultHelper.
//
// This is the ONLINE replacement for the eager finite-prefix mul() in multiply.hpp; it
// produces output limbs on demand and pulls operand limbs lazily. Phase 1 of #1061.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once

#include <universal/number/elreal/block.hpp>
#include <universal/number/elreal/block_eft.hpp>   // block_two_mult (twoMult)
#include <universal/number/elreal/zbcl.hpp>
#include <universal/number/elreal/series.hpp>
#include <universal/number/elreal/infsum.hpp>       // infsum, infinitesum

namespace sw { namespace universal {

// singleMultHelper f gs: lazy series of the 2-block products [f*g_0, f*g_1, ...].
// FCL.hs: singleMultHelper f (g:gs) = let (s,e) = twoMult f g in [s,e] : singleMultHelper f gs
template <typename FpType>
inline series<FpType> singleMultHelper(const block<FpType>& f, ZBCL<FpType> gs) {
    if (gs.is_empty()) return series<FpType>{};
    auto pr = block_two_mult(f, gs.head());               // (high, low), exact f*g
    ZBCL<FpType> term = ZBCL<FpType>::cons(pr.first, ZBCL<FpType>::singleton(pr.second));
    ZBCL<FpType> rest = gs.tail();
    block<FpType> fcopy = f;
    return series<FpType>::cons(term, [fcopy, rest]() { return singleMultHelper(fcopy, rest); });
}

// singleMult f gs = infiniteSum (singleMultHelper f gs): a single block times a ZBCL.
template <typename FpType>
inline ZBCL<FpType> singleMult(const block<FpType>& f, ZBCL<FpType> gs) {
    if (f.is_zero_block() || gs.is_empty()) return ZBCL<FpType>{};
    return infinitesum(singleMultHelper(f, std::move(gs)));
}

// infSumMultHelper fs gs: lazy series of [singleMult f_0 gs, singleMult f_1 gs, ...].
// FCL.hs: infSumMultHelper (f:fs) (g:gs) = singleMult f (g:gs) : infSumMultHelper fs (g:gs)
template <typename FpType>
inline series<FpType> infSumMultHelper(ZBCL<FpType> fs, ZBCL<FpType> gs) {
    if (fs.is_empty() || gs.is_empty()) return series<FpType>{};
    ZBCL<FpType> partial = singleMult(fs.head(), gs);
    ZBCL<FpType> frest = fs.tail();
    ZBCL<FpType> gsc = gs;
    return series<FpType>::cons(partial, [frest, gsc]() { return infSumMultHelper(frest, gsc); });
}

// mul_online(x, y) = infiniteSum (infSumMultHelper x y): the streaming product.
// (FCL.hs `mult`; the dissertation's `multiply` additionally shiftDown/shiftUp the
// operands into [-1,1] for the infinite-operand convergence precondition -- not needed
// for finite operands, which is the validated case; added later for general reals.)
template <typename FpType>
inline ZBCL<FpType> mul_online(ZBCL<FpType> x, ZBCL<FpType> y) {
    if (x.is_empty() || y.is_empty()) return ZBCL<FpType>{};   // 0 * y = x * 0 = 0
    return infinitesum(infSumMultHelper(std::move(x), std::move(y)));
}

}} // namespace sw::universal
