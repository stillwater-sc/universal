// normalization.cpp: test suite runner for normalization tests of classic cfloats
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

#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_suite_conversion.hpp>
#include <universal/verification/cfloat_test_suite.hpp>

namespace sw::universal {

	/// <summary>
	/// verify that normalization represents the same value
	/// </summary>
	/// <typeparam name="bt">block storage type of representation</typeparam>
	/// <param name="bReportIndividualTestCases">if true print individual test cases</param>
	/// <returns></returns>
	template<typename cfloatConfiguration>
	int VerifyCfloatNormalization(bool bReportIndividualTestCases) {
		using namespace sw::universal;
		constexpr size_t nbits = cfloatConfiguration::nbits;
		constexpr size_t es = cfloatConfiguration::es;
		using bt = typename cfloatConfiguration::BlockType;
		constexpr size_t fhbits = nbits - es;
		cfloat<nbits, es, bt> a;
		blocktriple<fhbits> b;  // representing significant
		int nrOfTestFailures{ 0 };
		for (size_t i = 0; i < 64; ++i) {
			a.setbits(i);
			if (a.iszero() || a.isinf() || a.isnan()) {
				// special values are not normalizable
				b.setzero();
			}
			else {
				a.normalize(b);
				if (double(a) != double(b)) {
					++nrOfTestFailures;
					if (bReportIndividualTestCases) std::cout << "FAIL: " << to_binary(a) << " : " << a << " != " << to_triple(b) << " : " << b << '\n';
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
	std::string tag = "cfloat normalization: ";

#if MANUAL_TESTING

	// cfloat<> is a linear floating-point

	// to track conversion in more detail
	std::cout << std::setprecision(8);
	std::cerr << std::setprecision(8);

	{
		constexpr size_t nbits = 10;
		constexpr size_t es = 4;
		constexpr size_t fbits = nbits - 1ull - es;
		cfloat<nbits, es, uint8_t> a;
		blocktriple<fbits + 1> b;  // representing significant
		a = 0.015625f;
//		a.normalize(b);
		std::cout << to_binary(a) << " : " << a << " : scale " << a.scale() << " : " << to_triple(b) << " : " << b << std::endl;

	}

#ifdef LATER
	nrOfFailedTestCases += VerifyCfloatNormalization< cfloat<3, 1, uint8_t> >(true);
	nrOfFailedTestCases += VerifyCfloatNormalization< cfloat<4, 1, uint8_t> >(true);
	return 0;
	nrOfFailedTestCases += VerifyCfloatNormalization< cfloat<5, 1, uint8_t> >(true);
	nrOfFailedTestCases += VerifyCfloatNormalization< cfloat<6, 1, uint8_t> >(true);
	nrOfFailedTestCases += VerifyCfloatNormalization< cfloat<7, 1, uint8_t> >(true);
	nrOfFailedTestCases += VerifyCfloatNormalization< cfloat<8, 1, uint8_t> >(true);
	nrOfFailedTestCases += VerifyCfloatNormalization< cfloat<9, 1, uint8_t> >(true);
#endif

	std::cout << "failed tests: " << nrOfFailedTestCases << '\n';
	nrOfFailedTestCases = 0; // in manual testing we ignore failures for the regression system

#if STRESS_TESTING

	// manual exhaustive test

#endif

#else  // !MANUAL_TESTING
	bool bReportIndividualTestCases = false;
	cout << "cfloat normalization validation" << endl;

	// es = 1
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<3, 1> >(bReportIndividualTestCases), tag, "cfloat<3,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<4, 1> >(bReportIndividualTestCases), tag, "cfloat<4,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<5, 1> >(bReportIndividualTestCases), tag, "cfloat<5,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<6, 1> >(bReportIndividualTestCases), tag, "cfloat<6,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<7, 1> >(bReportIndividualTestCases), tag, "cfloat<7,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<8, 1> >(bReportIndividualTestCases), tag, "cfloat<8,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<9, 1> >(bReportIndividualTestCases), tag, "cfloat<9,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<10, 1> >(bReportIndividualTestCases), tag, "cfloat<10,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<12, 1> >(bReportIndividualTestCases), tag, "cfloat<12,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<16, 1> >(bReportIndividualTestCases), tag, "cfloat<16,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<18, 1> >(bReportIndividualTestCases), tag, "cfloat<18,1>");   // 3 blocks


	// es = 2
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<4, 2> >(bReportIndividualTestCases), tag, "cfloat<4,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<5, 2> >(bReportIndividualTestCases), tag, "cfloat<5,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<6, 2> >(bReportIndividualTestCases), tag, "cfloat<6,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<7, 2> >(bReportIndividualTestCases), tag, "cfloat<7,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<8, 2> >(bReportIndividualTestCases), tag, "cfloat<8,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<10, 2> >(bReportIndividualTestCases), tag, "cfloat<10,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<12, 2> >(bReportIndividualTestCases), tag, "cfloat<12,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<14, 2> >(bReportIndividualTestCases), tag, "cfloat<14,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<16, 2> >(bReportIndividualTestCases), tag, "cfloat<16,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<18, 2> >(bReportIndividualTestCases), tag, "cfloat<18,2>");   // 3 blocks


	// es = 3
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<5, 3> >(bReportIndividualTestCases), tag, "cfloat<5,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<6, 3> >(bReportIndividualTestCases), tag, "cfloat<6,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<7, 3> >(bReportIndividualTestCases), tag, "cfloat<7,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<8, 3> >(bReportIndividualTestCases), tag, "cfloat<8,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<10, 3> >(bReportIndividualTestCases), tag, "cfloat<10,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<12, 3> >(bReportIndividualTestCases), tag, "cfloat<12,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<14, 3> >(bReportIndividualTestCases), tag, "cfloat<14,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<18, 3> >(bReportIndividualTestCases), tag, "cfloat<18,3>");   // 3 blocks


	// es = 4
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<6, 4> >(bReportIndividualTestCases), tag, "cfloat<6,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<7, 4> >(bReportIndividualTestCases), tag, "cfloat<7,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<8, 4> >(bReportIndividualTestCases), tag, "cfloat<8,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<10, 4> >(bReportIndividualTestCases), tag, "cfloat<10,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<12, 4> >(bReportIndividualTestCases), tag, "cfloat<12,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<14, 4> >(bReportIndividualTestCases), tag, "cfloat<14,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<18, 4> >(bReportIndividualTestCases), tag, "cfloat<18,4>");   // 3 blocks


	// es = 5
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<7, 5> >(bReportIndividualTestCases), tag, "cfloat<7,5>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<8, 5> >(bReportIndividualTestCases), tag, "cfloat<8,5>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<10, 5> >(bReportIndividualTestCases), tag, "cfloat<10,5>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<12, 5> >(bReportIndividualTestCases), tag, "cfloat<12,5>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<14, 5> >(bReportIndividualTestCases), tag, "cfloat<14,5>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<18, 5> >(bReportIndividualTestCases), tag, "cfloat<18,5>");   // 3 blocks


	// es = 6
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<8, 6> >(bReportIndividualTestCases), tag, "cfloat<8,6>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<9, 6> >(bReportIndividualTestCases), tag, "cfloat<9,6>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<10, 6> >(bReportIndividualTestCases), tag, "cfloat<10,6>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<12, 6> >(bReportIndividualTestCases), tag, "cfloat<12,6>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<14, 6> >(bReportIndividualTestCases), tag, "cfloat<14,6>");


	// es = 7
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat< 9, 7> >(bReportIndividualTestCases), tag, "cfloat<9,7>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<10, 7> >(bReportIndividualTestCases), tag, "cfloat<10,7>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<12, 7> >(bReportIndividualTestCases), tag, "cfloat<12,7>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<14, 7> >(bReportIndividualTestCases), tag, "cfloat<14,7>");

	// still failing
	// es = 8
//	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<11, 8> >(bReportIndividualTestCases), tag, "cfloat<11,8>");
//	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<12, 8> >(bReportIndividualTestCases), tag, "cfloat<12,8>");
//	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<14, 8> >(bReportIndividualTestCases), tag, "cfloat<14,8>");


#if STRESS_TESTING

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
