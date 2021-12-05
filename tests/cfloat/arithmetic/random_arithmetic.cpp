// random_arithmetic.cpp: test suite runner for arithmetic operators for classic floats using randoms
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <iomanip>
// use default number system configuration
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_status.hpp>
#include <universal/verification/test_suite_random.hpp>

template<typename Cfloat>
int Randoms(bool reportTestCases, const std::string& test_tag, size_t nrTests) 
{
	using namespace sw::universal;

	int fails{ 0 };
	std::stringstream s;
	s << test_tag << ' ' << nrTests;
	fails += ReportTestResult(VerifyBinaryOperatorThroughRandoms< Cfloat >(reportTestCases, OPCODE_ADD, nrTests), s.str(), "addition      ");
	fails += ReportTestResult(VerifyBinaryOperatorThroughRandoms< Cfloat >(reportTestCases, OPCODE_SUB, nrTests), s.str(), "subtraction   ");
//	fails += ReportTestResult(VerifyBinaryOperatorThroughRandoms< Cfloat >(reportTestCases, OPCODE_MUL, nrTests), s.str(), "multiplication");
//	fails += ReportTestResult(VerifyBinaryOperatorThroughRandoms< Cfloat >(reportTestCases, OPCODE_DIV, nrTests), s.str(), "division      ");
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
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "Random test generation for large classic floatint-point configurations";
	std::string test_tag    = "randoms";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	std::cout << test_suite << '\n';

#if MANUAL_TESTING

	// bool reportTestCases = true;
	constexpr bool hasSubnormals = true;
	constexpr bool hasSupernormals = true;
	constexpr bool isSaturating = true;


	{
		using Cfloat = cfloat<32, 8, uint8_t, hasSubnormals, hasSupernormals, !isSaturating>;
//		nrOfFailedTestCases += Randoms<Cfloat>(reportTestCases, test_tag, 1000);
		/*
		FAIL - 1.439613800092129973e+30 + -4.6796573332097633664e+38 != -4.6796573332097633664e+38 golden reference is - 3.4028236692093846346e+38
			result 0b1.11111111.01100000000011101110110 vs ref 0b1.11111111.00000000000000000000000
			0b1.11100011.00100010101110100100101 + 0b1.11111111.01100000000011101110110
		*/
		Cfloat a, b, c;
		a = sw::universal::parse<32, 8, uint8_t, hasSubnormals, hasSupernormals, !isSaturating>("0b1.11100011.00100010101110100100101");
		b = sw::universal::parse<32, 8, uint8_t, hasSubnormals, hasSupernormals, !isSaturating>("0b1.11111111.01100000000011101110110");
		c = a + b;
		std::cout << a << " + " << b << " = " << c << '\n';

		double da = double(a);
		double db = double(b);
		double dc = da + db;
		std::cout << da << " + " << db << " = " << dc << '\n';

		std::cout << to_binary(c) << '\n' << to_binary(dc) << '\n' << to_binary(float(dc)) << '\n';

	}

	{
		using Cfloat = cfloat<40, 8, uint8_t, hasSubnormals, hasSupernormals, !isSaturating>;
//		nrOfFailedTestCases += Randoms<Cfloat>(reportTestCases, test_tag, 10);

	/*
FAIL -0.021134873604751192033 +         407433878912 != 15.431546136736869812 golden reference is         407433878912
 result 0b0.10000010.1110110111001111001110011101100 vs ref 0b0.10100101.0111101101110011110011100111011
0b1.01111001.0101101001000110000101011011110 + 0b0.10100101.0111101101110011110011100111011
	*/

		Cfloat a, b, c;
		a = sw::universal::parse<40, 8, uint8_t, hasSubnormals, hasSupernormals, !isSaturating>("0b1.01111001.0101101001000110000101011011110");
		b = sw::universal::parse<40, 8, uint8_t, hasSubnormals, hasSupernormals, !isSaturating>("0b0.10100101.0111101101110011110011100111011");
		c = a + b;
		std::cout << a << " + " << b << " = " << c << '\n';

		double da = double(a);
		double db = double(b);
		double dc = da + db;
		std::cout << da << " + " << db << " = " << dc <<  '\n';

		std::cout << to_binary(c) << '\n' << to_binary(dc) << '\n' << to_binary(float(dc)) << '\n';
	}

	{
		using Cfloat = cfloat<16, 5, uint8_t, hasSubnormals, hasSupernormals, !isSaturating>;
		nrOfFailedTestCases += Randoms<Cfloat>(reportTestCases, test_tag, 100);
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors

#else

#if REGRESSION_LEVEL_1
	constexpr bool hasSubnormals = true;
	constexpr bool hasSupernormals = true;
	constexpr bool isSaturating = true;

	{
		using Cfloat = cfloat<16, 5, uint8_t, hasSubnormals, hasSupernormals, !isSaturating>;
		nrOfFailedTestCases += Randoms<Cfloat>(reportTestCases, test_tag, 1000000);
	}
	{
		using Cfloat = cfloat<16, 7, uint8_t, hasSubnormals, hasSupernormals, !isSaturating>;
		nrOfFailedTestCases += Randoms<Cfloat>(reportTestCases, test_tag, 1000000);
	}
	{
		using Cfloat = cfloat<16, 8, uint8_t, hasSubnormals, hasSupernormals, !isSaturating>;
		nrOfFailedTestCases += Randoms<Cfloat>(reportTestCases, test_tag, 1000000);
	}
	{
		using Cfloat = cfloat<20, 8, uint8_t, hasSubnormals, hasSupernormals, !isSaturating>;
		nrOfFailedTestCases += Randoms<Cfloat>(reportTestCases, test_tag, 1000000);
	}
	{
		using Cfloat = cfloat<24, 8, uint8_t, hasSubnormals, hasSupernormals, !isSaturating>;
		nrOfFailedTestCases += Randoms<Cfloat>(reportTestCases, test_tag, 1000000);
	}
	{
		using Cfloat = cfloat<28, 8, uint8_t, hasSubnormals, hasSupernormals, !isSaturating>;
		nrOfFailedTestCases += Randoms<Cfloat>(reportTestCases, test_tag, 1000000);
	}
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
