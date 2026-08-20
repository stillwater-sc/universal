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
#include <cstddef>

#include <universal/number/elreal/elreal_fwd.hpp>
#include <universal/number/elreal/elreal_impl.hpp>
#include <universal/number/elreal/math/sqrt.hpp>
#include <universal/number/elreal/math/hypot.hpp>
#include <universal/number/elreal/math/exponent.hpp>
#include <universal/number/elreal/math/hyperbolic.hpp>
#include <universal/number/elreal/math/trigonometry.hpp>
#include <universal/number/elreal/math/constants.hpp>

namespace sw { namespace universal {

// ---------------------------------------------------------------------------
// unary functions -- refine to the operand's runtime precision.
// Each forwards to the ZBCL overload of the same name (resolved by argument
// type: stream() yields a ZBCL<FpType>, so the two-argument ZBCL overload is
// selected, never this single-argument elreal one).
// ---------------------------------------------------------------------------
template <typename FpType> inline elreal<FpType> sqrt(const elreal<FpType>& x) { const std::size_t d = x.precision(); return elreal<FpType>(sqrt(x.stream(), d), d); }
template <typename FpType> inline elreal<FpType> exp (const elreal<FpType>& x) { const std::size_t d = x.precision(); return elreal<FpType>(exp (x.stream(), d), d); }
template <typename FpType> inline elreal<FpType> log (const elreal<FpType>& x) { const std::size_t d = x.precision(); return elreal<FpType>(log (x.stream(), d), d); }
template <typename FpType> inline elreal<FpType> sin (const elreal<FpType>& x) { const std::size_t d = x.precision(); return elreal<FpType>(sin (x.stream(), d), d); }
template <typename FpType> inline elreal<FpType> cos (const elreal<FpType>& x) { const std::size_t d = x.precision(); return elreal<FpType>(cos (x.stream(), d), d); }
template <typename FpType> inline elreal<FpType> tan (const elreal<FpType>& x) { const std::size_t d = x.precision(); return elreal<FpType>(tan (x.stream(), d), d); }
template <typename FpType> inline elreal<FpType> asin(const elreal<FpType>& x) { const std::size_t d = x.precision(); return elreal<FpType>(asin(x.stream(), d), d); }
template <typename FpType> inline elreal<FpType> acos(const elreal<FpType>& x) { const std::size_t d = x.precision(); return elreal<FpType>(acos(x.stream(), d), d); }
template <typename FpType> inline elreal<FpType> atan(const elreal<FpType>& x) { const std::size_t d = x.precision(); return elreal<FpType>(atan(x.stream(), d), d); }
template <typename FpType> inline elreal<FpType> sinh(const elreal<FpType>& x) { const std::size_t d = x.precision(); return elreal<FpType>(sinh(x.stream(), d), d); }
template <typename FpType> inline elreal<FpType> cosh(const elreal<FpType>& x) { const std::size_t d = x.precision(); return elreal<FpType>(cosh(x.stream(), d), d); }
template <typename FpType> inline elreal<FpType> tanh(const elreal<FpType>& x) { const std::size_t d = x.precision(); return elreal<FpType>(tanh(x.stream(), d), d); }

// ---------------------------------------------------------------------------
// binary functions -- refine to the deeper of the two operands.
// ---------------------------------------------------------------------------
template <typename FpType>
inline elreal<FpType> pow(const elreal<FpType>& x, const elreal<FpType>& y) {
    const std::size_t d = x.precision() > y.precision() ? x.precision() : y.precision();
    return elreal<FpType>(pow(x.stream(), y.stream(), d), d);
}
template <typename FpType>
inline elreal<FpType> hypot(const elreal<FpType>& x, const elreal<FpType>& y) {
    const std::size_t d = x.precision() > y.precision() ? x.precision() : y.precision();
    return elreal<FpType>(hypot(x.stream(), y.stream(), d), d);
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
