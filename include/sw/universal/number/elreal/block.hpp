// block.hpp: McCleeary LFPERA block<FpType> -- the unit of co-list storage.
//
// A block is the dissertation's `blockk = (s, e, bv)` where:
//   s in {-1, +1}: the sign
//   e in Z:        the per-unit exponent in the dissertation's sense
//   bv:            a k-bit significand vector with bv[0] = 1 (normalised) or
//                  all zeros (the zero block)
//
// We pack (s, bv) into a host FpType `v` and carry `e` as an explicit
// wide integer `exp` (block<FpType>::exp_t). The relationship is:
//
//     E(b) = scale_of(v) + b.exp        (non-zero v)
//     E(b) = b.exp                      (zero v)
//
// where scale_of(v) is v's unbiased binary exponent (the integer `e` such
// that |v| in [2^e, 2^(e+1)) for non-zero v). The dissertation's getExp(b)
// returns E(b) above. The dissertation's 0-overlap predicate compares E(.)
// of consecutive blocks: `E(b_n) >= E(b_{n+1}) + k`.
//
// Why a WIDE exponent (integer<256>, not int32):
//   McCleeary's exponents live in Z (Haskell `Integer`, unbounded). The
//   streaming division (online_divide.hpp, dissertation 4.2.6) recurses with
//   newdiv = g0*divisor each level, so the divisor's magnitude -- hence its
//   exponent -- roughly DOUBLES per level. A 32- or 64-bit exponent overflows
//   after only ~11 / ~59 levels, capping multi-block division precision. We
//   follow the dissertation and carry the exponent in the library's fixed-size
//   integer<256, uint32_t> -- a trivially-copyable POD (so block stays
//   hardware-shareable per #925) that never overflows for any realizable host
//   (its ~2^255 range outlasts even a quad host's denormal floor by orders of
//   magnitude). This is the spirit of Ryan's unbounded exponent realized
//   without giving up the trivial block layout. See online_divide.hpp.
//
// Note on the historical exp_offset:
//   Earlier drafts of this file used `exp_offset` as a coarse multiplier
//   (units of 2^E_FpType) intended to escape FpType's hardware exponent
//   range. McCleeary's algorithms require fine-grained per-unit exponents
//   in Z, so the field has been renamed `exp` and the multiplier removed.
//   The combined exponent is now `scale_of_v() + exp` directly.
//
// References:
//   McCleeary, R. (2019). Lazy Floating Point Exact Real Arithmetic.
//   Ph.D. dissertation, University of Iowa. Chapter 4.1.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once

#include <cmath>
#include <cstdint>
#include <limits>
#include <type_traits>

#include <universal/number/cfloat/cfloat_fwd.hpp>
#include <universal/number/bfloat16/bfloat16_fwd.hpp>
#include <universal/number/integer/integer.hpp>   // exp_t: wide, trivially-copyable exponent

namespace sw { namespace universal {

// Detect a Universal type that provides .sign() and .scale() member accessors.
// All Universal floating-point wrappers (cfloat, bfloat16, half, posit, ...)
// expose these; native float/double/long double do not.
template <typename T, typename = void>
struct has_universal_fp_api : std::false_type {};

template <typename T>
struct has_universal_fp_api<
    T,
    std::void_t<
        decltype(std::declval<const T&>().sign()),
        decltype(std::declval<const T&>().scale())
    >
> : std::true_type {};

template <typename T>
inline constexpr bool has_universal_fp_api_v = has_universal_fp_api<T>::value;

namespace detail_block {

// Scale by a power of two, exactly, without forming the multiplier.
template <typename FpType>
constexpr FpType ldexp_block(FpType v, int n) {
    if constexpr (std::is_floating_point_v<FpType>) {
        return std::ldexp(v, n);
    } else {
        using std::ldexp;
        if constexpr (requires(FpType x) { ldexp(x, 0); }) return ldexp(v, n);
        else return static_cast<FpType>(std::ldexp(static_cast<double>(v), n));
    }
}

}  // namespace detail_block

// block<FpType>: McCleeary's (sign, exp, bv) packed into FpType's value field
// plus an explicit wide exponent (exp_t).
//
// Trivial layout: no in-class member initialisers. Use {v, exp} literal
// construction to initialise. Default construction leaves members
// indeterminate (consistent with Universal's other trivial number types).
// exp_t (integer<256>) is itself a trivially-copyable POD, so block remains
// trivially copyable / destructible -- the #925 hardware-shareable invariant.
template <typename FpType>
struct block {
    using fp_type = FpType;

