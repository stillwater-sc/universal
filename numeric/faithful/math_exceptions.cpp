// math_exceptions.cpp: faithful's math functions with FAITHFUL_THROW_ARITHMETIC_EXCEPTION set
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// With the switch set, a domain error throws instead of returning NaN with EDOM (#1955): sqrt of a
// negative throws faithful_negative_sqrt_arg, log, log2, log10 and log1p of an argument outside
// their domain throw faithful_negative_log_arg, and sin, cos and tan of an infinity and asin and
// acos beyond [-1, 1] throw faithful_domain_error (#1956). A range result, inf or 0, is a value, not an
// error, and does not throw. The default, non-throwing configuration is tested in mathlib.cpp and
// exponential.cpp.
#include <universal/utility/directives.hpp>
#define FAITHFUL_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/faithful/faithful.hpp>
#include <universal/verification/test_suite.hpp>
#include <cmath>
#include <limits>
#include <string>

namespace {

template<typename Real>
int VerifyThrows(bool reportTestCases) {
	using namespace sw::universal;
	using F = faithful<Real>;
	int fails = 0;
	auto check = [&](bool ok, const std::string& what) {
		if (!ok) { ++fails; if (reportTestCases) std::cerr << "FAIL " << type_tag(F()) << ' ' << what << '\n'; }
	};
	// expect fn() to throw E
	auto throwsAs = [&](auto fn, auto tag, const char* what) {
		using E = decltype(tag);
		bool caught = false;
		try { (void)fn(); }
		catch (const E&) { caught = true; }
		catch (...) {}
		check(caught, std::string(what) + " throws");
	};
	throwsAs([] { return sqrt(F(Real(-1))); }, faithful_negative_sqrt_arg{}, "sqrt(-1)");
	throwsAs([] { return log(F(Real(-1))); }, faithful_negative_log_arg{}, "log(-1)");
	throwsAs([] { return log2(F(Real(-2))); }, faithful_negative_log_arg{}, "log2(-2)");
	throwsAs([] { return log10(F(Real(-10))); }, faithful_negative_log_arg{}, "log10(-10)");
	throwsAs([] { return log1p(F(Real(-2))); }, faithful_negative_log_arg{}, "log1p(-2)");
	throwsAs([] { return log1p(F(-std::numeric_limits<Real>::infinity())); }, faithful_negative_log_arg{}, "log1p(-inf)");
	throwsAs([] { return sin(F(std::numeric_limits<Real>::infinity())); }, faithful_domain_error{"sin"}, "sin(inf)");
	throwsAs([] { return cos(F(-std::numeric_limits<Real>::infinity())); }, faithful_domain_error{"cos"}, "cos(-inf)");
	throwsAs([] { return tan(F(std::numeric_limits<Real>::infinity())); }, faithful_domain_error{"tan"}, "tan(inf)");
	throwsAs([] { return asin(F(Real(2))); }, faithful_domain_error{"asin"}, "asin(2)");
	throwsAs([] { return acos(F(Real(-2))); }, faithful_domain_error{"acos"}, "acos(-2)");
	// the exception is a faithful arithmetic exception, so a caller can catch the family
	{
		bool caught = false;
		try { (void)log(F(Real(-1))); }
		catch (const faithful_arithmetic_exception&) { caught = true; }
		check(caught, "faithful_negative_log_arg is a faithful_arithmetic_exception");
	}
	// range results are values, not errors
	try {
		check(isinf(log(F(Real(0)))), "log(0) = -inf");
		check(isinf(log1p(F(Real(-1)))), "log1p(-1) = -inf");
		check(isinf(exp(F(std::numeric_limits<Real>::max()))), "exp overflow = inf");
		check(iszero(exp(F(-std::numeric_limits<Real>::max()))), "exp underflow = 0");
		check(log(F(Real(1))) == F(Real(0)), "log(1) = 0");
	}
	catch (...) {
		check(false, "a range result does not throw");
	}
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

	std::string test_suite  = "faithful math functions with FAITHFUL_THROW_ARITHMETIC_EXCEPTION";
	std::string test_tag    = "faithful";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	try { (void)log(faithful<double>(-1.0)); }
	catch (const faithful_negative_log_arg& e) { std::cout << e.what() << '\n'; }

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyThrows<float>(reportTestCases), "faithful<float>", "domain errors throw");
	nrOfFailedTestCases += ReportTestResult(VerifyThrows<double>(reportTestCases), "faithful<double>", "domain errors throw");
#endif

#if REGRESSION_LEVEL_2
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
	std::cerr << msg << std::endl;
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
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
