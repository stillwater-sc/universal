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
#include <cstring>
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
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyAtPrecision<64>(reportTestCases, 4), "efloat<64>", "2048 bits");
#endif

#if REGRESSION_LEVEL_3
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
