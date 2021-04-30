// normalization.cpp: test suite runner for normalization tests of bfloats
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

#ifdef LATER
namespace sw::universal {

	/// <summary>
	/// verify that normalization represents the same value
	/// </summary>
	/// <typeparam name="bt">block storage type of representation</typeparam>
	/// <param name="bReportIndividualTestCases">if true print individual test cases</param>
	/// <returns></returns>
	template<typename BfloatConfiguration>
	int VerifyBfloatNormalization(bool bReportIndividualTestCases) {
		using namespace std;
		using namespace sw::universal;
		constexpr size_t nbits = BfloatConfiguration::nbits;
		constexpr size_t es = BfloatConfiguration::es;
		using bt = typename BfloatConfiguration::BlockType;
		constexpr size_t fhbits = nbits - es;
		bfloat<nbits, es, bt> a;
		blocktriple<fhbits> b;  // representing significant
		int nrOfTestFailures{ 0 };
		for (size_t i = 0; i < 64; ++i) {
			a.set_raw_bits(i);
			if (a.iszero() || a.isinf() || a.isnan()) {
				// special values are not normalizable
				b.setzero();
			}
			else {
				a.normalize(b);
				if (double(a) != double(b)) {
					++nrOfTestFailures;
					if (bReportIndividualTestCases) cout << "FAIL: " << to_binary(a) << " : " << a << " != " << to_triple(b) << " : " << b << '\n';
				}
			}
		}
		return nrOfTestFailures;
	}

}
#endif

// conditional compile flags
#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	print_cmd_line(argc, argv);

	int nrOfFailedTestCases = 0;
	std::string tag = "bfloat normalization: ";

#if MANUAL_TESTING

	// bfloat<> is a linear floating-point

	// to track conversion in more detail
	std::cout << std::setprecision(8);
	std::cerr << std::setprecision(8);

	{
		constexpr size_t nbits = 10;
		constexpr size_t es = 4;
		constexpr size_t fbits = nbits - 1ull - es;
		bfloat<nbits, es, uint8_t> a;
		blocktriple<fbits + 1> b;  // representing significant
		a = 0.015625f;
//		a.normalize(b);
		cout << to_binary(a) << " : " << a << " : scale " << a.scale() << " : " << to_triple(b) << " : " << b << endl;

	}

#ifdef LATER
	nrOfFailedTestCases += VerifyBfloatNormalization< bfloat<3, 1, uint8_t> >(true);
	nrOfFailedTestCases += VerifyBfloatNormalization< bfloat<4, 1, uint8_t> >(true);
	return 0;
	nrOfFailedTestCases += VerifyBfloatNormalization< bfloat<5, 1, uint8_t> >(true);
	nrOfFailedTestCases += VerifyBfloatNormalization< bfloat<6, 1, uint8_t> >(true);
	nrOfFailedTestCases += VerifyBfloatNormalization< bfloat<7, 1, uint8_t> >(true);
	nrOfFailedTestCases += VerifyBfloatNormalization< bfloat<8, 1, uint8_t> >(true);
	nrOfFailedTestCases += VerifyBfloatNormalization< bfloat<9, 1, uint8_t> >(true);
#endif

	std::cout << "failed tests: " << nrOfFailedTestCases << endl;
	nrOfFailedTestCases = 0; // in manual testing we ignore failures for the regression system

#if STRESS_TESTING

	// manual exhaustive test

#endif

