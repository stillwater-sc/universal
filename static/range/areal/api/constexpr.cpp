// constexpr.cpp: compile time tests for areal constexpr
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// Configure the areal template environment
// first: enable general or specialized fixed-point configurations
#define AREAL_FAST_SPECIALIZATION
// second: enable/disable areal arithmetic exceptions
#define AREAL_THROW_ARITHMETIC_EXCEPTION 1

#include <universal/number/areal/areal.hpp>
#include <universal/verification/test_reporters.hpp> 

// stylistic constexpr of pi that we'll assign constexpr to an areal
constexpr double pi = 3.14159265358979323846;

template<typename Real>
void TestConstexprConstruction() {
	// decorated constructors
	{
		Real a(1l);  // signed long
		std::cout << a << '\n';
	}
	{
		constexpr Real a(1ul);  // unsigned long
		std::cout << a << '\n';
	}
	{
		BIT_CAST_CONSTEXPR Real a(1.0f);  // float
		std::cout << a << '\n';
	}
	{
		BIT_CAST_CONSTEXPR Real a(pi);   // double
		std::cout << a << '\n';
	}
	{
		BIT_CAST_CONSTEXPR Real a(1.0l);  // long double
		std::cout << a << '\n';
	}
}

template<typename Real>
void TestConstexprAssignment() {
	// decorated constructors
	{
		Real a = 1l;  // signed long
		std::cout << a << '\n';
	}
	{
		constexpr Real a = 1ul;  // unsigned long
		std::cout << a << '\n';
	}
	{
		BIT_CAST_CONSTEXPR Real a = 1.0f;  // float
		std::cout << a << '\n';
	}
	{
		BIT_CAST_CONSTEXPR Real a = pi;   // double
		std::cout << a << '\n';
	}
	{
		BIT_CAST_CONSTEXPR Real a = 1.0l;  // long double
		std::cout << a << '\n';
	}
}

//
template<typename Real>
void TestConstexprSpecificValues() {
	{
		constexpr Real positiveMax(sw::universal::SpecificValue::maxpos);
		std::cout << "maxpos  : " << to_binary(positiveMax) << " : " << positiveMax << '\n';
	}
	{
		constexpr Real positiveMin(sw::universal::SpecificValue::minpos);
		std::cout << "minpos  : " << to_binary(positiveMin) << " : " << positiveMin << '\n';
	}
	{
		constexpr Real zero(sw::universal::SpecificValue::zero);
		std::cout << "zero    : " << to_binary(zero) << " : " << zero << '\n';
	}
	{
		constexpr Real negativeMin(sw::universal::SpecificValue::minneg);
		std::cout << "minneg  : " << to_binary(negativeMin) << " : " << negativeMin << '\n';
	}
	{
		constexpr Real negativeMax(sw::universal::SpecificValue::maxneg);
		std::cout << "maxneg  : " << to_binary(negativeMax) << " : " << negativeMax << '\n';
	}
}

// conditional compile flags
#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "areal constexpr ";
	int nrOfFailedTestCases = 0;

	std::cout << test_suite << '\n';

	using Real = areal<12, 2>;
	Real a;
	a.constexprClassParameters();

	TestConstexprConstruction<Real>();
	TestConstexprAssignment<Real>();
	TestConstexprSpecificValues<Real>();

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught unexpected runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
