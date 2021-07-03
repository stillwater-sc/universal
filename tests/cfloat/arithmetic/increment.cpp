// increment.cpp: test suite runner for increment operator on classic floats
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// minimum set of include files to reflect source code dependencies
#include <universal/number/cfloat/cfloat_impl.hpp>
#include <universal/verification/test_status.hpp>
#include <universal/verification/cfloat_test_suite.hpp>
#include <universal/utility/bit_cast.hpp>

#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main()
try {
	using namespace std;
	using namespace sw::universal;

	cout << "classic floating-point increment operator validation" << endl;
	int nrOfFailedTestCases = 0;

#if MANUAL_TESTING

/*
	{
		using Cfloat = cfloat<4, 1, uint8_t>;
		std::vector<Cfloat> set;
		GenerateOrderedCfloatSet<Cfloat>(set);
		for (auto v : set) {
			std::cout << to_binary(v) << " : " << v << '\n';
		}
	}
		{
		using Cfloat = cfloat<17,2, uint8_t, hasSubnormals, hasSupernormals, !isSaturating>;
		Cfloat c;
		c.setbits(0x0FEFD);
		for (int i = 0; i < 10; ++i) {
			std::cout << to_binary(c) << " : " << c++ << '\n';
		}
	}

*/

	{	
		constexpr bool hasSubnormals = true;
		constexpr bool hasSupernormals = true;
		constexpr bool isSaturating = true;
		using Cfloat = cfloat<17, 3, uint8_t, hasSubnormals, hasSupernormals, !isSaturating>;
		nrOfFailedTestCases += ReportTestResult(VerifyCfloatIncrement< Cfloat >(true), "cfloat<17,3,uint8_t,subnormals,supernormals,!saturating>", "increment");
	}
	std::cout << "Number of failed test cases : " << nrOfFailedTestCases << std::endl;
	nrOfFailedTestCases = 0; // disregard any test failures in manual testing mode

#else


	bool bReportIndividualTestCases = false;

	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloatIncrement< cfloat<4, 1> >(bReportIndividualTestCases), "cfloat<4,1,uint8_t,subnormals,supernormals,!saturating>", "increment");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloatIncrement < cfloat<8, 2> >(bReportIndividualTestCases), "cfloat<8,2,uint8_t,subnormals,supernormals,!saturating>", "increment");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloatIncrement < cfloat<9, 2> >(bReportIndividualTestCases), "cfloat<9,2,uint8_t,subnormals,supernormals,!saturating>", "increment");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloatIncrement < cfloat<10, 3> >(bReportIndividualTestCases), "cfloat<10,3,uint8_t,subnormals,supernormals,!saturating>", "increment");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloatIncrement < cfloat<17, 3> >(bReportIndividualTestCases), "cfloat<17,3,uint8_t,subnormals,supernormals,!saturating>", "increment");

	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloatIncrementSpecialCases< cfloat<32, 8, uint32_t, true, true, false> >(bReportIndividualTestCases), "cfloat<32, 8, subnormals, supernormals, !saturating", "increment");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloatIncrementSpecialCases< cfloat<64, 11, uint32_t, true, true, false> >(bReportIndividualTestCases), "cfloat<64, 11, subnormals, supernormals, !saturating", "increment");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloatIncrementSpecialCases< cfloat<128, 11, uint32_t, true, true, false> >(bReportIndividualTestCases), "cfloat<128, 11, subnormals, supernormals, !saturating", "increment");

#if STRESS_TESTING

#endif  // STRESS_TESTING

#endif  // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

}
catch (char const* msg) {
	std::cerr << "Caught exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::cfloat_divide_by_zero& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
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
