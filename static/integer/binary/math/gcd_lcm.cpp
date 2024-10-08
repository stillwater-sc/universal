// gcd_lcm.cpp: greatest common divisor and least common multiple tests on arbitrary precision integers
//
// Binomial coefficients are useful to generate the inverse of a Hilbert matrix
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal number project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <universal/number/integer/integer.hpp>
#include <universal/verification/test_suite.hpp>

template<size_t nbits, typename BlockType>
sw::universal::integer<nbits, BlockType> greatest_common_divisor(const sw::universal::integer<nbits, BlockType>& a, const sw::universal::integer<nbits, BlockType>& b) {
	std::cout << "gcd(" << a << ", " << b << ")\n";
	return b.iszero() ? a : greatest_common_divisor(b, a % b);
}

int TestGCDCase() {
	using namespace sw::universal;

	int nrFailures = 0;



	return nrFailures;
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
#define REGRESSION_LEVEL_2 0
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main() 
try {
	using namespace sw::universal;

	std::string test_suite  = "Integer GCD and LCM verfication";
	std::string test_tag    = "gcd/lcm";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	using Integer = integer<1024, uint32_t>;
	Integer a, b, c;
	a = 1234567890500;
	b = 92875085904958;
	c = a * b * 10;
	std::cout << greatest_common_divisor(a, c) << " a = " << a << '\n';
	std::cout << sw::universal::gcd(a, c) << " a = " << a << '\n';

	a = 252;
	b = 105;
	c = a * b;
	std::cout << "gcd(" << a << "," << b << ") = " << gcd(a, b) << " answer should be 21" << '\n';
	std::cout << "gcd(" << a << "," << c << ") = " << gcd(a, c) << " answer should be 252" << '\n';
	std::cout << "gcd(" << b << "," << c << ") = " << gcd(b, c) << " answer should be 105" << '\n';
	std::cout << "gcd(" << a << "," << gcd(b, c) << ") = " << gcd(a, gcd(b, c)) << '\n';
	std::cout << "gcd(" << a << "," << gcd(a, c) << ") = " << gcd(b, gcd(a, c)) << '\n';
	std::cout << "gcd(" << a << "," << gcd(a, b) << ") = " << gcd(c, gcd(a, b)) << '\n';

	std::vector< Integer > v;
	v.push_back(a);
	v.push_back(b);
	v.push_back(c);
	std::cout << gcd(v) << '\n';

	a = 3;
	b = 7;
	c = a * b;
	std::cout << "lcm(" << a << "," << b << ") = " << lcm(a, b) << " answer should be 21" << '\n';
	std::cout << "lcm(" << a << "," << lcm(b, c) << ") = " << lcm(a, lcm(b, c)) << '\n';
	std::cout << "lcm(" << b << "," << lcm(a, c) << ") = " << lcm(b, lcm(a, c)) << '\n';
	std::cout << "lcm(" << c << "," << lcm(a, b) << ") = " << lcm(c, lcm(a, b)) << '\n';

	v.clear();
	v.push_back(2);
	v.push_back(3);
	v.push_back(4);
	v.push_back(5);
	v.push_back(6);
	v.push_back(7);
	v.push_back(8);
	v.push_back(9);
	v.push_back(10);
	v.push_back(11);
	v.push_back(12);
	v.push_back(13);
	v.push_back(14);
	v.push_back(15);
	std::cout << "lcm( 2 through 15 ) = " << lcm(v) << '\n';
	v.push_back(16);
	v.push_back(17);
	std::cout << "lcm( 2 through 17 ) = " << lcm(v) << '\n';
	v.push_back(18);
	v.push_back(19);
	std::cout << "lcm( 2 through 19 ) = " << lcm(v) << '\n';
	v.push_back(20);
	v.push_back(21);
	std::cout << "lcm( 2 through 21 ) = " << lcm(v) << '\n';
	v.push_back(22);
	std::cout << "lcm( 2 through 22 ) = " << lcm(v) << '\n';
	v.push_back(91);
	std::cout << "lcm( 2 through 91 ) = " << lcm(v) << '\n';

	Integer leastCM = lcm(v);
	std::cout << leastCM / 17 << " " << leastCM % 17 << '\n';
	std::cout << leastCM / 21 << " " << leastCM % 21 << '\n';
	std::cout << leastCM / 91 << " " << leastCM % 91 << '\n';

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else // MANUAL_TESTING


#if REGRESSION_LEVEL_1
	{
		using Integer = integer<1024, uint32_t>;
		Integer a, b, c;

		// GCD of three numbers is
		// gcd(a, b, c) == gcd(a, gcd(b, c)) == gcd(gcd(a, b), c) == gcd(b, gcd(a, c))
		a = 252;
		b = 105;
		c = a * b;
		if (gcd(a, b) != 21) ++nrOfFailedTestCases;
		if (gcd(a, c) != 252) ++nrOfFailedTestCases;
		if (gcd(b, c) != 105) ++nrOfFailedTestCases;

		a = 3;
		b = 7;
		c = a * b;
		if (reportTestCases) std::cout << "lcm(" << a << "," << b << ") = " << lcm(a, b) << " answer should be 21" << '\n';
		if (lcm(a, b) != 21) ++nrOfFailedTestCases;
		if (lcm(a, lcm(b, c)) != 21) ++nrOfFailedTestCases;
		if (lcm(b, lcm(a, c)) != 21) ++nrOfFailedTestCases;
		if (lcm(c, lcm(a, b)) != 21) ++nrOfFailedTestCases;
	}
#endif

#if REGRESSION_LEVEL_2
	{
		using Integer = integer<1024, uint32_t>;
		Integer a, b, c;
		a = 1234567890500;
		b = 92875085904958;
		c = a * b * 10;
		if (gcd(a, c) != 1234567890500) ++nrOfFailedTestCases;
		if (gcd(b, c) != 92875085904958) ++nrOfFailedTestCases;
	}
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (std::runtime_error& err) {
	std::cerr << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
