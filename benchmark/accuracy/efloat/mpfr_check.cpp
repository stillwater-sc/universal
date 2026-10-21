// mpfr_check.cpp: every efloat function against MPFR, bit for bit, in all four rounding modes (#1782)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// For each function and each precision P in 32, 64, 128, 512, 2048 and 8192 bits, random P-bit
// arguments are evaluated by efloat in one of the four rounding modes and by MPFR, at P bits, in
// the same mode. A result is right when it has MPFR's value, MPFR's sign (zeros included), and
// precision P. The arguments come in five styles:
//
//   0  full-width, |x| in [1/2, 4)              1  full-width, 2^-40 <= |x| < 2^maxexp
//   2  a few significant bits, tiny             3  full-width, tiny (scale -P/4 .. -5P/4)
//   4  the function's special arguments: integers, halfway cases, points next to a switch of
//      algorithm, a pole or a domain boundary, exact powers
//
// so a misrounding near a boundary, a lost cancellation guard and a wrong special value all
// show up. The case counts fall with the precision, as the cost of a case rises:
//
//   mpfr_check                      every function, scale 1, up to 8192 bits
//   mpfr_check 0.1 all 512          the quick run the ctest registers
//   mpfr_check 4 erf,erfc           four times the cases, two functions
//
// The exit status is the number of wrong results of the functions that are correctly rounded;
// the ones that are not yet (`known`, below) are reported and do not fail the run.
//
// MPFR is a dependency of this program only, never of the library: the CMakeLists.txt next to
// this file builds it when it finds mpfr.h, libmpfr and libgmp (Debian/Ubuntu: libmpfr-dev).

#include <universal/utility/directives.hpp>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>
#include <universal/number/efloat/efloat.hpp>
#include <mpfr.h>

namespace sw { namespace universal {

	std::mt19937_64 rng(4242);

	// the functions checked; `arity` 2 takes a second efloat, `integer` a second int
	struct Function {
		const char* name;
		int         arity;
		bool        integer;
		const char* known;   // nullptr when correctly rounded, else the issue that tracks it
	};

	const Function functions[] = {
		{ "add", 2, false, nullptr },   { "sub", 2, false, nullptr },   { "mul", 2, false, nullptr },
		{ "div", 2, false, nullptr },   { "sqrt", 1, false, nullptr },  { "cbrt", 1, false, nullptr },
		{ "hypot", 2, false, nullptr }, { "fma", 3, false, nullptr },   { "fmod", 2, false, nullptr },
		{ "remainder", 2, false, nullptr },
		{ "exp", 1, false, nullptr },   { "exp2", 1, false, nullptr },  { "exp10", 1, false, nullptr },
		{ "expm1", 1, false, nullptr }, { "log", 1, false, nullptr },   { "log2", 1, false, nullptr },
		{ "log10", 1, false, nullptr }, { "log1p", 1, false, nullptr },
		{ "sin", 1, false, nullptr },   { "cos", 1, false, nullptr },   { "tan", 1, false, nullptr },
		{ "asin", 1, false, nullptr },  { "acos", 1, false, nullptr },  { "atan", 1, false, nullptr },
		{ "atan2", 2, false, nullptr },
		{ "sinh", 1, false, nullptr },  { "cosh", 1, false, nullptr },  { "tanh", 1, false, nullptr },
		{ "asinh", 1, false, nullptr }, { "acosh", 1, false, nullptr }, { "atanh", 1, false, nullptr },
		{ "pow", 2, false, nullptr },   { "powi", 2, true, nullptr },
		{ "tgamma", 1, false, nullptr },{ "lgamma", 1, false, nullptr },
		{ "erf", 1, false, nullptr },   { "erfc", 1, false, nullptr },
		{ "double", 1, false, nullptr },{ "float", 1, false, nullptr },
	};

	const char* const style_names[] = { "|x|<4", "wide", "short", "tiny", "special" };

	bool is(const Function& f, const char* name) { return std::strcmp(f.name, name) == 0; }

