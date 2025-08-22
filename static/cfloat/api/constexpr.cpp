// constexpr.cpp: compile time tests for classic float constexpr
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// minimum set of include files to reflect source code dependencies
// Configure the cfloat template environment
// first: enable general or specialized fixed-point configurations
#define CFLOAT_FAST_SPECIALIZATION
// second: enable/disable cfloat arithmetic exceptions
#define CFLOAT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/cfloat/math/constants/constants.hpp>
#include <universal/verification/test_suite.hpp>

// stylistic constexpr of pi that we'll assign constexpr to an cfloat
constexpr double pi = 3.14159265358979323846;

template<typename Real>
void TestConstexprConstruction() {
	// decorated constructors
	{
		constexpr Real a(1l);  // signed long
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
#if LONG_DOUBLE_SUPPORT
	{
		Real a(1.0l);  // long double
		std::cout << a << '\n';
	}
#endif
}

template<typename Real>
void TestConstexprAssignment() {
	// decorated constructors
	{
		constexpr Real a = 1l;  // signed long
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
#if LONG_DOUBLE_SUPPORT
	{
		Real a = 1.0l;  // long double
		std::cout << a << '\n';
	}
#endif
}

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

template<typename Cfloat>
void TestConstexprConstants() {
	//std::cout << "constexpr constants for cfloat<" << Cfloat::nbits << ',' << int(Cfloat::es) << "> :\n";
	std::cout << "constexpr constants for " << sw::universal::type_tag<Cfloat>() << " :\n";
#if LONG_DOUBLE_SUPPORT
	constexpr Cfloat pi = sw::universal::cf_pi;
	std::cout << "pi          : " << to_binary(pi) << " : " << pi << '\n';
	constexpr Cfloat pi_2 = sw::universal::cf_pi_2;
	std::cout << "pi/2        : " << to_binary(pi_2) << " : " << pi_2 << '\n';
	constexpr Cfloat pi_4 = sw::universal::cf_pi_4;
	std::cout << "pi/4        : " << to_binary(pi_4) << " : " << pi_4 << '\n';
	constexpr Cfloat pi_3 = sw::universal::cf_pi_3;
	std::cout << "pi/3        : " << to_binary(pi_3) << " : " << pi_3 << '\n';
	constexpr Cfloat threexpi_4 = sw::universal::cf_3pi_4;
	std::cout << "3*pi/4      : " << to_binary(threexpi_4) << " : " << threexpi_4 << '\n';
	constexpr Cfloat twoxpi = sw::universal::cf_2pi;
	std::cout << "2*pi        : " << to_binary(twoxpi) << " : " << twoxpi << '\n';
	constexpr Cfloat threexpi = sw::universal::cf_3pi;
	std::cout << "3*pi        : " << to_binary(threexpi) << " : " << threexpi << '\n';
	constexpr Cfloat fourxpi = sw::universal::cf_4pi;
	std::cout << "4*pi        : " << to_binary(fourxpi) << " : " << fourxpi << '\n';
	constexpr Cfloat four_pi = sw::universal::cf_4_pi;
	std::cout << "4/pi        : " << to_binary(four_pi) << " : " << four_pi << '\n';
	constexpr Cfloat three_pi = sw::universal::cf_3_pi;
	std::cout << "3/pi        : " << to_binary(three_pi) << " : " << three_pi << '\n';
	constexpr Cfloat two_pi = sw::universal::cf_2_pi;
	std::cout << "2/pi        : " << to_binary(two_pi) << " : " << two_pi << '\n';

	constexpr Cfloat one_pi = sw::universal::cf_1_pi;
	std::cout << "1/pi        : " << to_binary(one_pi) << " : " << one_pi << '\n';
	constexpr Cfloat two_pi2 = sw::universal::cf_2_pi;
	std::cout << "2/pi        : " << to_binary(two_pi2) << " : " << two_pi2 << '\n';
	constexpr Cfloat two_sqrtpi = sw::universal::cf_2_sqrtpi;
	std::cout << "2/sqrt(pi)  : " << to_binary(two_sqrtpi) << " : " << two_sqrtpi << '\n';
	constexpr Cfloat sqrt2 = sw::universal::cf_sqrt2;
	std::cout << "sqrt(2)     : " << to_binary(sqrt2) << " : " << sqrt2 << '\n';
	constexpr Cfloat one_sqrt2 = sw::universal::cf_1_sqrt2;
	std::cout << "1/sqrt(2)   : " << to_binary(one_sqrt2) << " : " << one_sqrt2 << '\n';

	constexpr Cfloat e = sw::universal::cf_e;
	std::cout << "e           : " << to_binary(e) << " : " << e << '\n';
	constexpr Cfloat euler_gamma = sw::universal::cf_e_gamma;
	std::cout << "euler gamma : " << to_binary(euler_gamma) << " : " << euler_gamma << '\n';
	constexpr Cfloat log2e = sw::universal::cf_log2e;
	std::cout << "log2(e)     : " << to_binary(log2e) << " : " << log2e << '\n';
	constexpr Cfloat log10e = sw::universal::cf_log10e;
	std::cout << "log10(e)    : " << to_binary(log10e) << " : " << log10e << '\n';
	constexpr Cfloat ln2 = sw::universal::cf_ln2;
	std::cout << "ln(2)       : " << to_binary(ln2) << " : " << ln2 << '\n';
	constexpr Cfloat ln3 = sw::universal::cf_ln3;
	std::cout << "ln(3)       : " << to_binary(ln3) << " : " << ln3 << '\n';
	constexpr Cfloat ln4 = sw::universal::cf_ln4;
	std::cout << "ln(4)       : " << to_binary(ln4) << " : " << ln4 << '\n';
	constexpr Cfloat ln10 = sw::universal::cf_ln10;
	std::cout << "ln(10)      : " << to_binary(ln10) << " : " << ln10 << '\n';
#else
	constexpr Cfloat pi = double(sw::universal::cf_pi);
	std::cout << "pi          : " << to_binary(pi) << " : " << pi << '\n';
	constexpr Cfloat pi_2 = double(sw::universal::cf_pi_2);
	std::cout << "pi/2        : " << to_binary(pi_2) << " : " << pi_2 << '\n';
	constexpr Cfloat pi_4 = double(sw::universal::cf_pi_4);
	std::cout << "pi/4        : " << to_binary(pi_4) << " : " << pi_4 << '\n';
	constexpr Cfloat pi_3 = double(sw::universal::cf_pi_3);
	std::cout << "pi/3        : " << to_binary(pi_3) << " : " << pi_3 << '\n';
	constexpr Cfloat threexpi_4 = double(sw::universal::cf_3pi_4);
	std::cout << "3*pi/4      : " << to_binary(threexpi_4) << " : " << threexpi_4 << '\n';
	constexpr Cfloat twoxpi = double(sw::universal::cf_2pi);
	std::cout << "2*pi        : " << to_binary(twoxpi) << " : " << twoxpi << '\n';
	constexpr Cfloat threexpi = double(sw::universal::cf_3pi);
	std::cout << "3*pi        : " << to_binary(threexpi) << " : " << threexpi << '\n';
	constexpr Cfloat fourxpi = double(sw::universal::cf_4pi);
	std::cout << "4*pi        : " << to_binary(fourxpi) << " : " << fourxpi << '\n';
	constexpr Cfloat four_pi = double(sw::universal::cf_4_pi);
	std::cout << "4/pi        : " << to_binary(four_pi) << " : " << four_pi << '\n';
	constexpr Cfloat three_pi = double(sw::universal::cf_3_pi);
	std::cout << "3/pi        : " << to_binary(three_pi) << " : " << three_pi << '\n';
	constexpr Cfloat two_pi = double(sw::universal::cf_2_pi);
	std::cout << "2/pi        : " << to_binary(two_pi) << " : " << two_pi << '\n';

	constexpr Cfloat one_pi = double(sw::universal::cf_1_pi);
	std::cout << "1/pi        : " << to_binary(one_pi) << " : " << one_pi << '\n';
	constexpr Cfloat two_pi2 = double(sw::universal::cf_2_pi);
	std::cout << "2/pi        : " << to_binary(two_pi2) << " : " << two_pi2 << '\n';
	constexpr Cfloat two_sqrtpi = double(sw::universal::cf_2_sqrtpi);
	std::cout << "2/sqrt(pi)  : " << to_binary(two_sqrtpi) << " : " << two_sqrtpi << '\n';
	constexpr Cfloat sqrt2 = double(sw::universal::cf_sqrt2);
	std::cout << "sqrt(2)     : " << to_binary(sqrt2) << " : " << sqrt2 << '\n';
	constexpr Cfloat one_sqrt2 = double(sw::universal::cf_1_sqrt2);
	std::cout << "1/sqrt(2)   : " << to_binary(one_sqrt2) << " : " << one_sqrt2 << '\n';

	constexpr Cfloat e = double(sw::universal::cf_e);
	std::cout << "e           : " << to_binary(e) << " : " << e << '\n';
	constexpr Cfloat euler_gamma = double(sw::universal::cf_e_gamma);
	std::cout << "euler gamma : " << to_binary(euler_gamma) << " : " << euler_gamma << '\n';
	constexpr Cfloat log2e = double(sw::universal::cf_log2e);
	std::cout << "log2(e)     : " << to_binary(log2e) << " : " << log2e << '\n';
	constexpr Cfloat log10e = double(sw::universal::cf_log10e);
	std::cout << "log10(e)    : " << to_binary(log10e) << " : " << log10e << '\n';
	constexpr Cfloat ln2 = double(sw::universal::cf_ln2);
	std::cout << "ln(2)       : " << to_binary(ln2) << " : " << ln2 << '\n';
	constexpr Cfloat ln3 = double(sw::universal::cf_ln3);
	std::cout << "ln(3)       : " << to_binary(ln3) << " : " << ln3 << '\n';
	constexpr Cfloat ln4 = double(sw::universal::cf_ln4);
	std::cout << "ln(4)       : " << to_binary(ln4) << " : " << ln4 << '\n';
	constexpr Cfloat ln10 = double(sw::universal::cf_ln10);
	std::cout << "ln(10)      : " << to_binary(ln10) << " : " << ln10 << '\n';
#endif
}

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "cfloat constexpr demonstration";
	std::string test_tag    = "constexpr";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	using Real = cfloat<12, 2>;
	Real a(0);
	a.constexprClassParameters();

	TestConstexprConstruction<Real>();
	TestConstexprAssignment<Real>();
	TestConstexprSpecificValues<Real>();
	TestConstexprConstants<Real>();
	TestConstexprConstants<cfloat<48, 11>>();
	//TestConstexprConstants<cfloat<80, 11>>();   TODO: not yet constexpr

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
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
