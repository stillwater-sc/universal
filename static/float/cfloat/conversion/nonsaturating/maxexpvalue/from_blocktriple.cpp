// from_blocktriple.cpp: test suite runner for conversion tests between blocktriple and cfloats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// minimum set of include files to reflect source code dependencies
// Configure the cfloat template environment
// first: enable general or specialized configurations
#define CFLOAT_FAST_SPECIALIZATION
// second: enable/disable arithmetic exceptions
#define CFLOAT_THROW_ARITHMETIC_EXCEPTION 0
// third: enable trace conversion
#define TRACE_CONVERSION 0
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/test_suite_randoms.hpp>
#include <universal/verification/cfloat_test_suite.hpp>
#include <universal/number/cfloat/table.hpp>

/*
How do you test the conversion state space of blocktriple to cfloat.
We need to convert the blocktriple that comes out of an ADD, a MUL, and a DIV operation.
The blocktriples have bits that need to be rounded by convert.
How do you test that rounding?

Convert the blocktriple to a value.
Use the cfloat operator=() to round. That is your reference. This assumes that cfloat operator=() has been validated.
Use convert() to convert to a cfloat.
Compare the operator=() and convert() cfloat patterns to check correctness

Regression test for the fix of the wide-significand convert() path (PR #608):
When rounding overflows into the max-exponent binade, nonsaturating hasMaxExpValues
cfloats must map to +/-inf rather than incorrectly using the isnan() remap path.
With hasMaxExpValues=true, a max-exponent encoding with zero fraction is a valid
finite value, not NaN, so the isnan() check did not catch this overflow case.
 */

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

	// testing cfloat with max-exponent values, but without subnormals or saturation
	constexpr bool hasSubnormals   = false;
	constexpr bool hasMaxExpValues = true;
	constexpr bool isSaturating    = false;

	std::string test_suite         = "blocktriple to nonsaturating cfloat with max-exp values conversion validation";
	std::string test_tag           = "conversion blocktriple -> nonsaturating cfloat ftf";
	bool reportTestCases           = false;
	int nrOfFailedTestCases        = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// to track conversion in more detail
	std::cout << std::setprecision(8);

	{
		// demonstrate that rounding overflow with hasMaxExpValues maps to +/-inf
		// rather than mapping through isnan() (the bug fixed in PR #608)
		using Cfloat = cfloat<8, 5, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>;
		Cfloat a, inf;

		a.maxpos();
		ReportValue(a, "nonsaturating cfloat maxpos");
		inf.setinf(false);
		ReportValue(inf, "nonsaturating cfloat +inf");
		inf.setinf(true);
		ReportValue(inf, "nonsaturating cfloat -inf");
	}

	{
		using Cfloat = cfloat<4, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>;
		Cfloat a{ 0 };
		a.constexprClassParameters();
		std::cout << dynamic_range(a) << '\n';
		GenerateConversionTest<Cfloat, BlockTripleOperator::ADD>(1, 0x70ull);
		nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion<Cfloat, BlockTripleOperator::ADD>(true), test_tag, "cfloat<4,2, uint8_t, ftf> from blocktriple ADD");
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else  // !MANUAL_TESTING

#if REGRESSION_LEVEL_1

	// es = 1 is invalid for this cfloat configuration: need at least 2 exponent bits for a normal region to exist

	// es = 2
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 4, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 4,2, uint8_t, ftf>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 5, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 5,2, uint8_t, ftf>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 6, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 6,2, uint8_t, ftf>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 7, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 7,2, uint8_t, ftf>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 8, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 8,2, uint8_t, ftf>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<10, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<10,2, uint8_t, ftf>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<12, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<12,2, uint8_t, ftf>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<14, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<14,2, uint8_t, ftf>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<16, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<16,2, uint8_t, ftf>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<18, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<18,2, uint8_t, ftf>");   // 3 blocks


	// es = 3
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 5, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 5,3, uint8_t, ftf>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 6, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 6,3, uint8_t, ftf>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 7, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 7,3, uint8_t, ftf>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 8, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 8,3, uint8_t, ftf>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<10, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<10,3, uint8_t, ftf>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<12, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<12,3, uint8_t, ftf>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<14, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<14,3, uint8_t, ftf>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<18, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<18,3, uint8_t, ftf>");   // 3 blocks


	// es = 4
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 6, 4, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 6,4, uint8_t, ftf>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 7, 4, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 7,4, uint8_t, ftf>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 8, 4, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 8,4, uint8_t, ftf>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<10, 4, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<10,4, uint8_t, ftf>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<12, 4, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<12,4, uint8_t, ftf>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<14, 4, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<14,4, uint8_t, ftf>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<18, 4, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<18,4, uint8_t, ftf>");   // 3 blocks


	// es = 5
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 7, 5, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 7,5, uint8_t, ftf>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 8, 5, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 8,5, uint8_t, ftf>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<10, 5, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<10,5, uint8_t, ftf>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<12, 5, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<12,5, uint8_t, ftf>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<14, 5, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<14,5, uint8_t, ftf>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<18, 5, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<18,5, uint8_t, ftf>");   // 3 blocks


	// es = 6
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 8, 6, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 8,6, uint8_t, ftf>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 9, 6, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 9,6, uint8_t, ftf>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<10, 6, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<10,6, uint8_t, ftf>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<12, 6, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<12,6, uint8_t, ftf>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<14, 6, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<14,6, uint8_t, ftf>");


	// es = 7
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 9, 7, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 9,7, uint8_t, ftf>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<10, 7, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<10,7, uint8_t, ftf>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<12, 7, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<12,7, uint8_t, ftf>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<14, 7, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<14,7, uint8_t, ftf>");

	// es = 8
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<11, 8, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<11,8, uint8_t, ftf>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<12, 8, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<12,8, uint8_t, ftf>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<14, 8, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<14,8, uint8_t, ftf>");
#endif

#if REGRESSION_LEVEL_2


#endif

#if REGRESSION_LEVEL_3


#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<25, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<25,2, uint8_t, ftf>");   // 4 blocks

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
