// conversion.cpp: test suite runner for blocktriple conversions
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/long_double.hpp>
#include <universal/utility/bit_cast.hpp>
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>

// minimum set of include files to reflect source code dependencies
#include <universal/native/integers.hpp>
#include <universal/internal/blocktriple/blocktriple.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal { 

	template<size_t fbits, sw::universal::BlockTripleOperator op, typename Ty>
	std::string convert(Ty f) {
		using default_bt = uint8_t;
		std::stringstream s;
		sw::universal::blocktriple<fbits, op, default_bt> a(f);
		s << std::setw(31) << sw::universal::to_binary(a) << " : " << std::setw(40) << std::left << to_triple(a) << " : " << std::setw(10) << a << " : " << type_tag(a);
		return s.str();
	}

	template<size_t fbits, sw::universal::BlockTripleOperator op, typename ConversionType>
	int VerifyBlocktripleConversion(bool reportTestCases) {
		using namespace sw::universal;

		int nrOfFailures = 0;
		blocktriple<fbits, op, uint8_t> a, nut;
		constexpr size_t NR_VALUES = (1ull << (fbits + 1));
		for (size_t i = 0; i < NR_VALUES; ++i) {
			if (i == 0) a.setzero(); else a.setnormal();
			a.setbits(i);
			ConversionType v = ConversionType(a);
			nut = v;
			if (v != float(nut)) {
				++nrOfFailures;
				if (reportTestCases) std::cout << "FAIL: " << std::setw(10) << i << " : " << to_binary(a) << " != " << to_binary(nut) << '\n';
			}
			else {
				// if (reportTestCases) std::cout << "PASS: " << std::setw(10) << i << " : " << to_binary(a) << " == " << to_binary(nut) << '\n';
			}
		}

		return nrOfFailures;
	}

}}  // namespace sw::universal

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

	std::string test_suite  = "blocktriple conversion validation";
	std::string test_tag    = "conversion";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	{
		std::cout << "\n\n";
		std::cout << convert<8, BlockTripleOperator::REP, float>(1.0f) << '\n';
		std::cout << convert<22, BlockTripleOperator::REP, float>(1.0f) << '\n';
		std::cout << convert<23, BlockTripleOperator::REP, float>(1.0f) << '\n';
		std::cout << convert<32, BlockTripleOperator::REP, float>(1.0f) << '\n';
	}

	float f;
	f = 511.875f;
	std::cout << to_binary(f, true) << '\n';
	std::cout << convert<12, BlockTripleOperator::REP, float>(f) << '\n';
	std::cout << convert<11, BlockTripleOperator::REP, float>(f) << '\n';
	std::cout << convert<10, BlockTripleOperator::REP, float>(f) << '\n';
	std::cout << convert<9, BlockTripleOperator::REP, float>(f) << '\n';
	std::cout << convert<8, BlockTripleOperator::REP, float>(f) << '\n';		

	std::cout << convert<12, BlockTripleOperator::ADD, float>(f) << '\n';
	std::cout << convert<11, BlockTripleOperator::ADD, float>(f) << '\n';
	std::cout << convert<10, BlockTripleOperator::ADD, float>(f) << '\n';
	std::cout << convert<9, BlockTripleOperator::ADD, float>(f) << '\n';
	std::cout << convert<8, BlockTripleOperator::ADD, float>(f) << '\n';

	std::cout << "convert floats\n";
	f = 1.0f;
	for (int i = 0; i < 10; ++i) {
		std::cout << convert<12, BlockTripleOperator::ADD, float>(f) << '\n';
		f *= 2.0f;
	}
	std::cout << "rounding floats\n";
	std::cout << convert<1, BlockTripleOperator::ADD, float>(15.0f) << '\n'; // 16
	std::cout << convert<2, BlockTripleOperator::ADD, float>(15.0f) << '\n'; // 16
	std::cout << convert<3, BlockTripleOperator::ADD, float>(15.0f) << '\n'; // 15
	std::cout << convert<4, BlockTripleOperator::ADD, float>(15.0f) << '\n'; // 15
	std::cout << convert<5, BlockTripleOperator::ADD, float>(15.0f) << '\n'; // 15
	
	///////////////////////////////////////////////////
	std::cout << "convert doubles\n";
	double d;
	d = 1.0;
	for (int i = 0; i < 10; ++i) {
		std::cout << convert<12, BlockTripleOperator::ADD, double>(d) << '\n';
		d *= 2.0;
	}
	std::cout << "rounding doubles\n";
	std::cout << convert<1, BlockTripleOperator::ADD, double>(15.0) << '\n'; // 16
	std::cout << convert<2, BlockTripleOperator::ADD, double>(15.0) << '\n'; // 16
	std::cout << convert<3, BlockTripleOperator::ADD, double>(15.0) << '\n'; // 15
	std::cout << convert<4, BlockTripleOperator::ADD, double>(15.0) << '\n'; // 15
	std::cout << convert<5, BlockTripleOperator::ADD, double>(15.0) << '\n'; // 15

	///////////////////////////////////////////////////
	std::cout << "convert negative long long to blocktriple with fbits = 10\n";
	for (long long i = 1; i < 257; i *= 2) {
		std::cout << convert<10, BlockTripleOperator::ADD, long long>(-i) << '\n';
	}
	std::cout << "convert positive long long to blocktriple with fbits = 10\n";
	for (long long i = 1; i < 257; i *= 2) {
		std::cout << convert<10, BlockTripleOperator::ADD, long long>(i) << '\n';
	}

	std::cout << "maxpos of long long and blocktriple with fbits = 10\n";
	{
		constexpr long long maxpos = std::numeric_limits<long long>::max();
		std::cout << convert<10, BlockTripleOperator::ADD, long long>(maxpos) << " : " << maxpos << " : " << to_binary(maxpos, true, 64) << '\n';
		std::cout << convert<10, BlockTripleOperator::ADD, long long>(-maxpos) << " : " << -maxpos << '\n';
		float fmaxpos = float(maxpos);
		std::cout << convert<10, BlockTripleOperator::ADD, float>(fmaxpos) << " : " << fmaxpos << '\n';
	}

	std::cout << "convert unsigned long long to blocktriple with fbits = 32\n";
	for (unsigned long long i = 1; i < 257; i *= 2) {
		std::cout << convert<32, BlockTripleOperator::ADD, unsigned long long>(i) << '\n';
	}
	std::cout << "maxpos of unsigned long long and blocktriple with fbits = 10\n";
	{
		constexpr unsigned long long maxpos = std::numeric_limits<unsigned long long>::max();
		std::cout << convert<10, BlockTripleOperator::ADD, unsigned long long>(maxpos) << " : " << maxpos << " : " << to_binary(maxpos, true, 64) << '\n';
		float fmaxpos = float(maxpos);
		std::cout << convert<10, BlockTripleOperator::ADD, float>(fmaxpos) << " : " << fmaxpos << '\n';
	}

	///////////////////////////////////////////////////
	std::cout << "rounding signed integers\n";
	long l = 0xFFF;
	std::cout << to_binary(l, true, 16) << " : " << l << '\n';
	std::cout << convert<16, BlockTripleOperator::ADD, long>(l) << '\n';
	std::cout << convert<13, BlockTripleOperator::ADD, long>(l) << '\n';
	std::cout << convert<12, BlockTripleOperator::ADD, long>(l) << '\n';
	std::cout << convert<11, BlockTripleOperator::ADD, long>(l) << '\n';
	std::cout << convert<10, BlockTripleOperator::ADD, long>(l) << '\n';
	std::cout << convert<9, BlockTripleOperator::ADD, long>(l) << '\n';
	std::cout << convert<8, BlockTripleOperator::ADD, long>(l) << '\n';

