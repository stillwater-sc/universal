// division.cpp: test suite runner for division of elastic precision binary integers
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
#include <algorithm>

// minimum set of include files to reflect source code dependencies
#define EINTEGER_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/einteger/einteger.hpp>
#include <universal/verification/test_reporters.hpp>

namespace sw { namespace universal {

	// enumerate all division cases for an integer<nbits, BlockType> configuration
	template<size_t nbits, typename BlockType>
	int VerifyElasticDivision(bool reportTestCases) {
		using Integer = einteger<BlockType>;
		constexpr size_t NR_ENCODINGS = (size_t(1) << nbits);

		Integer ia{}, ib{}, iq{}, iref{}, ir{};

		int nrOfFailedTests = 0;
		size_t increment = std::max(1ull, NR_ENCODINGS / 1024ull);
		for (size_t i = 0; i < NR_ENCODINGS; i += increment) {
			ia.setbits(i);
			int64_t i64a = int64_t(ia);
			for (size_t j = 0; j < NR_ENCODINGS; j += increment) {
				ib.setbits(j);
				int64_t i64b = int64_t(ib);
#if EINTEGER_THROW_ARITHMETIC_EXCEPTION
				try {
					iq.reduce(ia, ib, ir);
				}
				catch (const einteger_divide_by_zero& e) {
					if (ib.iszero()) {
						// correctly caught the exception
						continue;
					}
					else {
						std::cerr << "unexpected : " << e.what() << std::endl;
						nrOfFailedTests++;
					}
				}
				catch (...) {
					std::cerr << "unexpected exception" << std::endl;
					nrOfFailedTests++;
				}
#else
				iq.reduce(ia, ib, ir);
#endif
				if (j == 0) {
					iref = 0; // or maxneg?
				}
				else {
					iref = i64a / i64b;
				}
				if (iq != iref) {
					nrOfFailedTests++;
					if (reportTestCases) ReportBinaryArithmeticError("FAIL", "/", ia, ib, iq, iref);
				}
				else {
					//if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "/", ia, ib, iq, iref);
				}
				if (nrOfFailedTests > 100) return nrOfFailedTests;
			}
			if (reportTestCases) if (i % 1024 == 0) std::cout << '.';
		}
		if (reportTestCases) std::cout << std::endl;
		return nrOfFailedTests;
	}
} } // namespace sw::universal


// generate specific test case that you can trace with the trace conditions in mpreal.hpp
// for most bugs they are traceable with _trace_conversion and _trace_add
template<typename Ty, typename BlockType = std::uint32_t>
void GenerateTestCase(Ty _a, Ty _b) {
	Ty ref;
	sw::universal::einteger<BlockType> a, b, c, aref;
	ref = _a / _b;
	aref = ref;

	a = _a;
	b = _b;
	c = a / b;

	constexpr size_t ndigits = 30;
	std::cout << std::setw(ndigits) << _a << " / " << std::setw(ndigits) << _b << " = " << std::setw(ndigits) << ref << std::endl;
	std::cout << a << " / " << b << " = " << c << " (reference: " << aref << ")   ";
	std::cout << (aref == c ? "PASS" : "FAIL") << std::endl << std::endl;
}

struct TestRecord {
	std::int64_t a;
	std::int64_t b;
	std::int64_t q;
	std::int64_t r;
};

