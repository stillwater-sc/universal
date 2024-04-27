// normalization.cpp: test suite runner for normalization tests of classic cfloats
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
#include <universal/verification/cfloat_test_suite.hpp>

namespace sw::universal {

	/// <summary>
	/// verify that normalization represents the same value
	/// </summary>
	/// <typeparam name="bt">block storage type of representation</typeparam>
	/// <param name="reportTestCases">if true print individual test cases</param>
	/// <returns></returns>
	template<typename CfloatConfiguration>
	int VerifyCfloatNormalization(bool reportTestCases) {
		using namespace sw::universal;
		constexpr size_t nbits = CfloatConfiguration::nbits;
		constexpr size_t es = CfloatConfiguration::es;
		using bt = typename CfloatConfiguration::BlockType;
		cfloat<nbits, es, bt> a;
		constexpr size_t fbits = CfloatConfiguration::fbits;
		blocktriple<fbits, BlockTripleOperator::REP, bt> b;  // representing significant
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
					if (reportTestCases) std::cout << "FAIL: " << to_binary(a) << " : " << a << " != " << to_triple(b) << " : " << b << '\n';
				}
			}
		}
		return nrOfTestFailures;
	}

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
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "cfloat<> normalization";
	std::string test_tag    = "normalization";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// to track conversion in more detail
	std::cout << std::setprecision(8);
	std::cerr << std::setprecision(8);

	{
		// normalization of a cfloat to a blocktriple specialized for different arithmetic operators: REPRESENTATION, ADD, MUL, DIV
		constexpr size_t nbits = 8;
		constexpr size_t es = 3;
		using bt = uint8_t;
		constexpr bool hasSubnormals = true;
		constexpr bool hasSupernormals = true;
		constexpr bool isSaturating = false;
		using Real = cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>;
		CONSTEXPRESSION Real a(1.0f + 0.5f + 0.25f + 0.125f + 0.0625f);
		Real b(-1.0f - 0.5f - 0.25f - 0.125f - 0.0625f);
		constexpr size_t fbits = Real::fbits;
		{
			// normalize conversion to blocktriple
			blocktriple<fbits, BlockTripleOperator::REP, bt> _a, _b;
			a.normalize(_a);
			b.normalize(_b);
			std::cout << to_binary(a) << " : " << to_triple(_a) << '\n';
			std::cout << to_binary(b) << " : " << to_triple(_b) << '\n';
			std::cout << "========  end of representation  =========\n\n";
		}

		{
			Real c = a + b;
			std::cout << "Result of addition       : " << color_print(c) << " : "  << c << '\n';

			// normalize for addition
			blocktriple<fbits, BlockTripleOperator::ADD, bt> _a, _b, _c;
			a.normalizeAddition(_a);
			b.normalizeAddition(_b);
			_c.add(_a, _b);
			std::cout << to_binary(a) << " : " << to_triple(_a) << " : " << _a << '\n';
			std::cout << to_binary(b) << " : " << to_triple(_b) << " : " << _b << '\n';
			std::cout << to_binary(c) << " : " << to_triple(_c) << " : " << _c << '\n';
			std::cout << "+++++++++    end of addition    ++++++++++\n\n";
		}

		{
			Real c = a * b;
			std::cout << "result of multiplication : " << color_print(c) << " : " << c << '\n';

			// normalize for multiplication
			blocktriple<fbits, BlockTripleOperator::MUL, bt> _a, _b, _c;
			a.normalizeMultiplication(_a);
			b.normalizeMultiplication(_b);
			_c.mul(_a, _b);
			std::cout << to_binary(a) << " : " << to_triple(_a) << " : " << _a << '\n';
			std::cout << to_binary(b) << " : " << to_triple(_b) << " : " << _b << '\n';
			std::cout << to_binary(c) << " : " << to_triple(_c) << " : " << _c << '\n';
			std::cout << "********* end of multiplication **********\n\n";
		}

		{
			Real c = a / b;
			std::cout << "Result of division       : " << color_print(c) << " : " << c << '\n';

			// normalize for division
			blocktriple<fbits, BlockTripleOperator::DIV, bt> _a, _b, _c;
			a.normalizeDivision(_a);
			b.normalizeDivision(_b);
			_c.div(_a, _b);
			std::cout << to_binary(a) << " : " << to_triple(_a) << " : " << _a << '\n';
			std::cout << to_binary(b) << " : " << to_triple(_b) << " : " << _b << '\n';
			std::cout << to_binary(c) << " : " << to_triple(_c) << " : " << _c << '\n';
			std::cout << "/////////    end of division    //////////\n\n";
		}
	}

	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<4, 2> >(reportTestCases), test_tag, "cfloat<4,2>");
