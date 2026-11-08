// exponent.hpp: McCleeary LFPERA exp / log / pow on ZBCL<FpType> (Phase 7.3, #931).
//
// exp(x): argument reduction exp(x) = 2^k * exp(x - k*ln2), |x - k*ln2| <= ln2/2, then
//         the Taylor series sum x^n/n!, then an exact scale by 2^k. (It used to halve
//         the argument and square the result up to 64 times, which made exp(40) take
//         most of a minute at the default precision, #2003.)
// log(x): range reduction x = m * 2^e with m in [1,2); log = e*ln2 + ln(m), and
//         ln(m) = 2*artanh((m-1)/(m+1)) (|u| <= 1/3, fast).
// pow(x,y): integer y (decided from the stream) with |y| <= 64 -> binary powering;
//         else exp(y*log(|x|)), with the sign of x for odd integer y.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once

#include <cmath>
#include <cstddef>
#include <vector>

#include <universal/number/elreal/block.hpp>
#include <universal/number/elreal/zbcl.hpp>
#include <universal/number/elreal/zbcl_helpers.hpp>
#include <universal/number/elreal/threeAdd.hpp>     // add
#include <universal/number/elreal/negate.hpp>
#include <universal/number/elreal/multiply.hpp>     // mul, mul_scalar
#include <universal/number/elreal/divide.hpp>       // div
#include <universal/number/elreal/series.hpp>
#include <universal/number/elreal/sum.hpp>
#include <universal/number/elreal/math/constants.hpp>   // ln2_zbcl, detail::odd_power_series
#include <universal/number/elreal/stream_utilities.hpp> // first_nonzero_block, from_integer, is_integer

