// normalization.cpp: test suite runner for conversion tests between bfloats and blocktriples
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// minimum set of include files to reflect source code dependencies
// Configure the bfloat template environment
// first: enable general or specialized configurations
#define BFLOAT_FAST_SPECIALIZATION
// second: enable/disable arithmetic exceptions
#define BFLOAT_THROW_ARITHMETIC_EXCEPTION 0
// third: enable trace conversion
#define TRACE_CONVERSION 0

#include <universal/number/bfloat/bfloat.hpp>
#include <universal/number/bfloat/manipulators.hpp>
#include <universal/number/bfloat/math_functions.hpp>
#include <universal/verification/test_suite_conversion.hpp>
#include <universal/verification/bfloat_test_suite.hpp>

namespace sw::universal {

	/// <summary>
	/// verify that normalization represents the same value
	/// </summary>
	/// <typeparam name="bt">block storage type of representation</typeparam>
	/// <param name="bReportIndividualTestCases">if true print individual test cases</param>
	/// <returns></returns>
	template<typename BfloatConfiguration>
	int VerifyBfloatToBlocktripleConversion(bool bReportIndividualTestCases) {
		using namespace std;
		using namespace sw::universal;
		constexpr size_t nbits = BfloatConfiguration::nbits;
		constexpr size_t es = BfloatConfiguration::es;
		using bt = typename BfloatConfiguration::BlockType;
		constexpr size_t fbits = BfloatConfiguration::fbits;
		constexpr size_t abits = BfloatConfiguration::abits;

		int nrOfTestFailures{ 0 };
		constexpr size_t NR_VALUES = (1u << nbits);
		bfloat<nbits, es, bt> a;
		blocktriple<fbits, bt> b;   // the size of the blocktriple is configured by the number of fraction bits of the source number system
		blocktriple<abits, bt> bAdd;

		if (bReportIndividualTestCases) a.constexprClassParameters();

		for (size_t i = 0; i < NR_VALUES; ++i) {
			a.setbits(i);
			a.normalize(b);           // if normalize had a fraction bit parameter, we could support arbitrary conversions: TODO
			if (double(a) != double(b)) {
				if (a.isnan() && b.isnan()) continue;
				if (a.isinf() && b.isinf()) continue;

				++nrOfTestFailures;
				if (bReportIndividualTestCases) cout << "FAIL: " << to_binary(a) << " : " << a << " != " << to_triple(b) << " : " << b << '\n';
			}
		}
		return nrOfTestFailures;
	}


