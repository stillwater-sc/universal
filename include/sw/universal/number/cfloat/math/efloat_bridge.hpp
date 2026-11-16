#pragma once
// efloat_bridge.hpp: cfloat to and from efloat, and Ziv rounding, for the cfloat layouts that do not fit
// a double (#2088)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The cfloat math library evaluates its transcendental functions in double when the layout fits one --
// its range inside double's and at most 52 fraction bits -- and in efloat otherwise (the decision on
// #2088). A layout past double lost range (exp(710) on cfloat<64,15> was inf) and precision
// (pow(3, 40) on cfloat<80,11> was inexact) through double.
//
// to_efloat is exact: the efloat carries the cfloat's whole significand. from_efloat rounds once, through
// convert(blocktriple, cfloat), so subnormals, saturation and max-exponent values round as every other
// cfloat operation rounds them. evaluate() applies Ziv's test on the cfloat side, as posit's evaluate
// does (#1855): y = f(x) rounded at W bits is within half an ulp of the exact result, so when y - 1 ulp
// and y + 1 ulp round to the same cfloat, so does the exact result; when they do not, W doubles.
//
// cfloat's range ends far inside efloat's (2^(2^62)), so an efloat inf or 0 from a valid argument is the
// cfloat's own overflow or underflow, and from_efloat rounds it as such: no saturation policy is needed.
#include <cstddef>
#include <cstdint>
#include <universal/number/efloat/core.hpp>
#include <universal/number/efloat/mathlib.hpp>

namespace sw { namespace universal {
namespace cfloat_math {

	// the layout's largest finite binade is no higher than double's and its smallest value no smaller
	// than double's smallest: its range ends inside double's.  MAX_EXP counts the top exponent binade,
	// which holds finite values only with hasMaxExpValues.
	template<typename Cfloat>
	constexpr bool range_fits_double() {
		constexpr int maxFiniteExp = Cfloat::hasMaxExpValues ? Cfloat::MAX_EXP : Cfloat::MAX_EXP - 1;
		return maxFiniteExp <= 1023 && (Cfloat::hasSubnormals ? Cfloat::MIN_EXP_SUBNORMAL : Cfloat::MIN_EXP_NORMAL) >= -1074;
	}
	// the layout evaluates through double: its range and its precision fit one
	template<typename Cfloat>
	constexpr bool fits_double() {
		return range_fits_double<Cfloat>() && Cfloat::fbits <= 52u;
	}

	// 64-bit limbs that hold a cfloat's significand
	template<typename Cfloat>
	constexpr std::size_t significand_limbs() { return (Cfloat::fbits + 1u + 63u) / 64u; }
	// an efloat that holds a cfloat exactly, with room to spare: 32-bit units
	template<typename Cfloat>
	using efloat_for = efloat<static_cast<unsigned>(2u * significand_limbs<Cfloat>() + 2u)>;

	// x, exactly; a NaN is a quiet NaN
	template<unsigned nbits, unsigned es, typename bt, bool sub, bool sup, bool sat>
	efloat_for<cfloat<nbits, es, bt, sub, sup, sat>> to_efloat(const cfloat<nbits, es, bt, sub, sup, sat>& x) {
		using Cfloat = cfloat<nbits, es, bt, sub, sup, sat>;
		using E = efloat_for<Cfloat>;
		E e(0);
		if (x.isnan()) { e.setnan(); return e; }
		if (x.isinf()) { e.setinf(x.sign()); return e; }
		if (x.iszero()) { e.setsign(x.sign()); return e; }
		constexpr std::size_t n = significand_limbs<Cfloat>();
		const bool hidden = x.isnormal() || x.ismaxexpvalue();
		const int top = x.scale();   // the weight of the leading bit
		// the leading bit's index in the significand: the hidden bit, or a subnormal's leading 1
		const int lead = hidden ? static_cast<int>(Cfloat::fbits) : top - Cfloat::MIN_EXP_NORMAL + static_cast<int>(Cfloat::fbits);
		typename E::limb_t r[n];
		for (std::size_t j = 0; j < n; ++j) {
			// limb j, least significant first, its top bit at significand index lead - 64 (n - 1 - j)
			const int lo = lead - static_cast<int>(64u * (n - j)) + 1;
			r[j] = x.significand_window(lo, 64u, hidden);
		}
		e.assign_rounded_limbs(x.sign(), r, n, top, false, static_cast<unsigned>(2u * n));
		return e;
	}