    // exp_t: the per-block exponent type. A fixed-size, trivially-copyable
    // integer wide enough that the division recursion (which doubles the
    // exponent per level) never overflows for any realizable host. See the
    // file header for the rationale.
    using exp_t = integer<256, std::uint32_t>;

    // k = total significand-vector width per the dissertation. For IEEE-style
    // formats this includes the hidden bit. Tracks numeric_limits::digits
    // (Universal's bfloat16 reports 7 -- a minor inconsistency the
    // dissertation k would put at 8; not worth special-casing here).
    static constexpr int k = std::numeric_limits<FpType>::digits;

    FpType        v;
    exp_t         exp;

    // sign: -1 for negative, +1 for non-negative. Sign of +0.0 and -0.0 follow
    // signbit / Universal's .sign() respectively.
    constexpr int sign() const noexcept {
        if constexpr (has_universal_fp_api_v<FpType>) {
            return v.sign() ? -1 : 1;
        } else {
            return std::signbit(v) ? -1 : 1;
        }
    }

    // scale_of_v: unbiased binary exponent of `v` (the integer e such that
    // |v| in [2^e, 2^(e+1)) for non-zero v; defined to 0 for v == 0).
    constexpr int scale_of_v() const noexcept {
        if constexpr (has_universal_fp_api_v<FpType>) {
            return v.scale();
        } else {
            if (v == FpType{0}) return 0;
            return std::ilogb(v);
        }
    }

    // Combined exponent E(b) per the dissertation's getExp.
    constexpr exp_t exponent() const noexcept {
        if (is_zero_block()) return exp;
        return exp_t(scale_of_v()) + exp;
    }

    constexpr bool is_zero_block() const noexcept {
        return v == FpType{0};
    }

    // needs_scale_normalisation: every host. There is no wide host.
    //
    // This was gated, first on k >= 24 and then on min_exponent > -500, on the
    // theory that a wide-exponent host never approaches its subnormal wall. The
    // measurement said otherwise and the gate was wrong twice.
    //
    // double's ceiling was 307 decimal digits, and 2^-1021 -- its min_exponent --
    // is 1e-307. That is not a coincidence: double was sitting exactly on its wall,
    // and the gate preserved it there. The earlier reading that double had "33 bits
    // of headroom" at 2^-988 missed that 33 < k = 53, i.e. less than a single block.
    //
    // The inconsistency it produced is what exposed it: normalised float reached
    // 319 digits while unnormalised double stopped at 307, even though double's
    // exponent range is eight times wider. Two hosts limited by different
    // mechanisms cannot be compared, and the ordering was physically impossible.
    //
    // Normalised, double reaches the 320-digit reference at 22 blocks and its
    // representation extends past its own wall -- trailing exponent -1097 at depth
    // 16, -2793 (about 1e-841) at depth 48. Every host is now limited only by block
    // count, at k * log10(2) digits per block, which is the property the type is
    // supposed to have.
    //
    // The cost is that double is no longer bit-identical to the pre-change library.
    // That was never the right thing to preserve: the old bits were a value cut off
    // at the host's exponent wall, and the new ones carry further. It is a change in
    // reach, not a change in what the retained digits say.
    static constexpr bool needs_scale_normalisation = true;   // EXPERIMENT: all hosts

