// exponent.hpp: McCleeary LFPERA exp / log / pow on ZBCL<FpType> (Phase 7.3, #931).
//
// exp(x): argument reduction exp(x) = exp(x/2^r)^(2^r) with |x/2^r| <= 0.5, then
//         the Taylor series sum x^n/n!, then square r times.
// log(x): range reduction x = m * 2^e with m in [1,2); log = e*ln2 + ln(m), and
//         ln(m) = 2*artanh((m-1)/(m+1)) (|u| <= 1/3, fast).
// pow(x,y): small non-negative integer y -> repeated multiply; else exp(y*log(x)).
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
    if (x.is_empty()) return from_native<FpType>(1.0);     // exp(0) = 1

    // Argument reduction to |xr| <= 0.5 by halving.
    double xa = to_double_approx(x, 2);
    int r = 0;
    ZBCL<FpType> xr = x;
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

    // Reconstruction: square r times.
    for (int i = 0; i < r; ++i) result = mul(result, result, depth);
    return result;
}

// log(x, depth): natural logarithm of x > 0; x <= 0 -> empty (undefined).
template <typename FpType>
inline ZBCL<FpType> log(ZBCL<FpType> x, std::size_t depth = 4) {
    using B = block<FpType>;
    if (x.is_empty() || x.head().sign() < 0) return ZBCL<FpType>{};  // log(<=0) undefined

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

// pow(x, y, depth): x^y. Small non-negative integer y -> repeated multiply
// (exact for representable bases); otherwise exp(y * log(x)).
template <typename FpType>
inline ZBCL<FpType> pow(ZBCL<FpType> x, ZBCL<FpType> y, std::size_t depth = 4) {
    const double yd = y.is_empty() ? 0.0 : to_double_approx(y, 4);
    if (yd == 0.0) return from_native<FpType>(1.0);        // x^0 = 1
    if (std::floor(yd) == yd && yd > 0.0 && yd <= 64.0) {  // small positive integer exponent
        ZBCL<FpType> result = x;
        for (int i = 1; i < static_cast<int>(yd); ++i) result = mul(result, x, depth);
        return result;
    }
    return exp(mul(y, log(x, depth), depth), depth);
}

}} // namespace sw::universal
