// wide_transcendentals.cpp: cfloat layouts past double evaluate their transcendental functions in efloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// A layout whose range or precision does not fit a double evaluates exp, log, pow, the trigonometric and
// hyperbolic functions, erf, erfc and hypot in efloat, correctly rounded (#2088). Through double, exp(710)
// on cfloat<64,15> was inf and pow(3, 40) on cfloat<80,11> was inexact.
//
// The reference is qd (212 bits), rounded once into the cfloat: the four doubles of a qd are summed
// exactly in a 256-bit cfloat and that is converted to the layout. qd's functions are accurate to about
// 2^-200, so the rounded reference is the correctly rounded value except within that distance of a tie.
//   - precision: cfloat<80,11> has double's range and 68 fraction bits;
//   - range: cfloat<64,15> reaches 2^16383; exp and log there are reduced in qd (exp(x) = 2^k exp(r),
//     log(2^k m) = k ln 2 + log(m)), and exp2 of an integer is exact;
//   - a saturating layout clamps an overflow to maxpos, and a max-exponent layout rounds as any other.
#include <universal/utility/directives.hpp>
#define CFLOAT_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/qd/qd.hpp>
#include <universal/verification/test_suite.hpp>

#include <cmath>
#include <cstdint>
#include <iostream>
#include <random>
#include <string>