	// v exactly, at a precision wide enough to hold every limb
	template<unsigned nlimbs>
	void to_mpfr(mpfr_t r, const efloat<nlimbs>& v) {
		if (v.isnan()) { mpfr_set_nan(r); return; }
		if (v.isinf()) { mpfr_set_inf(r, v.sign()); return; }
		if (v.iszero()) { mpfr_set_zero(r, v.sign()); return; }
		const std::vector<uint32_t> limbs = v.bits();
		mpfr_t t;
		mpfr_init2(t, 64);
		mpfr_set_ui(r, 0, MPFR_RNDN);
		for (size_t i = 0; i < limbs.size(); ++i) {
			mpfr_set_ui(t, limbs[i], MPFR_RNDN);
			mpfr_mul_2si(t, t, static_cast<long>(v.scale()) - 31 - 32 * static_cast<long>(i), MPFR_RNDN);
			mpfr_add(r, r, t, MPFR_RNDN);   // exact: r is wide enough
		}
		if (v.sign() == -1) mpfr_neg(r, r, MPFR_RNDN);
		mpfr_clear(t);
	}

	// a P-bit efloat from a double
	template<unsigned nlimbs>
	efloat<nlimbs> make(double d, unsigned P) { efloat<nlimbs> v(d); v.set_precision(P); return v; }

	// a random P-bit argument of the given style, signed at random
	template<unsigned nlimbs>
	efloat<nlimbs> random_arg(unsigned P, unsigned style, int maxexp) {
		using Real = efloat<nlimbs>;
		Real a(1);
		a.set_precision(P);
		for (unsigned j = 0; j < (P + 31) / 32; ++j) a.setlimb(j, static_cast<uint32_t>(rng()) | (j == 0 ? 0x80000000u : 0u));
		a.set_precision(P);
		if (style == 0) a.setexponent(static_cast<int64_t>(rng() % 3) - 1);
		else if (style == 1) a.setexponent(static_cast<int64_t>(rng() % static_cast<unsigned>(41 + maxexp)) - 40);
		else if (style == 2) {
			const unsigned bits = 1 + static_cast<unsigned>(rng() % 24);
			Real t = make<nlimbs>(static_cast<double>((rng() & ((1u << bits) - 1)) | 1u), P);
			t.setexponent(t.scale() - static_cast<int64_t>(P / 4 + rng() % (P / 2)));
			a = t;
		}
		else a.setexponent(-static_cast<int64_t>(P / 4 + rng() % P));
		if (rng() & 1) a = -a;
		a.set_precision(P);
		return a;
	}

