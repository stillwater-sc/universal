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
	constexpr bool isSaturating    = false;
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

	// testing cfloat without subnormals, max-exponent values, or saturation
	constexpr bool hasSubnormals   = false;
	constexpr bool hasMaxExpValues = false;
	constexpr bool isSaturating    = false;

	std::string test_suite         = "blocktriple to nonsaturating cfloat conversion validation";
	std::string test_tag           = "conversion blocktriple -> nonsaturating cfloat";
	bool reportTestCases           = false;
	int nrOfFailedTestCases        = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// to track conversion in more detail
	std::cout << std::setprecision(8);
	std::cerr << std::setprecision(8);

	{
		constexpr size_t nbits = 30;		// nbits = 28 is the last size that fits in the fast path
		constexpr size_t es = 8;
		constexpr size_t fbits = nbits - es - 1ull;
		using BlockType = uint8_t;
		using Cfloat = cfloat<nbits, es, BlockType, hasSubnormals, hasMaxExpValues, isSaturating>;
		Cfloat a; // uninitialized
		a.assign(std::string("0b0.0111'1111.0'0000'0000'0000'0000'0001"));
		std::cout << "a =        eps : " << to_binary(a) << " : " << a << '\n';
	}

	{
		constexpr size_t nbits = 32;		// nbits = 28 is the last size that fits in the fast path
		constexpr size_t es = 8;
		constexpr size_t fbits = nbits - es - 1ull;
		using BlockType = uint8_t;
		using Cfloat = cfloat<nbits, es, BlockType, hasSubnormals, hasMaxExpValues, isSaturating>;
		constexpr BlockTripleOperator op = BlockTripleOperator::ADD;
		using Btriple = blocktriple<fbits, op, BlockType>;

		Cfloat a; // uninitialized
		a = std::numeric_limits<Cfloat>::epsilon();

		std::cout << '\n';
		a += -1.0f;
		std::cout << "a = -1.0: " << to_binary(a) << " : " << a << '\n';
		a = 0;
		std::cout << "a =  0.0: " << to_binary(a) << " : " << a << '\n';
		a += 1.0f;
		std::cout << "a = +1.0: " << to_binary(a) << " : " << a << '\n';

		/// btriple manipulation to support a cfloat<32,8>
		std::string input("0b11'0000'1111'0000'1111'1111'0000'1111'0000'1111'0000'1111'0000'1111'0000'1111'0000'1111'0000");
		Btriple b;
		b.assign(input);
		std::cout << to_binary(b) << " : " << input << '\n';
		b.constexprClassParameters();
	}

	{
		constexpr size_t nbits = 8;
		constexpr size_t es = 2;
		constexpr size_t fbits = nbits - es - 1ull;
		using BlockType = uint8_t;
		using Cfloat = cfloat<nbits, es, BlockType, hasSubnormals, hasMaxExpValues, isSaturating>;

		int fails = 0;
		size_t nrTests = 10;
		Cfloat minpos(SpecificValue::minpos);
		fails += ReportTestResult(VerifyUnaryOperatorThroughRandoms< Cfloat >(true, RandomsOp::OPCODE_ASSIGN, nrTests, double(minpos)), "random assignment test", "assignment      ");
	}


	// how do you round a non-normalized blocktriple, i.e. >= 2.0?
	// you would need to modify the lsb/guard/round/sticky bit masks
	// so that you use all info to make the rounding decision,
	// then normalize and apply the rounding decision.
	{
		using Cfloat = cfloat<4, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>;
		Cfloat a; // uninitialized
		a.constexprClassParameters();
		std::cout << dynamic_range(a) << '\n';
		std::cout << "maxpos : " << a.maxpos() << '\n';
		a.setinf(false); // +inf
		std::cout << "+inf   : " << a << '\n';
		a.setinf(true); // -inf
		std::cout << "-inf   : " << a << '\n';
		// FAIL : (+, 0, 0b011.1) : 3.5 -> 0b0.11.1 != ref 0b0.11.0 or nan != nan
		GenerateConversionTest<Cfloat, BlockTripleOperator::ADD>(1, 0x70ull);
		nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion<Cfloat, BlockTripleOperator::ADD>(true), test_tag, "cfloat<4,2,uint8_t, fff> from blocktriple ADD");
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

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion<cfloat<4, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD>(reportTestCases), test_tag, "cfloat<4,2,uint8_t, fff> from blocktriple ADD");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion<cfloat<5, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD>(reportTestCases), test_tag, "cfloat<5,2,uint8_t, fff> from blocktriple ADD");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion<cfloat<6, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD>(reportTestCases), test_tag, "cfloat<6,2,uint8_t, fff> from blocktriple ADD");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion<cfloat<7, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD>(reportTestCases), test_tag, "cfloat<7,2,uint8_t, fff> from blocktriple ADD");


	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion<cfloat<8, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD>(reportTestCases), test_tag, "cfloat<8,2,uint8_t, fff> from blocktriple ADD");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion<cfloat<8, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD>(reportTestCases), test_tag, "cfloat<8,3,uint8_t, fff> from blocktriple ADD");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion<cfloat<8, 4, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD>(reportTestCases), test_tag, "cfloat<8,4,uint8_t, fff> from blocktriple ADD");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion<cfloat<8, 5, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD>(reportTestCases), test_tag, "cfloat<8,5,uint8_t, fff> from blocktriple ADD");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion<cfloat<8, 6, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD>(reportTestCases), test_tag, "cfloat<8,6,uint8_t, fff> from blocktriple ADD");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else  // !MANUAL_TESTING