namespace sw { namespace universal {

namespace detail {

// exp_term_stream: the lazy Taylor term co-list of exp -- [1, xr, xr^2/2!, ...],
// term_{n} = term_{n-1} * xr / n. Online realisation (#1061 Phase 3b): each next
// term is materialised only to its significance window (take_while_above, from
// constants.hpp), so the deep tail is nearly free and the lazy mul/div nesting is
// broken. Folded by the streaming infSum in exp() below.
template <typename FpType>
inline series<FpType> exp_term_stream(ZBCL<FpType> term, ZBCL<FpType> xr,
                                      double n, int floor_exp) {
    if (term.is_empty() || static_cast<int>(term.head().exponent()) < floor_exp)
        return series<FpType>{};
    ZBCL<FpType> next = take_while_above(
        div_online(mul_online(term, xr), from_native<FpType>(n)), floor_exp);
    return series<FpType>::cons(term, [next, xr, n, floor_exp]() {
        return exp_term_stream(next, xr, n + 1.0, floor_exp);
    });
}

} // namespace detail

// exp(x, depth): e^x refined to ~depth blocks. The Taylor series is summed online
// (streaming infSum over a lazy, significance-windowed term co-list, #1061 Phase 3b),
// so high depth is no longer the steep cost it was. depth=4 (~200 bits / ~60 digits)
// remains the default; pass a larger depth for more precision.
template <typename FpType>
inline ZBCL<FpType> exp(ZBCL<FpType> x, std::size_t depth = 4) {
    using B = block<FpType>;
    const auto lead = detail::first_nonzero_block(x, depth + 1);
    if (!lead.found) return from_native<FpType>(1.0);      // exp(0) = 1
    x = lead.rest;                                         // skip leading zero blocks

    // Reduce by multiples of ln2: exp(x) = 2^k * exp(x - k*ln2). ln2 carries enough guard
    // blocks that k*ln2 keeps the working precision for any k below 2^62; the elreal
    // facade maps |x| >= 2^62 to inf or 0 before it gets here.
    double xa = to_double_approx(x, 2);
    long long k = 0;
    ZBCL<FpType> xr = x;
    if (std::fabs(xa) > 0.5 && std::fabs(xa) < 0x1p62) {
        k = static_cast<long long>(std::nearbyint(xa / 0.693147180559945309417));
        const std::size_t guard = 64 / static_cast<std::size_t>(B::k) + 2;
        const unsigned long long kmag = k < 0 ? 0ull - static_cast<unsigned long long>(k) : static_cast<unsigned long long>(k);
        const ZBCL<FpType> kln2 = mul(detail::from_integer<FpType>(kmag, k < 0), ln2_zbcl<FpType>(depth + guard), depth + guard);
        const auto rlead = detail::first_nonzero_block(add(x, negate(kln2)), depth + guard + 1);
        if (!rlead.found) return mul_scalar(B{ static_cast<FpType>(1.0), typename B::exp_t(k) }, from_native<FpType>(1.0), depth);
        xr = rlead.rest;
        xa = to_double_approx(xr, 2);
    }

    // Halving, for the |x| >= 2^62 case only: the reduction above leaves |xr| <= 0.35.
    int r = 0;
    const B half{ static_cast<FpType>(0.5), 0 };
    while (std::abs(xa) > 0.5 && r < 64) {
        xr = mul_scalar(half, xr, depth);
        xa *= 0.5;
        ++r;
    }

    // Taylor: sum_{n>=0} xr^n / n!, term_n = term_{n-1} * xr / n. Each term divides by
    // the integer n EXACTLY (not by a host-double 1.0/n, which capped the series at ~17
    // digits, #1058/#1061 Phase 3a) and the whole series is summed online (#1061 Phase 3b).
    const int floor_exp = -static_cast<int>(depth) * block<FpType>::k - 8;
    ZBCL<FpType> result =
        infsum(detail::exp_term_stream(from_native<FpType>(1.0), xr, 1.0, floor_exp));

    // Reconstruction: square r times, then scale by 2^k exactly.
    for (int i = 0; i < r; ++i) result = mul(result, result, depth);
    if (k != 0) result = mul_scalar(B{ static_cast<FpType>(1.0), typename B::exp_t(k) }, result, depth);
    return result;
}

// log(x, depth): natural logarithm of x > 0; x <= 0 -> empty (undefined). Leading zero
// blocks are skipped: the range reduction reads the scale of the leading block (#2003).
template <typename FpType>
inline ZBCL<FpType> log(ZBCL<FpType> x, std::size_t depth = 4) {
    using B = block<FpType>;
    const auto lead = detail::first_nonzero_block(x, depth + 1);
    if (!lead.found || lead.blk.sign() < 0) return ZBCL<FpType>{};  // log(<=0) undefined
    x = lead.rest;

    // Range reduction: x = m * 2^e with m in [1,2). e is the binary scale of the
    // leading block; m = x * 2^-e.
    const int e = static_cast<int>(x.head().exponent());
    ZBCL<FpType> m = mul_scalar(B{ static_cast<FpType>(1.0), -e }, x, depth);   // m = x / 2^e

    // ln(m) = 2 * artanh(u), u = (m-1)/(m+1), |u| <= 1/3.
    ZBCL<FpType> one = from_native<FpType>(1.0);
    ZBCL<FpType> u = div(add(m, negate(one)), add(m, one), depth);
    ZBCL<FpType> lnm = mul_scalar(B{ static_cast<FpType>(2.0), 0 },
                                  detail::odd_power_series(u, false, depth), depth);

    // log(x) = e*ln2 + ln(m).
    if (e == 0) return lnm;
    ZBCL<FpType> eln2 = mul_scalar(B{ static_cast<FpType>(static_cast<double>(e)), 0 },
                                   ln2_zbcl<FpType>(depth), depth);
    return add(eln2, lnm);
}

// pow(x, y, depth): x^y.
//   - y = 0: 1, for every x.
//   - y an integer, decided from the stream (#2003: 3 + 2^-200 rounds to 3.0 in a double,
//     and pow(2, 3 + 2^-200) came out as exactly 8), with |y| <= 64: binary powering of |x|,
//     inverted for negative y.
//   - otherwise exp(y * log(|x|)).
//   - negative x: defined for integer y only, with the sign of x when y is odd. pow(-2, 100)
//     and pow(-2, -1) used to be 1, because log of a negative is the empty stream.
//   - x = 0: 0 for y > 0. A stream holds neither inf (y < 0) nor NaN (negative x with a
//     non-integer y); both come out empty here, and the elreal facade returns inf and NaN.
template <typename FpType>
inline ZBCL<FpType> pow(ZBCL<FpType> x, ZBCL<FpType> y, std::size_t depth = 4) {
    if (!detail::first_nonzero_block(y, depth + 1).found) return from_native<FpType>(1.0);   // x^0 = 1
    const auto xlead = detail::first_nonzero_block(x, depth + 1);
    if (!xlead.found) return ZBCL<FpType>{};                                                  // 0^y
    bool odd = false;
    const bool yint = detail::is_integer(y, depth + 1, odd);
    const bool xneg = xlead.blk.sign() < 0;
    if (xneg && !yint) return ZBCL<FpType>{};                                                 // undefined
    const ZBCL<FpType> ax = xneg ? negate(xlead.rest) : xlead.rest;
    const double yd = to_double_approx(y, depth + 1);
    ZBCL<FpType> result;
    if (yint && std::fabs(yd) <= 64.0) {
        unsigned n = static_cast<unsigned>(std::fabs(std::nearbyint(yd)));
        ZBCL<FpType> base = ax, acc{};
        bool have = false;
        while (n != 0) {
            if (n & 1u) { acc = have ? mul(acc, base, depth) : base; have = true; }
            n >>= 1;
            if (n != 0) base = mul(base, base, depth);
        }
        result = (yd < 0.0) ? div(from_native<FpType>(1.0), acc, depth) : acc;
    }
    else {
        result = exp(mul(y, log(ax, depth), depth), depth);
    }
    return (xneg && odd) ? negate(result) : result;
}

}} // namespace sw::universal