	// shape the arguments of f to its domain, and draw its special ones in style 4
	template<unsigned nlimbs>
	void shape(const Function& f, unsigned P, unsigned style, efloat<nlimbs>& a, efloat<nlimbs>& b, efloat<nlimbs>& c, int& k) {
		using Real = efloat<nlimbs>;
		const Real one = make<nlimbs>(1.0, P);
		const bool special = (style == 4);
		const unsigned kind = static_cast<unsigned>(rng() % 6);
		if (is(f, "add") || is(f, "sub")) {
			// operands of near or equal magnitude, so that the sum cancels
			if (special) { b = -a; Real d = random_arg<nlimbs>(P, 3, 0); b = b + d * a; }
		}
		else if (is(f, "mul") || is(f, "div")) {
			if (special) { a = make<nlimbs>(static_cast<double>(1 + rng() % 1000), P); b = make<nlimbs>(static_cast<double>(1 + rng() % 1000), P); }
		}
		else if (is(f, "sqrt") || is(f, "cbrt")) {
			if (!is(f, "cbrt")) a.setsign(false);
			if (special) { Real s = make<nlimbs>(static_cast<double>(1 + rng() % 100000), P); a = s * s; if (is(f, "cbrt")) a = a * s; if (rng() & 1) a = a + random_arg<nlimbs>(P, 3, 0) * a; }
		}
		else if (is(f, "hypot")) {
			// Pythagorean triples, or y around the gap past which hypot rounds as |x| nudged up
			if (special && (rng() & 1)) { a = make<nlimbs>(static_cast<double>(3 * (1 + rng() % 100)), P); b = a * make<nlimbs>(4.0 / 3.0, P); }
			else if (special) b.setexponent(a.scale() - static_cast<int64_t>(P / 2 + rng() % (2 * P + 32)));
		}
		else if (is(f, "fma")) {
			// c cancels most of a b, or lies around the gap past which it only nudges a b
			if (special && (rng() & 1)) { c = -(a * b); c = c + random_arg<nlimbs>(P, 3, 0) * c; }
			else if (special) c.setexponent(a.scale() + b.scale() - static_cast<int64_t>(P + rng() % (3 * P)));
		}
		else if (is(f, "fmod") || is(f, "remainder")) {
			// halfway cases, x = (k + 1/2) y, or quotients wider than the precision, up to 2^(2^40)
			if (special && rng() % 3 == 0) { a = b * make<nlimbs>(static_cast<double>(rng() % 1000) + 0.5, P); }
			else if (special) a.setexponent(a.scale() + static_cast<int64_t>((rng() & 1) ? rng() % (2 * P) : rng() % (1ull << (rng() % 41))));
		}
		else if (is(f, "exp2") || is(f, "exp10")) {
			if (special) a = make<nlimbs>(static_cast<double>(static_cast<int>(rng() % 80) - 40), P);
		}
		else if (is(f, "log") || is(f, "log2") || is(f, "log10")) {
			a.setsign(false);
			if (special) {
				a = is(f, "log10") ? make<nlimbs>(std::pow(10.0, static_cast<double>(rng() % 20)), P)
				                   : make<nlimbs>(std::ldexp(1.0, static_cast<int>(rng() % 200) - 100), P);
				if (rng() & 1) a = a * make<nlimbs>(static_cast<double>(1 + 2 * (rng() % 1000)), P);
				if (kind == 0) a = one + random_arg<nlimbs>(P, 3, 0);   // next to 1
			}
		}
		else if (is(f, "log1p")) {
			if (a.sign() == -1 && a.scale() >= 0) a.setexponent(-1 - static_cast<int64_t>(rng() % 3));
			if (special) a = make<nlimbs>(static_cast<double>(rng() % 100), P);
		}
		else if (is(f, "sin") || is(f, "cos") || is(f, "tan")) {
			// next to a multiple of pi/2
			if (special) { Real hp = efloat_pi<nlimbs>(); hp.set_precision(P); hp.setexponent(hp.scale() - 1); a = make<nlimbs>(static_cast<double>(1 + rng() % 20), P) * hp; a.set_precision(P); }
		}
		else if (is(f, "acosh")) {
			a.setsign(false);
			a = one + a;
			if (special) a = make<nlimbs>(static_cast<double>(1 + rng() % 100), P);
		}
		else if (is(f, "atanh") || is(f, "asin") || is(f, "acos")) {
			if (a.scale() >= 0) a.setexponent(-1 - static_cast<int64_t>(rng() % 4));
			if (special) {
				Real d = random_arg<nlimbs>(P, 2, 0);
				d.setsign(false);
				a = one - d;
				if (rng() & 1) a = -a;
				if (rng() % 8 == 0 && !is(f, "atanh")) a = (rng() & 1) ? one : -one;
			}
		}
		else if (is(f, "atan2")) {
			if (special) { a = make<nlimbs>(static_cast<double>(static_cast<int>(rng() % 7) - 3), P); b = make<nlimbs>(static_cast<double>(static_cast<int>(rng() % 7) - 3), P); }
		}
		else if (is(f, "pow")) {
			a.setsign(false);
			if (a.scale() > 8 || a.scale() < -8) a.setexponent(static_cast<int64_t>(rng() % 17) - 8);
			b = random_arg<nlimbs>(P, static_cast<unsigned>(rng() % 3), 3);
			if (special) {
				// exact powers, square roots of squares, negative bases at integer exponents
				const unsigned which = static_cast<unsigned>(rng() % 4);
				if (which == 0) { a = make<nlimbs>(static_cast<double>(1 + rng() % 30), P); b = make<nlimbs>(static_cast<double>(static_cast<int>(rng() % 40) - 20), P); }
				else if (which == 1) { const double s = static_cast<double>(1 + rng() % 1000); a = make<nlimbs>(s * s, P); b = make<nlimbs>((static_cast<int>(rng() % 9) - 4) + 0.5, P); }
				else if (which == 2) { a = make<nlimbs>(-static_cast<double>(1 + rng() % 30), P); b = make<nlimbs>(static_cast<double>(static_cast<int>(rng() % 40) - 20), P); }
				else b = make<nlimbs>(static_cast<double>(static_cast<int>(rng() % 9) - 4), P);
			}
		}
		else if (is(f, "powi")) {
			a.setsign(rng() & 1);
			if (a.scale() > 8 || a.scale() < -8) a.setexponent(static_cast<int64_t>(rng() % 17) - 8);
			k = static_cast<int>(rng() % 41) - 20;
			if (special) a = make<nlimbs>(static_cast<double>(static_cast<int>(rng() % 61) - 30), P);
		}
		else if (is(f, "tgamma") || is(f, "lgamma")) {
			if (special) {
				if (kind == 0) a = make<nlimbs>(static_cast<double>(1 + rng() % 400), P);                              // integers
				else if (kind == 1) a = make<nlimbs>(static_cast<double>(static_cast<int>(rng() % 200) - 100) + 0.5, P);   // half-integers
				else if (kind <= 3) {                                                                                 // next to 1 and 2
					Real d = one; d.setexponent(-static_cast<int64_t>(1 + rng() % (P - 1))); if (rng() & 1) d = -d;
					a = make<nlimbs>(kind == 2 ? 1.0 : 2.0, P) + d;
				}
				else if (kind == 4) {                                                                                 // next to a pole
					Real d = one; d.setexponent(-static_cast<int64_t>(1 + rng() % (P - 8))); if (rng() & 1) d = -d;
					a = make<nlimbs>(-static_cast<double>(1 + rng() % 50), P) + d;
				}
				else { a = random_arg<nlimbs>(P, 0, 0); a.setsign(false); a.setexponent(static_cast<int64_t>(12 + rng() % 40)); }   // large
			}
		}
		else if (is(f, "erf") || is(f, "erfc")) {
			if (special) {
				// |x| next to the switches: erf -> +/-1 at x^2 log2 e ~ P + 6, erfc's asymptotic series at ~ P + 104
				const double t = (kind == 0) ? P + 6.0 : (kind == 1) ? P + 104.0 : (kind == 2) ? P / 2.0 : 0.0;
				if (t > 0) {
					const double x = std::sqrt(t / 1.4426950408889634) * (1.0 + (static_cast<double>(rng() % 2001) - 1000.0) * 1e-5);
					a = make<nlimbs>(x, P) + random_arg<nlimbs>(P, 3, 0) * make<nlimbs>(0.001, P);
				}
				else if (kind == 3) { a = random_arg<nlimbs>(P, 0, 0); a.setexponent(-static_cast<int64_t>(1 + rng() % (2 * P))); }   // tiny
				else { a = random_arg<nlimbs>(P, 0, 0); a.setexponent(static_cast<int64_t>(3 + rng() % 10)); }                       // large
				if (rng() & 1) a = -a;
			}
		}
		else if (is(f, "double") || is(f, "float")) {
			// the whole range of the format and past its ends, or, half the time, its subnormals,
			// where rounding to fewer bits than the format's width goes wrong first
			const int lo = is(f, "double") ? -1100 : -160, hi = is(f, "double") ? 1030 : 135;
			const int sub = is(f, "double") ? -1022 : -126, width = is(f, "double") ? 53 : 24;
			if (rng() & 1) a.setexponent(lo + static_cast<int64_t>(rng() % static_cast<unsigned>(hi - lo)));
			else a.setexponent(sub - static_cast<int64_t>(rng() % static_cast<unsigned>(width + 64)));   // and below them
			if (special) {
				// a power of 2, or one just above it, below the smallest normal
				for (unsigned j = 0; j < (P + 31) / 32; ++j) a.setlimb(j, j == 0 ? 0x80000000u : 0u);
				if (rng() & 1) a.setlimb((P + 31) / 32 - 1, a.bits().size() == 1 ? 0x80000001u : 1u);
				a.setexponent(sub - static_cast<int64_t>(rng() % static_cast<unsigned>(width + 3)));
				if (rng() & 1) a.setsign(true);
			}
		}
		a.set_precision(P);
		b.set_precision(P);
		c.set_precision(P);
	}