	template<typename BfloatConfiguration>
	int VerifyBfloatToBlocktripleAddConversion(bool bReportIndividualTestCases) {
		using namespace std;
		using namespace sw::universal;
		constexpr size_t nbits = BfloatConfiguration::nbits;
		constexpr size_t es = BfloatConfiguration::es;
		using bt = typename BfloatConfiguration::BlockType;
		constexpr size_t fbits = BfloatConfiguration::fbits;
		constexpr size_t abits = BfloatConfiguration::abits;

		int nrOfTestFailures{ 0 };
		constexpr size_t NR_VALUES = (1u << nbits);
		bfloat<nbits, es, bt> a;
		blocktriple<abits, bt> b; // now we want to create a blocktriple that goes into an add or subtract operation

		if (bReportIndividualTestCases) a.constexprClassParameters();

		for (size_t i = 0; i < NR_VALUES; ++i) {
			a.setbits(i);
			a.normalizeAddition(b);           // if normalize had a fraction bit parameter, we could support arbitrary conversions: TODO
			if (double(a) != double(b)) {
				if (a.isnan() && b.isnan()) continue;
				if (a.isinf() && b.isinf()) continue;

				++nrOfTestFailures;
				if (bReportIndividualTestCases) cout << "FAIL: " << to_binary(a) << " : " << a << " != " << to_triple(b) << " : " << b << '\n';
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
	using namespace std;
	using namespace sw::universal;

	print_cmd_line(argc, argv);

	int nrOfFailedTestCases = 0;
	std::string tag = "bfloat <-> blocktriple conversion: ";

#if MANUAL_TESTING

	// bfloat<> is a linear floating-point

	// to track conversion in more detail
	std::cout << std::setprecision(8);
	std::cerr << std::setprecision(8);

	{
		constexpr size_t nbits = 64;
		constexpr size_t es = 11;
		constexpr size_t fbits = nbits - 1ull - es;
		using bt = uint32_t;
		bfloat<nbits, es, bt> a;
		blocktriple<fbits, bt> b;
//		a = 0.015625f;
		a = 2.0f;
		a.normalize(b);
		a.constexprClassParameters();
		blockbinary<es, bt> exponent; a.exponent(exponent);
		blockbinary<fbits, bt> fraction; a.fraction(fraction);
		cout << "bfloat     : " << to_binary(a) << " : " << a << " : scale " << a.scale() << " : " << exponent << " : " << fraction << '\n';
		cout << "blocktriple: " << to_triple(b) << " : " << b << endl;
	}

	nrOfFailedTestCases += VerifyBfloatToBlocktripleConversion< bfloat< 3, 1, uint8_t> >(false);
	nrOfFailedTestCases += VerifyBfloatToBlocktripleConversion< bfloat< 4, 2, uint8_t> >(false);
	nrOfFailedTestCases += VerifyBfloatToBlocktripleConversion< bfloat< 5, 3, uint8_t> >(false);
	nrOfFailedTestCases += VerifyBfloatToBlocktripleConversion< bfloat< 8, 4, uint8_t> >(false);

	nrOfFailedTestCases += VerifyBfloatToBlocktripleConversion< bfloat< 9, 1, uint8_t> >(true);
	nrOfFailedTestCases += VerifyBfloatToBlocktripleConversion< bfloat<10, 2, uint8_t> >(true);
	nrOfFailedTestCases += VerifyBfloatToBlocktripleConversion< bfloat<18, 5, uint8_t> >(true);

	std::cout << "failed tests: " << nrOfFailedTestCases << endl;
	nrOfFailedTestCases = 0; // in manual testing we ignore failures for the regression system

#if STRESS_TESTING

	// manual exhaustive test

#endif

#else  // !MANUAL_TESTING
	bool bReportIndividualTestCases = false;
	std::cout << "bfloat to blocktriple conversion validation" << '\n';

	// es = 1
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatToBlocktripleConversion< bfloat< 3, 1, uint8_t> >(bReportIndividualTestCases), tag, "bfloat< 3,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatToBlocktripleConversion< bfloat< 4, 1, uint8_t> >(bReportIndividualTestCases), tag, "bfloat< 4,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatToBlocktripleConversion< bfloat< 5, 1, uint8_t> >(bReportIndividualTestCases), tag, "bfloat< 5,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatToBlocktripleConversion< bfloat< 6, 1, uint8_t> >(bReportIndividualTestCases), tag, "bfloat< 6,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatToBlocktripleConversion< bfloat< 7, 1, uint8_t> >(bReportIndividualTestCases), tag, "bfloat< 7,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatToBlocktripleConversion< bfloat< 8, 1, uint8_t> >(bReportIndividualTestCases), tag, "bfloat< 8,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatToBlocktripleConversion< bfloat< 9, 1, uint8_t> >(bReportIndividualTestCases), tag, "bfloat< 9,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatToBlocktripleConversion< bfloat<10, 1, uint8_t> >(bReportIndividualTestCases), tag, "bfloat<10,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatToBlocktripleConversion< bfloat<12, 1, uint8_t> >(bReportIndividualTestCases), tag, "bfloat<12,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatToBlocktripleConversion< bfloat<16, 1, uint8_t> >(bReportIndividualTestCases), tag, "bfloat<16,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatToBlocktripleConversion< bfloat<18, 1, uint8_t> >(bReportIndividualTestCases), tag, "bfloat<18,1>");   // 3 blocks


	// es = 2
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatToBlocktripleConversion< bfloat< 4, 2, uint8_t> >(bReportIndividualTestCases), tag, "bfloat< 4,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatToBlocktripleConversion< bfloat< 5, 2, uint8_t> >(bReportIndividualTestCases), tag, "bfloat< 5,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatToBlocktripleConversion< bfloat< 6, 2, uint8_t> >(bReportIndividualTestCases), tag, "bfloat< 6,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatToBlocktripleConversion< bfloat< 7, 2, uint8_t> >(bReportIndividualTestCases), tag, "bfloat< 7,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatToBlocktripleConversion< bfloat< 8, 2, uint8_t> >(bReportIndividualTestCases), tag, "bfloat< 8,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatToBlocktripleConversion< bfloat<10, 2, uint8_t> >(bReportIndividualTestCases), tag, "bfloat<10,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatToBlocktripleConversion< bfloat<12, 2, uint8_t> >(bReportIndividualTestCases), tag, "bfloat<12,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatToBlocktripleConversion< bfloat<14, 2, uint8_t> >(bReportIndividualTestCases), tag, "bfloat<14,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatToBlocktripleConversion< bfloat<16, 2, uint8_t> >(bReportIndividualTestCases), tag, "bfloat<16,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatToBlocktripleConversion< bfloat<18, 2, uint8_t> >(bReportIndividualTestCases), tag, "bfloat<18,2>");   // 3 blocks


	// es = 3
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatToBlocktripleConversion< bfloat< 5, 3, uint8_t> >(bReportIndividualTestCases), tag, "bfloat< 5,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatToBlocktripleConversion< bfloat< 6, 3, uint8_t> >(bReportIndividualTestCases), tag, "bfloat< 6,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatToBlocktripleConversion< bfloat< 7, 3, uint8_t> >(bReportIndividualTestCases), tag, "bfloat< 7,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatToBlocktripleConversion< bfloat< 8, 3, uint8_t> >(bReportIndividualTestCases), tag, "bfloat< 8,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatToBlocktripleConversion< bfloat<10, 3, uint8_t> >(bReportIndividualTestCases), tag, "bfloat<10,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatToBlocktripleConversion< bfloat<12, 3, uint8_t> >(bReportIndividualTestCases), tag, "bfloat<12,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatToBlocktripleConversion< bfloat<14, 3, uint8_t> >(bReportIndividualTestCases), tag, "bfloat<14,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatToBlocktripleConversion< bfloat<18, 3, uint8_t> >(bReportIndividualTestCases), tag, "bfloat<18,3>");   // 3 blocks


	// es = 4
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatToBlocktripleConversion< bfloat< 6, 4, uint8_t> >(bReportIndividualTestCases), tag, "bfloat< 6,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatToBlocktripleConversion< bfloat< 7, 4, uint8_t> >(bReportIndividualTestCases), tag, "bfloat< 7,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatToBlocktripleConversion< bfloat< 8, 4, uint8_t> >(bReportIndividualTestCases), tag, "bfloat< 8,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatToBlocktripleConversion< bfloat<10, 4, uint8_t> >(bReportIndividualTestCases), tag, "bfloat<10,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatToBlocktripleConversion< bfloat<12, 4, uint8_t> >(bReportIndividualTestCases), tag, "bfloat<12,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatToBlocktripleConversion< bfloat<14, 4, uint8_t> >(bReportIndividualTestCases), tag, "bfloat<14,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatToBlocktripleConversion< bfloat<18, 4, uint8_t> >(bReportIndividualTestCases), tag, "bfloat<18,4>");   // 3 blocks


	// es = 5
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatToBlocktripleConversion< bfloat< 7, 5, uint8_t> >(bReportIndividualTestCases), tag, "bfloat< 7,5>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatToBlocktripleConversion< bfloat< 8, 5, uint8_t> >(bReportIndividualTestCases), tag, "bfloat< 8,5>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatToBlocktripleConversion< bfloat<10, 5, uint8_t> >(bReportIndividualTestCases), tag, "bfloat<10,5>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatToBlocktripleConversion< bfloat<12, 5, uint8_t> >(bReportIndividualTestCases), tag, "bfloat<12,5>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatToBlocktripleConversion< bfloat<14, 5, uint8_t> >(bReportIndividualTestCases), tag, "bfloat<14,5>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatToBlocktripleConversion< bfloat<18, 5, uint8_t> >(bReportIndividualTestCases), tag, "bfloat<18,5>");   // 3 blocks


	// es = 6
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatToBlocktripleConversion< bfloat< 8, 6, uint8_t> >(bReportIndividualTestCases), tag, "bfloat< 8,6>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatToBlocktripleConversion< bfloat< 9, 6, uint8_t> >(bReportIndividualTestCases), tag, "bfloat< 9,6>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatToBlocktripleConversion< bfloat<10, 6, uint8_t> >(bReportIndividualTestCases), tag, "bfloat<10,6>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatToBlocktripleConversion< bfloat<12, 6, uint8_t> >(bReportIndividualTestCases), tag, "bfloat<12,6>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatToBlocktripleConversion< bfloat<14, 6, uint8_t> >(bReportIndividualTestCases), tag, "bfloat<14,6>");


