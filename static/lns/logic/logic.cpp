// logic.cpp: test suite runner for logic operation on arbitrary logarithmic number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <limits>
#include <universal/number/lns/lns.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw {
	namespace universal {

template<typename TestType>
int VerifyZeroEncoding(bool reportTestCases) {
	int nrOfFailedTestCases = 0;

	TestType a, b;
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
	nrOfFailedTestCases += ReportTestResult(VerifyZeroEncoding< lns< 8, 4, std::uint8_t > >(reportTestCases), "lns< 8, 4,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyZeroEncoding< lns<16, 8, std::uint16_t> >(reportTestCases), "lns<16, 8,uint16_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyZeroEncoding< lns<32, 16, std::uint32_t> >(reportTestCases), "lns<32,16,uint32_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyZeroEncoding< lns<64, 32, std::uint64_t> >(reportTestCases), "lns<64,32,uint64_t>", test_tag);

	// double block configurations with all special bits in the MSU
	nrOfFailedTestCases += ReportTestResult(VerifyZeroEncoding< lns<10, 4, std::uint8_t > >(reportTestCases), "lns<10, 4,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyZeroEncoding< lns<18, 8, std::uint16_t> >(reportTestCases), "lns<18, 8,uint16_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyZeroEncoding< lns<34, 16, std::uint32_t> >(reportTestCases), "lns<34,16,uint32_t>", test_tag);
//	nrOfFailedTestCases += ReportTestResult(VerifyZeroEncoding< lns<66, 32, std::uint64_t> >(reportTestCases), "lns<66,32,uint64_t>", test_tag);

	// double block configurations with special bits split between MSU and MSU - 1
	nrOfFailedTestCases += ReportTestResult(VerifyZeroEncoding< lns< 9, 4, std::uint8_t > >(reportTestCases), "lns< 9, 4,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyZeroEncoding< lns<17, 8, std::uint16_t> >(reportTestCases), "lns<17, 8,uint16_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyZeroEncoding< lns<33, 16, std::uint32_t> >(reportTestCases), "lns<33,16,uint32_t>", test_tag);
//	nrOfFailedTestCases += ReportTestResult(VerifyZeroEncoding< lns<65, 32, std::uint64_t> >(reportTestCases), "lns<65,32,uint64_t>", test_tag);

	// triple block configurations with all special bits in the MSU
	nrOfFailedTestCases += ReportTestResult(VerifyZeroEncoding< lns<26, 13, std::uint8_t > >(reportTestCases), "lns<26,13,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyZeroEncoding< lns<50, 25, std::uint16_t> >(reportTestCases), "lns<50,25,uint16_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyZeroEncoding< lns<98, 60, std::uint32_t> >(reportTestCases), "lns<98,60,uint32_t>", test_tag);
//	nrOfFailedTestCases += ReportTestResult(VerifyZeroEncoding< lns<66, 32, std::uint64_t> >(reportTestCases), "lns<66,32,uint64_t>", test_tag);

	// triple block configurations with special bits split between MSU and MSU - 1
	nrOfFailedTestCases += ReportTestResult(VerifyZeroEncoding< lns<25, 13, std::uint8_t > >(reportTestCases), "lns<25,13,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyZeroEncoding< lns<49, 25, std::uint16_t> >(reportTestCases), "lns<49,25,uint16_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyZeroEncoding< lns<97, 60, std::uint32_t> >(reportTestCases), "lns<97,60,uint32_t>", test_tag);
//	nrOfFailedTestCases += ReportTestResult(VerifyZeroEncoding< lns<65, 32, std::uint64_t> >(reportTestCases), "lns<65,32,uint64_t>", test_tag);

	return nrOfFailedTestCases;
}

template<typename TestType>
int VerifyNaNEncoding(bool reportTestCases) {
	int nrOfFailedTestCases = 0;

	// single block configurations
	TestType a, b;
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
	nrOfFailedTestCases += ReportTestResult(VerifyNaNEncoding< lns< 8, 4, std::uint8_t > >(reportTestCases), "lns< 8, 4,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNaNEncoding< lns<16, 8, std::uint16_t> >(reportTestCases), "lns<16, 8,uint16_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNaNEncoding< lns<32, 16, std::uint32_t> >(reportTestCases), "lns<32,16,uint32_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNaNEncoding< lns<64, 32, std::uint64_t> >(reportTestCases), "lns<64,32,uint64_t>", test_tag);

	// double block configurations with all special bits in the MSU
	nrOfFailedTestCases += ReportTestResult(VerifyNaNEncoding< lns<10, 4, std::uint8_t > >(reportTestCases), "lns<10, 4,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNaNEncoding< lns<18, 8, std::uint16_t> >(reportTestCases), "lns<18, 8,uint16_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNaNEncoding< lns<34, 16, std::uint32_t> >(reportTestCases), "lns<34,16,uint32_t>", test_tag);
	//	nrOfFailedTestCases += ReportTestResult(VerifyZeroEncoding< lns<66, 32, std::uint64_t> >(reportTestCases), "lns<66,32,uint64_t>", test_tag);

		// double block configurations with special bits split between MSU and MSU - 1
	nrOfFailedTestCases += ReportTestResult(VerifyNaNEncoding< lns< 9, 4, std::uint8_t > >(reportTestCases), "lns< 9, 4,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNaNEncoding< lns<17, 8, std::uint16_t> >(reportTestCases), "lns<17, 8,uint16_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNaNEncoding< lns<33, 16, std::uint32_t> >(reportTestCases), "lns<33,16,uint32_t>", test_tag);
	//	nrOfFailedTestCases += ReportTestResult(VerifyZeroEncoding< lns<65, 32, std::uint64_t> (reportTestCases), "lns<65,32,uint64_t>", test_tag);

		// triple block configurations with all special bits in the MSU
	nrOfFailedTestCases += ReportTestResult(VerifyNaNEncoding< lns<26,13, std::uint8_t > >(reportTestCases), "lns<26,13,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNaNEncoding< lns<50,25, std::uint16_t> >(reportTestCases), "lns<50,25,uint16_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNaNEncoding< lns<98,60, std::uint32_t> >(reportTestCases), "lns<98,60,uint32_t>", test_tag);
	//	nrOfFailedTestCases += ReportTestResult(VerifyZeroEncoding< lns<66, 32, std::uint64_t> >(reportTestCases), "lns<66,32,uint64_t>", test_tag);

		// triple block configurations with special bits split between MSU and MSU - 1
	nrOfFailedTestCases += ReportTestResult(VerifyNaNEncoding< lns<25,13, std::uint8_t > >(reportTestCases), "lns<25,13,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNaNEncoding< lns<49,25, std::uint16_t> >(reportTestCases), "lns<49,25,uint16_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNaNEncoding< lns<97,60, std::uint32_t> >(reportTestCases), "lns<97,60,uint32_t>", test_tag);
	//	nrOfFailedTestCases += ReportTestResult(VerifyZeroEncoding< lns<65, 32, std::uint64_t> >(reportTestCases), "lns<65,32,uint64_t>", test_tag);

	return nrOfFailedTestCases;
}

} }  // namespace sw::universal

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

	ReportTestSuiteHeader(test_suite, reportTestCases);

	using SaturatingLns = lns<7, 4>;
	SaturatingLns a;
	std::string typetag = type_tag(a);

#if MANUAL_TESTING

	{
		float fa, fb;
		fa = 1;
		fb = std::numeric_limits<float>::quiet_NaN();
		std::cout << (fa < fb ? "a < b" : "a !< b") << '\n';
		std::cout << (fa > fb ? "a > b" : "a !> b") << '\n';
		std::cout << (fa == fb ? "a == b" : "a != b") << '\n';
		std::cout << (fa != fb ? "a != b" : "a == b") << '\n';
	}

	nrOfFailedTestCases += VerifyZeroEncodings(reportTestCases);
	nrOfFailedTestCases += VerifyNaNEncodings(reportTestCases);

	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual< SaturatingLns >(reportTestCases), typetag, "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan< SaturatingLns >(reportTestCases), typetag, "<");

	// these are derived from equal and less then
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual< SaturatingLns >(reportTestCases), typetag, "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan< SaturatingLns >(reportTestCases), typetag, ">");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan< SaturatingLns >(reportTestCases), typetag, "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan< SaturatingLns >(reportTestCases), typetag, ">=");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += VerifyZeroEncodings(reportTestCases);
	nrOfFailedTestCases += VerifyNaNEncodings(reportTestCases);

	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual< SaturatingLns >(reportTestCases), typetag, "==");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual< SaturatingLns >(reportTestCases), typetag, "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan< SaturatingLns >(reportTestCases), typetag, "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan< SaturatingLns >(reportTestCases), typetag, ">");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan< SaturatingLns >(reportTestCases), typetag, "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan< SaturatingLns >(reportTestCases), typetag, ">=");

	a = 0;

	if (!(a == 0)) {
		nrOfFailedTestCases += ReportTestResult(1, typetag + std::string(" == 0"), "== int literal");
	}
	else {
		ReportTestResult(0, typetag + std::string(" == 0"), " == int literal");
	}
	if (!(a == 0.0f)) {
		nrOfFailedTestCases += ReportTestResult(1, typetag + std::string(" == 0.0"), " == float literal");
	}
	else {
		ReportTestResult(0, typetag + std::string(" == 0.0"), " == float literal");
	}
	if (!(a == 0.0)) {
		nrOfFailedTestCases += ReportTestResult(1, typetag + std::string(" == 0.0"), " == double literal");
}
	else {
		ReportTestResult(0, typetag + std::string(" == 0.0"), " == double literal");
	}
#if LONG_DOUBLE_SUPPORT
	if (!(a == 0.0l)) {
		nrOfFailedTestCases += ReportTestResult(1, typetag + std::string(" == 0.0"), " == long double literal");
	}
	else {
		ReportTestResult(0, typetag + std::string(" == 0.0"), " == long double literal");
	}
#endif
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
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
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
