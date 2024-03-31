// ieee754_subnormals.cpp: test suite runner for conversion tests of ieee754 subnormals to classic cfloats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_status.hpp>
#include <universal/verification/cfloat_test_suite.hpp>

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

	std::string test_suite  = "cfloat<> ieee754 subnormal assignment";
	std::string test_tag    = "assignment";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// generate individual testcases to hand trace/debug
	constexpr bool hasSubnormals   = true;
	constexpr bool hasSupernormals = true;
	constexpr bool isSaturating    = false;

	// single precision
	{
		constexpr size_t nbits = 28;
		constexpr size_t es = 8;
		using bt = uint32_t;
		using Cfloat = cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>;
		constexpr size_t fbits = Cfloat::fbits;
		Cfloat a{};
		++a;
		for (int i = 0; i < static_cast<int>(fbits); ++i) {
			float f = float(a);
			Cfloat b = f;
			std::cout << to_binary(f) << " : " << color_print(f) << " : " << f << '\n';
			std::cout << to_binary(a) << " : " << color_print(a) << " : " << a << '\n';
			std::cout << to_binary(b) << " : " << color_print(b) << " : " << b << '\n';
			// when we have mul
			// a *= 2.0f;
			uint64_t fraction = a.fraction_ull();
			fraction <<= 1;
			a.setfraction(fraction);
		}
	}

	// double precision
	{
		constexpr size_t nbits = 56;
		constexpr size_t es = 11;
		using bt = uint32_t;
		using Cfloat = cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>;
		constexpr size_t fbits = Cfloat::fbits;
		Cfloat a{};
		++a;
		for (int i = 0; i < static_cast<int>(fbits); ++i) {
			double f = double(a);
			Cfloat b = f;
			std::cout << to_binary(f) << " : " << color_print(f) << " : " << f << '\n';
			std::cout << to_binary(a) << " : " << color_print(a) << " : " << a << '\n';
			std::cout << to_binary(b) << " : " << color_print(b) << " : " << b << '\n';
			// when we have mul
			// a *= 2.0f;
			uint64_t fraction = a.fraction_ull();
			fraction <<= 1;
			a.setfraction(fraction);
}
	}
	{
		// convert a normal number
		constexpr size_t nbits = 28;
		constexpr size_t es = 8;
		using bt = uint32_t;
		cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> a{ 0 }, b;
		a = 1.0e25f;
		std::cout << to_binary(a) << " : " << color_print(a) << " : " << a << '\n';
	}

	nrOfFailedTestCases = 0;

#else
	std::cout << "subnormal validation\n";

	bool bReportIndividualTestCases = false;
	std::string tag = "IEEE-754      single precision subnormal conversion: ";

	nrOfFailedTestCases += ReportTestResult(VerifyIeee754FloatSubnormals<uint8_t >(bReportIndividualTestCases), tag, "cfloat<32,  8, uint8_t ,1,1,0>");
	nrOfFailedTestCases += ReportTestResult(VerifyIeee754FloatSubnormals<uint16_t>(bReportIndividualTestCases), tag, "cfloat<32,  8, uint16_t,1,1,0>");
	nrOfFailedTestCases += ReportTestResult(VerifyIeee754FloatSubnormals<uint32_t>(bReportIndividualTestCases), tag, "cfloat<32,  8, uint32_t,1,1,0>");
	nrOfFailedTestCases += ReportTestResult(VerifyIeee754FloatSubnormals<uint64_t>(bReportIndividualTestCases), tag, "cfloat<32,  8, uint64_t,1,1,0>");

	tag = "IEEE-754      double precision subnormal conversion: ";

	nrOfFailedTestCases += ReportTestResult(VerifyIeee754DoubleSubnormals<uint8_t >(bReportIndividualTestCases), tag, "cfloat<64, 11, uint8_t ,1,1,0>");
	nrOfFailedTestCases += ReportTestResult(VerifyIeee754DoubleSubnormals<uint16_t>(bReportIndividualTestCases), tag, "cfloat<64, 11, uint16_t,1,1,0>");
	nrOfFailedTestCases += ReportTestResult(VerifyIeee754DoubleSubnormals<uint32_t>(bReportIndividualTestCases), tag, "cfloat<64, 11, uint32_t,1,1,0>");
	nrOfFailedTestCases += ReportTestResult(VerifyIeee754DoubleSubnormals<uint64_t>(bReportIndividualTestCases), tag, "cfloat<64, 11, uint64_t,1,1,0>");

#if LONG_DOUBLE_SUPPORT

	// long double support tends to be just extended precision support (that implies afbits = 64)
	constexpr size_t fbits = 64;
	if constexpr (ieee754_parameter<long double>::fbits == fbits) {
		tag = "IEEE-754 long double precision subnormal conversion: ";

		nrOfFailedTestCases += ReportTestResult(VerifyIeee754LongDoubleSubnormals<uint8_t >(bReportIndividualTestCases), tag, "cfloat<80, 15, uint8_t ,1,1,0>");
		nrOfFailedTestCases += ReportTestResult(VerifyIeee754LongDoubleSubnormals<uint16_t>(bReportIndividualTestCases), tag, "cfloat<80, 15, uint16_t,1,1,0>");
		nrOfFailedTestCases += ReportTestResult(VerifyIeee754LongDoubleSubnormals<uint32_t>(bReportIndividualTestCases), tag, "cfloat<80, 15, uint32_t,1,1,0>");
		nrOfFailedTestCases += ReportTestResult(VerifyIeee754LongDoubleSubnormals<uint64_t>(bReportIndividualTestCases), tag, "cfloat<80, 15, uint64_t,1,1,0>");
	}
	else {
		std::cerr << "long double for this compiler environment is not extended precision\n";
	}

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
