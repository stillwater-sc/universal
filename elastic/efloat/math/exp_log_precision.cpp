// exp_log_precision.cpp: regression tests for the accuracy of efloat's exp/log family at high precision.
//
// Guards issue #1712: exp, exp2, exp10, log, log2 and log10 reduced their arguments with ln 2 and
// ln 10 parsed from 59-digit literals (~196 bits), so they agreed with mpmath to ~2^-200 whatever
// precision was asked for; exp's Taylor series, log's Newton iteration and log1p's series ran a
// fixed number of steps; and the trigonometric functions took pi at the type's width, not the
// value's, and atan(1/2) from a 64-digit literal.  Every function here must now be accurate to nearly the full
// precision of its result, against mpmath references (tools/generators/efloat_exp_log_gen.py).
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <random>
#include <string>
#include <universal/number/efloat/efloat.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/efloat_exp_log_vectors.hpp>

namespace {

using namespace sw::universal;

template<unsigned nlimbs>
efloat<nlimbs> evaluate(const char* function, const efloat<nlimbs>& x, const efloat<nlimbs>& y) {
	const std::string f(function);
	if (f == "exp")   return exp(x);
	if (f == "exp2")  return exp2(x);
	if (f == "exp10") return exp10(x);
	if (f == "expm1") return expm1(x);
	if (f == "log")   return log(x);
	if (f == "log2")  return log2(x);
	if (f == "log10") return log10(x);
	if (f == "log1p") return log1p(x);
	if (f == "sinh")  return sinh(x);
	if (f == "cosh")  return cosh(x);
	if (f == "tanh")  return tanh(x);
	if (f == "asinh") return asinh(x);
	if (f == "acosh") return acosh(x);
	if (f == "atanh") return atanh(x);
	if (f == "atan")  return atan(x);
	if (f == "pow")   return pow(x, y);
	if (f == "sin")   return sin(x);
	if (f == "cos")   return cos(x);
	if (f == "tan")   return tan(x);
	if (f == "asin")  return asin(x);
	if (f == "acos")  return acos(x);
	if (f == "atan2") return atan2(x, y);
	efloat<nlimbs> nan;
	nan.setnan();
	return nan;
}

// The number of bits to which got agrees with the reference, relative to the reference:
// scale(reference) - scale(got - reference).  An exact result counts as 100000.
template<unsigned nlimbs>
int64_t agreement_bits(const efloat<nlimbs>& got, const efloat<nlimbs>& reference) {
	if (got.isnan() || got.isinf()) return -100000;
	efloat<nlimbs> difference = got - reference;
	if (difference.iszero()) return 100000;
	return reference.scale() - difference.scale();
}

// Each function at the precision of efloat<nlimbs>, nlimbs * 32 bits; it must agree with the
// reference to within `slack` bits of that.
template<unsigned nlimbs>
int VerifyAtPrecision(bool reportTestCases, int64_t slack) {
	using namespace efloat_exp_log_verification;
	const int64_t precision = static_cast<int64_t>(nlimbs) * 32;
	int fails = 0;
	for (const auto& v : function_vectors) {
		const efloat<nlimbs> x(v.x), y(v.y);
		efloat<nlimbs> reference;
		reference.set_precision(nlimbs * 32 + 64);
		if (!parse<16384>(std::string(v.value), reference)) {
			std::cerr << "FAIL: cannot parse the reference for " << v.function << '\n';
			++fails;
			continue;
		}
		const efloat<nlimbs> got = evaluate(v.function, x, y);
		const int64_t bits = agreement_bits(got, reference);
		const bool ok = (bits >= precision - slack) && (got.get_precision() == static_cast<unsigned>(precision));
		if (!ok) ++fails;
		if (reportTestCases && !ok) {
			std::cerr << (ok ? "     " : "FAIL ") << "efloat<" << nlimbs << "> " << v.function << '(' << v.x;
			if (std::strcmp(v.function, "pow") == 0 || std::strcmp(v.function, "atan2") == 0) std::cerr << ", " << v.y;
			std::cerr << "): " << bits << " of " << precision << " bits, result precision " << got.get_precision() << '\n';
		}
	}
	return fails;
}

// ln 2, ln 10, pi and atan(1/2) are computed at the working precision: the functions are accurate
// at every width, including widths past nlimbs * 32 that set_precision() asks for
template<unsigned nlimbs>
int VerifySetPrecision(bool reportTestCases, unsigned bits, int64_t slack) {
	int fails = 0;
	efloat<nlimbs> x(0.75);
	x.set_precision(bits);
	efloat<nlimbs> two(2.0);
	two.set_precision(bits);
	const efloat<nlimbs> ln2 = log(two);
	// exp(log 2) == 2
	const int64_t a = agreement_bits(exp(ln2), two);
	// exp(1000.5 - 1443 ln 2) == exp(1000.5) 2^-1443 through the range reduction
	efloat<nlimbs> big(1000.5);
	big.set_precision(bits);
	efloat<nlimbs> k(1443.0);
	k.set_precision(bits);
	efloat<nlimbs> lhs = exp(big - k * ln2), rhs = exp(big);
	rhs.setexponent(rhs.scale() - 1443);
	const int64_t b = agreement_bits(lhs, rhs);
	// sin(10) reduces by 2 pi once and atan(0.75) adds atan(1/2): pi and atan(1/2) at the
	// widened precision, not at nlimbs * 32 bits
	int64_t c = 100000, d = 100000;
	for (const auto& v : efloat_exp_log_verification::function_vectors) {
		const bool is_sin = std::strcmp(v.function, "sin") == 0 && v.x == 10.0;
		const bool is_atan = std::strcmp(v.function, "atan") == 0 && v.x == 0.75;
		if (!is_sin && !is_atan) continue;
		efloat<nlimbs> arg(v.x), reference;
		arg.set_precision(bits);
		reference.set_precision(bits + 64);
		parse<16384>(std::string(v.value), reference);
		if (is_sin) c = agreement_bits(sin(arg), reference);
		else d = agreement_bits(atan(arg), reference);
	}
	if (c < static_cast<int64_t>(bits) - slack) ++fails;
	if (d < static_cast<int64_t>(bits) - slack) ++fails;
	if (a < static_cast<int64_t>(bits) - slack) ++fails;
	if (b < static_cast<int64_t>(bits) - slack - 12) ++fails;        // k ln 2 carries log2(1443) bits more error
	if (reportTestCases && fails) std::cerr << "FAIL: set_precision(" << bits << "): exp(log 2) " << a << " bits, reduction " << b << " bits, sin(10) " << c << " bits, atan(0.75) " << d << " bits\n";
	return fails;
}

// Results near the zeros of sin, cos and tan, and acos and asin near 1, where a reduction or a
// subtraction cancels.  The arguments are pi and pi/2 rounded to the full precision, and 1 - e
// for a full-mantissa e; the references are delta = pi - pi_P from the 1000-digit pi literal,
// sin(delta) = delta - delta^3/6, and the series of acos(1 - e) in e, all exact far below the
// precision checked, and none computed by the functions under test.
template<unsigned nlimbs>
int VerifyNearZeros(bool reportTestCases, int64_t slack) {
	const unsigned P = nlimbs * 32;
	const int64_t precision = static_cast<int64_t>(P);
	int fails = 0;
	auto check = [&](const char* what, const efloat<nlimbs>& got, efloat<nlimbs> reference) {
		reference.set_precision(P);
		const int64_t bits = agreement_bits(got, reference);
		if (bits < precision - slack) {
			++fails;
			if (reportTestCases) std::cerr << "FAIL efloat<" << nlimbs << "> " << what << ": " << bits << " of " << P << " bits\n";
		}
	};
	efloat<nlimbs> pi_p, pi_h;
	pi_p.set_precision(P);
	pi_h.set_precision(3 * P);
	parse<16384>(EFLOAT_PI_DIGITS, pi_p);
	parse<16384>(EFLOAT_PI_DIGITS, pi_h);
	const efloat<nlimbs> delta = pi_h - pi_p;                      // exact
	check("sin(pi rounded)", sin(pi_p), delta - delta * delta * delta / efloat<nlimbs>(6.0));
	efloat<nlimbs> half_pi_p(pi_p), half_delta(delta);
	half_pi_p.setexponent(half_pi_p.scale() - 1);
	half_delta.setexponent(half_delta.scale() - 1);
	check("cos(pi/2 rounded)", cos(half_pi_p), half_delta);       // sin(d) = d to 2^-3P
	check("tan(pi/2 rounded)", tan(half_pi_p), efloat<nlimbs>(1.0) / half_delta);
	check("sin(-pi rounded)", sin(efloat<nlimbs>(0.0) - pi_p), efloat<nlimbs>(0.0) - delta + delta * delta * delta / efloat<nlimbs>(6.0));

	efloat<nlimbs> half_pi_h(pi_h);
	half_pi_h.setexponent(half_pi_h.scale() - 1);
	for (int shift : { 100, 300, static_cast<int>(P / 2 + 50) }) {
		efloat<nlimbs> e(1.0);
		e.set_precision(P);
		e = e / efloat<nlimbs>(3.0);
		e.setexponent(e.scale() - shift);
		efloat<nlimbs> x = efloat<nlimbs>(1.0) - e;
		x.set_precision(P);
		e = efloat<nlimbs>(1.0) - x;                               // the exact distance of the rounded x
		e.set_precision(3 * P);
		efloat<nlimbs> two_e(e);
		two_e.setexponent(two_e.scale() + 1);
		// acos(1 - e) = sqrt(2e) sum_n c_n e^n, c_n = (2n)! / (2^(3n) (n!)^2 (2n + 1)), summed
		// until the terms fall below 2^-3P: c_(n+1) / c_n = (2n + 1)^2 / (4 (n + 1) (2n + 3))
		efloat<nlimbs> series(1.0), c(1.0), power(1.0);
		series.set_precision(3 * P);
		for (unsigned n = 0; n < 10000; ++n) {
			c = c * efloat<nlimbs>(static_cast<double>((2 * n + 1) * (2 * n + 1))) / efloat<nlimbs>(static_cast<double>(4 * (n + 1) * (2 * n + 3)));
			power = power * e;
			const efloat<nlimbs> term = c * power;
			if (term.iszero() || term.scale() < -static_cast<int64_t>(3 * P)) break;
			series += term;
		}
		const efloat<nlimbs> acos_ref = sqrt(two_e) * series;
		const std::string tag = "(1 - 2^-" + std::to_string(shift) + "/3)";
		check(("acos" + tag).c_str(), acos(x), acos_ref);
		check(("asin" + tag).c_str(), asin(x), half_pi_h - acos_ref);
		check(("acos(-" + tag.substr(1)).c_str(), acos(efloat<nlimbs>(0.0) - x), pi_h - acos_ref);
	}
	return fails;
}

// A deterministic stream of 32-bit values for the arguments below
struct ArgumentStream {
	uint64_t state;
	uint32_t next() {
		state = state * 6364136223846793005ull + 1442695040888963407ull;
		return static_cast<uint32_t>(state >> 32);
	}
};

// The AGM (#1763) against the table and series: two independent evaluations of log at the same
// working precision W = nlimbs * 32 + 64, which must agree to within 2^-(W - log_error_bits)
// relative to the result. The arguments are full-precision significands with small and large
// exponents, x within 2^-k of 1 on both sides of the precision where log() switches to the AGM
// and out to k = P/2 (the AGM then carries the ~k bits log s and m ln 2 cancel), and powers of 2.
template<unsigned nlimbs>
int VerifyAgmAgainstSeries(bool reportTestCases, unsigned count) {
	const unsigned P = nlimbs * 32;
	const unsigned W = P + 64;
	ArgumentStream g{ 0x9E3779B97F4A7C15ull ^ W };
	int fails = 0;
	for (unsigned i = 0; i < count; ++i) {
		efloat<nlimbs> x(1.0);
		x.set_precision(P);
		for (unsigned j = 0; j < nlimbs; ++j) x.setlimb(j, g.next() | (j == 0 ? 0x80000000u : 0u));
		const unsigned style = i % 5;
		if (style == 0) {
			x.setexponent(static_cast<int64_t>(g.next() % 64) - 32);
		}
		else if (style == 1) {
			x.setexponent(static_cast<int64_t>(g.next() % 20001) - 10000);
		}
		else if (style == 2 || style == 3) {
			// past where log() itself would take the AGM, out to x within 2^-(P/2) of 1, where the
			// AGM carries ~P/2 cancelled bits
			const unsigned k = 1 + g.next() % ((i % 2) ? W / efloat_detail::log_agm_bits_per_series_bit + 16 : P / 2);
			x.setexponent(-static_cast<int64_t>(k));
			efloat<nlimbs> one(1.0);
			one.set_precision(P);
			x = (style == 2) ? one + x : one - x;
			x.set_precision(P);
		}
		else {
			x = efloat<nlimbs>(1.0);
			x.set_precision(P);
			x.setexponent(static_cast<int64_t>(g.next() % 4000) - 2000 + 1);   // never 2^0
		}
		const efloat<nlimbs> agm    = efloat_detail::log_agm(x, W);
		const efloat<nlimbs> series = efloat_detail::log_table_series(x, W);
		const int64_t bits = agreement_bits(agm, series);
		if (bits < static_cast<int64_t>(W - efloat_detail::log_error_bits)) {
			++fails;
			if (reportTestCases) std::cerr << "FAIL efloat<" << nlimbs << "> log AGM vs series, style " << style << ": " << bits << " of " << W << " bits\n";
		}
		// the AGM's estimate of the bits it cancels: floor(log2 |log x|), or up to two less
		const int64_t magnitude = efloat_detail::log_magnitude(x);
		if (magnitude > series.scale() || magnitude < series.scale() - 2) {
			++fails;
			if (reportTestCases) std::cerr << "FAIL efloat<" << nlimbs << "> log_magnitude " << magnitude << " for a log of scale " << series.scale() << '\n';
		}
	}
	return fails;
}

// log(1 + t) for a short t = +-r 2^-k, r < 2^24, rounded in all four modes. log(1 + t) =
// t - t^2/2 + t^3/3 - ... is then a short value plus a tail far below it, so it lies within
// ~2^-2k of a rounding boundary, past what the working precision's guard bits resolve: only a
// retry at a wider precision rounds it correctly (#1763). The reference sums the series at 4P bits.
template<unsigned nlimbs>
int VerifyNearRoundingBoundaries(bool reportTestCases, unsigned count) {
	const unsigned P = nlimbs * 32;
	const RoundingMode modes[] = { RoundingMode::RoundToNearest, RoundingMode::RoundToZero,
	                               RoundingMode::RoundTowardPositive, RoundingMode::RoundTowardNegative };
	const char* names[] = { "nearest", "toward zero", "toward +inf", "toward -inf" };
	const RoundingMode saved = efloat_rounding_mode;
	ArgumentStream g{ 0xD1B54A32D192ED03ull ^ P };
	int fails = 0;
	for (unsigned i = 0; i < count; ++i) {
		const bool below = (i % 2) == 0;
		const unsigned rbits = 1 + g.next() % 24;
		const double r = static_cast<double>((g.next() & ((1u << rbits) - 1u)) | 1u);
		const unsigned k = P / 2 + g.next() % (P / 2);   // 1 +- r 2^-k is exact at P bits
		efloat<nlimbs> t(r);
		t.set_precision(4 * P);
		t.setexponent(t.scale() - static_cast<int64_t>(k));
		if (below) t = efloat<nlimbs>(0.0) - t;
		efloat<nlimbs> x(1.0);
		x.set_precision(P);
		x = x + t;
		x.set_precision(P);

		efloat<nlimbs> reference(t), power(t);
		reference.set_precision(4 * P);
		for (unsigned n = 2; n < 64; ++n) {
			power = power * t;
			const efloat<nlimbs> term = power / efloat<nlimbs>(static_cast<double>(n));
			if (term.iszero() || term.scale() < reference.scale() - static_cast<int64_t>(4 * P)) break;
			if (n % 2 == 0) reference -= term; else reference += term;
		}
		for (unsigned m = 0; m < 4; ++m) {
			efloat_rounding_mode = modes[m];
			efloat<nlimbs> expected(reference);
			expected.set_precision(P);
			const efloat<nlimbs> got = log(x);
			efloat_rounding_mode = saved;
			if (!(got == expected) || got.get_precision() != P) {
				++fails;
				if (reportTestCases) std::cerr << "FAIL efloat<" << nlimbs << "> log(1 " << (below ? "- " : "+ ") << r << " 2^-" << k << ") rounded " << names[m] << '\n';
			}
		}
	}
	efloat_rounding_mode = saved;
	return fails;
}

// exp, sin, cos and tan of a short t = +-r 2^-k (r < 2^24) and of a full-width tiny t, rounded in
// all four modes (#1763). Their series are then a short or exact leading part plus a tail far
// below it: sin t = t - t^3/6 + ... lies within ~2^-2k of a rounding boundary, past what the
// working precision's guard bits resolve, and for |t| < 2^-(P/2) the tail lies below any working
// precision, so only the tiny-argument rounding gets the directed modes right. The references
// are the series summed at 6P bits, which resolve the tail for every t generated here.
template<unsigned nlimbs>
int VerifyFunctionsNearRoundingBoundaries(bool reportTestCases, unsigned count) {
	const unsigned P = nlimbs * 32;
	const unsigned R = 6 * P;
	const RoundingMode modes[] = { RoundingMode::RoundToNearest, RoundingMode::RoundToZero,
	                               RoundingMode::RoundTowardPositive, RoundingMode::RoundTowardNegative };
	const char* names[] = { "nearest", "toward zero", "toward +inf", "toward -inf" };
	const char* functions[] = { "exp", "sin", "cos", "tan" };
	const RoundingMode saved = efloat_rounding_mode;
	ArgumentStream g{ 0x8CB92BA72F3D8DD7ull ^ P };
	int fails = 0;
	for (unsigned i = 0; i < count; ++i) {
		efloat<nlimbs> t;
		if (i % 2 == 0) {   // short: r 2^-k, k in [P/4, P)
			const unsigned rbits = 1 + g.next() % 24;
			t = efloat<nlimbs>(static_cast<double>((g.next() & ((1u << rbits) - 1u)) | 1u));
			t.set_precision(P);
			t.setexponent(-static_cast<int64_t>(P / 4 + g.next() % (3 * P / 4)));
		}
		else {              // full width, scale in [-2P, -P/4)
			t = efloat<nlimbs>(1.0);
			t.set_precision(P);
			for (unsigned j = 0; j < nlimbs; ++j) t.setlimb(j, g.next() | (j == 0 ? 0x80000000u : 0u));
			t.set_precision(P);
			t.setexponent(-static_cast<int64_t>(P / 4 + g.next() % (7 * P / 4)));
		}
		if (g.next() & 1u) t = efloat<nlimbs>(0.0) - t;
		t.set_precision(P);

		// the series at R bits: exp = sum t^n / n!, sin and cos its odd and even terms
		efloat<nlimbs> tr(t), term(1.0);
		tr.set_precision(R);
		term.set_precision(R);
		efloat<nlimbs> exp_ref(1.0), sin_ref(0.0), cos_ref(1.0);
		exp_ref.set_precision(R);
		sin_ref.set_precision(R);
		cos_ref.set_precision(R);
		for (unsigned n = 1; n < 1000; ++n) {
			term = term * tr / efloat<nlimbs>(static_cast<double>(n));
			if (term.iszero() || term.scale() < tr.scale() - static_cast<int64_t>(R)) break;   // below every series' R bits
			exp_ref += term;
			const bool negative = ((n / 2) % 2) == 1;   // the signs of sin and cos: + - - + + - ...
			if (n % 2) { if (negative) sin_ref -= term; else sin_ref += term; }
			else       { if (negative) cos_ref -= term; else cos_ref += term; }
		}
		const efloat<nlimbs> tan_ref = sin_ref / cos_ref;
		const efloat<nlimbs>* references[] = { &exp_ref, &sin_ref, &cos_ref, &tan_ref };

		for (unsigned f = 0; f < 4; ++f) {
			for (unsigned m = 0; m < 4; ++m) {
				efloat_rounding_mode = modes[m];
				efloat<nlimbs> expected(*references[f]);
				expected.set_precision(P);
				const efloat<nlimbs> got = (f == 0) ? exp(t) : (f == 1) ? sin(t) : (f == 2) ? cos(t) : tan(t);
				efloat_rounding_mode = saved;
				if (!(got == expected) || got.get_precision() != P) {
					++fails;
					if (reportTestCases) std::cerr << "FAIL efloat<" << nlimbs << "> " << functions[f] << "(t), t of scale " << t.scale() << ", rounded " << names[m] << '\n';
				}
			}
		}
	}
	efloat_rounding_mode = saved;
	return fails;
}

// The remaining functions near their rounding boundaries (#1763), in all four modes: expm1, sinh,
// cosh, tanh, asinh, atanh, atan, asin, log1p, exp2, exp10 and atan2(t, 1), atan2(t, 3) of a short
// or full-width tiny t, as above, and log2, log10, 1 / x and x^3 of x = 1 + t for a short t, where
// 1 + t is exact. Each is a short or exact leading part plus a tail far below it. The references
// are power series in t summed at 6P bits, apart from the library's ln 2, ln 10 and division.
template<unsigned nlimbs>
int VerifyRemainingFunctionsNearRoundingBoundaries(bool reportTestCases, unsigned count) {
	using R_t = efloat<nlimbs>;
	constexpr unsigned P = nlimbs * 32;
	constexpr unsigned R = 6 * P;
	const RoundingMode modes[] = { RoundingMode::RoundToNearest, RoundingMode::RoundToZero,
	                               RoundingMode::RoundTowardPositive, RoundingMode::RoundTowardNegative };
	const char* names[] = { "nearest", "toward zero", "toward +inf", "toward -inf" };
	const char* functions[] = { "expm1", "sinh", "cosh", "tanh", "asinh", "atanh", "atan", "asin", "log1p",
	                            "exp2", "exp10", "atan2(t, 1)", "atan2(t, 3)", "log2(1 + t)", "log10(1 + t)",
	                            "pow(1 + t, -1)", "pow(1 + t, 3)" };
	constexpr unsigned F = 17;
	const RoundingMode saved = efloat_rounding_mode;
	ArgumentStream g{ 0x5851F42D4C957F2Dull ^ P };
	int fails = 0;
	for (unsigned i = 0; i < count; ++i) {
		const bool shortarg = (i % 2) == 0;
		R_t t;
		if (shortarg) {     // r 2^-k, r odd of rbits <= min(24, P/4) bits, k in [P/4 + rbits, P): below 2^-(P/4), and 1 + t is exact at P bits
			const unsigned rbits = 1 + g.next() % std::min(24u, P / 4);
			const unsigned k     = P / 4 + rbits + g.next() % (3 * P / 4 - rbits);
			t = R_t(static_cast<double>((g.next() & ((1u << rbits) - 1u)) | 1u));
			t.set_precision(P);
			t.setexponent(t.scale() - static_cast<int64_t>(k));
		}
		else {              // full width, scale in [-2P, -P/4)
			t = R_t(1.0);
			t.set_precision(P);
			for (unsigned j = 0; j < nlimbs; ++j) t.setlimb(j, g.next() | (j == 0 ? 0x80000000u : 0u));
			t.set_precision(P);
			t.setexponent(-static_cast<int64_t>(P / 4 + g.next() % (7 * P / 4)));
		}
		if (g.next() & 1u) t = -t;
		t.set_precision(P);

		// power series in u at R bits, with coefficients c(n) for n = 1, 2, ...
		auto series = [](const R_t& u, auto coefficient) {
			R_t ur(u), power(1.0), sum(0.0);
			ur.set_precision(R);
			power.set_precision(R);
			sum.set_precision(R);
			for (unsigned n = 1; n < 2000; ++n) {
				power *= ur;
				if (power.scale() < ur.scale() - static_cast<int64_t>(R) - 8) break;   // below R bits of every result
				const R_t term = coefficient(n, power);
				if (!term.iszero()) sum += term;
			}
			return sum;
		};
		auto odd = [](unsigned n) { return (n % 2) == 1; };
		// exp(u) - 1 = sum u^n / n!: the factorial falls with the power, so the terms are carried
		auto expm1_series = [&](const R_t& u) {
			R_t term(1.0);
			term.set_precision(R);
			return series(u, [&term](unsigned n, const R_t& power) { term /= n; return power * term; });
		};
		const R_t em1   = expm1_series(t);
		R_t sinh_ref(0.0), cosh_ref(1.0);
		{
			R_t term(1.0);
			term.set_precision(R);
			sinh_ref = series(t, [&](unsigned n, const R_t& power) { term /= n; return odd(n) ? power * term : R_t(0.0); });
			term = R_t(1.0);
			term.set_precision(R);
			cosh_ref = series(t, [&](unsigned n, const R_t& power) { term /= n; return odd(n) ? R_t(0.0) : power * term; }) + 1;
		}
		const R_t tanh_ref = sinh_ref / cosh_ref;
		// asin u = sum a_k u^(2k + 1) / (2k + 1), a_k = (2k - 1)!! / (2k)!!; asinh alternates it
		R_t a(1.0);
		a.set_precision(R);
		const R_t asin_ref = series(t, [&](unsigned n, const R_t& power) {
			if (!odd(n)) return R_t(0.0);
			if (n > 1) { a *= n - 2; a /= n - 1; }
			return power * a / n;
		});
		a = R_t(1.0);
		a.set_precision(R);
		const R_t asinh_ref = series(t, [&](unsigned n, const R_t& power) {
			if (!odd(n)) return R_t(0.0);
			if (n > 1) { a *= n - 2; a /= n - 1; }
			const R_t term = power * a / n;
			return ((n / 2) % 2) ? -term : term;
		});
		const R_t atanh_ref = series(t, [&](unsigned n, const R_t& power) { return odd(n) ? power / n : R_t(0.0); });
		auto atan_series = [&](const R_t& u) {
			return series(u, [&](unsigned n, const R_t& power) {
				if (!odd(n)) return R_t(0.0);
				const R_t term = power / n;
				return ((n / 2) % 2) ? -term : term;
			});
		};
		const R_t atan_ref  = atan_series(t);
		const R_t log1p_ref = series(t, [&](unsigned n, const R_t& power) { const R_t term = power / n; return odd(n) ? term : -term; });
		R_t tr(t);
		tr.set_precision(R);
		const R_t exp2_ref  = expm1_series(tr * efloat_detail::ln2<nlimbs>(R)) + 1;
		const R_t exp10_ref = expm1_series(tr * efloat_detail::ln10<nlimbs>(R)) + 1;
		const R_t atan3_ref = atan_series(tr / 3);
		const R_t log2_ref  = log1p_ref / efloat_detail::ln2<nlimbs>(R);
		const R_t log10_ref = log1p_ref / efloat_detail::ln10<nlimbs>(R);
		const R_t x1        = tr + 1;   // exact
		const R_t recip_ref = 1 / x1;
		const R_t cube_ref  = x1 * x1 * x1;
		const R_t* references[F] = { &em1, &sinh_ref, &cosh_ref, &tanh_ref, &asinh_ref, &atanh_ref, &atan_ref, &asin_ref,
		                             &log1p_ref, &exp2_ref, &exp10_ref, &atan_ref, &atan3_ref, &log2_ref, &log10_ref,
		                             &recip_ref, &cube_ref };

		R_t x(t);
		x += 1;   // 1 + t at P bits, exact for a short t
		R_t one(1.0), three(3.0);
		one.set_precision(P);
		three.set_precision(P);
		for (unsigned f = 0; f < F; ++f) {
			if (f >= 13 && !shortarg) continue;
			for (unsigned m = 0; m < 4; ++m) {
				efloat_rounding_mode = modes[m];
				R_t expected(*references[f]);
				expected.set_precision(P);
				R_t got;
				switch (f) {
				case 0: got = expm1(t); break;
				case 1: got = sinh(t); break;
				case 2: got = cosh(t); break;
				case 3: got = tanh(t); break;
				case 4: got = asinh(t); break;
				case 5: got = atanh(t); break;
				case 6: got = atan(t); break;
				case 7: got = asin(t); break;
				case 8: got = log1p(t); break;
				case 9: got = exp2(t); break;
				case 10: got = exp10(t); break;
				case 11: got = atan2(t, one); break;
				case 12: got = atan2(t, three); break;
				case 13: got = log2(x); break;
				case 14: got = log10(x); break;
				case 15: got = pow(x, -1); break;
				default: got = pow(x, 3); break;
				}
				efloat_rounding_mode = saved;
				if (!(got == expected) || got.get_precision() != P) {
					++fails;
					if (reportTestCases) std::cerr << "FAIL efloat<" << nlimbs << "> " << functions[f] << ", t of scale " << t.scale() << (shortarg ? " (short)" : "") << ", rounded " << names[m] << '\n';
				}
			}
		}
	}
	efloat_rounding_mode = saved;
	return fails;
}

// The exact values the remaining functions take (#1763), in all four modes: 2^k, 10^k, log2(2^k),
// log10(10^k), x^n and x^(m/2) of perfect squares, which may lie on rounding boundaries, and the
// zeros acosh(1) and acos(1), which are +0 in every mode, and atan2 of signed zeros. Each is compared with the value formed
// exactly and rounded once. Then the shortcuts for arguments too small for any working precision.
template<unsigned nlimbs>
int VerifyExactValues(bool reportTestCases) {
	using R_t = efloat<nlimbs>;
	constexpr unsigned P = nlimbs * 32;
	const RoundingMode modes[] = { RoundingMode::RoundToNearest, RoundingMode::RoundToZero,
	                               RoundingMode::RoundTowardPositive, RoundingMode::RoundTowardNegative };
	const char* names[] = { "nearest", "toward zero", "toward +inf", "toward -inf" };
	const RoundingMode saved = efloat_rounding_mode;
	auto at_P = [](double v) { R_t r(v); r.set_precision(P); return r; };
	// v rounded to P bits in the current mode
	auto rounded = [](R_t v) { v.set_precision(P); return v; };
	int fails = 0;
	auto check = [&](const char* what, int k, const R_t& got, const R_t& expected, unsigned m) {
		const bool ok = (got == expected) && got.sign() == expected.sign() && got.get_precision() == P;
		if (!ok) {
			++fails;
			if (reportTestCases) std::cerr << "FAIL efloat<" << nlimbs << "> " << what << ", k = " << k << ", rounded " << names[m] << '\n';
		}
	};
	for (unsigned m = 0; m < 4; ++m) {
		for (int k = -60; k <= 60; ++k) {
			R_t p2(1.0);
			p2.set_precision(P);
			p2.setexponent(k);
			R_t p10(1.0);
			p10.set_precision(16 * P);
			for (int j = 0; j < k; ++j) p10 *= 10;   // exact: 10^60 has 140 significant bits
			efloat_rounding_mode = modes[m];
			check("exp2(k)", k, exp2(at_P(k)), rounded(p2), m);
			if (k >= 0) check("exp10(k)", k, exp10(at_P(k)), rounded(p10), m);
			check("log2(2^k)", k, log2(p2), rounded(at_P(k)), m);
			// 10^k = 5^k 2^k is a P-bit value while 5^k has at most P bits
			if (k >= 0 && 2.33 * k < P) check("log10(10^k)", k, log10(rounded(p10)), rounded(at_P(k)), m);
			efloat_rounding_mode = saved;
		}
		for (int k = 1; k <= 30; ++k) {
			// 3^k, 7^k and s^3 for s = 2k + 1, as pow(3, k), pow(7, k) and pow(s^2, 3/2): exact, and
			// wider than P bits for the larger k
			R_t p3(1.0), p7(1.0);
			p3.set_precision(4 * P + 128);
			p7.set_precision(4 * P + 128);
			for (int j = 0; j < k; ++j) { p3 *= 3; p7 *= 7; }
			const double s = 2.0 * k + 1.0;
			R_t s3(s);
			s3.set_precision(P);
			s3 = s3 * s3 * s3;
			efloat_rounding_mode = modes[m];
			check("pow(3, k)", k, pow(at_P(3.0), k), rounded(p3), m);
			check("pow(-7, k)", k, pow(at_P(-7.0), at_P(k)), rounded((k % 2) ? -p7 : p7), m);
			check("pow(s^2, 3/2)", k, pow(at_P(s * s), at_P(1.5)), rounded(s3), m);
			check("acosh(1)", k, acosh(at_P(1.0)), at_P(0.0), m);
			check("acos(1)", k, acos(at_P(1.0)), at_P(0.0), m);
			efloat_rounding_mode = saved;
		}
		{
			// atan2 of signed zeros, as IEEE-754: +/-0 against +0, +/-pi against -0
			const R_t pz = at_P(0.0), nz = -pz;
			R_t pi_wide = efloat_detail::pi<nlimbs>(8 * P);
			efloat_rounding_mode = modes[m];
			check("atan2(+0, +0)", 0, atan2(pz, pz), pz, m);
			check("atan2(-0, +0)", 0, atan2(nz, pz), nz, m);
			check("atan2(+0, -0)", 0, atan2(pz, nz), rounded(pi_wide), m);
			check("atan2(-0, -0)", 0, atan2(nz, nz), rounded(-pi_wide), m);
			R_t inf;
			inf.setinf(false);
			inf.set_precision(P);
			const R_t ninf = -inf;
			check("atan2(+0, 1)", 0, atan2(pz, at_P(1.0)), pz, m);
			check("atan2(-0, 1)", 0, atan2(nz, at_P(1.0)), nz, m);
			check("atan2(-0, -1)", 0, atan2(nz, at_P(-1.0)), rounded(-pi_wide), m);
			check("atan2(+0, +inf)", 0, atan2(pz, inf), pz, m);
			check("atan2(-1, +inf)", 0, atan2(at_P(-1.0), inf), nz, m);
			check("atan2(-0, -inf)", 0, atan2(nz, ninf), rounded(-pi_wide), m);
			check("atan2(1, -inf)", 0, atan2(at_P(1.0), ninf), rounded(pi_wide), m);
			efloat_rounding_mode = saved;
		}
		{
			// exact powers far past 2^17 are returned outright, as the exponent holds them
			R_t big(3.0);
			big.set_precision(P);
			big.setexponent(200001);   // 3 2^200000
			R_t p2(1.0);
			p2.set_precision(P);
			p2.setexponent(int64_t(1) << 20);
			efloat_rounding_mode = modes[m];
			check("exp2(2^20)", 20, exp2(at_P(std::ldexp(1.0, 20))), p2, m);
			check("pow(3 2^200000, 1)", 1, pow(big, 1), big, m);
			check("pow(2^(2^20), 1/2)", 1, pow(p2, at_P(0.5)), [&] { R_t r(1.0); r.set_precision(P); r.setexponent(int64_t(1) << 19); return r; }(), m);
			efloat_rounding_mode = saved;
		}
		// arguments so small that the next term lies below any working precision: atan2(t, 1) =
		// t - t^3/3 for t = 2^-(300P + k), and 3^y = 1 + y ln 3 for y = +-2^-(2P + k). Each must round
		// as its leading term moved toward the next by less than a unit in its last place.
		auto nudged = [](R_t v, int direction) {
			const int64_t s = v.scale();
			v.set_precision(P + 64);
			R_t tick(1.0);
			tick.set_precision(P + 64);
			tick.setexponent(s - static_cast<int64_t>(P) - 32);
			if (direction > 0) v += tick; else v -= tick;
			v.set_precision(P);
			return v;
		};
		for (int k = 0; k < 4; ++k) {
			R_t t(1.0);
			t.set_precision(P);
			t.setexponent(-300 * static_cast<int64_t>(P) - k);
			R_t y(1.0);
			y.set_precision(P);
			y.setexponent(-2 * static_cast<int64_t>(P) - k);
			const R_t neg_y = -y;
			efloat_rounding_mode = modes[m];
			check("atan2(2^-300P, 1)", k, atan2(t, at_P(1.0)), nudged(t, -1), m);
			check("pow(3, 2^-2P)", k, pow(at_P(3.0), y), nudged(at_P(1.0), 1), m);
			check("pow(3, -2^-2P)", k, pow(at_P(3.0), neg_y), nudged(at_P(1.0), -1), m);
			check("pow(1/3, 2^-2P)", k, pow(at_P(1.0) / 3, y), nudged(at_P(1.0), -1), m);
			efloat_rounding_mode = saved;
		}
	}
	efloat_rounding_mode = saved;
	return fails;
}

// log(2^e) = e ln 2 for exponents past the type's default precision (#1754 review): an efloat
// made from an integer has the type's default precision, so the table path rounded e to 32 bits
// in efloat<1>, and Newton on main was off by 65 units at e = 2^32 + 1 and returned inf at
// e = -(2^32 + 1). Each result must equal e ln 2 computed at 256 bits and rounded to 32.
int VerifyLargeExponents(bool reportTestCases) {
	int fails = 0;
	for (int64_t e : { int64_t(4294967297), int64_t(4294967299), -int64_t(4294967297), (int64_t(1) << 40) + 3,
	                   (int64_t(1) << 52) + 1, (int64_t(1) << 62) - 1 }) {
		efloat<1> x(1.0);
		x.setexponent(e);
		const efloat<1> y = log(x);
		efloat<8> ex;
		ex.set_precision(256);
		ex = e;
		efloat<8> reference = ex * efloat_detail::ln2<8>(256);
		reference.set_precision(32);
		if (!(double(y) == double(reference))) {
			++fails;
			if (reportTestCases) std::cerr << "FAIL: log(2^" << e << ") = " << double(y) << ", expected " << double(reference) << '\n';
		}
	}
	return fails;
}

// exp(x) for |x| < 2^62 as exp(x / 2^j)^(2^j), |x / 2^j| < 1, at x's precision: the squarings
// lose j bits, and the k ln 2 reduction exp uses for a large argument plays no part
template<unsigned nlimbs>
efloat<nlimbs> exp_by_squaring(const efloat<nlimbs>& x) {
	const int64_t j = std::max<int64_t>(0, x.scale() + 1);
	efloat<nlimbs> t(x);
	t.setexponent(t.scale() - j);
	efloat<nlimbs> e = exp(t);
	for (int64_t i = 0; i < j; ++i) e *= e;
	return e;
}

// exp's range reaches |k| < 2^62, k = round(x / ln 2), where the int64_t exponent holds 2^k; it
// stopped at |x| < 2^17, and returned inf or 0 past it for values the exponent holds. exp, exp2,
// exp10, expm1, sinh, cosh and pow of arguments from 2^17 to 2^61 must be correctly rounded in
// every mode, against exp_by_squaring at 768 bits, or exactly, 2^(k +- 1/2) = 2^k sqrt(2);
// past the range they must return inf or 0 and raise Overflow or Underflow.
int VerifyBeyondDoubleRange(bool reportTestCases) {
	using R_t = efloat<24>;
	constexpr unsigned P = 128, W = 24 * 32;
	const RoundingMode modes[] = { RoundingMode::RoundToNearest, RoundingMode::RoundToZero,
	                               RoundingMode::RoundTowardPositive, RoundingMode::RoundTowardNegative };
	const char* names[] = { "nearest", "toward zero", "toward +inf", "toward -inf" };
	const RoundingMode saved = efloat_rounding_mode;
	int fails = 0;
	auto check = [&](const char* what, const R_t& x, const R_t& got, const R_t& expected, unsigned m) {
		const bool ok = (got == expected) && got.sign() == expected.sign() && got.get_precision() == P;
		if (!ok) {
			++fails;
			if (reportTestCases) std::cerr << "FAIL " << what << " at x = " << double(x) << ", rounded " << names[m] << ": got scale " << got.scale() << ", expected scale " << expected.scale() << '\n';
		}
	};
	// a W-bit reference rounded to P bits in mode m
	auto rounded = [&](R_t v, unsigned m) {
		efloat_rounding_mode = modes[m];
		v.set_precision(P);
		efloat_rounding_mode = RoundingMode::RoundToNearest;
		return v;
	};
	auto at = [](R_t v, unsigned bits) { v.set_precision(bits); return v; };
	std::mt19937_64 rng(1774);
	auto random_value = [&rng](int64_t scale) {   // a P-bit value of the given scale
		R_t v(1.0);
		for (unsigned j = 0; j < P / 32; ++j) v.setlimb(j, static_cast<uint32_t>(rng()) | (j == 0 ? 0x80000000u : 0u));
		v.set_precision(P);
		v.setexponent(scale);
		return v;
	};
	const R_t ln10_W = efloat_detail::ln10<24>(W);
	const R_t log_2_5 = log(at(R_t(2.5), W));
	const R_t log_1_5 = log(at(R_t(1.5), W));
	for (int64_t s : { 17, 18, 24, 33, 47, 52, 53, 57, 60 }) {
		for (int sample = 0; sample < 2; ++sample) {
			const R_t a = random_value(s);
			const R_t e = exp_by_squaring(at(a, W));   // e^a, 2^(s + 1) bits above 1
			R_t half_e(e);
			half_e.setexponent(half_e.scale() - 1);
			const R_t neg_a = -a;
			const R_t e_neg = 1 / e;
			R_t neg_half_e(-half_e);
			// 10^b and 2.5^c with b = a / 4 and c = a / 2: log2 of each about scale(a)
			R_t b(a), c(a);
			b.setexponent(b.scale() - 2);
			c.setexponent(c.scale() - 1);
			const R_t e10 = exp_by_squaring(at(b, W) * ln10_W);
			const R_t e25 = exp_by_squaring(at(c, W) * log_2_5);
			// exp_working, unrounded, within the bound Ziv's test assumes: k ln 2 must not spend its guard bits
			const R_t difference = efloat_detail::exp_working(a, P) - e;
			if (!difference.iszero() && difference.scale() >= e.scale() - static_cast<int64_t>(P + efloat_detail::exp_log_guard_bits - efloat_detail::exp_error_bits)) {
				++fails;
				if (reportTestCases) std::cerr << "FAIL exp_working error bound at scale " << s << '\n';
			}
			for (unsigned m = 0; m < 4; ++m) {
				efloat_rounding_mode = modes[m];
				const R_t got_exp = exp(a), got_exp_neg = exp(neg_a), got_expm1 = expm1(a);
				const R_t got_sinh = sinh(a), got_sinh_neg = sinh(neg_a), got_cosh = cosh(neg_a);
				const R_t got_exp10 = exp10(b), got_pow = pow(at(R_t(2.5), P), c);
				efloat_rounding_mode = RoundingMode::RoundToNearest;
				check("exp", a, got_exp, rounded(e, m), m);
				check("exp", neg_a, got_exp_neg, rounded(e_neg, m), m);
				check("expm1", a, got_expm1, rounded(e, m), m);   // e^a - 1 rounds as e^a: 1 lies 2^(2^17) below
				check("sinh", a, got_sinh, rounded(half_e, m), m);
				check("sinh", neg_a, got_sinh_neg, rounded(neg_half_e, m), m);
				check("cosh", neg_a, got_cosh, rounded(half_e, m), m);
				check("exp10", b, got_exp10, rounded(e10, m), m);
				check("pow(2.5, y)", c, got_pow, rounded(e25, m), m);
			}
		}
	}
	// an integer power by binary powering, 1.5^n with n up to 2^40, and 2^(k +- 1/2) = 2^k sqrt(2) exactly
	for (int64_t k : { int64_t(1) << 17, (int64_t(1) << 20) + 1, (int64_t(1) << 40) + 3, (int64_t(1) << 52) + 5, (int64_t(1) << 61) - 1, (int64_t(1) << 62) - (int64_t(1) << 20) }) {
		R_t n;
		n.set_precision(P);
		n = std::min<int64_t>(k, (int64_t(1) << 40) + 3);
		const R_t e15 = exp_by_squaring(at(n, W) * log_1_5);
		R_t x_plus(n), x_minus;
		x_plus = k;
		x_plus.set_precision(P);
		x_minus = -x_plus;
		x_plus  += 0.5;
		x_minus -= 0.5;   // -(k + 1/2)
		for (unsigned m = 0; m < 4; ++m) {
			R_t root2 = rounded(sqrt(at(R_t(2.0), W)), m);
			R_t up(root2), down(root2);
			up.setexponent(k);
			down.setexponent(-k - 1);
			efloat_rounding_mode = modes[m];
			const R_t got_up = exp2(x_plus), got_down = exp2(x_minus), got_pow = pow(at(R_t(1.5), P), n);
			efloat_rounding_mode = RoundingMode::RoundToNearest;
			check("exp2(k + 1/2)", x_plus, got_up, up, m);
			check("exp2(-(k + 1/2))", x_minus, got_down, down, m);
			check("pow(1.5, n)", n, got_pow, rounded(e15, m), m);
		}
	}
	// just inside the range, |x| a little below 2^62 ln 2, where k nears 2^62 and is corrected
	// from the double estimate: x = 2^62 ln 2 - 2^40 and 2^62 ln 2 (1 - 2^-50), both signs
	{
		R_t limit = efloat_detail::ln2<24>(P);
		limit.setexponent(limit.scale() + 62);
		R_t tick(1.0);
		tick.setexponent(40);
		R_t tock(limit);
		tock.setexponent(tock.scale() - 50);
		for (const R_t& a : { R_t(limit - tick), R_t(limit - tock) }) {
			for (const R_t& x : { a, R_t(-a) }) {
				const R_t e = exp_by_squaring(at(x, W));
				for (unsigned m = 0; m < 4; ++m) {
					efloat_rounding_mode = modes[m];
					const R_t got = exp(x), got_m1 = expm1(x);
					efloat_rounding_mode = RoundingMode::RoundToNearest;
					check("exp near 2^62 ln 2", x, got, rounded(e, m), m);
					if (x.sign() == 1) check("expm1 near 2^62 ln 2", x, got_m1, rounded(e, m), m);
				}
			}
		}
	}
	// an integer power whose log nears the range, x = 3 2^(2^20): x^(2^40 + 1) is held, the
	// exponent of x^(2^43 + 1) passes 2^62
	R_t huge(3.0);
	huge.set_precision(P);
	huge.setexponent((int64_t(1) << 20) + 1);
	const R_t log_huge = log(at(huge, W));
	{
		R_t n;
		n.set_precision(P);
		n = (int64_t(1) << 40) + 1;
		const R_t reference = exp_by_squaring(at(n, W) * log_huge);
		for (unsigned m = 0; m < 4; ++m) {
			efloat_rounding_mode = modes[m];
			const R_t got = pow(huge, n);
			efloat_rounding_mode = RoundingMode::RoundToNearest;
			check("pow(3 2^(2^20), 2^40 + 1)", n, got, rounded(reference, m), m);
		}
	}
	R_t n43;
	n43.set_precision(P);
	n43 = (int64_t(1) << 43) + 1;
	// past the range: inf or 0, with Overflow or Underflow
	R_t big(1.0);
	big.set_precision(P);
	big.setexponent(62);   // 2^62: exp's k is 2^62 log2 e
	auto beyond = [&](const char* what, const R_t& got, bool overflow, bool negative) {
		const bool flagged = has_efloat_exception(overflow ? ExceptionFlag::Overflow : ExceptionFlag::Underflow);
		const bool ok = flagged && (overflow ? got.isinf() : got.iszero()) && (got.sign() == -1) == negative && got.get_precision() == P;
		clear_efloat_exceptions();
		if (!ok) {
			++fails;
			if (reportTestCases) std::cerr << "FAIL " << what << " past exp's range\n";
		}
	};
	clear_efloat_exceptions();
	beyond("exp(2^62)", exp(big), true, false);
	beyond("exp(-2^62)", exp(-big), false, false);
	beyond("exp2(2^62 + 1/2)", exp2(big + 0.5), true, false);
	beyond("exp10(2^61)", exp10(at(big, P) / 2), true, false);
	beyond("expm1(2^62)", expm1(big), true, false);
	beyond("sinh(-2^62)", sinh(-big), true, true);
	beyond("cosh(-2^62)", cosh(-big), true, false);
	beyond("pow(2.5, 2^62)", pow(at(R_t(2.5), P), big), true, false);
	beyond("pow(2.5, -2^62)", pow(at(R_t(2.5), P), -big), false, false);
	beyond("pow(-2.5, 2^62 + 1)", pow(at(R_t(-2.5), P), big + 1), true, true);
	beyond("pow(3 2^(2^20), 2^43 + 1)", pow(huge, n43), true, false);
	beyond("pow(3 2^(2^20), -(2^43 + 1))", pow(huge, -n43), false, false);
	efloat_rounding_mode = saved;
	return fails;
}

}  // anonymous namespace

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
//#undef REGRESSION_LEVEL_OVERRIDE
#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "efloat exp/log precision";
	std::string test_tag    = "exp/log precision";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyAtPrecision<20>(true, 4), "efloat<20>", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyAtPrecision<10>(reportTestCases, 4), "efloat<10>", "320 bits");
	nrOfFailedTestCases += ReportTestResult(VerifyAtPrecision<20>(reportTestCases, 4), "efloat<20>", "640 bits");
	nrOfFailedTestCases += ReportTestResult(VerifySetPrecision<4>(reportTestCases, 600, 4), "efloat<4>", "set_precision(600)");
	nrOfFailedTestCases += ReportTestResult(VerifyNearZeros<10>(reportTestCases, 4), "efloat<10>", "near zeros");
	nrOfFailedTestCases += ReportTestResult(VerifyLargeExponents(reportTestCases), "efloat<1>", "log(2^e), e past 2^32");
	nrOfFailedTestCases += ReportTestResult(VerifyNearZeros<20>(reportTestCases, 4), "efloat<20>", "near zeros");
	nrOfFailedTestCases += ReportTestResult(VerifyAgmAgainstSeries<4>(reportTestCases, 100), "efloat<4>", "log AGM vs series");
	nrOfFailedTestCases += ReportTestResult(VerifyAgmAgainstSeries<48>(reportTestCases, 20), "efloat<48>", "log AGM vs series");
	nrOfFailedTestCases += ReportTestResult(VerifyNearRoundingBoundaries<4>(reportTestCases, 3000), "efloat<4>", "log near rounding boundaries");
	nrOfFailedTestCases += ReportTestResult(VerifyNearRoundingBoundaries<8>(reportTestCases, 1000), "efloat<8>", "log near rounding boundaries");
	nrOfFailedTestCases += ReportTestResult(VerifyFunctionsNearRoundingBoundaries<1>(reportTestCases, 1000), "efloat<1>", "exp/sin/cos/tan near rounding boundaries");
	nrOfFailedTestCases += ReportTestResult(VerifyFunctionsNearRoundingBoundaries<4>(reportTestCases, 500), "efloat<4>", "exp/sin/cos/tan near rounding boundaries");
	nrOfFailedTestCases += ReportTestResult(VerifyFunctionsNearRoundingBoundaries<16>(reportTestCases, 60), "efloat<16>", "exp/sin/cos/tan near rounding boundaries");
	nrOfFailedTestCases += ReportTestResult(VerifyRemainingFunctionsNearRoundingBoundaries<1>(reportTestCases, 400), "efloat<1>", "remaining functions near rounding boundaries");
	nrOfFailedTestCases += ReportTestResult(VerifyRemainingFunctionsNearRoundingBoundaries<4>(reportTestCases, 150), "efloat<4>", "remaining functions near rounding boundaries");
	nrOfFailedTestCases += ReportTestResult(VerifyRemainingFunctionsNearRoundingBoundaries<16>(reportTestCases, 40), "efloat<16>", "remaining functions near rounding boundaries");
	nrOfFailedTestCases += ReportTestResult(VerifyExactValues<1>(reportTestCases), "efloat<1>", "exact values");
	nrOfFailedTestCases += ReportTestResult(VerifyExactValues<4>(reportTestCases), "efloat<4>", "exact values");
	nrOfFailedTestCases += ReportTestResult(VerifyBeyondDoubleRange(reportTestCases), "efloat<24>", "exp family from 2^17 to 2^62");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyAtPrecision<64>(reportTestCases, 4), "efloat<64>", "2048 bits");
	nrOfFailedTestCases += ReportTestResult(VerifyNearRoundingBoundaries<64>(reportTestCases, 40), "efloat<64>", "log near rounding boundaries");
	nrOfFailedTestCases += ReportTestResult(VerifyFunctionsNearRoundingBoundaries<64>(reportTestCases, 20), "efloat<64>", "exp/sin/cos/tan near rounding boundaries");
	nrOfFailedTestCases += ReportTestResult(VerifyRemainingFunctionsNearRoundingBoundaries<64>(reportTestCases, 6), "efloat<64>", "remaining functions near rounding boundaries");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyAgmAgainstSeries<128>(reportTestCases, 20), "efloat<128>", "log AGM vs series");
#endif

#if REGRESSION_LEVEL_4
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