//	nrOfFailedTestCases += VerifyBlocktripleConversion<5, BlockTripleOperator::REP, float>(reportTestCases);
	nrOfFailedTestCases += VerifyBlocktripleConversion<5, BlockTripleOperator::ADD, float>(reportTestCases);
//	nrOfFailedTestCases += VerifyBlocktripleConversion<5, BlockTripleOperator::MUL, float>(reportTestCases);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else  // !MANUAL_TESTING

#if REGRESSION_LEVEL_1

	constexpr BlockTripleOperator rep = BlockTripleOperator::REP;
	nrOfFailedTestCases += ReportTestResult(VerifyBlocktripleConversion<5, rep, float>(reportTestCases), type_tag(blocktriple<5, rep>()), test_tag + std::string(" to and from float"));
	nrOfFailedTestCases += ReportTestResult(VerifyBlocktripleConversion<9, rep, float>(reportTestCases), type_tag(blocktriple<9, rep>()), test_tag + std::string(" to and from float"));
	nrOfFailedTestCases += ReportTestResult(VerifyBlocktripleConversion<12, rep, float>(reportTestCases), type_tag(blocktriple<12, rep>()), test_tag + std::string(" to and from float"));

	nrOfFailedTestCases += ReportTestResult(VerifyBlocktripleConversion<5, rep, double>(reportTestCases), type_tag(blocktriple<5, rep>()), test_tag + std::string(" to and from double"));
	nrOfFailedTestCases += ReportTestResult(VerifyBlocktripleConversion<9, rep, double>(reportTestCases), type_tag(blocktriple<9, rep>()), test_tag + std::string(" to and from double"));
	nrOfFailedTestCases += ReportTestResult(VerifyBlocktripleConversion<12, rep, double>(reportTestCases), type_tag(blocktriple<12, rep>()), test_tag + std::string(" to and from double"));

	constexpr BlockTripleOperator add = BlockTripleOperator::ADD;
	nrOfFailedTestCases += ReportTestResult(VerifyBlocktripleConversion<5, add, float>(reportTestCases), type_tag(blocktriple<5, add>()), test_tag + std::string(" to and from float"));
	nrOfFailedTestCases += ReportTestResult(VerifyBlocktripleConversion<9, add, float>(reportTestCases), type_tag(blocktriple<9, add>()), test_tag + std::string(" to and from float"));
	nrOfFailedTestCases += ReportTestResult(VerifyBlocktripleConversion<12, add, float>(reportTestCases), type_tag(blocktriple<12, add>()), test_tag + std::string(" to and from float"));

	nrOfFailedTestCases += ReportTestResult(VerifyBlocktripleConversion<5, add, double>(reportTestCases), type_tag(blocktriple<5, add>()), test_tag + std::string(" to and from double"));
	nrOfFailedTestCases += ReportTestResult(VerifyBlocktripleConversion<9, add, double>(reportTestCases), type_tag(blocktriple<9, add>()), test_tag + std::string(" to and from double"));
	nrOfFailedTestCases += ReportTestResult(VerifyBlocktripleConversion<12, add, double>(reportTestCases), type_tag(blocktriple<12, add>()), test_tag + std::string(" to and from double"));

	// test scale progression
	int scaleTestFailures = 0;
	for (int i = 1; i < 1025; i *= 2) {
		float f = float(i);
		blocktriple<9, BlockTripleOperator::ADD, uint8_t> nut(f);
		// std::cout << std::setw(4) << i << " : " << to_binary(nut) << '\n';
		if (f != float(nut)) {
			++scaleTestFailures;
		}
	}
	nrOfFailedTestCases += ReportTestResult(scaleTestFailures, "blocktriple scale progression", "=");

#endif

#if REGRESSION_LEVEL_2
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
	for (int i = 1; i < 257; i *= 2) {
		float f = float(i);
		blocktriple<9, BlockTripleOperator::ADD, uint8_t> nut = f;
		if (f != float(nut)) {
			++nrOfFailedTestCases;
		}
	}
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
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