	template<unsigned nlimbs>
	efloat<nlimbs> evaluate(const Function& f, const efloat<nlimbs>& a, const efloat<nlimbs>& b, const efloat<nlimbs>& c, int k) {
		if (is(f, "add")) return a + b;
		if (is(f, "sub")) return a - b;
		if (is(f, "mul")) return a * b;
		if (is(f, "div")) return a / b;
		if (is(f, "sqrt")) return sqrt(a);
		if (is(f, "cbrt")) return cbrt(a);
		if (is(f, "hypot")) return hypot(a, b);
		if (is(f, "fma")) return fma(a, b, c);
		if (is(f, "fmod")) return fmod(a, b);
		if (is(f, "remainder")) return remainder(a, b);
		if (is(f, "exp")) return exp(a);
		if (is(f, "exp2")) return exp2(a);
		if (is(f, "exp10")) return exp10(a);
		if (is(f, "expm1")) return expm1(a);
		if (is(f, "log")) return log(a);
		if (is(f, "log2")) return log2(a);
		if (is(f, "log10")) return log10(a);
		if (is(f, "log1p")) return log1p(a);
		if (is(f, "sin")) return sin(a);
		if (is(f, "cos")) return cos(a);
		if (is(f, "tan")) return tan(a);
		if (is(f, "asin")) return asin(a);
		if (is(f, "acos")) return acos(a);
		if (is(f, "atan")) return atan(a);
		if (is(f, "atan2")) return atan2(a, b);
		if (is(f, "sinh")) return sinh(a);
		if (is(f, "cosh")) return cosh(a);
		if (is(f, "tanh")) return tanh(a);
		if (is(f, "asinh")) return asinh(a);
		if (is(f, "acosh")) return acosh(a);
		if (is(f, "atanh")) return atanh(a);
		if (is(f, "pow")) return pow(a, b);
		if (is(f, "powi")) return pow(a, k);
		if (is(f, "tgamma")) return tgamma(a);
		if (is(f, "lgamma")) return lgamma(a);
		if (is(f, "erf")) return erf(a);
		if (is(f, "erfc")) return erfc(a);
		throw std::runtime_error(std::string("no efloat evaluation for ") + f.name);
	}

