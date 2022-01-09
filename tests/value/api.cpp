// api.cpp: functional tests of the value type API
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// configure the value<> environment
#define BITBLOCK_THROW_ARITHMETIC_EXCEPTION 0
#define VALUE_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/internal/value/value.hpp>  // INTERNAL class: not part of the public Universal API
#include <universal/verification/test_status.hpp>
#include <universal/verification/test_reporters.hpp>

using namespace sw::universal::internal;

template<size_t fbits>
int Check(const value<fbits>& v, double ref, bool bReportIndividualTestCases) {
	int fails = 0;
	if (v.to_double() != ref) {
		++fails;
		if (bReportIndividualTestCases) {
			std::cout << v << " != " << ref << '\n';
		}
	}
	return fails;
}

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 1
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
	using namespace sw::universal::internal;

	std::string test_suite = "value class API";
	std::string test_tag = "value";
	std::cout << test_suite << '\n';
	int nrOfFailedTestCases = 0;
	bool bReportIndividualTestCases = true;

	std::cout << (bReportIndividualTestCases ? " " : "not ") << "reporting individual testcases\n";

#if MANUAL_TESTING

	{
		value<4> a;
		a = 0.03125f;
		std::cout << convert_to_decimal_string(a) << " vs " << a << '\n';
		a = 0.0625f;
		std::cout << convert_to_decimal_string(a) << " vs " << a << '\n';
		a = 0.125f;
		std::cout << convert_to_decimal_string(a) << " vs " << a << '\n';
		a = 0.25f;
		std::cout << convert_to_decimal_string(a) << " vs " << a << '\n';
		a = 0.5f;
		std::cout << convert_to_decimal_string(a) << " vs " << a << '\n';
		a = 1.0f;
		std::cout << convert_to_decimal_string(a) << " vs " << a << '\n';
		a = 1.5f;
		std::cout << convert_to_decimal_string(a) << " vs " << a << '\n';
		a = 1.0625f;
		std::cout << convert_to_decimal_string(a) << " vs " << a << '\n';
		a = 2.0f;
		std::cout << convert_to_decimal_string(a) << " vs " << a << '\n';
		a = 4.0f;
		std::cout << convert_to_decimal_string(a) << " vs " << a << '\n';
		a = 8.0f;
		std::cout << convert_to_decimal_string(a) << " vs " << a << '\n';
		a = 16.0f;
		std::cout << convert_to_decimal_string(a) << " vs " << a << '\n';
	}

	// assignment
	{
		constexpr double reference = 8;
		signed char        sc  = (signed char)reference;
		short              ss  = (short)reference;
		int                si  = (int)reference;
		long               sl  = (long)reference;
		long long          sll = (long long)reference;
		char               uc  = (char)reference;
		unsigned short     us  = (unsigned short)reference;
		unsigned int       ui  = (unsigned int)reference;
		unsigned long      ul  = (unsigned long)reference;
		unsigned long long ull = (unsigned long long)reference;
		float              f   = (float)reference;
		double             d   = (double)reference;
		long double        ld  = (long double)reference;

		value<11> v;
		v = sc;
		nrOfFailedTestCases += Check(v, reference, bReportIndividualTestCases);
		v = ss;
		nrOfFailedTestCases += Check(v, reference, bReportIndividualTestCases);
		v = si;
		nrOfFailedTestCases += Check(v, reference, bReportIndividualTestCases);
		v = sl;
		nrOfFailedTestCases += Check(v, reference, bReportIndividualTestCases);
		v = sll;
		nrOfFailedTestCases += Check(v, reference, bReportIndividualTestCases);
		v = uc;
		nrOfFailedTestCases += Check(v, reference, bReportIndividualTestCases);
		v = us;
		nrOfFailedTestCases += Check(v, reference, bReportIndividualTestCases);
		v = ui;
		nrOfFailedTestCases += Check(v, reference, bReportIndividualTestCases);
		v = ul;
		nrOfFailedTestCases += Check(v, reference, bReportIndividualTestCases);
		v = ull;
		nrOfFailedTestCases += Check(v, reference, bReportIndividualTestCases);
		v = f;
		nrOfFailedTestCases += Check(v, reference, bReportIndividualTestCases);
		v = d;
		nrOfFailedTestCases += Check(v, reference, bReportIndividualTestCases);
		v = ld;
		nrOfFailedTestCases += Check(v, reference, bReportIndividualTestCases);
	}

	{
		float f = 1.23456789f;
		auto components = ieee_components(f);
		std::cout << std::get<0>(components) << ", " << std::get<1>(components) << ", " << std::get<2>(components) << '\n';
	}

	{
		double d = 1.23456789;
		auto components = ieee_components(d);
		std::cout << std::get<0>(components) << ", " << std::get<1>(components) << ", " << std::get<2>(components) << '\n';
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1

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
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