	// v rounded once to the cfloat: through convert(blocktriple), which applies the layout's subnormals,
	// saturation and max-exponent values
	template<unsigned nbits, unsigned es, typename bt, bool sub, bool sup, bool sat, unsigned nlimbs>
	cfloat<nbits, es, bt, sub, sup, sat>& from_efloat(cfloat<nbits, es, bt, sub, sup, sat>& r, const efloat<nlimbs>& v) {
		using Cfloat = cfloat<nbits, es, bt, sub, sup, sat>;
		if (v.isnan()) { r.setnan(NAN_TYPE_QUIET); return r; }
		if (v.isinf()) { r.setinf(v.sign() < 0); return r; }
		if (v.iszero()) { r.setzero(); r.setsign(v.sign() < 0); return r; }
		using BT = blocktriple<Cfloat::fbits, BlockTripleOperator::ADD, bt>;
		constexpr int radix = BT::radix;   // the hidden bit; radix - 1 .. 1 below it, bit 0 the sticky bit
		const typename efloat<nlimbs>::limb_t* w = v.limb_data();
		const std::size_t m = v.limb_count();
		// bit d below the leading bit of v (d = 0 is the leading bit)
		auto bit = [&](std::size_t d) -> bool {
			if (d / 64u >= m) return false;
			return ((w[m - 1u - d / 64u] >> (63u - d % 64u)) & 1u) != 0;
		};
		BT b;
		b.clear();
		b.setnormal();
		b.setsign(v.sign() < 0);
		// a scale far past the layout's range rounds the same as one just past it: keep it in an int
		std::int64_t s = v.scale();
		constexpr std::int64_t hi = Cfloat::MAX_EXP + 2, lo = Cfloat::MIN_EXP_SUBNORMAL - 2;
		if (s > hi) s = hi;
		if (s < lo) s = lo;
		b.setscale(static_cast<int>(s));
		for (int i = radix; i >= 1; --i) b.setbit(static_cast<unsigned>(i), bit(static_cast<std::size_t>(radix - i)));
		bool sticky = false;
		for (std::size_t d = static_cast<std::size_t>(radix); d < 64u * m && !sticky; ++d) sticky = bit(d);
		b.setbit(0, sticky);
		convert(b, r);
		return r;
	}

	// rounds efloat to nearest while it lives, and puts back efloat's rounding mode and status flags on
	// every exit
	class efloat_environment_guard {
	public:
		efloat_environment_guard() noexcept : mode(efloat_rounding_mode), flags(get_efloat_exceptions()) {
			efloat_rounding_mode = RoundingMode::RoundToNearest;
		}
		~efloat_environment_guard() {
			efloat_rounding_mode = mode;
			set_efloat_exceptions(flags);
		}
		efloat_environment_guard(const efloat_environment_guard&) = delete;
		efloat_environment_guard& operator=(const efloat_environment_guard&) = delete;
	private:
		RoundingMode  mode;
		std::uint32_t flags;
	};

	// the cfloat nearest f(W), where f(W) returns a function value correctly rounded to nearest at (at
	// least) W bits as an efloat_for<Cfloat>.  The first W carries 30 bits past the cfloat's fraction,
	// so the test fails about once in 2^28 evaluations; an exact result on a tie point never passes it,
	// and after the last retry it is rounded as it is, ties to even, which is correct for it.
	template<typename Cfloat, typename Function>
	Cfloat evaluate(Function&& f) {
		using E = efloat_for<Cfloat>;
		const efloat_environment_guard environment;
		unsigned W = Cfloat::fbits + 1u + 30u;
		Cfloat c;
		for (int attempt = 0; ; ++attempt) {
			const E y = f(W);
			from_efloat(c, y);
			if (y.iszero() || y.isnan() || y.isinf() || attempt == 3) break;
			// past the layout's range y and y +- 1 ulp round alike (to inf, maxpos or 0), and the ulp's
			// exponent below would overflow an int: exp2(-2^31) has scale INT_MIN (CodeRabbit on #2093)
			if (y.scale() > Cfloat::MAX_EXP + 2 || y.scale() < Cfloat::MIN_EXP_SUBNORMAL - 2) break;
			// y +- 1 ulp of y's precision: exact at that precision
			const E ulp = ldexp(E(1), static_cast<int>(y.scale()) - static_cast<int>(y.get_precision()) + 1);
			Cfloat below, above;
			from_efloat(below, y - ulp);
			from_efloat(above, y + ulp);
			if (below == c && above == c) break;
			W *= 2u;
		}
		return c;
	}

	// x as an efloat that rounds at W bits: exact, since W is wider than the cfloat's significand
	template<unsigned nbits, unsigned es, typename bt, bool sub, bool sup, bool sat>
	efloat_for<cfloat<nbits, es, bt, sub, sup, sat>> argument(const cfloat<nbits, es, bt, sub, sup, sat>& x, unsigned W) {
		auto e = to_efloat(x);
		e.set_precision(W);
		return e;
	}

	// the wide path of a unary function f: a NaN is quieted, a zero is returned as it is when
	// f(+-0) = +-0 (oddAtZero), and anything else is f evaluated in efloat and rounded once
	template<typename Cfloat, typename Function>
	Cfloat unary(const Cfloat& x, Function&& f, bool oddAtZero) {
		if (x.isnan()) return x.quieted();
		if (oddAtZero && x.iszero()) return x;
		return evaluate<Cfloat>([&](unsigned W) { return f(argument(x, W)); });
	}

	// a double, exactly, as an efloat of the same type, rounding at W bits.  set_precision rounds the
	// value, so W is at least 64 here: a layout with a narrow fraction starts at W < 53, which rounded
	// pow's exponent 1 + 2^-40 to 1 and made pow(-1, 1 + 2^-40) -1 instead of NaN (CodeRabbit on #2093).
	template<typename Cfloat>
	efloat_for<Cfloat> argument_from(double v, unsigned W) {
		efloat_for<Cfloat> e(v);
		e.set_precision(W < 64u ? 64u : W);
		return e;
	}

} // namespace cfloat_math
}} // namespace sw::universal