	void reference(const Function& f, mpfr_t r, const mpfr_t a, const mpfr_t b, const mpfr_t c, int k, mpfr_rnd_t m) {
		int sign = 0;
		if (is(f, "add")) mpfr_add(r, a, b, m);
		else if (is(f, "sub")) mpfr_sub(r, a, b, m);
		else if (is(f, "mul")) mpfr_mul(r, a, b, m);
		else if (is(f, "div")) mpfr_div(r, a, b, m);
		else if (is(f, "sqrt")) mpfr_sqrt(r, a, m);
		else if (is(f, "cbrt")) mpfr_cbrt(r, a, m);
		else if (is(f, "hypot")) mpfr_hypot(r, a, b, m);
		else if (is(f, "fma")) mpfr_fma(r, a, b, c, m);
		else if (is(f, "fmod")) mpfr_fmod(r, a, b, m);
		else if (is(f, "remainder")) mpfr_remainder(r, a, b, m);
		else if (is(f, "exp")) mpfr_exp(r, a, m);
		else if (is(f, "exp2")) mpfr_exp2(r, a, m);
		else if (is(f, "exp10")) mpfr_exp10(r, a, m);
		else if (is(f, "expm1")) mpfr_expm1(r, a, m);
		else if (is(f, "log")) mpfr_log(r, a, m);
		else if (is(f, "log2")) mpfr_log2(r, a, m);
		else if (is(f, "log10")) mpfr_log10(r, a, m);
		else if (is(f, "log1p")) mpfr_log1p(r, a, m);
		else if (is(f, "sin")) mpfr_sin(r, a, m);
		else if (is(f, "cos")) mpfr_cos(r, a, m);
		else if (is(f, "tan")) mpfr_tan(r, a, m);
		else if (is(f, "asin")) mpfr_asin(r, a, m);
		else if (is(f, "acos")) mpfr_acos(r, a, m);
		else if (is(f, "atan")) mpfr_atan(r, a, m);
		else if (is(f, "atan2")) mpfr_atan2(r, a, b, m);
		else if (is(f, "sinh")) mpfr_sinh(r, a, m);
		else if (is(f, "cosh")) mpfr_cosh(r, a, m);
		else if (is(f, "tanh")) mpfr_tanh(r, a, m);
		else if (is(f, "asinh")) mpfr_asinh(r, a, m);
		else if (is(f, "acosh")) mpfr_acosh(r, a, m);
		else if (is(f, "atanh")) mpfr_atanh(r, a, m);
		else if (is(f, "pow")) mpfr_pow(r, a, b, m);
		else if (is(f, "powi")) mpfr_pow_si(r, a, k, m);
		else if (is(f, "tgamma")) mpfr_gamma(r, a, m);
		else if (is(f, "lgamma")) mpfr_lgamma(r, &sign, a, m);
		else if (is(f, "erf")) mpfr_erf(r, a, m);
		else if (is(f, "erfc")) mpfr_erfc(r, a, m);
		else throw std::runtime_error(std::string("no MPFR reference for ") + f.name);
	}

