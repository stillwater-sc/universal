// threeAdd.hpp: McCleeary LFPERA addition algorithm.
//
// This file is a direct translation of FCL.hs (McCleeary 2019 dissertation
// Appendix A.4). The structure deliberately mirrors the Haskell. Names and
// case analysis are preserved verbatim to allow line-by-line cross-checking
// against the dissertation.
//
// Entry points:
//   twoSumRN(a, b)      -- the dissertation's Definition 4.1.9
//   threeAdd(a, b, c)   -- Definition 4.2.1 / FCL.hs threeAdd
//   priestAdd(as, bs)   -- Priest workspace addition (used inside addRec)
//   isSafe(...)         -- safety predicate for committing an output block
//   add(x, y)           -- the public lazy ZBCL combinator
//
// Reference: McCleeary, R. (2019). Lazy Floating Point Exact Real Arithmetic.
//   Ph.D. dissertation, University of Iowa, sections 4.1-4.2.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>

#include <universal/number/elreal/block.hpp>
#include <universal/number/elreal/block_eft.hpp>  // brings UNIVERSAL_ELREAL_EFT_NOINLINE
#include <universal/numerics/error_free_ops.hpp>
#include <universal/number/elreal/zbcl.hpp>

namespace sw { namespace universal {

namespace detail_mccleeary {

// Generic ldexp wrapper. Native float/double via std::ldexp; Universal types
// via ADL (cfloat has ldexp); fallback via promotion through double.
template <typename FpType>
inline FpType ldexp_block(FpType v, int n) {
    if constexpr (std::is_arithmetic_v<FpType>) {
        return std::ldexp(v, n);
    } else {
        using std::ldexp;
        if constexpr (requires(FpType x) { ldexp(x, 0); }) {
            return ldexp(v, n);
        } else {
            return static_cast<FpType>(std::ldexp(static_cast<double>(v), n));
        }
    }
}

// host_two_sum (volatile-protected on double): inline duplicate of the
// double specialisation in block_eft.hpp, so we don't depend on Phase 3's
// same-exp precondition. UNIVERSAL_ELREAL_EFT_NOINLINE (defined in
// block_eft.hpp) maps to the right cross-compiler noinline attribute --
// raw __attribute__((noinline)) would only work on gcc/clang.
template <typename T>
UNIVERSAL_ELREAL_EFT_NOINLINE
inline void host_two_sum(T a, T b, T& s, T& r) {
    s = a + b;
    T bb = s - a;
    r = (a - (s - bb)) + (b - bb);
}

template <>
UNIVERSAL_ELREAL_EFT_NOINLINE
inline void host_two_sum<double>(double a, double b, double& s, double& r) {
    s = sw::universal::two_sum(a, b, r);
}

} // namespace detail_mccleeary

// twoSumRN(a, b): the dissertation's Definition 4.1.9.
//
// Properties on output (b1', b2'):
//   1.  JaK + JbK = Jb1'K + Jb2'K        (value preservation)
//   2.  e1' >= e2' + k                   (0-overlap of outputs)
//   3.  e1' <= max(e1, e2) + 1           (exponent upper bound)
//   4.  e1' >= max(e1, e2) - (k+1)       (exponent lower bound)
//   5.  dominate b1' b2'                 (RN-specific: residual <= ulp/2)
//
// Implementation: align both inputs to max(a.exp, b.exp), perform host EFT,
// pack the results back as blocks at the aligned exp.
template <typename FpType>
inline std::pair<block<FpType>, block<FpType>>
twoSumRN(const block<FpType>& a, const block<FpType>& b) {
    constexpr int k = block<FpType>::k;
    if (a.is_zero_block()) {
        // 0 + b = b, with zero residual at exp k below b's exponent.
        return { b, createZero<FpType>(b.exp - k) };
    }
    if (b.is_zero_block()) {
        return { a, createZero<FpType>(a.exp - k) };
    }
    std::int32_t e_max = std::max(a.exp, b.exp);
    FpType va = (a.exp == e_max) ? a.v
                                 : detail_mccleeary::ldexp_block(a.v, a.exp - e_max);
    FpType vb = (b.exp == e_max) ? b.v
                                 : detail_mccleeary::ldexp_block(b.v, b.exp - e_max);
    FpType s, r;
    detail_mccleeary::host_two_sum(va, vb, s, r);
    return { block<FpType>{s, e_max}, block<FpType>{r, e_max} };
}

// threeAdd(ia, ib, ic): the dissertation's Definition 4.2.1, transcribed
// literally from FCL.hs Appendix A.4:
//
//   threeAdd ia ib ic =
//     let (a:b:c:[]) = sortBy negFloatCompare [ia, ib, ic] in
//     let (s1, e1)   = twoSumRN b c                       in
//     let (temp1, e2) = twoSumRN a s1                     in
//     let (temp2, temp3) = twoSumRN e1 e2                 in
//     let (out1, ttemp2) = twoSumRN temp1 temp2           in
//     let (out2, out3)   = twoSumRN ttemp2 temp3          in
//     (out1, out2, out3)
template <typename FpType>
struct threeAdd_result {
    block<FpType> out1;
    block<FpType> out2;
    block<FpType> out3;
};

template <typename FpType>
inline threeAdd_result<FpType>
threeAdd(const block<FpType>& ia,
         const block<FpType>& ib,
         const block<FpType>& ic) {
    using B = block<FpType>;
    // Sort by exp descending (negFloatCompare in Haskell).
    B v[3] = { ia, ib, ic };
    std::sort(v, v + 3, [](const B& x, const B& y) {
        return x.exponent() > y.exponent();
    });
    const B& a = v[0];
    const B& b = v[1];
    const B& c = v[2];

    auto [s1,    e1]    = twoSumRN(b, c);
    auto [temp1, e2]    = twoSumRN(a, s1);
    auto [temp2, temp3] = twoSumRN(e1, e2);
    auto [out1,  ttemp2] = twoSumRN(temp1, temp2);
    auto [out2,  out3]  = twoSumRN(ttemp2, temp3);
    return { out1, out2, out3 };
}

// removeZeros: drop zero blocks from a DBL_k list.
template <typename FpType>
inline std::vector<block<FpType>>
removeZeros(const std::vector<block<FpType>>& xs) {
    std::vector<block<FpType>> out;
    out.reserve(xs.size());
    for (const auto& b : xs) {
        if (!b.is_zero_block()) out.push_back(b);
    }
    return out;
}

// priestAdd: Priest's renormalisation-based addition for DBL_k lists.
// Translated from FCL.hs priestAddStep1 / priestAddStep2 / priestRenorm.
//
// priestAdd works on FINITE lists. Outputs a DBL_k list whose total value
// equals the sum of the input lists' values.
template <typename FpType> std::vector<block<FpType>>
priestRenorm_pass(std::vector<block<FpType>> xs);
template <typename FpType> std::vector<block<FpType>>
priestRenorm(std::vector<block<FpType>> xs);

template <typename FpType>
inline std::vector<block<FpType>>
priestAddStep2(std::vector<block<FpType>> as,
               std::vector<block<FpType>> bs,
               block<FpType> a, block<FpType> b) {
    using B = block<FpType>;
    // Following FCL.hs priestAddStep2 case-by-case.
    if (as.empty() && bs.empty()) {
        // [] [] a b: return [e, s] where (s, e) = twoSumRN a b
        auto [s, e] = twoSumRN(a, b);
        return { e, s };
    }
    if (bs.empty()) {
        // (na:as) [] a b: let (s, e) = twoSumRN a b in e : priestAddStep2 as [] s na
        B na = as.front();
        std::vector<B> as_rest(as.begin() + 1, as.end());
        auto [s, e] = twoSumRN(a, b);
        std::vector<B> rest = priestAddStep2(as_rest, std::vector<B>{}, s, na);
        std::vector<B> result;
        result.reserve(rest.size() + 1);
        result.push_back(e);
        result.insert(result.end(), rest.begin(), rest.end());
        return result;
    }
    if (as.empty()) {
        // [] (nb:bs) a b: symmetric
        B nb = bs.front();
        std::vector<B> bs_rest(bs.begin() + 1, bs.end());
        auto [s, e] = twoSumRN(a, b);
        std::vector<B> rest = priestAddStep2(std::vector<B>{}, bs_rest, s, nb);
        std::vector<B> result;
        result.reserve(rest.size() + 1);
        result.push_back(e);
        result.insert(result.end(), rest.begin(), rest.end());
        return result;
    }
    // (na:as) (nb:bs) a b
    B na = as.front();
    B nb = bs.front();
    auto [s, e] = twoSumRN(a, b);
    if (expGreater(na, nb)) {
        std::vector<B> bs_rest(bs.begin() + 1, bs.end());
        // priestAddStep2 (na:as) bs s nb
        std::vector<B> rest = priestAddStep2(as, bs_rest, s, nb);
        std::vector<B> result;
        result.reserve(rest.size() + 1);
        result.push_back(e);
        result.insert(result.end(), rest.begin(), rest.end());
        return result;
    } else {
        std::vector<B> as_rest(as.begin() + 1, as.end());
        // priestAddStep2 as (nb:bs) s na
        std::vector<B> rest = priestAddStep2(as_rest, bs, s, na);
        std::vector<B> result;
        result.reserve(rest.size() + 1);
        result.push_back(e);
        result.insert(result.end(), rest.begin(), rest.end());
        return result;
    }
}

template <typename FpType>
inline std::vector<block<FpType>>
priestAddStep1(std::vector<block<FpType>> as,
               std::vector<block<FpType>> bs) {
    using B = block<FpType>;
    if (as.empty()) return bs;
    if (bs.empty()) return as;
    B a = as.front();
    B b = bs.front();
    if (dominate(a, b)) {
        std::vector<B> bs_rest(bs.begin() + 1, bs.end());
        std::vector<B> rest = priestAddStep1(as, bs_rest);
        std::vector<B> result;
        result.reserve(rest.size() + 1);
        result.push_back(b);
        result.insert(result.end(), rest.begin(), rest.end());
        return result;
    }
    if (dominate(b, a)) {
        std::vector<B> as_rest(as.begin() + 1, as.end());
        std::vector<B> rest = priestAddStep1(as_rest, bs);
        std::vector<B> result;
        result.reserve(rest.size() + 1);
        result.push_back(a);
        result.insert(result.end(), rest.begin(), rest.end());
        return result;
    }
    std::vector<B> as_rest(as.begin() + 1, as.end());
    std::vector<B> bs_rest(bs.begin() + 1, bs.end());
    return priestAddStep2(as_rest, bs_rest, a, b);
}

template <typename FpType>
inline std::vector<block<FpType>>
priestAddNonNorm(std::vector<block<FpType>> as,
                 std::vector<block<FpType>> bs) {
    std::reverse(as.begin(), as.end());
    std::reverse(bs.begin(), bs.end());
    auto reversed = priestAddStep1(as, bs);
    std::reverse(reversed.begin(), reversed.end());
    return removeZeros(reversed);
}

template <typename FpType>
inline std::vector<block<FpType>>
sweepUpRec(std::vector<block<FpType>> as, block<FpType> b) {
    using B = block<FpType>;
    if (as.empty()) return { b };
    B a = as.front();
    std::vector<B> as_rest(as.begin() + 1, as.end());
    auto [s, e] = twoSumRN(a, b);
    std::vector<B> rest = sweepUpRec(as_rest, s);
    std::vector<B> result;
    result.reserve(rest.size() + 1);
    result.push_back(e);
    result.insert(result.end(), rest.begin(), rest.end());
    return result;
}

template <typename FpType>
inline std::vector<block<FpType>>
sweepUp(std::vector<block<FpType>> as) {
    if (as.empty()) return {};
    std::reverse(as.begin(), as.end());
    block<FpType> ra = as.front();
    std::vector<block<FpType>> rest(as.begin() + 1, as.end());
    auto result = sweepUpRec(rest, ra);
    std::reverse(result.begin(), result.end());
    return result;
}

template <typename FpType>
inline std::vector<block<FpType>>
sweepDownRec(std::vector<block<FpType>> as, block<FpType> b);

template <typename FpType>
inline std::vector<block<FpType>>
sweepDown(std::vector<block<FpType>> as) {
    using B = block<FpType>;
    if (as.empty()) return {};
    B a = as.front();
    std::vector<B> rest(as.begin() + 1, as.end());
    return sweepDownRec(rest, a);
}

template <typename FpType>
inline std::vector<block<FpType>>
sweepDownRec(std::vector<block<FpType>> as, block<FpType> b) {
    using B = block<FpType>;
    if (as.empty()) return { b };
    B a = as.front();
    std::vector<B> as_rest(as.begin() + 1, as.end());
    auto [s, e] = twoSumRN(a, b);
    std::vector<B> tail = e.is_zero_block() ? sweepDown(as_rest)
                                            : sweepDownRec(as_rest, e);
    std::vector<B> result;
    result.reserve(tail.size() + 1);
    result.push_back(s);
    result.insert(result.end(), tail.begin(), tail.end());
    return result;
}

// priestRenorm_pass: ONE Priest renormalisation pass (sweepUp / recurse /
// sweepDown). A single pass produces a 0-overlap (DBL_k) list for ordinary
// inputs, but for a pool with catastrophic cancellation between nearly-equal
// high-precision values the leading term can collapse below a following limb,
// leaving an adjacent pair closer than k (#1044). priestRenorm() below iterates
// this pass to a 0-overlap fixpoint (the pass is value-preserving and idempotent
// on an already-0-overlap list, and converges in a second pass in practice).
template <typename FpType>
inline std::vector<block<FpType>>
priestRenorm_pass(std::vector<block<FpType>> as) {
    using B = block<FpType>;
    if (as.empty()) return {};
    auto up = sweepUp(as);
    if (up.empty()) return {};
    B f = up.front();
    std::vector<B> fs(up.begin() + 1, up.end());
    auto recursed = priestRenorm_pass(fs);
    auto cleaned = removeZeros(recursed);
    auto down = sweepDown(cleaned);
    std::vector<B> result;
    result.reserve(down.size() + 1);
    result.push_back(f);
    result.insert(result.end(), down.begin(), down.end());
    return result;
}

// priestRenorm: renormalise to a 0-overlap (DBL_k) list.
//
// A single Priest pass is 0-overlap for ordinary inputs, but a pool with
// catastrophic cancellation between nearly-equal high-precision values can
// collapse the leading term below a following limb, leaving an adjacent pair
// closer than k (#1044). For such pools a second pass converges to 0-overlap.
//
// The catch is that this rescue is only correct for a *wide* host. On a narrow
// host (bfloat16, k=8; fp16, k=11) a deep expansion bottoms out at the denormal
// floor, where the two smallest limbs sit closer than k and twoSumRN cannot push
// the error any lower: a second pass cannot reach 0-overlap, it only reshuffles
// and lengthens the list. Worse, *any* change to a narrow-host renorm result
// perturbs the lazy ZBCL structure so that a tail overlap main never forces lands
// where it IS forced, tripping the 0-overlap assertion downstream. The series /
// trig functions that need the #1044 rescue are wide-host only anyway (a narrow
// mantissa cannot hold the expansions), so we gate the rescue on k >= 24 (float
// and wider). For narrow hosts priestRenorm is exactly the single Priest pass --
// byte-identical to the behaviour every merged test was validated against.
template <typename FpType>
inline bool zero_overlap_list(const std::vector<block<FpType>>& r) {
    for (std::size_t i = 0; i + 1 < r.size(); ++i)
        if (!zero_overlap(r[i], r[i + 1])) return false;
    return true;
}

template <typename FpType>
inline std::vector<block<FpType>>
priestRenorm(std::vector<block<FpType>> as) {
    std::vector<block<FpType>> single = priestRenorm_pass(std::move(as));
    if constexpr (block<FpType>::k >= 24) {
        // #1044's signature is a *leading-pair* overlap: catastrophic cancellation
        // collapses the high-order term so it sits closer than k to the next limb.
        // Rescue only that signature with one extra pass. A deep-tail overlap, by
        // contrast, is benign -- a single pass already leaves it where the lazy
        // ZBCL never forces it, exactly as main does -- so we leave those results
        // untouched (no divergence, no extra work) to keep well-conditioned
        // computations identical and fast.
        if (single.size() >= 2 && !zero_overlap(single[0], single[1])) {
            std::vector<block<FpType>> twice = priestRenorm_pass(single);
            if (zero_overlap_list(twice)) return twice; // one extra pass rescued it (#1044)
            // fall through: could not reach 0-overlap; return the single pass (== main)
        }
    }
    return single;                                      // narrow host, benign, or unrescuable
}

template <typename FpType>
inline std::vector<block<FpType>>
priestAdd(std::vector<block<FpType>> as,
          std::vector<block<FpType>> bs) {
    return priestRenorm(priestAddNonNorm(as, bs));
}

// isSafe(out, fs, gs, es, prev): the dissertation's safety predicate.
// Mirrors FCL.hs Appendix A.4 isSafe verbatim.
template <typename FpType>
inline bool
isSafe(const block<FpType>& out,
       const ZBCL<FpType>& fs,
       const ZBCL<FpType>& gs,
       const std::vector<block<FpType>>& es,
       const block<FpType>& prev) {
    using B = block<FpType>;
    // isSafe out [] [] [] prev = not (expGreater prev out)
    if (fs.is_empty() && gs.is_empty() && es.empty()) {
        return !expGreater(prev, out);
    }
    // isSafe out [] [] (e:es) prev = not (expGreater prev out) && (expGreaterBy (getSize e) out e)
    if (fs.is_empty() && gs.is_empty()) {
        const B& e = es.front();
        return !expGreater(prev, out)
            && expGreaterBy(block<FpType>::k, out, e);
    }
    // isSafe out fs [] es prev = isSafe out fs fs es prev
    if (gs.is_empty()) {
        return isSafe(out, fs, fs, es, prev);
    }
    // isSafe out [] gs es prev = isSafe out gs gs es prev
    if (fs.is_empty()) {
        return isSafe(out, gs, gs, es, prev);
    }
    // isSafe out fs gs [] prev = isSafe out fs gs gs prev
    // (gs is non-empty here -- when es is empty, reuse gs as the workspace)
    if (es.empty()) {
        // Convert gs (a ZBCL) into a finite es-like vector by taking its head
        // and continuing recursion. The Haskell re-uses gs as the [a] argument;
        // our isSafe wants std::vector for es, so we materialise just enough.
        std::vector<B> gs_es{ gs.head() };
        return isSafe(out, fs, gs, gs_es, prev);
    }
    // Main case: isSafe out (f:fs) (g:gs) (e:es) prev
    const B& f = fs.head();
    const B& g = gs.head();
    const B& e = es.front();
    constexpr int k = block<FpType>::k;
    return expGreaterBy(2 * k + 3, out, f)
        && expGreaterBy(2 * k + 3, out, g)
        && !expGreater(prev, out)
        && expGreaterBy(k, out, e);
}

// addRec_state holds the streaming state. We pass it by ref through the
// add-step loop and via shared_ptr-captured closures for ZBCL thunks.
template <typename FpType>
struct addRec_state {
    ZBCL<FpType> fs;
    ZBCL<FpType> gs;
    std::vector<block<FpType>> workspace; // [prev, e1, e2, ...] (front-first)
    std::int32_t bound;
    bool initialised; // whether `bound` and workspace have been seeded
};

// Helper to take the head + tail of a workspace vector cheaply.
template <typename T>
inline std::vector<T> tail_vec(const std::vector<T>& xs) {
    if (xs.empty()) return {};
    return std::vector<T>(xs.begin() + 1, xs.end());
}

// Build a finite ZBCL from a 0-overlap block list (front-first). Used to
// re-inject a leftover workspace tail into an operand slot (see addRec_step).
template <typename FpType>
inline ZBCL<FpType> zbcl_of_vec(const std::vector<block<FpType>>& bs) {
    ZBCL<FpType> out{};
    for (std::size_t i = bs.size(); i-- > 0;) out = ZBCL<FpType>::cons(bs[i], out);
    return out;
}

// addRec_step: produce the next emitted block, mutating the state.
// Returns nullopt when the stream terminates.
//
// Mirrors FCL.hs addRec cases. The Haskell function is tail-recursive
// without emitting (the "not isSafe" case), so the C++ wrapper loops
// internally until either an output is produced or termination.
template <typename FpType>
inline std::optional<block<FpType>>
addRec_step(addRec_state<FpType>& st) {
    using B = block<FpType>;

    // Seed: the dissertation's `add` initialises workspace = [s, e] from
    // twoSumRN of the two leading blocks, and bound = max(exp_f, exp_g) + 3.
    if (!st.initialised) {
        if (st.fs.is_empty() && st.gs.is_empty()) return std::nullopt;
        if (st.fs.is_empty()) {
            // add [] gs = gs : we just relay gs as the result.
            B head = st.gs.head();
            st.gs = st.gs.tail();
            return head;
        }
        if (st.gs.is_empty()) {
            B head = st.fs.head();
            st.fs = st.fs.tail();
            return head;
        }
        B f = st.fs.head();
        B g = st.gs.head();
        st.fs = st.fs.tail();
        st.gs = st.gs.tail();
        st.bound = std::max(f.exponent(), g.exponent()) + 3;
        auto [s, e] = twoSumRN(f, g);
        st.workspace = { s, e };
        st.initialised = true;
    }

    constexpr int k = block<FpType>::k;
    // Guard against infinite loops in early development. Each iteration of
    // addRec_step either emits a block (returns) or makes progress in the
    // streaming state (advances st.fs / st.gs or alters st.workspace);
    // sustained non-progress for 1 million iterations indicates a bug, not
    // legitimate non-convergence on a real stream. Fail loudly so the bug
    // surfaces instead of being silently returned as end-of-stream.
    int safety_counter = 1000000;
    while (--safety_counter > 0) {
        // addRec [] [] es _ = es : recurse depleting es
        // addRec fs [] [] _ = fs : finish by streaming fs
        // addRec [] gs [] _ = gs : finish by streaming gs
        // addRec fs [] (e:es) bound = e : addRec fs es [] ...
        // addRec [] gs (e:es) bound = e : addRec gs es [] ...
        // addRec (f:fs) (g:gs) [] bound = twoSumRN; recurse with workspace
        if (st.fs.is_empty() && st.gs.is_empty()) {
            if (st.workspace.empty()) return std::nullopt;
            B e = st.workspace.front();
            st.workspace = tail_vec(st.workspace);
            return e;
        }
        if (st.gs.is_empty() && st.workspace.empty()) {
            B head = st.fs.head();
            st.fs = st.fs.tail();
            return head;
        }
        if (st.fs.is_empty() && st.workspace.empty()) {
            B head = st.gs.head();
            st.gs = st.gs.tail();
            return head;
        }
        if (st.gs.is_empty()) {
            // fs non-empty, gs empty, workspace = (e:es). Haskell:
            //   addRec fs [] (e:es) bound = e : addRec fs es [] (getExp e - k)
            // i.e. emit e, then RE-INJECT the workspace tail `es` as the (empty)
            // gs operand so it keeps merging with fs. Previously this drained the
            // workspace without merging fs against es, emitting fs's blocks after
            // es's even when an fs block belonged between two es blocks -> a
            // non-0-overlap result (#1034).
            B e = st.workspace.front();
            st.gs = zbcl_of_vec(removeZeros(tail_vec(st.workspace)));
            st.workspace.clear();
            st.bound = e.exponent() - k;
            return e;
        }
        if (st.fs.is_empty()) {
            // Symmetric: gs non-empty, fs empty. Re-inject the workspace tail as
            // the (empty) fs operand so it merges with gs (#1034).
            B e = st.workspace.front();
            st.fs = zbcl_of_vec(removeZeros(tail_vec(st.workspace)));
            st.workspace.clear();
            st.bound = e.exponent() - k;
            return e;
        }
        if (st.workspace.empty()) {
            // addRec (f:fs) (g:gs) [] bound = let (s, e) = twoSumRN f g in
            //   addRec fs gs [s, e] bound
            B f = st.fs.head();
            B g = st.gs.head();
            st.fs = st.fs.tail();
            st.gs = st.gs.tail();
            auto [s, e] = twoSumRN(f, g);
            st.workspace = { s, e };
            continue;
        }

        // Main case: workspace = (prev : es), inputs (f:fs) (g:gs)
        B prev = st.workspace.front();
        std::vector<B> es = tail_vec(st.workspace);
        B f = st.fs.head();
        B g = st.gs.head();

        if (st.bound > prev.exponent() + k + 2) {
            // Cancellation path: try to emit a zero at exp = bound.
            B zero = createZero<FpType>(st.bound);
            if (isSafe(zero, st.fs, st.gs, st.workspace, prev)) {
                // emit zero, recurse with bound -= k (st.fs, st.gs unchanged)
                st.bound -= k;
                return zero;
            } else {
                auto ta = threeAdd(prev, f, g);
                st.fs = st.fs.tail();
                st.gs = st.gs.tail();
                std::vector<B> nes = priestAdd(es, std::vector<B>{ ta.out1, ta.out2, ta.out3 });
                st.workspace = std::move(nes);
                continue;
            }
        } else {
            auto ta = threeAdd(prev, f, g);
            st.fs = st.fs.tail();
            st.gs = st.gs.tail();
            std::vector<B> nes = priestAdd(es, std::vector<B>{ ta.out2, ta.out3 });
            if (!isSafe(ta.out1, st.fs, st.gs, nes, prev)) {
                std::vector<B> nnes = priestAdd(std::vector<B>{ ta.out1 }, nes);
                st.workspace = std::move(nnes);
                continue;
            } else {
                st.workspace = std::move(nes);
                st.bound = ta.out1.exponent() - k;
                return ta.out1;
            }
        }
    }
    // Safety counter exhausted without producing an output. This is a
    // BUG, not legitimate end-of-stream. Trip an assertion so the failure
    // surfaces in debug builds and throw in release so callers can't
    // mistake it for a clean termination.
    assert(false && "elreal addRec_step: safety_counter exhausted -- "
                    "non-convergence in the streaming addition state machine. "
                    "Inspect addRec_step state (fs/gs/workspace) at the call site.");
    throw std::runtime_error(
        "sw::universal::addRec_step: safety_counter exhausted; "
        "non-convergence in the streaming addition state machine (bug). "
        "See threeAdd.hpp for diagnosis.");
}

// add(x, y): the dissertation's add (Algorithm 4.2.1) wrapped as a lazy ZBCL.
template <typename FpType>
inline ZBCL<FpType> add(ZBCL<FpType> x, ZBCL<FpType> y) {
    using Z = ZBCL<FpType>;
    auto st = std::make_shared<addRec_state<FpType>>(
        addRec_state<FpType>{std::move(x), std::move(y), {}, 0, false});

    auto loop = std::make_shared<std::function<Z()>>();
    *loop = [st, loop]() -> Z {
        auto nxt = addRec_step(*st);
        if (!nxt) return Z{};
        return Z::cons(*nxt, [loop]() { return (*loop)(); });
    };

    auto first = addRec_step(*st);
    if (!first) return Z{};
    return Z::cons(*first, [loop]() { return (*loop)(); });
}

}} // namespace sw::universal
