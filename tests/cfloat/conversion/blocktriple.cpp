// normalization.cpp: test suite runner for conversion tests between classic cfloats and blocktriples
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
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

#include <universal/number/cfloat/cfloat_impl.hpp>
#include <universal/number/cfloat/manipulators.hpp>
#include <universal/number/cfloat/math_functions.hpp>
#include <universal/verification/test_suite_conversion.hpp>
#include <universal/verification/cfloat_test_suite.hpp>

/*
   DESIGN and IMPLEMENTATION HISTORY

   The first design using value<> had a fraction bit parameter to differentiate between
   different normalizations for addition, multiplication, and division.
   But the normalization is NOT a generic op, it is very specific for add, mul, div, or sqrt
   so having a fully parameterized interface would create a state space for bugs
   that could get triggered by incorrect calling of the normalize method. You couldn't
   write a proper unit test for that method as most of the state space would NOT be
   valid conversions.
   Given that context of the experience with value<> we decided to clamp down
   on this parameterization overkill and create explicit normalization conversions
   for add, mul, div, and sqrt. 
 */

namespace sw::universal {

	/// <summary>
	/// verify that normalization represents the same value
	/// </summary>
	/// <typeparam name="bt">block storage type of representation</typeparam>
	/// <param name="bReportIndividualTestCases">if true print individual test cases</param>
	/// <returns></returns>
	template<typename CfloatConfiguration>
	int VerifyCfloatToBlocktripleConversion(bool bReportIndividualTestCases) {
		using namespace sw::universal;
		constexpr size_t nbits = CfloatConfiguration::nbits;
		constexpr size_t es = CfloatConfiguration::es;
		using bt = typename CfloatConfiguration::BlockType;
		constexpr bool hasSubnormals = CfloatConfiguration::hasSubnormals;
		constexpr bool hasSupernormals = CfloatConfiguration::hasSupernormals;
		constexpr bool isSaturating = CfloatConfiguration::isSaturating;
		constexpr size_t fbits = CfloatConfiguration::fbits;
		constexpr size_t abits = CfloatConfiguration::abits;

		int nrOfTestFailures{ 0 };
		constexpr size_t NR_VALUES = (1u << nbits);
		cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> a;

		if (bReportIndividualTestCases) a.constexprClassParameters();

		{ // testing conversion of normalization for add and subtract
			blocktriple<fbits, BlockTripleOperator::ADD, bt> b;   // the size of the blocktriple is configured by the number of fraction bits of the source number system
			for (size_t i = 0; i < NR_VALUES; ++i) {
				a.setbits(i);
				a.normalizeAddition(b);
				if (double(a) != double(b)) {
					if (a.isnan() && b.isnan()) continue;
					if (a.isinf() && b.isinf()) continue;

					++nrOfTestFailures;
					if (bReportIndividualTestCases) std::cout << "FAIL: " << to_binary(a) << " : " << a << " != " << to_triple(b) << " : " << b << '\n';
				}
			}
		}
		return nrOfTestFailures;
	}


	template<typename CfloatConfiguration>
	int VerifyBlocktripleToCfloatConversion(bool bReportIndividualTestCases) {
		using namespace sw::universal;
		constexpr size_t nbits = CfloatConfiguration::nbits;
		constexpr size_t es = CfloatConfiguration::es;
		using bt = typename CfloatConfiguration::BlockType;
		constexpr bool hasSubnormals = CfloatConfiguration::hasSubnormals;
		constexpr bool hasSupernormals = CfloatConfiguration::hasSupernormals;
		constexpr bool isSaturating = CfloatConfiguration::isSaturating;
		constexpr size_t fbits = CfloatConfiguration::fbits;
//		constexpr size_t abits = CfloatConfiguration::abits;

		int nrOfTestFailures{ 0 };
		constexpr size_t NR_VALUES = (1ull << nbits);
		cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> a, nut;

		if (bReportIndividualTestCases) a.constexprClassParameters();

		{ // testing conversion of add and subtract outputs to cfloat
			//		blocktriple<abits+1, bt> b; // blocktriple that comes out of an addition or subtraction
			blocktriple<fbits, BlockTripleOperator::ADD, bt> b; // blocktriple type that comes out of an addition or subtraction operation

			for (size_t i = 0; i < NR_VALUES; ++i) {
				a.setbits(i);
				b = float(a); // we use a float as this verification test is only intended to be used for small cfloats and floats are easier to print and inspect
				convert(b, nut);
//				cout << "blocktriple: " << to_binary(b) << " : " << b << " vs " << to_binary(nut) << " : " << nut << '\n';
				if (a != nut) {
					if (a.isnan() && b.isnan()) continue;
					if (a.isinf() && b.isinf()) continue;

					++nrOfTestFailures;
					if (bReportIndividualTestCases) std::cout << "FAIL: " << to_binary(a) << " : " << a << " != " << to_binary(nut) << " blocktriple value marshalled: " << to_triple(b) << " : " << b << '\n';
				}
			}
		}


		return nrOfTestFailures;
	}

}