    // eft_scale_bias: how far ABOVE [1,2) an EFT's operands must sit so its residual
    // stays normal on this host.
    //
    // two_sum's residual can land a full 2k binades below the leading operand: the
    // smaller operand is aligned up to k down, and the residual is up to another k
    // below that. A host therefore needs 2k binades of room beneath a normalised
    // operand. Measured need against available:
    //
    //     half      k=11   need 22, has   14   -> short by 8
    //     bfloat16  k= 7   need 14, has  126   -> fine
    //     float     k=24   need 48, has  126   -> fine
    //     double    k=53   need 106, has 1022  -> fine
    //
    // half is the only host that comes up short, and the consequence was real:
    // add() was inexact on it (5 in 2000 random multi-block additions), which broke
    // the ZBCL exactness invariant and capped e at 19 digits no matter the depth
    // (universal#1363).
    //
    // The fix borrows from the other end. half has 16 binades of unused headroom
    // ABOVE 1.0, and a block's scale lives in its wide exponent, so shifting v up
    // and exp down by the same amount is exact and leaves exponent() unchanged. The
    // EFT then runs with room beneath it. Hosts with no shortfall get 0 and the
    // whole thing compiles out.
    static constexpr int eft_scale_bias() {
        constexpr int haveBelow = -(std::numeric_limits<FpType>::min_exponent - 1);
        constexpr int shortfall = 2 * k - haveBelow;
        if constexpr (shortfall <= 0) return 0;
        else {
            // leave 2 binades so the sum's carry cannot overflow the host
            constexpr int headroom = std::numeric_limits<FpType>::max_exponent - 2;
            return shortfall < headroom ? shortfall : headroom;
        }
    }

    // bias_for_eft(): shift v up (and exp down) by eft_scale_bias(). Exact -- a pure
    // exponent move -- and exponent() is invariant, so the block's value is unchanged.
    constexpr block& bias_for_eft() noexcept {
        if constexpr (eft_scale_bias() == 0) return *this;
        else {
            if (is_zero_block()) return *this;
            v = detail_block::ldexp_block(v, eft_scale_bias());
            exp = exp - exp_t(eft_scale_bias());
            return *this;
        }
    }

    // normalise(): rescale `v` into [1,2) in magnitude, folding the scale it was
    // carrying into `exp`. E(b) = scale_of_v() + exp is invariant, so the block's
    // value does not change; only its split between the two fields moves. Exact --
    // a pure exponent shift -- and it cannot overflow, since exp is unbounded.
    //
    // This is McCleeary's own representation: his block is (s, e, bv) with bv a
    // normalised significand and e a separate exponent in Z. Letting the host FpType
    // carry the scale as well is what makes the host's exponent range binding.
    //
    // Call this on OPERANDS, before the arithmetic -- not on results. An EFT that
    // runs at the operands' natural scale has already lost bits to the subnormal
    // range by the time it returns, and normalising its outputs cannot put them
    // back. See docs/design/elreal-narrow-host-blocks.md.
    constexpr block& normalise() noexcept {
        if constexpr (!needs_scale_normalisation) {
            return *this;                          // wide host: nothing to gain
        } else {
            if (is_zero_block()) return *this;
            const int sc = scale_of_v();
            if (sc == 0) return *this;             // already in [1,2)
            v = detail_block::ldexp_block(v, -sc);
            exp = exp + exp_t(sc);
            return *this;
        }
    }

    // is_normalised(): true iff `v` is a finite, non-zero, non-subnormal value.
    // For IEEE-style FpTypes this is equivalent to the hidden (leading)
    // significand bit being set, which is the McCleeary block invariant.
    // Subnormals fail this predicate; McCleeary blocks must avoid them
    // because 0-overlap accounting assumes the leading bit is set.
    constexpr bool is_normalised() const noexcept {
        if (is_zero_block()) return false;
        if constexpr (has_universal_fp_api_v<FpType>) {
            if constexpr (requires(const FpType& x) { x.isnormal(); }) {
                return v.isnormal();
            } else {
                FpType abs_v = v.sign() ? -v : v;
                return abs_v >= std::numeric_limits<FpType>::min();
            }
        } else {
            return std::fpclassify(v) == FP_NORMAL;
        }
    }

