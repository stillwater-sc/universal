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
 */

template<size_t nbits, size_t es>
void Test() 
{
	using BlockType = uint8_t;
	constexpr bool hasSubnormals   = false;
	constexpr bool hasMaxExpValues = false;
	constexpr bool isSaturating    = true;
	using Cfloat = sw::universal::cfloat<nbits, es, BlockType, hasSubnormals, hasMaxExpValues, isSaturating>;
//	constexpr BlockTripleOperator op = BlockTripleOperator::ADD;
// 	constexpr size_t fbits = nbits - es - 1ull;
//	using Btriple = blocktriple<fbits, op, BlockType>;

	Cfloat a; // uninitialized
	std::cout << "\n-----------------\n" << sw::universal::type_tag(a) << '\n';

	Cfloat eps = std::numeric_limits<Cfloat>::epsilon();
	a = -1.5f - eps;
	std::cout << "a = -1.5 - eps : " << to_binary(a) << " : " << a << '\n';
	a = -eps;
	std::cout << "a =  0.0 - eps : " << to_binary(a) << " : " << a << '\n';
	a = 0;
	std::cout << "a =  0.0       : " << to_binary(a) << " : " << a << '\n';
	a = 0.0f + eps;
	std::cout << "a =  0.0   eps : " << to_binary(a) << " : " << a << '\n';
	a = 1.5f + eps;
	std::cout << "a = +1.5 + eps : " << to_binary(a) << " : " << a << '\n';
	std::cout << '\n';
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
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;

	// testing cfloat without subnormals, max-exponent values, or saturation
	constexpr bool hasSubnormals   = false;
	constexpr bool hasMaxExpValues = false;
	constexpr bool isSaturating    = true;

	std::string test_suite         = "blocktriple to saturating cfloat conversion validation";
	std::string test_tag           = "conversion blocktriple -> saturating cfloat";
	bool reportTestCases           = false;
	int nrOfFailedTestCases        = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// to track conversion in more detail
	std::cout << std::setprecision(8);

	{
		constexpr size_t nbits = 8;
		constexpr size_t es = 5;
		using BlockType = uint8_t;
		using CfloatNonsat = cfloat<nbits, es, BlockType, hasSubnormals, hasMaxExpValues, false>;
		using CfloatSat = cfloat<nbits, es, BlockType, hasSubnormals, hasMaxExpValues, isSaturating>;
		CfloatNonsat a_nonsat, inf_nonsat;
		CfloatSat a_sat, b_sat, c_sat;

		a_nonsat.maxpos();
		a_sat.maxpos();
		ReportValue(a_nonsat, "nonsaturating cfloat maxpos");
		ReportValue(a_sat, "   saturating cfloat maxpos");
		inf_nonsat.setinf();
		ReportValue(inf_nonsat, "nonsaturating cfloat inf");

		std::cout << symmetry_range(a_nonsat) << '\n';
		std::cout << symmetry_range(a_sat) << '\n';

		b_sat = 0.5;
		c_sat = a_sat + b_sat;
		ReportValue(c_sat, "   saturating cfloat maxpos + 0.5");
		b_sat.maxpos();
		c_sat = a_sat + b_sat;
		ReportValue(c_sat, "   saturating cfloat 2*maxpos");
		a_sat.maxneg();
		b_sat = 0.5;
		c_sat = a_sat - b_sat;
		ReportValue(c_sat, "   saturating cfloat maxneg - 0.5");
		b_sat.maxpos();
		c_sat = a_sat - b_sat;
		ReportValue(c_sat, "   saturating cfloat 2*maxneg");
	}

	{
		constexpr size_t nbits = 8;
		constexpr size_t es = 2;
		using BlockType = uint8_t;
		using Cfloat = cfloat<nbits, es, BlockType, hasSubnormals, hasMaxExpValues, isSaturating>;

		size_t nrTests = 10;
		ReportTestResult(VerifyUnaryOperatorThroughRandoms< Cfloat >(true, RandomsOp::OPCODE_ASSIGN, nrTests), "random assignment test", "assignment      ");
	}


	// how do you round a non-normalized blocktriple, i.e. >= 2.0?
	// you would need to modify the lsb/guard/round/sticky bit masks
	// so that you use all info to make the rounding decision,
	// then normalize and apply the rounding decision.
	{
		using Cfloat = cfloat<4, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>;
		Cfloat a{ 0 }; // uninitialized works, but generates a compiler warning
		a.constexprClassParameters();
		std::cout << dynamic_range(a) << '\n';
		std::cout << "maxpos : " << a.maxpos() << '\n';
		a.setinf(false); // +inf
		std::cout << "+inf   : " << a << '\n';
		a.setinf(true); // -inf
		std::cout << "-inf   : " << a << '\n';
		// FAIL : (+, 0, 0b011.1) : 3.5 -> 0b0.11.1 != ref 0b0.11.0 or nan != nan
		GenerateConversionTest<Cfloat, BlockTripleOperator::ADD>(1, 0x70ull);
		nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion<Cfloat, BlockTripleOperator::ADD>(true), test_tag, "cfloat<4,2, uint8_t, fft> from blocktriple ADD");
		/*
		blocktriple<  1, ADD, unsigned char>  radix point at 4, smallest scale = 0, largest scale = 1
			FAIL: (+, 0, 0b011.1000) : 3.5 -> 0b0.11.0 != ref 0b0.10.1 or inf != 3
			FAIL : (+, 1, 0b010.0000) : 4 -> 0b0.11.0 != ref 0b0.10.1 or inf != 3
			FAIL : (+, 1, 0b010.1000) : 5 -> 0b0.11.0 != ref 0b0.10.1 or inf != 3
			FAIL : (+, 1, 0b011.1000) : 7 -> 0b0.11.0 != ref 0b0.10.1 or inf != 3
			FAIL : (-, 0, 0b011.1000) : -3.5 -> 0b1.11.0 != ref 0b1.10.1 or -inf != -3
			FAIL : (-, 1, 0b010.0000) : -4 -> 0b1.11.0 != ref 0b1.10.1 or -inf != -3
			FAIL : (-, 1, 0b010.1000) : -5 -> 0b1.11.0 != ref 0b1.10.1 or -inf != -3
			FAIL : (-, 1, 0b011.1000) : -7 -> 0b1.11.0 != ref 0b1.10.1 or -inf != -3
		*/
	}

	{
		// checking the other side of the exponential adjustments with cfloats
		// that expand on the dynamic range of IEEE-754
		using Cfloat = cfloat<80, 15, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>;
		Cfloat a; // uninitialized
		a = -1.0f;
		std::cout << type_tag(a) << '\n' << to_binary(a) << " : " << a << '\n';
		//			a.constexprClassParameters();
	}

	// es = 1 is invalid as a configuration when you do not have subnormals or max-exponent values as ALL values will be subnormals or max-exponent values
	// how do you deal with this?

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion<cfloat<4, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD>(reportTestCases), test_tag, "cfloat<4,2, uint8_t, fft> from blocktriple ADD");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion<cfloat<5, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD>(reportTestCases), test_tag, "cfloat<5,2, uint8_t, fft> from blocktriple ADD");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion<cfloat<6, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD>(reportTestCases), test_tag, "cfloat<6,2, uint8_t, fft> from blocktriple ADD");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion<cfloat<7, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD>(reportTestCases), test_tag, "cfloat<7,2, uint8_t, fft> from blocktriple ADD");


	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion<cfloat<8, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD>(reportTestCases), test_tag, "cfloat<8,2, uint8_t, fft> from blocktriple ADD");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion<cfloat<8, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD>(reportTestCases), test_tag, "cfloat<8,3, uint8_t, fft> from blocktriple ADD");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion<cfloat<8, 4, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD>(reportTestCases), test_tag, "cfloat<8,4, uint8_t, fft> from blocktriple ADD");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion<cfloat<8, 5, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD>(reportTestCases), test_tag, "cfloat<8,5, uint8_t, fft> from blocktriple ADD");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion<cfloat<8, 6, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD>(reportTestCases), test_tag, "cfloat<8,6, uint8_t, fft> from blocktriple ADD");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else  // !MANUAL_TESTING

