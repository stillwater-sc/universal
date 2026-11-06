#pragma once
// evaluate.hpp: a posit function value, correctly rounded, from efloat's correctly rounded math library (#1855)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The posit math library computes its functions in efloat, at every nbits: no native floating-point type
// is involved. efloat's functions are correctly rounded at the precision asked for, but a result rounded
// at W bits and then again into the posit is rounded twice, and that can miss the posit's correctly
// rounded value when the exact result lies within an efloat ulp of a posit tie point. evaluate() applies
// Ziv's test on the posit side: the value y = f(x) rounded at W bits is within 1/2 ulp of the exact
// result, so when y - 1 ulp and y + 1 ulp round to the same posit, so does the exact result, and that
// posit is the answer. When they do not, W doubles. The first W carries 30 bits past the posit's
// fraction, so the test fails about once in 2^28 evaluations.
//
// An exact result that sits on a posit tie point never passes the test, at any W. efloat forms exact
// results exactly (exp2 of an integer, hypot of a Pythagorean triple), so after the last retry y is
// rounded as it is, ties to even, which is correct for an exact result; an inexact result as close to
// a tie as 2^-(8 W) relative is not expected for these functions (the table maker's dilemma literature
// puts the hardest cases near 2^-(2 p)).
//
// efloat's rounding mode and status flags are per thread; evaluate() rounds to nearest whatever the
// mode, and leaves both as it found them. (The test's 1 ulp margin would hold a directed rounding's
// error as well; rounding to nearest keeps the retries as rare as stated.)
#include <universal/number/posit/math/efloat_bridge.hpp>
#include <universal/number/efloat/mathlib.hpp>

namespace sw { namespace universal {

	namespace posit_limbs {

		// rounds efloat to nearest while it lives, and puts back efloat's rounding mode and status
		// flags on every exit, an exception from the evaluation included
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

		// What an efloat inf or 0 from a valid argument means. efloat's range ends at 2^(2^62), and a
		// posit's well before: an inf is then an overflow, which a posit saturates at +-maxpos, and a 0
		// from a function that is never 0 is an underflow, which saturates at +-minpos. Poles and
		// domain errors are decided by the caller before it evaluates.
		enum class Saturation {
			none,       // an efloat inf is a pole: NaR
			overflow,   // an efloat inf saturates at +-maxpos
			both        // and an efloat 0 at +-minpos: the function is never 0 here
		};

		// the posit nearest f(W), where f(W) returns a function value correctly rounded to nearest at
		// (at least) W bits as an efloat_for<nbits>
		template<typename Posit, typename Function>
		inline Posit evaluate(Function&& f, Saturation saturation = Saturation::none) {
			using E = efloat_for<Posit::nbits>;
			const efloat_environment_guard environment;
			unsigned W = Posit::fbits + 1u + 30u;
			Posit p;
			for (int attempt = 0; ; ++attempt) {
				const E y = f(W);
				if (y.isinf() && saturation != Saturation::none) return Posit(y.sign() < 0 ? SpecificValue::maxneg : SpecificValue::maxpos);
				if (y.iszero() && saturation == Saturation::both) return Posit(y.sign() < 0 ? SpecificValue::minneg : SpecificValue::minpos);
				from_efloat(p, y);
				if (y.iszero() || y.isnan() || y.isinf() || attempt == 3) break;
				// y +- 1 ulp of y's precision: exact at that precision, also where y - ulp leaves y's binade
				const E ulp = ldexp(E(1), static_cast<int>(y.scale()) - static_cast<int>(y.get_precision()) + 1);
				Posit below, above;
				from_efloat(below, y - ulp);
				from_efloat(above, y + ulp);
				if (below == p && above == p) break;
				W *= 2u;
			}
			return p;
		}

		// x as an efloat that rounds at W bits: exact, since W is wider than the posit's significand
		template<unsigned nbits, unsigned es, typename bt>
		inline efloat_for<nbits> argument(const posit<nbits, es, bt>& x, unsigned W) {
			efloat_for<nbits> e = to_efloat(x);
			e.set_precision(W);
			return e;
		}

	} // namespace posit_limbs

}} // namespace sw::universal
