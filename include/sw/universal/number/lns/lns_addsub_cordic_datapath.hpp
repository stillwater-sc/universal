#pragma once
// lns_addsub_cordic_datapath.hpp: lns add/sub on a fixed-point hyperbolic CORDIC datapath (#2046)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// CORDICDatapathAddSub is the hardware-codesign sibling of CORDICAddSub (#783). Both compute the Gauss
// corrections
//
//     sb_add(d) = log2(1 + 2^d),   sb_sub(d) = log2(1 - 2^d),   d <= 0
//
// with hyperbolic CORDIC: rotation mode for 2^d, vectoring mode for the logarithm. CORDICAddSub iterates in
// double, so it models the algorithm. This policy iterates in fixpnt, through the CORDIC engine of
// fixpnt/math/cordic.hpp, so it models the datapath: every shift truncates, the guard width and the
// iteration count are the hardware's, and each unit rounds its result once. It is opt-in, since it brings in
// fixpnt:
//
//     #include <universal/number/lns/lns.hpp>
//     #include <universal/number/lns/lns_addsub_cordic_datapath.hpp>
//
//     namespace sw::universal {
//         template<>
//         struct lns_addsub_traits<lns<16, 8, std::uint16_t>> {
//             using type = CORDICDatapathAddSub<lns<16, 8, std::uint16_t>>;
//         };
//     }
//
// The datapath. Each correction is a chain of three units joined by a fixpnt bus of R fraction bits:
//
//     t = d * ln 2            a constant multiplier
//     v = cordic::exp(t)      hyperbolic rotation, v = 2^d
//     w = 1 +- v              an adder, exact on the bus
//     L = cordic::log(w)      hyperbolic vectoring
//     sb = L * (1 / ln 2)     a constant multiplier
//
// The bus holds the sign, enough integer bits for |d| <= dmax and for |log2 w|, and R = rbits + g
// fraction bits, with g = ceil(log2(rbits + 8)) + 2 guard bits by default. Below d = -dmax, with
// dmax = rbits + g + 1, the correction 2^d / ln 2 is under one unit of the bus, and the policy returns 0.
//
// The cancellation band. For d in (-1, 0), 1 - 2^d shrinks to about ln 2 * 2^-rbits at the smallest |d|,
// so an absolute error of 2^-R in v becomes a relative error of about 2^(rbits - R) in w, and a log-domain
// error of the same size. sb_sub there runs on a wider bus of R = 2 rbits + g + 2 fraction bits, which keeps
// w's relative error, and so the log-domain error, at about 2^-(rbits + g). Every path is CORDIC: no
// transcendental from <cmath> or constexpr_math is evaluated.
//
// The knobs. Iterations is the iteration count of every CORDIC unit, counted as the engine counts it, with
// the repeated shifts 4, 13, 40, ... included. The default, 0, gives each unit the engine's default for its
// bus: every shift down to the resolution of the unit's own datapath, which is wider than the bus, so the
// default runs past the point where the error stops improving (at lns<16,8>, 30 iterations on the narrow
// bus and 41 on the wide one, against 24 that already reach the same error). A smaller count models a
// truncated unit; truncated far enough, 2^d can round to 1 or above, and the subtractor 1 - 2^d then
// saturates at the smallest positive value of its bus, as an underflow detect in hardware does, so sb_sub
// stays finite for every d < 0. Guard is g, the guard bits of the bus beyond rbits; the units
// inside keep the engine's own guard bits for their datapath. benchmark/accuracy/lns/
// cordic_precision_assessment.cpp tabulates both knobs (docs/design/cordic-precision-assessment.md).
#include <cstdint>
#include <limits>

#include <universal/number/lns/lns.hpp>
#include <universal/number/fixpnt/fixpnt.hpp>

namespace sw { namespace universal {

template<typename Lns, unsigned Iterations = 0u, unsigned Guard = cordic::auto_guard>
struct CORDICDatapathAddSub {
	static constexpr unsigned rbits = Lns::rbits;
	static constexpr unsigned guard = (Guard == cordic::auto_guard) ? cordic::detail::clog2(rbits + 8u) + 2u : Guard;
	// below d = -dmax both corrections are under one bus unit: |sb(d)| ~= 2^d / ln 2 < 0.73 * 2^-(rbits + g)
	static constexpr unsigned dmax = rbits + guard + 1u;
	// integer bits, the sign included: 2^(ibits - 1) >= 2 (dmax + 1) holds |d| <= dmax, and |log2 w| down
	// to the smallest positive w of the wide bus, 2^-(2 rbits + g + 2)
	static constexpr unsigned ibits = cordic::detail::clog2(dmax + 1u) + 2u;
	static constexpr unsigned narrow_rbits = rbits + guard;
	static constexpr unsigned wide_rbits = 2u * rbits + guard + 2u;
	using Narrow = fixpnt<ibits + narrow_rbits, narrow_rbits, Modulo, std::uint32_t>;
	using Wide   = fixpnt<ibits + wide_rbits, wide_rbits, Modulo, std::uint32_t>;

