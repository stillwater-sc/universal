#pragma once
// mathlib.hpp: the elreal-class math facade.
//
// The math/*.hpp headers implement LFPERA sqrt / exp / log / trig / hyperbolic
// and the named constants on the raw lazy ZBCL<FpType> co-list. This facade
// lifts them to the class elreal so they participate in plug-in/templated kernels
// like every other Universal number system's mathlib.
//
// Precision policy (matches the lazy operators in elreal_impl.hpp):
//   - unary functions refine to the operand's runtime precision()
//   - binary functions refine to the deeper of the two operands
//   - named constants refine to the scoped default precision and carry it forward
// All results stay lazy: the wrapped ZBCL is only forced at a boundary
// (conversion / comparison / I/O), exactly as for the arithmetic operators.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cmath>
#include <cstddef>
#include <limits>

#include <universal/number/elreal/elreal_fwd.hpp>
#include <universal/number/elreal/elreal_impl.hpp>
#include <universal/number/elreal/math/sqrt.hpp>
#include <universal/number/elreal/math/hypot.hpp>
#include <universal/number/elreal/math/exponent.hpp>
#include <universal/number/elreal/math/hyperbolic.hpp>
#include <universal/number/elreal/math/trigonometry.hpp>
#include <universal/number/elreal/math/constants.hpp>
#include <universal/number/elreal/stream_utilities.hpp>

