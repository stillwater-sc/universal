// constexpr.cpp: compile time tests for fixed-point constexpr
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// Configure the fixpnt template environment
// first: enable general or specialized fixed-point configurations
#define FIXPNT_FAST_SPECIALIZATION
// second: enable/disable fixpnt arithmetic exceptions
#define FIXPNT_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/math/constants/double_constants.hpp>

template<typename Fixpnt>
int DecoratedConstructors() {
	using namespace sw::universal;

	int nrOfFailedTestCases = 0;

	{
		// decorated constructors
		{
			constexpr Fixpnt a(1l);  // signed long
			std::cout << a << '\n';
		}
		{
			constexpr Fixpnt a(1ul);  // unsigned long
			std::cout << a << '\n';
		}
		// constexpr for float depends on C++20 support and bit_cast<>
		{
			BIT_CAST_CONSTEXPR Fixpnt a(1.0f);  // float
			std::cout << a << '\n';
		}
		{
			BIT_CAST_CONSTEXPR Fixpnt a(1.0);   // double
			std::cout << a << '\n';
		}
#if LONG_DOUBLE_SUPPORT
		{
			#if defined(DEBUG_LONG_DOUBLE_CONSTEXPR)
			Fixpnt a(1.0l);  // long double
			std::cout << a << '\n';
			#endif
		}
#endif // LONG_DOUBLE_SUPPORT

	}

	return nrOfFailedTestCases;
}

template<typename Fixpnt>
int AssignmentOperators() {
	using namespace sw::universal;

	int nrOfFailedTestCases = 0;

	{
		// assignment operators
		{
			constexpr Fixpnt a = 1l;  // signed long
			std::cout << a << '\n';
		}
		{
			constexpr Fixpnt a = 1ul;  // unsigned long
			std::cout << a << '\n';
		}
		// constexpr for float depends on C++20 support and bit_cast<>
		{
			BIT_CAST_CONSTEXPR Fixpnt a = 1.0f;  // float
			std::cout << a << '\n';
		}
		{
			BIT_CAST_CONSTEXPR Fixpnt a = 1.0;   // double
			std::cout << a << '\n';
		}
#if LONG_DOUBLE_SUPPORT
		{
			Fixpnt a = 1.0l;  // long double
			std::cout << a << '\n';
		}
#endif // LONG_DOUBLE_SUPPORT
	}

	return nrOfFailedTestCases;
}

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

template<typename Fixpnt>
void ConstexprFixpnt() {
	CONSTEXPRESSION Fixpnt a(sw::universal::d_pi);
	std::cout << type_tag(a) << " : " << a << '\n';
}

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "fixed-point constexpr verification";
	std::string test_tag    = "constexpr";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	using Fixpnt = sw::universal::fixpnt<8, 4, Modulo, uint16_t>;

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug

	{
		fixpnt<8, 4> a(pi);
		std::cout << a << '\n';
	}

	nrOfFailedTestCases += ReportTestResult(DecoratedConstructors<Fixpnt>(), test_tag, "constructors");
	nrOfFailedTestCases += ReportTestResult(AssignmentOperators<Fixpnt>(), test_tag, "assignment");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

	constexpr size_t FIRST_COLUMN = 43;
	std::cout << "constexpr pi approximations\n";
	std::cout << std::setw(FIRST_COLUMN) << "type" << " : " << d_pi << '\n';
	ConstexprFixpnt<fixpnt<8, 4>>();
	ConstexprFixpnt<fixpnt<9, 6>>();
	ConstexprFixpnt<fixpnt<16, 4>>();
	ConstexprFixpnt<fixpnt<16, 8>>();
	ConstexprFixpnt<fixpnt<16, 12>>();
	ConstexprFixpnt<fixpnt<32, 28>>();
	auto oldPrecision = std::cout.precision();
	std::cout << std::setprecision(30);
	std::cout << std::setw(FIRST_COLUMN) << "double" << " : " << d_pi << '\n';
	std::cout << std::setprecision(oldPrecision);

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(DecoratedConstructors<Fixpnt>(), test_tag, "constructors");
	nrOfFailedTestCases += ReportTestResult(AssignmentOperators<Fixpnt>(), test_tag, "assignment");
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
catch (const sw::universal::fixpnt_arithmetic_exception& err) {
	std::cerr << "Uncaught fixpnt arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::fixpnt_internal_exception& err) {
	std::cerr << "Uncaught fixpnt internal exception: " << err.what() << std::endl;
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
