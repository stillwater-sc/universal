// decrement.cpp: test suite runner for decrement operator on classic floats
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_status.hpp>
#include <universal/verification/cfloat_test_suite.hpp>

#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main()
try {
	using namespace sw::universal;

	std::cout << "classic floating-point decrement operator validation\n";
	int nrOfFailedTestCases = 0;

#if MANUAL_TESTING

	{
		using Cfloat = cfloat<4, 1, uint8_t, true, true, false>;
		Cfloat c;
		c.setbits(0x00);
		--c;
		for (int i = 0; i < 5; ++i) {
			std::cout << to_binary(c) << " : " << c-- << '\n';
		}
	}

	{
		constexpr bool hasSubnormals = true;
		constexpr bool hasSupernormals = true;
		constexpr bool isSaturating = true;
		using Cfloat = cfloat<4, 1, uint8_t, hasSubnormals, hasSupernormals, !isSaturating>;
		nrOfFailedTestCases += ReportTestResult(VerifyCfloatDecrement< Cfloat >(true), "cfloat<4,1,uint8_t,subnormals,supernormals,!saturating>", "decrement");
	}

	{	
		constexpr bool hasSubnormals = true;
		constexpr bool hasSupernormals = true;
		constexpr bool isSaturating = true;
		using Cfloat = cfloat<17, 3, uint8_t, hasSubnormals, hasSupernormals, !isSaturating>;
		nrOfFailedTestCases += ReportTestResult(VerifyCfloatDecrement< Cfloat >(true), "cfloat<17,3,uint8_t,subnormals,supernormals,!saturating>", "decrement");
	}
	std::cout << "Number of failed test cases : " << nrOfFailedTestCases << std::endl;
	nrOfFailedTestCases = 0; // disregard any test failures in manual testing mode

#else


	bool bReportIndividualTestCases = false;

	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloatDecrement< cfloat<4, 1> >(bReportIndividualTestCases), "cfloat<4,1,uint8_t,subnormals,supernormals,!saturating>", "decrement");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloatDecrement < cfloat<8, 2> >(bReportIndividualTestCases), "cfloat<8,2,uint8_t,subnormals,supernormals,!saturating>", "decrement");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloatDecrement < cfloat<9, 2> >(bReportIndividualTestCases), "cfloat<9,2,uint8_t,subnormals,supernormals,!saturating>", "decrement");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloatDecrement < cfloat<10, 3> >(bReportIndividualTestCases), "cfloat<10,3,uint8_t,subnormals,supernormals,!saturating>", "decrement");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloatDecrement < cfloat<17, 3> >(bReportIndividualTestCases), "cfloat<17,3,uint8_t,subnormals,supernormals,!saturating>", "decrement");

#if STRESS_TESTING

#endif  // STRESS_TESTING

#endif  // MANUAL_TESTING

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