namespace sw { namespace universal {

// ---------------------------------------------------------------------------
// special values. The stream functions cannot hold NaN or inf, and they read an empty
// stream as zero, so the facade settles non-finite and out-of-domain arguments first, by
// C99 Annex F. Before #2003 they went straight to the series: sqrt(-4), log(0), exp(inf)
// and pow(-2, 0.5) came out as 0 or 1. elreal has one, unsigned zero, so a result that C99
// signs (log(-0) = -inf, pow(-0, -1) = -inf) takes the sign of +0.
// ---------------------------------------------------------------------------
namespace detail {
template <typename FpType>
inline elreal<FpType> elreal_special(double v, std::size_t d) { elreal<FpType> r(v); r.precision(d); return r; }
template <typename FpType>
inline elreal<FpType> elreal_nan(std::size_t d) { return elreal_special<FpType>(std::numeric_limits<double>::quiet_NaN(), d); }
template <typename FpType>
inline elreal<FpType> elreal_inf(int sign, std::size_t d) { return elreal_special<FpType>(sign < 0 ? -std::numeric_limits<double>::infinity() : std::numeric_limits<double>::infinity(), d); }
// |x| compared with 1, within the precision window: -1, 0 or +1
template <typename FpType>
inline int compare_magnitude_with_one(const elreal<FpType>& x) {
    const elreal<FpType> one(1.0);
    const elreal<FpType> ax = abs(x);
    return ax < one ? -1 : (ax > one ? 1 : 0);
}
// the exp argument beyond which 2^k no longer fits the reduction (|x| >= 2^62): inf or 0
inline constexpr double kExpReductionLimit = 4611686018427387904.0;   // 2^62
}  // namespace detail

// ---------------------------------------------------------------------------
// unary functions -- refine to the operand's runtime precision.
// Each forwards to the ZBCL overload of the same name (resolved by argument
// type: canonical() yields a ZBCL<FpType>, so the two-argument ZBCL overload is
// selected, never this single-argument elreal one). canonical() hands the stream over from
// its first nonzero block, and a value that is zero to precision as the empty stream: a
// cancelled zero is a stream of zero blocks, and log and sin searched it for a leading
// digit without end (#2003).
// ---------------------------------------------------------------------------
template <typename FpType> inline elreal<FpType> sqrt(const elreal<FpType>& x) {
    const std::size_t d = x.precision();
    if (x.isnan() || (!x.iszero() && x.sign() < 0)) return detail::elreal_nan<FpType>(d);    // sqrt(<0) = NaN
    if (x.isinf()) return detail::elreal_inf<FpType>(1, d);
    return elreal<FpType>(sqrt(x.canonical(), d), d);
}
template <typename FpType> inline elreal<FpType> exp(const elreal<FpType>& x) {
    const std::size_t d = x.precision();
    if (x.isnan()) return x;
    if (x.isinf()) return x.sign() < 0 ? detail::elreal_special<FpType>(0.0, d) : x;         // exp(-inf) = 0
    const double xa = double(x);
    if (std::fabs(xa) >= detail::kExpReductionLimit)                                          // beyond 2^(2^62 * 1.44)
        return xa < 0.0 ? detail::elreal_special<FpType>(0.0, d) : detail::elreal_inf<FpType>(1, d);
    return elreal<FpType>(exp(x.canonical(), d), d);
}
template <typename FpType> inline elreal<FpType> log(const elreal<FpType>& x) {
    const std::size_t d = x.precision();
    if (x.isnan()) return x;
    if (x.iszero()) return detail::elreal_inf<FpType>(-1, d);                                 // log(0) = -inf
    if (x.sign() < 0) return detail::elreal_nan<FpType>(d);                                   // log(<0), log(-inf)
    if (x.isinf()) return x;                                                                  // log(+inf) = +inf
    return elreal<FpType>(log(x.canonical(), d), d);
}
// sin, cos, tan: NaN for NaN and +-inf
template <typename FpType> inline elreal<FpType> sin(const elreal<FpType>& x) {
    const std::size_t d = x.precision();
    if (!x.isfinite()) return detail::elreal_nan<FpType>(d);
    return elreal<FpType>(sin(x.canonical(), d), d);
}
template <typename FpType> inline elreal<FpType> cos(const elreal<FpType>& x) {
    const std::size_t d = x.precision();
    if (!x.isfinite()) return detail::elreal_nan<FpType>(d);
    return elreal<FpType>(cos(x.canonical(), d), d);
}
template <typename FpType> inline elreal<FpType> tan(const elreal<FpType>& x) {
    const std::size_t d = x.precision();
    if (!x.isfinite()) return detail::elreal_nan<FpType>(d);
    return elreal<FpType>(tan(x.canonical(), d), d);
}
// asin, acos: NaN outside [-1, 1]
template <typename FpType> inline elreal<FpType> asin(const elreal<FpType>& x) {
    const std::size_t d = x.precision();
    if (!x.isfinite() || detail::compare_magnitude_with_one(x) > 0) return detail::elreal_nan<FpType>(d);
    return elreal<FpType>(asin(x.canonical(), d), d);
}
template <typename FpType> inline elreal<FpType> acos(const elreal<FpType>& x) {
    const std::size_t d = x.precision();
    if (!x.isfinite() || detail::compare_magnitude_with_one(x) > 0) return detail::elreal_nan<FpType>(d);
    return elreal<FpType>(acos(x.canonical(), d), d);
}
template <typename FpType> inline elreal<FpType> atan(const elreal<FpType>& x) {
    const std::size_t d = x.precision();
    if (x.isnan()) return x;
    if (x.isinf()) {                                                                          // atan(+-inf) = +-pi/2
        const elreal<FpType> halfPi = elreal<FpType>(pi_zbcl<FpType>(d), d) * elreal<FpType>(0.5);
        return x.sign() < 0 ? -halfPi : halfPi;
    }
    return elreal<FpType>(atan(x.canonical(), d), d);
}
template <typename FpType> inline elreal<FpType> sinh(const elreal<FpType>& x) {
    const std::size_t d = x.precision();
    if (!x.isfinite()) return x;                                                              // sinh(+-inf) = +-inf
    return elreal<FpType>(sinh(x.canonical(), d), d);
}
template <typename FpType> inline elreal<FpType> cosh(const elreal<FpType>& x) {
    const std::size_t d = x.precision();
    if (x.isnan()) return x;
    if (x.isinf()) return detail::elreal_inf<FpType>(1, d);                                   // cosh(+-inf) = +inf
    return elreal<FpType>(cosh(x.canonical(), d), d);
}
template <typename FpType> inline elreal<FpType> tanh(const elreal<FpType>& x) {
    const std::size_t d = x.precision();
    if (x.isnan()) return x;
    if (x.isinf()) return detail::elreal_special<FpType>(x.sign() < 0 ? -1.0 : 1.0, d);      // tanh(+-inf) = +-1
    return elreal<FpType>(tanh(x.canonical(), d), d);
}

// ---------------------------------------------------------------------------
// binary functions -- refine to the deeper of the two operands.
// ---------------------------------------------------------------------------
// pow(x, y), with the C99 special cases:
//   pow(x, 0) = 1 and pow(1, y) = 1, even for NaN; otherwise NaN in, NaN out.
//   y = +-inf: by |x| against 1 (pow(-1, +-inf) = 1).
//   x = +-inf: 0 for y < 0; +inf for y > 0, -inf for x = -inf and odd integer y.
//   x = 0: 0 for y > 0, +inf for y < 0.
//   x < 0: NaN unless y is an integer, decided from the stream; then the sign of x for odd y.
template <typename FpType>
inline elreal<FpType> pow(const elreal<FpType>& x, const elreal<FpType>& y) {
    const std::size_t d = x.precision() > y.precision() ? x.precision() : y.precision();
    if (y.iszero()) return detail::elreal_special<FpType>(1.0, d);
    if (x.isfinite() && x == elreal<FpType>(1.0)) return detail::elreal_special<FpType>(1.0, d);
    if (x.isnan() || y.isnan()) return detail::elreal_nan<FpType>(d);
    if (y.isinf()) {
        const int m = detail::compare_magnitude_with_one(x);
        if (m == 0) return detail::elreal_special<FpType>(1.0, d);
        const bool grows = (m > 0) == (y.sign() > 0);
        return grows ? detail::elreal_inf<FpType>(1, d) : detail::elreal_special<FpType>(0.0, d);
    }
    bool odd = false;
    const bool yint = detail::is_integer(y.canonical(), d + 1, odd);
    if (x.isinf()) {
        if (y.sign() < 0) return detail::elreal_special<FpType>(0.0, d);
        return detail::elreal_inf<FpType>((x.sign() < 0 && yint && odd) ? -1 : 1, d);
    }
    if (x.iszero()) return y.sign() > 0 ? detail::elreal_special<FpType>(0.0, d) : detail::elreal_inf<FpType>(1, d);
    if (x.sign() < 0 && !yint) return detail::elreal_nan<FpType>(d);
    return elreal<FpType>(pow(x.canonical(), y.canonical(), d), d);
}
// hypot: +inf when either operand is infinite, even with a NaN; then NaN in, NaN out
template <typename FpType>
inline elreal<FpType> hypot(const elreal<FpType>& x, const elreal<FpType>& y) {
    const std::size_t d = x.precision() > y.precision() ? x.precision() : y.precision();
    if (x.isinf() || y.isinf()) return detail::elreal_inf<FpType>(1, d);
    if (x.isnan() || y.isnan()) return detail::elreal_nan<FpType>(d);
    return elreal<FpType>(hypot(x.canonical(), y.canonical(), d), d);
}

namespace detail {
// today's per-generator default depths, so elreal_*(0) is unchanged behaviour
inline constexpr std::size_t default_depth_e = 32;
inline constexpr std::size_t default_depth_pi = 16;
inline constexpr std::size_t default_depth_ln2 = 16;
inline constexpr std::size_t default_depth_ln10 = 16;
inline constexpr std::size_t default_depth_log2_10 = 16;
inline constexpr std::size_t default_depth_sqrt2 = 16;
inline constexpr std::size_t default_depth_sqrt3 = 16;
inline constexpr std::size_t default_depth_sqrt5 = 16;
inline constexpr std::size_t default_depth_phi = 16;
inline constexpr std::size_t default_depth_euler_gamma = 16;
}  // namespace detail

// ---------------------------------------------------------------------------
// named constants -- elreal at the ZBCL factory's tuned series depth; the
// returned object reads at the scoped default precision.
//
// `depth` is optional and 0 means "the generator's own default", so existing
// callers are unaffected. It exists because those defaults are now genuinely
// expensive: with the refinement floors gone (universal#1051) a default-depth
// constant refines to hundreds of digits instead of stopping early, and a caller
// that only needs a few should be able to say so rather than pay for all of it. FpType defaults to
// double (elreal64); supply it explicitly for other hosts, e.g. elreal_pi<float>().
// ---------------------------------------------------------------------------
template <typename FpType = double> inline elreal<FpType> elreal_e(std::size_t depth = 0)
    { return elreal<FpType>((e_zbcl<FpType>)(depth ? depth : detail::default_depth_e)); }
template <typename FpType = double> inline elreal<FpType> elreal_pi(std::size_t depth = 0)
    { return elreal<FpType>((pi_zbcl<FpType>)(depth ? depth : detail::default_depth_pi)); }
template <typename FpType = double> inline elreal<FpType> elreal_ln2(std::size_t depth = 0)
    { return elreal<FpType>((ln2_zbcl<FpType>)(depth ? depth : detail::default_depth_ln2)); }
template <typename FpType = double> inline elreal<FpType> elreal_ln10(std::size_t depth = 0)
    { return elreal<FpType>((ln10_zbcl<FpType>)(depth ? depth : detail::default_depth_ln10)); }
template <typename FpType = double> inline elreal<FpType> elreal_log2_10(std::size_t depth = 0)
    { return elreal<FpType>((log2_10_zbcl<FpType>)(depth ? depth : detail::default_depth_log2_10)); }
template <typename FpType = double> inline elreal<FpType> elreal_sqrt2(std::size_t depth = 0)
    { return elreal<FpType>((sqrt2_zbcl<FpType>)(depth ? depth : detail::default_depth_sqrt2)); }
template <typename FpType = double> inline elreal<FpType> elreal_sqrt3(std::size_t depth = 0)
    { return elreal<FpType>((sqrt3_zbcl<FpType>)(depth ? depth : detail::default_depth_sqrt3)); }
template <typename FpType = double> inline elreal<FpType> elreal_sqrt5(std::size_t depth = 0)
    { return elreal<FpType>((sqrt5_zbcl<FpType>)(depth ? depth : detail::default_depth_sqrt5)); }
template <typename FpType = double> inline elreal<FpType> elreal_phi(std::size_t depth = 0)
    { return elreal<FpType>((phi_zbcl<FpType>)(depth ? depth : detail::default_depth_phi)); }
template <typename FpType = double> inline elreal<FpType> elreal_euler_gamma(std::size_t depth = 0)
    { return elreal<FpType>((euler_gamma_zbcl<FpType>)(depth ? depth : detail::default_depth_euler_gamma)); }

}} // namespace sw::universal