#if REGRESSION_LEVEL_1

	// es = 1 is invalid for this cfloat configuration: need at least 2 exponent bits for a normal region to exist

	// es = 2
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 4, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 4,2, uint8_t, fft>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 5, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 5,2, uint8_t, fft>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 6, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 6,2, uint8_t, fft>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 7, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 7,2, uint8_t, fft>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 8, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 8,2, uint8_t, fft>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<10, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<10,2, uint8_t, fft>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<12, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<12,2, uint8_t, fft>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<14, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<14,2, uint8_t, fft>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<16, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<16,2, uint8_t, fft>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<18, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<18,2, uint8_t, fft>");   // 3 blocks


	// es = 3
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 5, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 5,3, uint8_t, fft>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 6, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 6,3, uint8_t, fft>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 7, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 7,3, uint8_t, fft>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 8, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 8,3, uint8_t, fft>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<10, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<10,3, uint8_t, fft>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<12, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<12,3, uint8_t, fft>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<14, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<14,3, uint8_t, fft>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<18, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<18,3, uint8_t, fft>");   // 3 blocks


	// es = 4
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 6, 4, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 6,4, uint8_t, fft>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 7, 4, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 7,4, uint8_t, fft>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 8, 4, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 8,4, uint8_t, fft>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<10, 4, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<10,4, uint8_t, fft>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<12, 4, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<12,4, uint8_t, fft>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<14, 4, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<14,4, uint8_t, fft>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<18, 4, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<18,4, uint8_t, fft>");   // 3 blocks


	// es = 5
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 7, 5, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 7,5, uint8_t, fft>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 8, 5, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 8,5, uint8_t, fft>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<10, 5, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<10,5, uint8_t, fft>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<12, 5, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<12,5, uint8_t, fft>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<14, 5, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<14,5, uint8_t, fft>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<18, 5, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<18,5, uint8_t, fft>");   // 3 blocks


	// es = 6
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 8, 6, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 8,6, uint8_t, fft>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 9, 6, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 9,6, uint8_t, fft>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<10, 6, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<10,6, uint8_t, fft>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<12, 6, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<12,6, uint8_t, fft>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<14, 6, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<14,6, uint8_t, fft>");


	// es = 7
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 9, 7, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 9,7, uint8_t, fft>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<10, 7, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<10,7, uint8_t, fft>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<12, 7, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<12,7, uint8_t, fft>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<14, 7, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<14,7, uint8_t, fft>");

	// still failing
	// es = 8
//	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<11, 8, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<11,8, uint8_t, fft>");
//	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<12, 8, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<12,8, uint8_t, fft>");
//	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<14, 8, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<14,8, uint8_t, fft>");
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