int DirectedTests() {
	TestRecord tests[] = {
		{ 128, 127, 1, 1 },
		{ 128, 128, 1, 0 },
		{ 128, 129, 0, 128 },
		{ 128,  63, 2, 2 },

		{ 256, 255, 1, 1 },
		{ 256, 256, 1, 0 },
		{ 256, 257, 0, 256 },

		{ 0x0000'0000'0001'0000, 0x0000'FFFF, 1, 1 },
		{ 0x0000'0000'0001'0000, 0x0001'0000, 1, 0 },
		{ 0x0000'0000'0001'0000, 0x0001'0001, 0, 0x0001'0000 },

		{ 0x0000'0000'0100'0000, 0x00FF'FFFF, 1, 1 },
		{ 0x0000'0000'0100'0000, 0x0100'0000, 1, 0 },
		{ 0x0000'0000'0100'0000, 0x0100'0001, 0, 0x0100'0000 },

		{ 0x0000'0001'0000'0000, 0x0'FFFF'FFFF, 1, 1 },
		{ 0x0000'0001'0000'0000, 0x1'0000'0000, 1, 0 },
		{ 0x0000'0001'0000'0000, 0x1'0000'0001, 0, 0x0000'0001'0000'0000 },

		{ 0x0000'0100'0000'0000, 0x0000'FFFF'FFFF, 256, 256 } ,
		{ 0x0000'0100'0000'0000, 0x00FF'FFFF'FFFF, 1, 1 },
		{ 0x0000'0100'0000'0000, 0x0100'0000'0000, 1, 0 },
		{ 0x0000'0100'0000'0000, 0x0100'0000'0001, 0, 0x0000'0100'0000'0000 },

		{ 0x0001'0000'0000'0000, 0x0000'FFFF'FFFF'FFFF, 1, 1 },
		{ 0x0001'0000'0000'0000, 0x0001'0000'0000'0000, 1, 0 },
		{ 0x0001'0000'0000'0000, 0x0001'0000'0000'0001, 0, 0x0001'0000'0000'0000 },

		{ 0x0100'0000'0000'0000, 0x00FF'FFFF'FFFF'FFFF, 1, 1 },
		{ 0x0100'0000'0000'0000, 0x0100'0000'0000'0000, 1, 0 },
		{ 0x0100'0000'0000'0000, 0x0100'0000'0000'0001, 0, 0x0100'0000'0000'0000 }
	};
	constexpr size_t nrTests = sizeof(tests) / sizeof(TestRecord);

	int nrOfFailedTests = 0;
	for (size_t i = 0; i < nrTests; ++i) {
		std::int64_t _a, _b, _q, _r;
		_a = tests[i].a;
		_b = tests[i].b;
		_q = tests[i].q;
		_r = tests[i].r;
		//std::cout << "div " << (_a / _b) << " rem " << (_a % _b) << '\n';
		sw::universal::einteger<uint8_t> a(_a), b(_b), q, r;
		q.reduce(a, b, r);
		if ((long long)q != _q || (long long)r != _r) {
			std::cout << "FAIL: " << std::hex << "0x" << _a << " / 0x" << _b << std::dec << '\n';
			std::cout << "div " << (_a / _b) << " rem " << (_a % _b) << '\n';
			std::cout << _a << " / " << _b << " = " << _q << " with remainder " << _r << '\n';
			std::cout << (long long)a << " / " << (long long)b << " = " << (long long)q << " with remainder " << (long long)r << '\n';
			++nrOfFailedTests;
		}
	}
	return nrOfFailedTests;
}

template<typename BlockType>
void PrintPowersOfTwo(unsigned exponent = 100) {
	constexpr size_t COLUMN_WIDTH = 35;
	sw::universal::einteger<BlockType> a(1);
	for (int p = 0; p < exponent; ++p) {
		std::cout << std::setw(COLUMN_WIDTH) << std::oct << a << std::setw(COLUMN_WIDTH) << std::dec << a << std::setw(COLUMN_WIDTH) << std::hex << a << '\n';
		a += a;
	}
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

	std::string test_suite  = "elastic precision binary integer division";
	std::string test_tag    = "einteger division";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
//	bool bReportIndividualTestCases = false;

	// generate individual testcases to hand trace/debug
//	GenerateTestCase(1, 2);

	PrintPowersOfTwo<uint8_t>();
	PrintPowersOfTwo<uint16_t>();
	PrintPowersOfTwo<uint32_t>();
	
	{
		einteger<uint8_t> a;
		a = 16;
		std::cout << std::oct << a << '\n';
		std::cout << std::hex << a << '\n';
		std::cout << std::dec << a << '\n';
		einteger<uint32_t> b;
		b = 16;
		std::cout << b << '\n';
		einteger<uint16_t> c;
		c = 16;
		std::cout << c << '\n';
	}


	{
		einteger<uint16_t> a, b, q, r;
		a = 16;
		b = 10000;
		q.reduce(a, b, r);
		std::cout << "a   : " << to_binary(a) << " : " << (long long)(a) << '\n';
		std::cout << "b   : " << to_binary(b) << " : " << (long long)(b) << '\n';
		std::cout << "q   : " << to_binary(q) << " : " << (long long)(q) << '\n';
		std::cout << "r   : " << to_binary(r) << " : " << (long long)(r) << '\n';
	}


	{
		einteger a;
		a.assign("633825300114114700748351602688");
		std::cout << std::setw(50) << std::right << a << '\n';
	}

	{
		std::int32_t _a, _b, _q, _r;
		einteger<uint8_t> a, b, q, r;
		_a = 0x08040201;
		_b = 0x0804;
		_q = _a / _b;
		_r = _a % _b;
		a = _a;
		b = _b;
		q.reduce(a, b, r);
		std::cout << "a   : " << to_binary(a) << " : " << (long long)(a) << '\n';
		std::cout << "b   : " << to_binary(b) << " : " << (long long)(b) << '\n';
		std::cout << "q   : " << to_binary(q) << " : " << (long long)(q) << '\n';
		std::cout << "r   : " << to_binary(r) << " : " << (long long)(r) << '\n';

		std::cout << "_a  : " << to_binary(_a, 32, true) << " : " << _a << '\n';
		std::cout << "_b  : " << to_binary(_b, 32, true) << " : " << _b << '\n';
		std::cout << "_q  : " << to_binary(_q, 32, true) << " : " << _q << '\n';
		std::cout << "_r  : " << to_binary(_r, 32, true) << " : " << _r << '\n';
	}

	nrOfFailedTestCases += ReportTestResult(VerifyElasticDivision<uint8_t>(8, reportTestCases), "einteger<uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyElasticDivision<uint16_t>(8, reportTestCases), "einteger<uint16_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyElasticDivision<uint32_t>(8, reportTestCases), "einteger<uint32_t>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyElasticDivision<uint8_t>(16, reportTestCases), "einteger<uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyElasticDivision<uint16_t>(16, reportTestCases), "einteger<uint16_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyElasticDivision<uint32_t>(16, reportTestCases), "einteger<uint32_t>", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else


	//The testing strategy for einteger's creates directed tests
	//that enumerate the boundary conditions of the algorithm.

	//The single limb configurations are scanned exhaustively.

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += DirectedTests();
	nrOfFailedTestCases += ReportTestResult(VerifyElasticDivision<16, uint8_t>(reportTestCases), "einteger<uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyElasticDivision<16, uint16_t>(reportTestCases), "einteger<uint16_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyElasticDivision<32, uint32_t>(reportTestCases), "einteger<uint32_t>", test_tag);
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyElasticDivision<32, uint8_t>(reportTestCases), "einteger<uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyElasticDivision<32, uint16_t>(reportTestCases), "einteger<uint16_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyElasticDivision<32, uint32_t>(reportTestCases), "einteger<uint32_t>", test_tag);
#endif

#if REGRESSION_LEVEL_3

#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyElasticDivision<60, uint32_t>(reportTestCases), "einteger<uint32_t>", test_tag);
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