    // value_as<T>(): for testing only. Combines v and exp into a T value via
    // ldexp. Requires the combined exponent to fit in T's range.
    template <typename T = double>
    T value_as() const noexcept {
        static_assert(std::is_floating_point_v<T>,
                      "value_as<T>() requires a native floating-point T");
        T base = static_cast<T>(v);
        if (exp == 0) return base;
        return std::ldexp(base, static_cast<int>(exp));
    }
};

// createZero<FpType>(exp): the dissertation's createZero(k, e). Block size k
// is fixed by FpType; the returned block has v=0 and the requested exponent.
template <typename FpType>
constexpr block<FpType> createZero(typename block<FpType>::exp_t e) noexcept {
    return block<FpType>{ FpType{0}, e };
}

// singleBit(b): bv[0]=1 and all other bits of bv are 0. For our FpType-packed
// representation this means |v| equals exactly its leading-bit power-of-two,
// i.e. |v| = 2^scale_of_v(v). The IEEE rep then has all explicit fraction
// bits zero.
template <typename FpType>
constexpr bool singleBit(const block<FpType>& b) noexcept {
    if (b.is_zero_block()) return false;
    FpType abs_v = b.sign() < 0 ? -b.v : b.v;
    // abs_v is exactly a power of two iff it equals 2^scale_of_v(v).
    // ldexp(1, scale_of_v) gives that power; compare with abs_v.
    using T = std::conditional_t<std::is_floating_point_v<FpType>, FpType, double>;
    T abs_v_native = static_cast<T>(abs_v);
    T pow = std::ldexp(T{1}, b.scale_of_v());
    return abs_v_native == pow;
}

// dominate(b1, b2): the dissertation's dominate. True iff
//   e1 > e2 + k  OR  (e1 = e2 + k AND singleBit(b2))
//
// Per the Floating typeclass definition in the appendix the operational
// version is `let (s, e) = twoSumRN(a, b) in floatEq(a, s)`. We use the
// formal predicate here; the operational version is equivalent for properly
// constructed blocks under round-to-nearest twoSum.
template <typename FpType>
constexpr bool dominate(const block<FpType>& b1, const block<FpType>& b2) noexcept {
    if (b2.is_zero_block()) return true; // anything dominates the zero block
    if (b1.is_zero_block()) return false; // zero block dominates nothing
    constexpr int k = block<FpType>::k;
    typename block<FpType>::exp_t e1 = b1.exponent();
    typename block<FpType>::exp_t e2 = b2.exponent();
    if (e1 > e2 + k) return true;
    if (e1 == e2 + k && singleBit(b2)) return true;
    return false;
}

// expGreater(a, b): getExp(a) > getExp(b).
template <typename FpType>
constexpr bool expGreater(const block<FpType>& a, const block<FpType>& b) noexcept {
    return a.exponent() > b.exponent();
}

// expGreaterBy(n, a, b): getExp(a) >= getExp(b) + n  (per FCL.hs's usage;
// the Haskell function is `expGreaterBy n a b`, returning a's exponent is at
// LEAST n above b's).
template <typename FpType>
constexpr bool expGreaterBy(std::int32_t n,
                            const block<FpType>& a,
                            const block<FpType>& b) noexcept {
    return a.exponent() >= b.exponent() + n;
}

// zero_overlap(b1, b2): the McCleeary 0-overlap predicate. True iff
// E(b1) >= E(b2) + k. Slightly weaker than dominate (no singleBit
// requirement on the boundary case).
template <typename FpType>
constexpr bool zero_overlap(const block<FpType>& b1, const block<FpType>& b2) noexcept {
    if (b1.is_zero_block() || b2.is_zero_block()) return true;
    return b1.exponent() >= b2.exponent() + block<FpType>::k;
}

}} // namespace sw::universal