// conditional compile flags
#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	print_cmd_line(argc, argv);

	int nrOfFailedTestCases = 0;
	std::string tag = "cfloat <-> blocktriple conversion: ";

#if MANUAL_TESTING

	// cfloat<> is a linear floating-point

	// to track conversion in more detail
	std::cout << std::setprecision(8);
	std::cerr << std::setprecision(8);

#ifdef LATER
	{
		constexpr size_t nbits = 64;
		constexpr size_t es = 11;
		constexpr size_t fbits = nbits - 1ull - es;
		using bt = uint32_t;
		cfloat<nbits, es, bt> a;
		blocktriple<fbits, bt> b;
//		a = 0.015625f;
		a = 2.0f;
		a.normalize(b);
		a.constexprClassParameters();
		blockbinary<es, bt> exponent; a.exponent(exponent);
		blockbinary<fbits, bt> fraction; a.fraction(fraction);
		cout << "cfloat     : " << to_binary(a) << " : " << a << " : scale " << a.scale() << " : " << exponent << " : " << fraction << '\n';
		cout << "blocktriple: " << to_triple(b) << " : " << b << endl;
	}


	nrOfFailedTestCases += VerifyCfloatToBlocktripleConversion< cfloat< 3, 1, uint8_t> >(false);
	nrOfFailedTestCases += VerifyCfloatToBlocktripleConversion< cfloat< 4, 2, uint8_t> >(false);
	nrOfFailedTestCases += VerifyCfloatToBlocktripleConversion< cfloat< 5, 3, uint8_t> >(false);
	nrOfFailedTestCases += VerifyCfloatToBlocktripleConversion< cfloat< 8, 4, uint8_t> >(false);

	nrOfFailedTestCases += VerifyCfloatToBlocktripleConversion< cfloat< 9, 1, uint8_t> >(true);
	nrOfFailedTestCases += VerifyCfloatToBlocktripleConversion< cfloat<10, 2, uint8_t> >(true);
	nrOfFailedTestCases += VerifyCfloatToBlocktripleConversion< cfloat<18, 5, uint8_t> >(true);
#endif

	nrOfFailedTestCases += VerifyBlocktripleToCfloatConversion < cfloat < 8, 2, uint8_t > > (true);

	std::cout << "failed tests: " << nrOfFailedTestCases << '\n';
	nrOfFailedTestCases = 0; // in manual testing we ignore failures for the regression system

#if STRESS_TESTING

	// manual exhaustive test

#endif

