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
	fails += ReportTestResult(VerifyBinaryOperatorThroughRandoms< Cfloat >(bReportIndividualTestCases, OPCODE_ADD, nrTests), tag, "addition      ");
	fails += ReportTestResult(VerifyBinaryOperatorThroughRandoms< Cfloat >(bReportIndividualTestCases, OPCODE_SUB, nrTests), tag, "subtraction   ");
//	fails += ReportTestResult(VerifyBinaryOperatorThroughRandoms< Cfloat >(bReportIndividualTestCases, OPCODE_MUL, nrTests), tag, "multiplication");
//	fails += ReportTestResult(VerifyBinaryOperatorThroughRandoms< Cfloat >(bReportIndividualTestCases, OPCODE_DIV, nrTests), tag, "division      ");
	return fails;
}

#define MANUAL_TESTING 1
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
		using Cfloat = cfloat<24, 8, uint8_t, hasSubnormals, hasSupernormals, !isSaturating>;
		nrOfFailedTestCases += Randoms<Cfloat>(bReportIndividualTestCases, tag, 100);
	}
	{
		using Cfloat = cfloat<32, 8, uint8_t, hasSubnormals, hasSupernormals, !isSaturating>;
		nrOfFailedTestCases += Randoms<Cfloat>(bReportIndividualTestCases, tag, 100);
	}
	{
		using Cfloat = cfloat<40, 8, uint8_t, hasSubnormals, hasSupernormals, !isSaturating>;
		nrOfFailedTestCases += Randoms<Cfloat>(bReportIndividualTestCases, tag, 100);
	}

	nrOfFailedTestCases = 0; // manual testing ignores any test failures

#else // !MANUAL_TESTING
	cout << "Arithmetic tests " << RND_TEST_CASES << " randoms each" << endl;

	bool bReportIndividualTestCases = false;

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
