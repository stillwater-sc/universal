// subtraction.cpp: test suite runner for subtraction of elastic precision binary integers
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//  SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <iomanip>
#include <string>
#include <cmath>
#include <limits>

// minimum set of include files to reflect source code dependencies
#include <universal/number/einteger/einteger.hpp>
#include <universal/verification/test_reporters.hpp>

namespace sw { namespace universal {

	// enumerate all subtraction cases for an integer<nbits, BlockType> configuration
	template<size_t nbits, typename BlockType>
	int VerifyElasticSubtraction(bool reportTestCases) {
		using Integer = einteger<BlockType>;
		constexpr size_t NR_ENCODINGS = (size_t(1) << nbits);

		Integer ia, ib, ic, iref;

		int nrOfFailedTests = 0;
		size_t increment = std::max(1ull, NR_ENCODINGS / 1024ull);
		for (size_t i = 0; i < NR_ENCODINGS; i += increment) {
			ia.setbits(i);
			int64_t i64a = int64_t(ia);
			for (size_t j = 0; j < NR_ENCODINGS; j += increment) {
				ib.setbits(j);
				int64_t i64b = int64_t(ib);
				iref = i64a - i64b;
				ic = ia - ib;

				if (ic != iref) {
					nrOfFailedTests++;
					if (reportTestCases) ReportBinaryArithmeticError("FAIL", "-", ia, ib, ic, iref);
				}
				else {
					//if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "-", ia, ib, ic, iref);
				}
				if (nrOfFailedTests > 100) return nrOfFailedTests;
			}
			if (reportTestCases) if (i % 1024 == 0) std::cout << '.';
		}
		if (reportTestCases) std::cout << std::endl;
		return nrOfFailedTests;
	}

} } // namespace sw::univeral

// generate specific test case that you can trace with the trace conditions in mpreal.hpp
// for most bugs they are traceable with _trace_conversion and _trace_add
template<typename Ty, typename BlockType>
void GenerateTestCase(Ty _a, Ty _b) {
	Ty ref;
	sw::universal::einteger<BlockType> a, b, c, aref;
	ref = _a - _b;
	aref = ref;

	a = _a;
	b = _b;
	c = a - b;

	constexpr size_t ndigits = 30;
	std::cout << std::setw(ndigits) << _a << " - " << std::setw(ndigits) << _b << " = " << std::setw(ndigits) << ref << std::endl;
//	std::cout << a << " - " << b << " = " << c << " (reference: " << aref << ")   " ;
	std::cout << to_binary(a) << " - " << to_binary(b) << " = " << to_binary(c) << " : " << (long long)(c) << " (reference: " << to_binary(aref) << ")   ";
	std::cout << (aref == c ? "PASS" : "FAIL") << std::endl << std::endl;
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
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "elastic precision binary integer subtraction";
	std::string test_tag    = "einteger subtraction";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
//	bool bReportIndividualTestCases = false;

	// generate individual testcases to hand trace/debug
	GenerateTestCase<std::uint32_t, std::uint8_t>(1, 1);
	GenerateTestCase<std::int32_t, std::uint8_t>(0, 1);
	GenerateTestCase<std::int32_t, std::uint8_t>(1, 0);
	GenerateTestCase<std::int32_t, std::uint8_t>(0, -1);
	GenerateTestCase<std::int32_t, std::uint8_t>(1, 2);
	GenerateTestCase<std::int32_t, std::uint8_t>(4, 256);
	GenerateTestCase<std::int32_t, std::uint8_t>(4, 260);
	GenerateTestCase<std::int32_t, std::uint8_t>(260, 512);
	GenerateTestCase<std::int32_t, std::uint8_t>(260, 511);
	GenerateTestCase<std::int32_t, std::uint8_t>(512, 260);
	GenerateTestCase<std::int32_t, std::uint8_t>(512, 257);

	nrOfFailedTestCases += ReportTestResult(VerifyElasticSubtraction<8, uint8_t>(reportTestCases), "einteger<uint8_t> 1byte", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyElasticSubtraction<12, uint8_t>(reportTestCases), "einteger<uint8_t> 2bytes", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyElasticSubtraction<32, uint8_t>(reportTestCases), "einteger<uint8_t> 8bytes", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyElasticSubtraction<8, uint8_t>(reportTestCases), "einteger<uint8_t> 1byte", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyElasticSubtraction<10, uint8_t>(reportTestCases), "einteger<uint8_t> 2bytes", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyElasticSubtraction<8, uint16_t>(reportTestCases), "einteger<uint16_t> 1word", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyElasticSubtraction<16, uint16_t>(reportTestCases), "einteger<uint16_t> 2word", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyElasticSubtraction<16, uint32_t>(reportTestCases), "einteger<uint32_t> 1word", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyElasticSubtraction<32, uint32_t>(reportTestCases), "einteger<uint32_t> 2word", test_tag);
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
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