	// one conversion case: efloat -> double or float in mode m against MPFR's
	template<unsigned nlimbs>
	bool check_conversion(const Function& f, const efloat<nlimbs>& a, mpfr_t ma, mpfr_rnd_t m) {
		if (is(f, "double")) {
			const double got = double(a), want = mpfr_get_d(ma, m);
			return std::memcmp(&got, &want, sizeof(double)) == 0;
		}
		const float got = float(a), want = mpfr_get_flt(ma, m);
		return std::memcmp(&got, &want, sizeof(float)) == 0;
	}

	struct Tally { long cases = 0, wrong = 0; double seconds = 0.0; };

	template<unsigned nlimbs>
	Tally run(const Function& f, unsigned P, long count) {
		using Real = efloat<nlimbs>;
		const RoundingMode modes[] = { RoundingMode::RoundToNearest, RoundingMode::RoundToZero, RoundingMode::RoundTowardPositive, RoundingMode::RoundTowardNegative };
		const mpfr_rnd_t mpfr_modes[] = { MPFR_RNDN, MPFR_RNDZ, MPFR_RNDU, MPFR_RNDD };
		const bool exp_like = is(f, "exp2") || is(f, "exp10") || is(f, "expm1") || is(f, "sinh") || is(f, "cosh") || is(f, "tanh") || is(f, "exp");
		const int maxexp = exp_like ? 10 : 40;
		Tally t;
		long by_style[5] = { 0, 0, 0, 0, 0 };
		const mpfr_prec_t wide = static_cast<mpfr_prec_t>(32 * nlimbs + 64);
		mpfr_t ma, mb, mc, want, got;
		mpfr_inits2(wide, ma, mb, mc, got, static_cast<mpfr_ptr>(nullptr));
		mpfr_init2(want, static_cast<mpfr_prec_t>(P));
		for (long i = 0; i < count; ++i) {
			const unsigned style = static_cast<unsigned>(rng() % 5);
			Real a = random_arg<nlimbs>(P, style == 4 ? 0 : style, maxexp);
			Real b = random_arg<nlimbs>(P, static_cast<unsigned>(rng() % 4), 3);
			Real c = random_arg<nlimbs>(P, static_cast<unsigned>(rng() % 4), 3);
			int k = 0;
			shape<nlimbs>(f, P, style, a, b, c, k);
			const int mi = static_cast<int>(i % 4);
			to_mpfr(ma, a);
			to_mpfr(mb, b);
			to_mpfr(mc, c);
			bool ok = false;
			efloat_rounding_mode = modes[mi];
			if (is(f, "double") || is(f, "float")) {
				ok = check_conversion(f, a, ma, mpfr_modes[mi]);
				efloat_rounding_mode = RoundingMode::RoundToNearest;
			}
			else {
				const auto t0 = std::chrono::steady_clock::now();
				const Real r = evaluate(f, a, b, c, k);
				t.seconds += std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
				efloat_rounding_mode = RoundingMode::RoundToNearest;
				mpfr_clear_flags();
				reference(f, want, ma, mb, mc, k, mpfr_modes[mi]);
				// a result past the exponent range MPFR holds is not compared
				if (mpfr_overflow_p() || mpfr_underflow_p()) continue;
				to_mpfr(got, r);
				ok = (mpfr_nan_p(want) && mpfr_nan_p(got)) ||
				     (mpfr_equal_p(want, got) && (mpfr_signbit(want) != 0) == (mpfr_signbit(got) != 0) &&
				      (r.get_precision() == P || !mpfr_regular_p(want)));
				if (!ok && t.wrong < 3 && f.known == nullptr) {
					std::printf("  %s P=%u mode=%d style=%s precision=%u x=", f.name, P, mi, style_names[style], r.get_precision());
					mpfr_out_str(stdout, 10, 25, ma, MPFR_RNDN);
					if (f.arity >= 2) { std::printf(" y="); if (f.integer) std::printf("%d", k); else mpfr_out_str(stdout, 10, 25, mb, MPFR_RNDN); }
					std::printf(" got="); mpfr_out_str(stdout, 10, 25, got, MPFR_RNDN);
					std::printf(" want="); mpfr_out_str(stdout, 10, 25, want, MPFR_RNDN);
					std::printf("\n");
				}
			}
			++t.cases;
			if (!ok) { ++t.wrong; ++by_style[style]; }
		}
		mpfr_clears(ma, mb, mc, want, got, static_cast<mpfr_ptr>(nullptr));
		std::printf("%-9s P=%5u: %6ld cases, %5ld wrong (by style %ld %ld %ld %ld %ld), %10.1f us mean%s%s\n", f.name, P, t.cases, t.wrong,
		            by_style[0], by_style[1], by_style[2], by_style[3], by_style[4], t.cases ? 1e6 * t.seconds / static_cast<double>(t.cases) : 0.0,
		            f.known ? "  known, " : "", f.known ? f.known : "");
		std::fflush(stdout);
		return t;
	}

}} // namespace sw::universal

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	// MPFR's widest exponent range, so that efloat's results out to 2^(2^62) compare
	mpfr_set_emin(mpfr_get_emin_min());
	mpfr_set_emax(mpfr_get_emax_max());

	const double scale = (argc > 1) ? std::atof(argv[1]) : 1.0;
	const std::string selection = (argc > 2) ? argv[2] : "all";
	const unsigned max_bits = (argc > 3) ? static_cast<unsigned>(std::atoi(argv[3])) : 8192u;
	auto cases = [scale](long base) { return std::max(1L, static_cast<long>(scale * static_cast<double>(base))); };

	std::printf("efloat against MPFR %s (GMP %s), all four rounding modes, scale %g, up to %u bits (#1782)\n",
	            mpfr_get_version(), gmp_version, scale, max_bits);
	long wrong = 0, known_wrong = 0;
	for (const Function& f : functions) {
		if (selection != "all" && ("," + selection + ",").find(std::string(",") + f.name + ",") == std::string::npos) continue;
		long w = 0;
		w += run<1>(f, 32, cases(1000)).wrong;
		w += run<2>(f, 64, cases(1000)).wrong;
		w += run<4>(f, 128, cases(1000)).wrong;
		if (max_bits >= 512) w += run<16>(f, 512, cases(200)).wrong;
		if (max_bits >= 2048) w += run<64>(f, 2048, cases(30)).wrong;
		if (max_bits >= 8192) w += run<256>(f, 8192, cases(4)).wrong;
		(f.known ? known_wrong : wrong) += w;
	}
	std::printf("wrong: %ld in the correctly rounded functions, %ld in the known ones\n", wrong, known_wrong);
	return wrong == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