#else  // !MANUAL_TESTING
	bool bReportIndividualTestCases = false;
	std::cout << "cfloat to blocktriple conversion validation" << '\n';

	// es = 1
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 3, 1, uint8_t> >(bReportIndividualTestCases), tag, "cfloat< 3,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 4, 1, uint8_t> >(bReportIndividualTestCases), tag, "cfloat< 4,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 5, 1, uint8_t> >(bReportIndividualTestCases), tag, "cfloat< 5,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 6, 1, uint8_t> >(bReportIndividualTestCases), tag, "cfloat< 6,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 7, 1, uint8_t> >(bReportIndividualTestCases), tag, "cfloat< 7,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 8, 1, uint8_t> >(bReportIndividualTestCases), tag, "cfloat< 8,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 9, 1, uint8_t> >(bReportIndividualTestCases), tag, "cfloat< 9,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<10, 1, uint8_t> >(bReportIndividualTestCases), tag, "cfloat<10,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<12, 1, uint8_t> >(bReportIndividualTestCases), tag, "cfloat<12,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<16, 1, uint8_t> >(bReportIndividualTestCases), tag, "cfloat<16,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<18, 1, uint8_t> >(bReportIndividualTestCases), tag, "cfloat<18,1>");   // 3 blocks


	// es = 2
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 4, 2, uint8_t> >(bReportIndividualTestCases), tag, "cfloat< 4,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 5, 2, uint8_t> >(bReportIndividualTestCases), tag, "cfloat< 5,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 6, 2, uint8_t> >(bReportIndividualTestCases), tag, "cfloat< 6,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 7, 2, uint8_t> >(bReportIndividualTestCases), tag, "cfloat< 7,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 8, 2, uint8_t> >(bReportIndividualTestCases), tag, "cfloat< 8,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<10, 2, uint8_t> >(bReportIndividualTestCases), tag, "cfloat<10,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<12, 2, uint8_t> >(bReportIndividualTestCases), tag, "cfloat<12,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<14, 2, uint8_t> >(bReportIndividualTestCases), tag, "cfloat<14,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<16, 2, uint8_t> >(bReportIndividualTestCases), tag, "cfloat<16,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<18, 2, uint8_t> >(bReportIndividualTestCases), tag, "cfloat<18,2>");   // 3 blocks


	// es = 3
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 5, 3, uint8_t> >(bReportIndividualTestCases), tag, "cfloat< 5,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 6, 3, uint8_t> >(bReportIndividualTestCases), tag, "cfloat< 6,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 7, 3, uint8_t> >(bReportIndividualTestCases), tag, "cfloat< 7,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 8, 3, uint8_t> >(bReportIndividualTestCases), tag, "cfloat< 8,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<10, 3, uint8_t> >(bReportIndividualTestCases), tag, "cfloat<10,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<12, 3, uint8_t> >(bReportIndividualTestCases), tag, "cfloat<12,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<14, 3, uint8_t> >(bReportIndividualTestCases), tag, "cfloat<14,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<18, 3, uint8_t> >(bReportIndividualTestCases), tag, "cfloat<18,3>");   // 3 blocks


	// es = 4
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 6, 4, uint8_t> >(bReportIndividualTestCases), tag, "cfloat< 6,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 7, 4, uint8_t> >(bReportIndividualTestCases), tag, "cfloat< 7,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 8, 4, uint8_t> >(bReportIndividualTestCases), tag, "cfloat< 8,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<10, 4, uint8_t> >(bReportIndividualTestCases), tag, "cfloat<10,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<12, 4, uint8_t> >(bReportIndividualTestCases), tag, "cfloat<12,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<14, 4, uint8_t> >(bReportIndividualTestCases), tag, "cfloat<14,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<18, 4, uint8_t> >(bReportIndividualTestCases), tag, "cfloat<18,4>");   // 3 blocks


	// es = 5
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 7, 5, uint8_t> >(bReportIndividualTestCases), tag, "cfloat< 7,5>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 8, 5, uint8_t> >(bReportIndividualTestCases), tag, "cfloat< 8,5>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<10, 5, uint8_t> >(bReportIndividualTestCases), tag, "cfloat<10,5>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<12, 5, uint8_t> >(bReportIndividualTestCases), tag, "cfloat<12,5>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<14, 5, uint8_t> >(bReportIndividualTestCases), tag, "cfloat<14,5>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<18, 5, uint8_t> >(bReportIndividualTestCases), tag, "cfloat<18,5>");   // 3 blocks


	// es = 6
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 8, 6, uint8_t> >(bReportIndividualTestCases), tag, "cfloat< 8,6>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 9, 6, uint8_t> >(bReportIndividualTestCases), tag, "cfloat< 9,6>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<10, 6, uint8_t> >(bReportIndividualTestCases), tag, "cfloat<10,6>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<12, 6, uint8_t> >(bReportIndividualTestCases), tag, "cfloat<12,6>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<14, 6, uint8_t> >(bReportIndividualTestCases), tag, "cfloat<14,6>");


	// es = 7
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 9, 7, uint8_t> >(bReportIndividualTestCases), tag, "cfloat< 9,7>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<10, 7, uint8_t> >(bReportIndividualTestCases), tag, "cfloat<10,7>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<12, 7, uint8_t> >(bReportIndividualTestCases), tag, "cfloat<12,7>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<14, 7, uint8_t> >(bReportIndividualTestCases), tag, "cfloat<14,7>");

	// still failing
	// es = 8
//	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<11, 8, uint8_t> >(bReportIndividualTestCases), tag, "cfloat<11,8>");
//	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<12, 8, uint8_t> >(bReportIndividualTestCases), tag, "cfloat<12,8>");
//	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<14, 8, uint8_t> >(bReportIndividualTestCases), tag, "cfloat<14,8>");


#if STRESS_TESTING

	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<25, 2, uint8_t> >(bReportIndividualTestCases), tag, "cfloat<25,2>");   // 4 blocks

#endif  // STRESS_TESTING

#endif  // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::cfloat_arithmetic_exception& err) {
	std::cerr << "Uncaught cfloat arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::cfloat_internal_exception& err) {
	std::cerr << "Uncaught cfloat internal exception: " << err.what() << std::endl;
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


/*

  To generate:
  	GenerateFixedPointComparisonTable<4, 0>(std::string("-"));
	GenerateFixedPointComparisonTable<4, 1>(std::string("-"));
	GenerateFixedPointComparisonTable<4, 2>(std::string("-"));
	

 */
