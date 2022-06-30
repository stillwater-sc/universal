// logic.cpp: test suite runner for logic operation on arbitrary logarithmic number system
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

#include <universal/number/lns/lns.hpp>
#include <universal/verification/test_suite.hpp>

template<size_t nbits, size_t rbits, typename BlockType>
int VerifyZeroEncoding(bool reportTestCases) {
	int nrOfFailedTestCases = 0;

	// single block configurations
	sw::universal::lns<nbits, rbits, BlockType> a, b;
	a.setzero();
	b = a;
	if (!b.iszero()) {
		++nrOfFailedTestCases;
		if (reportTestCases) ReportLogicError("iszero", "==", a, b, false);
	}
	else {
		//if (reportTestCases) ReportLogicSuccess("iszero", "==", a, b, true);
	}

	return nrOfFailedTestCases;
}

int VerifyZeroEncodings(bool reportTestCases) {
	using namespace sw::universal;
	int nrOfFailedTestCases = 0;

	std::string test_tag = "iszero()";

	// single block configurations
	nrOfFailedTestCases += ReportTestResult(VerifyZeroEncoding< 8,  4, std::uint8_t>(reportTestCases), "lns<8,4,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyZeroEncoding<16,  8, std::uint16_t>(reportTestCases), "lns<16,8,uint16_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyZeroEncoding<32, 16, std::uint32_t>(reportTestCases), "lns<32,16,uint32_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyZeroEncoding<64, 32, std::uint64_t>(reportTestCases), "lns<64,32,uint64_t>", test_tag);

	// double block configurations with all special bits in the MSU
	nrOfFailedTestCases += ReportTestResult(VerifyZeroEncoding<10, 4, std::uint8_t>(reportTestCases), "lns<10,4,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyZeroEncoding<18, 8, std::uint16_t>(reportTestCases), "lns<18,8,uint16_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyZeroEncoding<34, 16, std::uint32_t>(reportTestCases), "lns<34,16,uint32_t>", test_tag);
//	nrOfFailedTestCases += ReportTestResult(VerifyZeroEncoding<66, 32, std::uint64_t>(reportTestCases), "lns<66,32,uint64_t>", test_tag);

	// double block configurations with special bits split between MSU and MSU - 1
	nrOfFailedTestCases += ReportTestResult(VerifyZeroEncoding< 9, 4, std::uint8_t>(reportTestCases), "lns<9,4,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyZeroEncoding<17, 8, std::uint16_t>(reportTestCases), "lns<17,8,uint16_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyZeroEncoding<33, 16, std::uint32_t>(reportTestCases), "lns<33,16,uint32_t>", test_tag);
//	nrOfFailedTestCases += ReportTestResult(VerifyZeroEncoding<65, 32, std::uint64_t>(reportTestCases), "lns<65,32,uint64_t>", test_tag);

	// triple block configurations with all special bits in the MSU
	nrOfFailedTestCases += ReportTestResult(VerifyZeroEncoding<26, 4, std::uint8_t>(reportTestCases), "lns<26,4,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyZeroEncoding<50, 8, std::uint16_t>(reportTestCases), "lns<50,8,uint16_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyZeroEncoding<98, 16, std::uint32_t>(reportTestCases), "lns<98,16,uint32_t>", test_tag);
//	nrOfFailedTestCases += ReportTestResult(VerifyZeroEncoding<66, 32, std::uint64_t>(reportTestCases), "lns<66,32,uint64_t>", test_tag);

	// triple block configurations with special bits split between MSU and MSU - 1
	nrOfFailedTestCases += ReportTestResult(VerifyZeroEncoding<25, 4, std::uint8_t>(reportTestCases), "lns<25,4,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyZeroEncoding<49, 8, std::uint16_t>(reportTestCases), "lns<49,8,uint16_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyZeroEncoding<97, 16, std::uint32_t>(reportTestCases), "lns<97,16,uint32_t>", test_tag);
//	nrOfFailedTestCases += ReportTestResult(VerifyZeroEncoding<65, 32, std::uint64_t>(reportTestCases), "lns<65,32,uint64_t>", test_tag);

	return nrOfFailedTestCases;
}

template<size_t nbits, size_t rbits, typename BlockType>
int VerifyNaNEncoding(bool reportTestCases) {
	int nrOfFailedTestCases = 0;

	// single block configurations
	sw::universal::lns<nbits, rbits, BlockType> a, b;
	a.setnan();
	b = a;
	if (!b.isnan()) {
		++nrOfFailedTestCases;
		if (reportTestCases) ReportLogicError("isnan", "==", a, b, false);
	}
	else {
		// if (reportTestCases) ReportLogicSuccess("isnan", "==", a, b, true);
	}

	return nrOfFailedTestCases;
}

int VerifyNaNEncodings(bool reportTestCases) {
	using namespace sw::universal;
	int nrOfFailedTestCases = 0;

	std::string test_tag = "isnan()";

	// single block configurations
	nrOfFailedTestCases += ReportTestResult(VerifyNaNEncoding< 8, 4, std::uint8_t>(reportTestCases), "lns<8,4,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNaNEncoding<16, 8, std::uint16_t>(reportTestCases), "lns<16,8,uint16_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNaNEncoding<32, 16, std::uint32_t>(reportTestCases), "lns<32,16,uint32_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNaNEncoding<64, 32, std::uint64_t>(reportTestCases), "lns<64,32,uint64_t>", test_tag);

	// double block configurations with all special bits in the MSU
	nrOfFailedTestCases += ReportTestResult(VerifyNaNEncoding<10, 4, std::uint8_t>(reportTestCases), "lns<10,4,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNaNEncoding<18, 8, std::uint16_t>(reportTestCases), "lns<18,8,uint16_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNaNEncoding<34, 16, std::uint32_t>(reportTestCases), "lns<34,16,uint32_t>", test_tag);
	//	nrOfFailedTestCases += ReportTestResult(VerifyZeroEncoding<66, 32, std::uint64_t>(reportTestCases), "lns<66,32,uint64_t>", test_tag);

		// double block configurations with special bits split between MSU and MSU - 1
	nrOfFailedTestCases += ReportTestResult(VerifyNaNEncoding< 9, 4, std::uint8_t>(reportTestCases), "lns<9,4,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNaNEncoding<17, 8, std::uint16_t>(reportTestCases), "lns<17,8,uint16_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNaNEncoding<33, 16, std::uint32_t>(reportTestCases), "lns<33,16,uint32_t>", test_tag);
	//	nrOfFailedTestCases += ReportTestResult(VerifyZeroEncoding<65, 32, std::uint64_t>(reportTestCases), "lns<65,32,uint64_t>", test_tag);

		// triple block configurations with all special bits in the MSU
	nrOfFailedTestCases += ReportTestResult(VerifyNaNEncoding<26, 4, std::uint8_t>(reportTestCases), "lns<26,4,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNaNEncoding<50, 8, std::uint16_t>(reportTestCases), "lns<50,8,uint16_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNaNEncoding<98, 16, std::uint32_t>(reportTestCases), "lns<98,16,uint32_t>", test_tag);
	//	nrOfFailedTestCases += ReportTestResult(VerifyZeroEncoding<66, 32, std::uint64_t>(reportTestCases), "lns<66,32,uint64_t>", test_tag);

		// triple block configurations with special bits split between MSU and MSU - 1
	nrOfFailedTestCases += ReportTestResult(VerifyNaNEncoding<25, 4, std::uint8_t>(reportTestCases), "lns<25,4,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNaNEncoding<49, 8, std::uint16_t>(reportTestCases), "lns<49,8,uint16_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNaNEncoding<97, 16, std::uint32_t>(reportTestCases), "lns<97,16,uint32_t>", test_tag);
	//	nrOfFailedTestCases += ReportTestResult(VerifyZeroEncoding<65, 32, std::uint64_t>(reportTestCases), "lns<65,32,uint64_t>", test_tag);

	return nrOfFailedTestCases;
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
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "lns logic validation";
	std::string test_tag    = "logic ops";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	std::cout << test_suite << '\n';

#if MANUAL_TESTING

	nrOfFailedTestCases += VerifyZeroEncodings(reportTestCases);
	nrOfFailedTestCases += VerifyNaNEncodings(reportTestCases);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += VerifyZeroEncodings(reportTestCases);
	nrOfFailedTestCases += VerifyNaNEncodings(reportTestCases);
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