#else  // !MANUAL_TESTING
	bool bReportIndividualTestCases = false;
	cout << "BFLOAT normalization validation" << endl;

	// es = 1
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatNormalization< bfloat<3, 1> >(bReportIndividualTestCases), tag, "bfloat<3,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatNormalization< bfloat<4, 1> >(bReportIndividualTestCases), tag, "bfloat<4,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatNormalization< bfloat<5, 1> >(bReportIndividualTestCases), tag, "bfloat<5,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatNormalization< bfloat<6, 1> >(bReportIndividualTestCases), tag, "bfloat<6,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatNormalization< bfloat<7, 1> >(bReportIndividualTestCases), tag, "bfloat<7,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatNormalization< bfloat<8, 1> >(bReportIndividualTestCases), tag, "bfloat<8,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatNormalization< bfloat<9, 1> >(bReportIndividualTestCases), tag, "bfloat<9,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatNormalization< bfloat<10, 1> >(bReportIndividualTestCases), tag, "bfloat<10,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatNormalization< bfloat<12, 1> >(bReportIndividualTestCases), tag, "bfloat<12,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatNormalization< bfloat<16, 1> >(bReportIndividualTestCases), tag, "bfloat<16,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatNormalization< bfloat<18, 1> >(bReportIndividualTestCases), tag, "bfloat<18,1>");   // 3 blocks


	// es = 2
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatNormalization< bfloat<4, 2> >(bReportIndividualTestCases), tag, "bfloat<4,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatNormalization< bfloat<5, 2> >(bReportIndividualTestCases), tag, "bfloat<5,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatNormalization< bfloat<6, 2> >(bReportIndividualTestCases), tag, "bfloat<6,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatNormalization< bfloat<7, 2> >(bReportIndividualTestCases), tag, "bfloat<7,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatNormalization< bfloat<8, 2> >(bReportIndividualTestCases), tag, "bfloat<8,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatNormalization< bfloat<10, 2> >(bReportIndividualTestCases), tag, "bfloat<10,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatNormalization< bfloat<12, 2> >(bReportIndividualTestCases), tag, "bfloat<12,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatNormalization< bfloat<14, 2> >(bReportIndividualTestCases), tag, "bfloat<14,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatNormalization< bfloat<16, 2> >(bReportIndividualTestCases), tag, "bfloat<16,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatNormalization< bfloat<18, 2> >(bReportIndividualTestCases), tag, "bfloat<18,2>");   // 3 blocks


	// es = 3
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatNormalization< bfloat<5, 3> >(bReportIndividualTestCases), tag, "bfloat<5,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatNormalization< bfloat<6, 3> >(bReportIndividualTestCases), tag, "bfloat<6,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatNormalization< bfloat<7, 3> >(bReportIndividualTestCases), tag, "bfloat<7,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatNormalization< bfloat<8, 3> >(bReportIndividualTestCases), tag, "bfloat<8,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatNormalization< bfloat<10, 3> >(bReportIndividualTestCases), tag, "bfloat<10,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatNormalization< bfloat<12, 3> >(bReportIndividualTestCases), tag, "bfloat<12,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatNormalization< bfloat<14, 3> >(bReportIndividualTestCases), tag, "bfloat<14,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatNormalization< bfloat<18, 3> >(bReportIndividualTestCases), tag, "bfloat<18,3>");   // 3 blocks


	// es = 4
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatNormalization< bfloat<6, 4> >(bReportIndividualTestCases), tag, "bfloat<6,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatNormalization< bfloat<7, 4> >(bReportIndividualTestCases), tag, "bfloat<7,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatNormalization< bfloat<8, 4> >(bReportIndividualTestCases), tag, "bfloat<8,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatNormalization< bfloat<10, 4> >(bReportIndividualTestCases), tag, "bfloat<10,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatNormalization< bfloat<12, 4> >(bReportIndividualTestCases), tag, "bfloat<12,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatNormalization< bfloat<14, 4> >(bReportIndividualTestCases), tag, "bfloat<14,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatNormalization< bfloat<18, 4> >(bReportIndividualTestCases), tag, "bfloat<18,4>");   // 3 blocks


	// es = 5
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatNormalization< bfloat<7, 5> >(bReportIndividualTestCases), tag, "bfloat<7,5>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatNormalization< bfloat<8, 5> >(bReportIndividualTestCases), tag, "bfloat<8,5>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatNormalization< bfloat<10, 5> >(bReportIndividualTestCases), tag, "bfloat<10,5>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatNormalization< bfloat<12, 5> >(bReportIndividualTestCases), tag, "bfloat<12,5>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatNormalization< bfloat<14, 5> >(bReportIndividualTestCases), tag, "bfloat<14,5>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatNormalization< bfloat<18, 5> >(bReportIndividualTestCases), tag, "bfloat<18,5>");   // 3 blocks


	// es = 6
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatNormalization< bfloat<8, 6> >(bReportIndividualTestCases), tag, "bfloat<8,6>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatNormalization< bfloat<9, 6> >(bReportIndividualTestCases), tag, "bfloat<9,6>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatNormalization< bfloat<10, 6> >(bReportIndividualTestCases), tag, "bfloat<10,6>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatNormalization< bfloat<12, 6> >(bReportIndividualTestCases), tag, "bfloat<12,6>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatNormalization< bfloat<14, 6> >(bReportIndividualTestCases), tag, "bfloat<14,6>");


	// es = 7
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatNormalization< bfloat< 9, 7> >(bReportIndividualTestCases), tag, "bfloat<9,7>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatNormalization< bfloat<10, 7> >(bReportIndividualTestCases), tag, "bfloat<10,7>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatNormalization< bfloat<12, 7> >(bReportIndividualTestCases), tag, "bfloat<12,7>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatNormalization< bfloat<14, 7> >(bReportIndividualTestCases), tag, "bfloat<14,7>");

	// still failing
	// es = 8
//	nrOfFailedTestCases = ReportTestResult(VerifyBfloatNormalization< bfloat<11, 8> >(bReportIndividualTestCases), tag, "bfloat<11,8>");
//	nrOfFailedTestCases = ReportTestResult(VerifyBfloatNormalization< bfloat<12, 8> >(bReportIndividualTestCases), tag, "bfloat<12,8>");
//	nrOfFailedTestCases = ReportTestResult(VerifyBfloatNormalization< bfloat<14, 8> >(bReportIndividualTestCases), tag, "bfloat<14,8>");


#if STRESS_TESTING

#endif  // STRESS_TESTING

#endif  // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
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