#if REGRESSION_LEVEL_1

	// es = 1 is invalid for this cfloat configuration


	// es = 2
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 4, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 4,2, uint8_t, fff>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 5, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 5,2, uint8_t, fff>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 6, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 6,2, uint8_t, fff>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 7, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 7,2, uint8_t, fff>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 8, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 8,2, uint8_t, fff>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<10, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<10,2, uint8_t, fff>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<12, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<12,2, uint8_t, fff>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<14, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<14,2, uint8_t, fff>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<16, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<16,2, uint8_t, fff>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<18, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<18,2, uint8_t, fff>");   // 3 blocks


	// es = 3
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 5, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 5,3, uint8_t, fff>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 6, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 6,3, uint8_t, fff>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 7, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 7,3, uint8_t, fff>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 8, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 8,3, uint8_t, fff>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<10, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<10,3, uint8_t, fff>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<12, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<12,3, uint8_t, fff>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<14, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<14,3, uint8_t, fff>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<18, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<18,3, uint8_t, fff>");   // 3 blocks


	// es = 4
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 6, 4, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 6,4, uint8_t, fff>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 7, 4, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 7,4, uint8_t, fff>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 8, 4, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 8,4, uint8_t, fff>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<10, 4, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<10,4, uint8_t, fff>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<12, 4, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<12,4, uint8_t, fff>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<14, 4, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<14,4, uint8_t, fff>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<18, 4, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<18,4, uint8_t, fff>");   // 3 blocks


	// es = 5
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 7, 5, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 7,5, uint8_t, fff>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 8, 5, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 8,5, uint8_t, fff>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<10, 5, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<10,5, uint8_t, fff>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<12, 5, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<12,5, uint8_t, fff>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<14, 5, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<14,5, uint8_t, fff>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<18, 5, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<18,5, uint8_t, fff>");   // 3 blocks


	// es = 6
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 8, 6, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 8,6, uint8_t, fff>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 9, 6, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 9,6, uint8_t, fff>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<10, 6, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<10,6, uint8_t, fff>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<12, 6, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<12,6, uint8_t, fff>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<14, 6, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<14,6, uint8_t, fff>");


	// es = 7
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 9, 7, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 9,7, uint8_t, fff>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<10, 7, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<10,7, uint8_t, fff>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<12, 7, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<12,7, uint8_t, fff>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<14, 7, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<14,7, uint8_t, fff>");

	// still failing
	// es = 8
//	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<11, 8, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<11,8, uint8_t, fff>");
//	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<12, 8, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<12,8, uint8_t, fff>");
//	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<14, 8, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<14,8, uint8_t, fff>");
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
