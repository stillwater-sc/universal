// conversion.cpp: test suite runner for blocktriple conversions
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/bitcast.hpp>
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>

// minimum set of include files to reflect source code dependencies
#include <universal/native/integers.hpp>
#include <universal/internal/blocktriple/blocktriple.hpp>

template<size_t fbits, sw::universal::BlockTripleOperator op, typename Ty>
std::string convert(Ty f) {
	using default_bt = uint8_t;
	std::stringstream s;
	sw::universal::blocktriple<fbits, op, default_bt> a(f);
	s << std::setw(30) << sw::universal::to_binary(a) << " : " << a << " " << typeid(a).name();
	return s.str();
}

template<size_t fbits, sw::universal::BlockTripleOperator op, typename ConversionType>
int VerifyConversion() {
	using namespace sw::universal;

	std::cout << ' ' << typeid(ConversionType).name() << " to and from blocktriple<" << fbits << ", " << op << ", uint8_t>\n";
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
			std::cout << "FAIL: " << std::setw(10) << i << " : " << to_binary(a) << " != " << to_binary(nut) << '\n';
		}
		else {
			std::cout << "PASS: " << std::setw(10) << i << " : " << to_binary(a) << " == " << to_binary(nut) << '\n';
		}
	}
	std::cout << ' ' << typeid(ConversionType).name() << " to and from blocktriple<" << fbits << ", " << op << ", uint8_t>  ";
	std::cout << (nrOfFailures ? "FAIL\n" : "PASS\n");
	return nrOfFailures;
}

// conditional compile flags
#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	print_cmd_line(argc, argv);

	//bool bReportIndividualTestCases = true;
	int nrOfFailedTestCases = 0;

	std::string tag = "blocktriple conversion validation: ";

#if MANUAL_TESTING

	float f;
	f = 511.875f;
	std::cout << to_binary(f, true) << '\n';
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
		std::cout << convert<10, BlockTripleOperator::ADD, long long>(maxpos) << " : " << maxpos << " : " << to_binary(maxpos, 64, true) << '\n';
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
		std::cout << convert<10, BlockTripleOperator::ADD, unsigned long long>(maxpos) << " : " << maxpos << " : " << to_binary(maxpos, 64, true) << '\n';
		float fmaxpos = float(maxpos);
		std::cout << convert<10, BlockTripleOperator::ADD, float>(fmaxpos) << " : " << fmaxpos << '\n';
	}

	///////////////////////////////////////////////////
	std::cout << "rounding signed integers\n";
	long l = 0xFFF;
	std::cout << to_binary(l, 16) << " : " << l << '\n';
	std::cout << convert<16, BlockTripleOperator::ADD, long>(l) << '\n';
	std::cout << convert<13, BlockTripleOperator::ADD, long>(l) << '\n';
	std::cout << convert<12, BlockTripleOperator::ADD, long>(l) << '\n';
	std::cout << convert<11, BlockTripleOperator::ADD, long>(l) << '\n';
	std::cout << convert<10, BlockTripleOperator::ADD, long>(l) << '\n';
	std::cout << convert<9, BlockTripleOperator::ADD, long>(l) << '\n';
	std::cout << convert<8, BlockTripleOperator::ADD, long>(l) << '\n';

//	nrOfFailedTestCases += VerifyConversion<5, BlockTripleOperator::REPRESENTATION, float>();
	nrOfFailedTestCases += VerifyConversion<5, BlockTripleOperator::ADD, float>();
//	nrOfFailedTestCases += VerifyConversion<5, BlockTripleOperator::MUL, float>();
	nrOfFailedTestCases = 0;

#if STRESS_TESTING

	// manual exhaustive test

#endif

#else  // !MANUAL_TESTING

	std::cout << tag << endl;

	nrOfFailedTestCases += VerifyConversion<5, BlockTripleOperator::ADD, float>();
	nrOfFailedTestCases += VerifyConversion<9, BlockTripleOperator::ADD, float>();
	nrOfFailedTestCases += VerifyConversion<12, BlockTripleOperator::ADD, float>();

	nrOfFailedTestCases += VerifyConversion<5, BlockTripleOperator::ADD, double>();
	nrOfFailedTestCases += VerifyConversion<9, BlockTripleOperator::ADD, double>();
	nrOfFailedTestCases += VerifyConversion<12, BlockTripleOperator::ADD, double>();

	for (int i = 1; i < 257; i *= 2) {
		float f = float(i);
		blocktriple<9, BlockTripleOperator::ADD, uint8_t> nut = f;
		if (f != float(nut)) {
			++nrOfFailedTestCases;
		}
	}

	std::cout << tag << ((0 == nrOfFailedTestCases) ? "PASS\n" : "FAIL\n");

#if STRESS_TESTING

#endif  // STRESS_TESTING

#endif  // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
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
