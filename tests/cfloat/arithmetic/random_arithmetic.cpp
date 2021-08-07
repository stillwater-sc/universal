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
int Randoms(bool bReportIndividualTestCases, const std::string& tag, size_t nrTests) 
{
	using namespace sw::universal;

	int fails{ 0 };
	std::stringstream s;
	s << tag << ' ' << nrTests;
	fails += ReportTestResult(VerifyBinaryOperatorThroughRandoms< Cfloat >(bReportIndividualTestCases, OPCODE_ADD, nrTests), s.str(), "addition      ");
	fails += ReportTestResult(VerifyBinaryOperatorThroughRandoms< Cfloat >(bReportIndividualTestCases, OPCODE_SUB, nrTests), s.str(), "subtraction   ");
//	fails += ReportTestResult(VerifyBinaryOperatorThroughRandoms< Cfloat >(bReportIndividualTestCases, OPCODE_MUL, nrTests), s.str(), "multiplication");
//	fails += ReportTestResult(VerifyBinaryOperatorThroughRandoms< Cfloat >(bReportIndividualTestCases, OPCODE_DIV, nrTests), s.str(), "division      ");
	return fails;
}

#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	print_cmd_line(argc, argv);

	int nrOfFailedTestCases = 0;
	std::string tag = "randoms";

	cout << "Random test generation for large classic floatint-point configurations" << endl;

#if MANUAL_TESTING

	bool bReportIndividualTestCases = true;
	constexpr bool hasSubnormals = true;
	constexpr bool hasSupernormals = true;
	constexpr bool isSaturating = true;


	{
		using Cfloat = cfloat<32, 8, uint8_t, hasSubnormals, hasSupernormals, !isSaturating>;
//		nrOfFailedTestCases += Randoms<Cfloat>(bReportIndividualTestCases, tag, 1000);
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
//		nrOfFailedTestCases += Randoms<Cfloat>(bReportIndividualTestCases, tag, 10);

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
	nrOfFailedTestCases = 0; // manual testing ignores any test failures

#else // !MANUAL_TESTING

	bool bReportIndividualTestCases = false;
	constexpr bool hasSubnormals = true;
	constexpr bool hasSupernormals = true;
	constexpr bool isSaturating = true;

	{
		using Cfloat = cfloat<16, 5, uint8_t, hasSubnormals, hasSupernormals, !isSaturating>;
		nrOfFailedTestCases += Randoms<Cfloat>(bReportIndividualTestCases, tag, 1000000);
	}
	{
		using Cfloat = cfloat<16, 7, uint8_t, hasSubnormals, hasSupernormals, !isSaturating>;
		nrOfFailedTestCases += Randoms<Cfloat>(bReportIndividualTestCases, tag, 1000000);
	}
	{
		using Cfloat = cfloat<16, 8, uint8_t, hasSubnormals, hasSupernormals, !isSaturating>;
		nrOfFailedTestCases += Randoms<Cfloat>(bReportIndividualTestCases, tag, 1000000);
	}
	{
		using Cfloat = cfloat<20, 8, uint8_t, hasSubnormals, hasSupernormals, !isSaturating>;
		nrOfFailedTestCases += Randoms<Cfloat>(bReportIndividualTestCases, tag, 1000000);
	}
	{
		using Cfloat = cfloat<24, 8, uint8_t, hasSubnormals, hasSupernormals, !isSaturating>;
		nrOfFailedTestCases += Randoms<Cfloat>(bReportIndividualTestCases, tag, 1000000);
	}
	{
		using Cfloat = cfloat<28, 8, uint8_t, hasSubnormals, hasSupernormals, !isSaturating>;
		nrOfFailedTestCases += Randoms<Cfloat>(bReportIndividualTestCases, tag, 1000000);
	}
#endif

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
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
