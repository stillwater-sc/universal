// constexpr.cpp: compile time tests for areal constexpr
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#if defined(_MSC_VER)
#pragma warning(disable : 4514)  // unreferenced function is removed
#pragma warning(disable : 4710)  // function is not inlined
#pragma warning(disable : 5045)  // Compiler will insert Spectre mitigation for memory load if /Qspectre switch specified
#endif
// Configure the areal template environment
// first: enable general or specialized fixed-point configurations
#define AREAL_FAST_SPECIALIZATION
// second: enable/disable areal arithmetic exceptions
#define AREAL_THROW_ARITHMETIC_EXCEPTION 1

// minimum set of include files to reflect source code dependencies
#include <universal/number/areal/areal_impl.hpp>
// fixed-point type manipulators such as pretty printers
#include <universal/number/areal/manipulators.hpp>
#include <universal/number/areal/math_functions.hpp>

#if BIT_CAST_SUPPORT
// stylistic constexpr of pi that we'll assign constexpr to an areal
constexpr double pi = 3.14159265358979323846;
#endif // BIT_CAST_SUPPORT

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
#if BIT_CAST_SUPPORT
	{
		CONSTEXPRESSION Real a(1.0f);  // float
		std::cout << a << '\n';
	}
	{
		CONSTEXPRESSION Real a(pi);   // double
		std::cout << a << '\n';
	}
	{
		CONSTEXPRESSION Real a(1.0l);  // long double
		std::cout << a << '\n';
	}
#endif // BIT_CAST_SUPPORT
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
#if BIT_CAST_SUPPORT
	{
		CONSTEXPRESSION Real a = 1.0f;  // float
		std::cout << a << '\n';
	}
	{
		CONSTEXPRESSION Real a = pi;   // double
		std::cout << a << '\n';
	}
	{
		CONSTEXPRESSION Real a = 1.0l;  // long double
		std::cout << a << '\n';
	}
#endif // BIT_CAST_SUPPORT
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

	int nrOfFailedTestCases = 0;

	std::cout << "AREAL constexpr tests\n";
	
	using Real = areal<12, 2>;
	Real a;
	a.constexprClassParameters();

	TestConstexprConstruction<Real>();
	TestConstexprAssignment<Real>();
	TestConstexprSpecificValues<Real>();

	if (nrOfFailedTestCases > 0) {
		std::cout << "FAIL\n";
	}
	else {
		std::cout << "PASS\n";
	}
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::areal_arithmetic_exception& err) {
	std::cerr << "Uncaught areal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::areal_internal_exception& err) {
	std::cerr << "Uncaught areal internal exception: " << err.what() << std::endl;
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