/*
    TODO: normalize for ADD, MUL, DIV are different operators: can they be accessed through the same API?
	nrOfFailedTestCases += VerifyCfloatNormalization< cfloat<3, 1, uint8_t> >(true);
	nrOfFailedTestCases += VerifyCfloatNormalization< cfloat<4, 1, uint8_t> >(true);
	nrOfFailedTestCases += VerifyCfloatNormalization< cfloat<5, 1, uint8_t> >(true);
	nrOfFailedTestCases += VerifyCfloatNormalization< cfloat<6, 1, uint8_t> >(true);
	nrOfFailedTestCases += VerifyCfloatNormalization< cfloat<7, 1, uint8_t> >(true);
	nrOfFailedTestCases += VerifyCfloatNormalization< cfloat<8, 1, uint8_t> >(true);
	nrOfFailedTestCases += VerifyCfloatNormalization< cfloat<9, 1, uint8_t> >(true);
*/

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors

#else  // !MANUAL_TESTING

	// es = 1
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<3, 1> >(reportTestCases), test_tag, "cfloat<3,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<4, 1> >(reportTestCases), test_tag, "cfloat<4,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<5, 1> >(reportTestCases), test_tag, "cfloat<5,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<6, 1> >(reportTestCases), test_tag, "cfloat<6,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<7, 1> >(reportTestCases), test_tag, "cfloat<7,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<8, 1> >(reportTestCases), test_tag, "cfloat<8,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<9, 1> >(reportTestCases), test_tag, "cfloat<9,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<10, 1> >(reportTestCases), test_tag, "cfloat<10,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<12, 1> >(reportTestCases), test_tag, "cfloat<12,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<16, 1> >(reportTestCases), test_tag, "cfloat<16,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<18, 1> >(reportTestCases), test_tag, "cfloat<18,1>");   // 3 blocks


	// es = 2
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<4, 2> >(reportTestCases), test_tag, "cfloat<4,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<5, 2> >(reportTestCases), test_tag, "cfloat<5,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<6, 2> >(reportTestCases), test_tag, "cfloat<6,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<7, 2> >(reportTestCases), test_tag, "cfloat<7,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<8, 2> >(reportTestCases), test_tag, "cfloat<8,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<10, 2> >(reportTestCases), test_tag, "cfloat<10,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<12, 2> >(reportTestCases), test_tag, "cfloat<12,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<14, 2> >(reportTestCases), test_tag, "cfloat<14,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<16, 2> >(reportTestCases), test_tag, "cfloat<16,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<18, 2> >(reportTestCases), test_tag, "cfloat<18,2>");   // 3 blocks


	// es = 3
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<5, 3> >(reportTestCases), test_tag, "cfloat<5,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<6, 3> >(reportTestCases), test_tag, "cfloat<6,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<7, 3> >(reportTestCases), test_tag, "cfloat<7,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<8, 3> >(reportTestCases), test_tag, "cfloat<8,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<10, 3> >(reportTestCases), test_tag, "cfloat<10,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<12, 3> >(reportTestCases), test_tag, "cfloat<12,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<14, 3> >(reportTestCases), test_tag, "cfloat<14,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<18, 3> >(reportTestCases), test_tag, "cfloat<18,3>");   // 3 blocks


	// es = 4
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<6, 4> >(reportTestCases), test_tag, "cfloat<6,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<7, 4> >(reportTestCases), test_tag, "cfloat<7,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<8, 4> >(reportTestCases), test_tag, "cfloat<8,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<10, 4> >(reportTestCases), test_tag, "cfloat<10,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<12, 4> >(reportTestCases), test_tag, "cfloat<12,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<14, 4> >(reportTestCases), test_tag, "cfloat<14,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<18, 4> >(reportTestCases), test_tag, "cfloat<18,4>");   // 3 blocks


	// es = 5
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<7, 5> >(reportTestCases), test_tag, "cfloat<7,5>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<8, 5> >(reportTestCases), test_tag, "cfloat<8,5>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<10, 5> >(reportTestCases), test_tag, "cfloat<10,5>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<12, 5> >(reportTestCases), test_tag, "cfloat<12,5>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<14, 5> >(reportTestCases), test_tag, "cfloat<14,5>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<18, 5> >(reportTestCases), test_tag, "cfloat<18,5>");   // 3 blocks


	// es = 6
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<8, 6> >(reportTestCases), test_tag, "cfloat<8,6>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<9, 6> >(reportTestCases), test_tag, "cfloat<9,6>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<10, 6> >(reportTestCases), test_tag, "cfloat<10,6>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<12, 6> >(reportTestCases), test_tag, "cfloat<12,6>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<14, 6> >(reportTestCases), test_tag, "cfloat<14,6>");


	// es = 7
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat< 9, 7> >(reportTestCases), test_tag, "cfloat<9,7>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<10, 7> >(reportTestCases), test_tag, "cfloat<10,7>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<12, 7> >(reportTestCases), test_tag, "cfloat<12,7>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<14, 7> >(reportTestCases), test_tag, "cfloat<14,7>");

	// es = 8
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<11, 8> >(reportTestCases), test_tag, "cfloat<11,8>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<12, 8> >(reportTestCases), test_tag, "cfloat<12,8>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatNormalization< cfloat<14, 8> >(reportTestCases), test_tag, "cfloat<14,8>");


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