	// es = 7
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatToBlocktripleConversion< bfloat< 9, 7, uint8_t> >(bReportIndividualTestCases), tag, "bfloat< 9,7>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatToBlocktripleConversion< bfloat<10, 7, uint8_t> >(bReportIndividualTestCases), tag, "bfloat<10,7>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatToBlocktripleConversion< bfloat<12, 7, uint8_t> >(bReportIndividualTestCases), tag, "bfloat<12,7>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatToBlocktripleConversion< bfloat<14, 7, uint8_t> >(bReportIndividualTestCases), tag, "bfloat<14,7>");

	// still failing
	// es = 8
//	nrOfFailedTestCases = ReportTestResult(VerifyBfloatToBlocktripleConversion< bfloat<11, 8, uint8_t> >(bReportIndividualTestCases), tag, "bfloat<11,8>");
//	nrOfFailedTestCases = ReportTestResult(VerifyBfloatToBlocktripleConversion< bfloat<12, 8, uint8_t> >(bReportIndividualTestCases), tag, "bfloat<12,8>");
//	nrOfFailedTestCases = ReportTestResult(VerifyBfloatToBlocktripleConversion< bfloat<14, 8, uint8_t> >(bReportIndividualTestCases), tag, "bfloat<14,8>");


#if STRESS_TESTING

	nrOfFailedTestCases = ReportTestResult(VerifyBfloatToBlocktripleConversion< bfloat<25, 2, uint8_t> >(bReportIndividualTestCases), tag, "bfloat<25,2>");   // 4 blocks

#endif  // STRESS_TESTING

#endif  // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::bfloat_arithmetic_exception& err) {
	std::cerr << "Uncaught bfloat arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::bfloat_internal_exception& err) {
	std::cerr << "Uncaught bfloat internal exception: " << err.what() << std::endl;
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
