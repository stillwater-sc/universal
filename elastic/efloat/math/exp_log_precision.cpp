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