namespace {

using sw::universal::cfloat;
using sw::universal::qd;

// a qd rounded once into the layout C: its doubles summed exactly in a 256-bit cfloat of C's es
template<typename C>
C from_qd(const qd& v) {
	using Big = cfloat<256, C::es, uint32_t, true, false, false>;
	if (std::isnan(v[0])) return C(sw::universal::SpecificValue::qnan);
	if (std::isinf(v[0])) return C(v[0] > 0 ? sw::universal::SpecificValue::infpos : sw::universal::SpecificValue::infneg);
	Big s(0);
	for (int i = 0; i < 4; ++i) s = s + Big(v[i]);
	return C(s);
}

// a cfloat with at most 106 significant bits, exactly, as a qd
template<typename C>
qd to_qd(const C& x) {
	const double hi = double(x);
	const double lo = double(x - C(hi));   // exact: x less its rounding to double
	return qd(hi) + qd(lo);
}

// f on the layout against fq on qd, over random arguments in [lo, hi)
template<typename C, typename F, typename Fq>
int VerifyUnary(const char* name, F f, Fq fq, double lo, double hi, unsigned samples, bool reportTestCases) {
	int fails = 0;
	std::mt19937_64 rng(2088);
	std::uniform_real_distribution<double> u(lo, hi);
	for (unsigned k = 0; k < samples; ++k) {
		// a random 68-bit argument: a double plus a small exact tail
		C x = C(u(rng)) + C(std::ldexp(u(rng), -40));
		if (double(x) < lo || double(x) >= hi) x = C(u(rng));
		const C got = f(x);
		const C ref = from_qd<C>(fq(to_qd(x)));
		if (!(got == ref) && !(got.isnan() && ref.isnan())) {
			if (reportTestCases && fails < 5) std::cerr << "FAIL: " << name << '(' << to_binary(x) << ") = " << to_binary(got) << "\n      expected " << to_binary(ref) << '\n';
			++fails;
		}
	}
	return fails;
}

template<typename C, typename F, typename Fq>
int VerifyBinary(const char* name, F f, Fq fq, double lo, double hi, unsigned samples, bool reportTestCases) {
	int fails = 0;
	std::mt19937_64 rng(2089);
	std::uniform_real_distribution<double> u(lo, hi);
	for (unsigned k = 0; k < samples; ++k) {
		const C x = C(u(rng)) + C(std::ldexp(u(rng), -40));
		const C y = C(u(rng)) + C(std::ldexp(u(rng), -40));
		const C got = f(x, y);
		const C ref = from_qd<C>(fq(to_qd(x), to_qd(y)));
		if (!(got == ref) && !(got.isnan() && ref.isnan())) {
			if (reportTestCases && fails < 5) std::cerr << "FAIL: " << name << '(' << double(x) << ", " << double(y) << ") = " << to_binary(got) << "\n      expected " << to_binary(ref) << '\n';
			++fails;
		}
	}
	return fails;
}

// every function of a layout with 68 fraction bits, against qd.  qd holds 212 bits only while its last
// double is normal: results below about 2^-800 lose bits there, so the arguments keep results above it.
template<typename C>
int VerifyPrecisionPastDouble(unsigned samples, bool reportTestCases) {
	using namespace sw::universal;
	int f = 0;
	f += VerifyUnary<C>("exp",   [](C x) { return exp(x); },   [](qd q) { return exp(q); },   -550.0, 700.0, samples, reportTestCases);
	f += VerifyUnary<C>("exp2",  [](C x) { return exp2(x); },  [](qd q) { return exp2(q); },  -790.0, 1000.0, samples, reportTestCases);
	f += VerifyUnary<C>("exp10", [](C x) { return exp10(x); }, [](qd q) { return exp10(q); }, -235.0, 300.0, samples, reportTestCases);
	f += VerifyUnary<C>("expm1", [](C x) { return expm1(x); }, [](qd q) { return expm1(q); }, -2.0, 2.0, samples, reportTestCases);
	f += VerifyUnary<C>("log",   [](C x) { return log(x); },   [](qd q) { return log(q); },   1e-300, 1e300, samples, reportTestCases);
	f += VerifyUnary<C>("log2",  [](C x) { return log2(x); },  [](qd q) { return log2(q); },  1e-10, 1e10, samples, reportTestCases);
	f += VerifyUnary<C>("log10", [](C x) { return log10(x); }, [](qd q) { return log10(q); }, 1e-10, 1e10, samples, reportTestCases);
	f += VerifyUnary<C>("log1p", [](C x) { return log1p(x); }, [](qd q) { return log1p(q); }, -0.9, 4.0, samples, reportTestCases);
	f += VerifyUnary<C>("sin",   [](C x) { return sin(x); },   [](qd q) { return sin(q); },   -10.0, 10.0, samples, reportTestCases);
	f += VerifyUnary<C>("cos",   [](C x) { return cos(x); },   [](qd q) { return cos(q); },   -10.0, 10.0, samples, reportTestCases);
	f += VerifyUnary<C>("tan",   [](C x) { return tan(x); },   [](qd q) { return tan(q); },   -1.5, 1.5, samples, reportTestCases);
	f += VerifyUnary<C>("asin",  [](C x) { return asin(x); },  [](qd q) { return asin(q); },  -1.0, 1.0, samples, reportTestCases);
	f += VerifyUnary<C>("acos",  [](C x) { return acos(x); },  [](qd q) { return acos(q); },  -1.0, 1.0, samples, reportTestCases);
	f += VerifyUnary<C>("atan",  [](C x) { return atan(x); },  [](qd q) { return atan(q); },  -50.0, 50.0, samples, reportTestCases);
	f += VerifyUnary<C>("cot",   [](C x) { return cot(x); },   [](qd q) { return qd(1) / tan(q); }, 0.1, 3.0, samples, reportTestCases);
	f += VerifyUnary<C>("sec",   [](C x) { return sec(x); },   [](qd q) { return qd(1) / cos(q); }, -1.4, 1.4, samples, reportTestCases);
	f += VerifyUnary<C>("csc",   [](C x) { return csc(x); },   [](qd q) { return qd(1) / sin(q); }, 0.1, 3.0, samples, reportTestCases);
	f += VerifyUnary<C>("sinh",  [](C x) { return sinh(x); },  [](qd q) { return sinh(q); },  -20.0, 20.0, samples, reportTestCases);
	f += VerifyUnary<C>("cosh",  [](C x) { return cosh(x); },  [](qd q) { return cosh(q); },  -20.0, 20.0, samples, reportTestCases);
	f += VerifyUnary<C>("tanh",  [](C x) { return tanh(x); },  [](qd q) { return tanh(q); },  -5.0, 5.0, samples, reportTestCases);
	f += VerifyUnary<C>("asinh", [](C x) { return asinh(x); }, [](qd q) { return asinh(q); }, -100.0, 100.0, samples, reportTestCases);
	f += VerifyUnary<C>("acosh", [](C x) { return acosh(x); }, [](qd q) { return acosh(q); }, 1.0, 100.0, samples, reportTestCases);
	f += VerifyUnary<C>("atanh", [](C x) { return atanh(x); }, [](qd q) { return atanh(q); }, -0.99, 0.99, samples, reportTestCases);
	f += VerifyUnary<C>("erf",   [](C x) { return erf(x); },   [](qd q) { return erf(q); },   -4.0, 4.0, samples, reportTestCases);
	f += VerifyUnary<C>("erfc",  [](C x) { return erfc(x); },  [](qd q) { return erfc(q); },  -4.0, 20.0, samples, reportTestCases);
	f += VerifyBinary<C>("pow",   [](C x, C y) { return pow(x, y); },   [](qd a, qd b) { return pow(a, b); },   0.1, 20.0, samples, reportTestCases);
	f += VerifyBinary<C>("atan2", [](C x, C y) { return atan2(x, y); }, [](qd a, qd b) { return atan2(a, b); }, -10.0, 10.0, samples, reportTestCases);
	f += VerifyBinary<C>("hypot", [](C x, C y) { return hypot(x, y); }, [](qd a, qd b) { return hypot(a, b); }, -1e10, 1e10, samples, reportTestCases);
	return f;
}

// rsqrt is 1 / sqrt, rounded twice: within an ulp of the reference
template<typename C>
int VerifyRsqrt(unsigned samples, bool reportTestCases) {
	using namespace sw::universal;
	int fails = 0;
	std::mt19937_64 rng(2090);
	std::uniform_real_distribution<double> u(1e-5, 1e5);
	for (unsigned k = 0; k < samples; ++k) {
		const C x(u(rng));
		const C got = rsqrt(x), ref = from_qd<C>(qd(1) / sqrt(to_qd(x)));
		C below = ref, above = ref;
		--below; ++above;
		if (!(got == ref || got == below || got == above)) {
			if (reportTestCases && fails < 5) std::cerr << "FAIL: rsqrt(" << double(x) << ") = " << to_binary(got) << ", expected " << to_binary(ref) << '\n';
			++fails;
		}
	}
	return fails;
}

// a layout past double's range: cfloat<64,15>.  exp and log are reduced in qd; exp2 of an integer and
// powers of two are exact.
int VerifyRangePastDouble(bool reportTestCases) {
	using namespace sw::universal;
	using C = cfloat<64, 15, uint32_t, true, false, false>;
	int fails = 0;
	auto check = [&](bool ok, const std::string& what) {
		if (!ok) {
			if (reportTestCases) std::cerr << "FAIL: cfloat<64,15> " << what << '\n';
			++fails;
		}
	};
	const qd ln2 = log(qd(2));
	for (double x : { 710.0, 1000.5, -800.25, 5000.0, -9000.75, 11000.0, -11300.0 }) {
		// exp(x) = 2^k exp(x - k ln 2), with the power of two applied in the cfloat, exactly
		const int k = static_cast<int>(std::floor(x / std::log(2.0)));
		const C reduced = from_qd<C>(exp(qd(x) - qd(k) * ln2));
		C ref = reduced;
		const C two(2), half(0.5);
		for (int i = 0; i < (k > 0 ? k : -k); ++i) ref = (k > 0) ? ref * two : ref * half;
		check(exp(C(x)) == ref, "exp(" + std::to_string(x) + ")");
		// log(2^k m) = k ln 2 + log(m)
		const C y = ref;
		check(log(y) == from_qd<C>(qd(k) * ln2 + log(to_qd(reduced))), "log(exp(" + std::to_string(x) + "))");
	}
	for (int n : { 1024, 2000, -1100, -16000, 16383 }) {
		C p(1);
		const C two(2), half(0.5);
		for (int i = 0; i < (n > 0 ? n : -n); ++i) p = (n > 0) ? p * two : p * half;
		check(exp2(C(n)) == p, "exp2(" + std::to_string(n) + ")");
		check(pow(C(2), n) == p, "pow(2, " + std::to_string(n) + ")");
	}
	check(exp(C(12000)).isinf(), "exp(12000) overflows");
	check(exp(C(-12000)).iszero(), "exp(-12000) underflows");
	return fails;
}

// special values, a saturating layout and a max-exponent layout
int VerifySpecialsAndPolicies(bool reportTestCases) {
	using namespace sw::universal;
	using C = cfloat<80, 11, uint32_t, true, false, false>;
	using Sat = cfloat<80, 11, uint32_t, true, false, true>;
	using Max = cfloat<80, 11, uint32_t, true, true, false>;
	int fails = 0;
	auto check = [&](bool ok, const char* what) {
		if (!ok) {
			if (reportTestCases) std::cerr << "FAIL: " << what << '\n';
			++fails;
		}
	};
	const C nan(SpecificValue::qnan), pinf(SpecificValue::infpos), ninf(SpecificValue::infneg), zero(0), nzero = -C(0);
	check(exp(nan).isnan() && sin(nan).isnan() && pow(nan, C(2)).isnan() && atan2(nan, C(1)).isnan(), "NaN arguments");
	check(exp(ninf).iszero() && exp(pinf).isinf() && log(zero).isinf() && log(zero).sign() && log(C(-1)).isnan(), "exp/log specials");
	check(sin(nzero).iszero() && sin(nzero).sign() && tan(nzero).sign() && atan(nzero).sign() && sinh(nzero).sign()
	      && tanh(nzero).sign() && asinh(nzero).sign() && atanh(nzero).sign() && erf(nzero).sign() && expm1(nzero).sign()
	      && log1p(nzero).sign() && asin(nzero).sign(), "odd functions keep -0");
	check(hypot(pinf, nan).isinf() && hypotf(C(3), C(4)) == C(5) && hypotl(C(3), C(4)) == C(5), "hypot specials and aliases");
	check(exp(Sat(1e6)) == Sat(SpecificValue::maxpos) && exp(C(1e6)).isinf(), "overflow: maxpos when saturating, inf otherwise");
	check(exp(Max(1)) == from_qd<Max>(exp(qd(1))) && log(Max(10)) == from_qd<Max>(log(qd(10))), "max-exponent layout");
	check(pow(C(3), C(40)) == from_qd<C>(pow(qd(3), qd(40))) && pow(C(3), 40) == pow(C(3), C(40)) && pow(C(3), 40.0) == pow(C(3), C(40)), "pow(3, 40) exact");
	// from review: pow's identities hold for a NaN, a double exponent keeps its bits, and a result
	// far below the range rounds without overflowing the ulp's exponent
	check(pow(nan, C(0)) == C(1) && pow(C(1), nan) == C(1) && pow(nan, 0) == C(1) && pow(nan, 0.0) == C(1), "pow(NaN, 0) = pow(1, NaN) = 1");
	using Narrow = cfloat<14, 12, uint16_t, true, false, false>;   // past double's range, 1 fraction bit
	check(pow(Narrow(-1), 1.0 + 0x1p-40).isnan() && pow(Narrow(-1), 3.0) == Narrow(-1), "pow(-1, non-integer double) is NaN");
	check(exp2(C(-2147483648.0)).iszero() && exp2(C(2147483648.0)).isinf(), "exp2 of +-2^31 rounds to 0 and inf");
	return fails;
}

}  // namespace

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

	std::string test_suite  = "cfloat transcendentals past double";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	using C = cfloat<64, 15, uint32_t, true, false, false>;
	std::cout << "exp(710) on cfloat<64,15>: " << to_binary(exp(C(710))) << '\n';
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors

#else

#if REGRESSION_LEVEL_1
	using X = cfloat<80, 11, uint32_t, true, false, false>;
	nrOfFailedTestCases += ReportTestResult(VerifyPrecisionPastDouble<X>(40, reportTestCases), "cfloat<80,11>", "functions vs qd");
	nrOfFailedTestCases += ReportTestResult(VerifyRsqrt<X>(200, reportTestCases),              "cfloat<80,11>", "rsqrt within 1 ulp");
	nrOfFailedTestCases += ReportTestResult(VerifyRangePastDouble(reportTestCases),            "cfloat<64,15>", "range past double");
	nrOfFailedTestCases += ReportTestResult(VerifySpecialsAndPolicies(reportTestCases),        "cfloat<80,11>", "specials and policies");
#endif

#if REGRESSION_LEVEL_2
	using Y = cfloat<96, 15, uint16_t, true, false, false>;
	nrOfFailedTestCases += ReportTestResult(VerifyPrecisionPastDouble<Y>(200, reportTestCases), "cfloat<96,15>", "functions vs qd");
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
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
