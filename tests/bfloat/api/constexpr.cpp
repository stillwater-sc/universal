// constexpr.cpp: compile time tests for bfloat constexpr
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// minimum set of include files to reflect source code dependencies
// Configure the bfloat template environment
// first: enable general or specialized fixed-point configurations
#define BFLOAT_FAST_SPECIALIZATION
// second: enable/disable bfloat arithmetic exceptions
#define BFLOAT_THROW_ARITHMETIC_EXCEPTION 1

#include <universal/number/bfloat/bfloat.hpp>
#include <universal/number/bfloat/manipulators.hpp>
#include <universal/number/bfloat/math_functions.hpp>

#if BIT_CAST_SUPPORT
// stylistic constexpr of pi that we'll assign constexpr to an bfloat
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

template<typename Real>
void TestConstexprSpecificValues() {
	constexpr size_t nbits = Real::nbits;
	constexpr size_t es    = Real::es;
	using bt = typename Real::BlockType;
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

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	print_cmd_line(argc, argv);

	int nrOfFailedTestCases = 0;

	cout << "BFLOAT constexpr tests" << endl;
	
	using Real = bfloat<12, 2>;
	Real a;
	a.constexprClassParameters();

	TestConstexprConstruction<Real>();
	TestConstexprAssignment<Real>();
	TestConstexprSpecificValues<Real>();

	if (nrOfFailedTestCases > 0) {
		cout << "FAIL" << endl;
	}
	else {
		cout << "PASS" << endl;
	}
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::bfloat_arithmetic_exception& err) {
	std::cerr << "Uncaught bfloat arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::bfloat_internal_exception& err) {
	std::cerr << "Uncaught bfloat internal exception: " << err.what() << std::endl;
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