	// the iteration count of a unit on bus F
	template<typename F>
	static constexpr unsigned iterations() noexcept {
		return (Iterations == 0u) ? cordic::hyperbolic_iterations<F> : Iterations;
	}

	// 2^d = exp(d ln 2), for -dmax <= d <= 0
	template<typename F>
	static F pow2(const F& d) {
		const auto& C = cordic::detail::ln2_reduction_of<F>();
		return cordic::exp(F(d * C.ln2), iterations<F>());
	}

	// log2 w = ln w / ln 2, for 0 < w <= 2

	// 1 - v, saturating at the smallest positive value of the bus: a truncated exp unit can give v >= 1
	template<typename F>
	static F one_minus(const F& v) {
		F w = F(1) - v;
		if (w.iszero() || w.isneg()) w.setbits(1u);
		return w;
	}
	template<typename F>
	static F log2(const F& w) {
		const auto& C = cordic::detail::ln2_reduction_of<F>();
		return F(cordic::log(w, iterations<F>()) * C.inv_ln2);
	}

	// log2(1 + 2^d) for d <= 0
	static double sb_add(double d) {
		if (!(d < 0.0)) d = 0.0;
		if (d < -static_cast<double>(dmax)) return 0.0;
		const Narrow w = Narrow(1) + pow2(Narrow(d));     // [1, 2]
		return double(log2(w));
	}

	// log2(1 - 2^d) for d < 0; d = 0 is the pole 1 - 2^0 = 0
	static double sb_sub(double d) {
		if (!(d < 0.0)) return -std::numeric_limits<double>::infinity();
		if (d < -static_cast<double>(dmax)) return 0.0;
		if (d > -1.0) {
			const Wide w = one_minus(pow2(Wide(d)));      // (0, 1/2), with rbits more fraction bits
			return double(log2(w));
		}
		const Narrow w = one_minus(pow2(Narrow(d)));      // [1/2, 1)
		return double(log2(w));
	}

	static Lns& add_assign(Lns& lhs, const Lns& rhs) {
		double result = detail::gauss_log_add<CORDICDatapathAddSub>(double(lhs), double(rhs));
		return lhs = result;
	}
	static Lns& sub_assign(Lns& lhs, const Lns& rhs) {
		double result = detail::gauss_log_add<CORDICDatapathAddSub>(double(lhs), double(-rhs));
		return lhs = result;
	}
};

// the log-domain error contract of lns_eq_within_alg_tolerance. With every iteration run, the chain's
// truncation and rounding errors are a few units of the bus, and so is the correction dropped below
// d = -dmax: 8 * 2^-(rbits + g) bounds them (measured: at most 0.044 lns units with the default g, 2.4 with
// g = 0). A truncated unit leaves an angle of about 2^-(Iterations - 3) unabsorbed (the repeats push the
// last shift below the count), and the cancellation band amplifies it by up to 1 / (ln 2 * w) ~= 2^(rbits + 1):
// 2^(rbits + 4 - Iterations) estimates both. Whatever the count, the saturating subtractor keeps every w at
// or above the smallest value of its bus, 2^-(2 rbits + g + 2), and every correction lies in [-(rbits + 2), 1],
// so no error exceeds 2 rbits + g + 3: the estimate is capped there.
template<typename Lns, unsigned Iterations, unsigned Guard>
struct lns_addsub_log_error_bound<CORDICDatapathAddSub<Lns, Iterations, Guard>> {
	using Alg = CORDICDatapathAddSub<Lns, Iterations, Guard>;
	static constexpr double full = 8.0 * sw::math::constexpr_math::exp2(-double(Lns::rbits + Alg::guard));
	static constexpr double cap = double(Alg::wide_rbits + 1u);
	static constexpr double estimate = (Iterations == 0u)
	    ? 0.0
	    : sw::math::constexpr_math::exp2(double(Lns::rbits + 4u) - double(Iterations));
	static constexpr double truncated = (estimate < cap) ? estimate : cap;
	static constexpr double value = (truncated > full) ? truncated : full;
};

}}  // namespace sw::universal
