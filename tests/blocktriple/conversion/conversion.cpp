// conversion.cpp: test suite runner for blocktriple conversions
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>

// minimum set of include files to reflect source code dependencies
#include <universal/native/integers.hpp>
#include <universal/internal/blocktriple/blocktriple.hpp>

template<size_t fbits, typename Ty>
std::string convert(Ty f) {
	std::stringstream s;
	sw::universal::blocktriple<fbits> a(f);
	s << std::setw(30) << sw::universal::to_binary(a) << " : " << a;
	return s.str();
}

template<size_t fbits, typename ConversionType>
int VerifyConversion() {
	using namespace std;
	using namespace sw::universal;

	cout << ' ' << typeid(ConversionType).name() << " to and from blocktriple<" << fbits << ", uint8_t>    ";
	int nrOfFailures = 0;
	blocktriple<fbits, uint8_t> a, nut;
	constexpr size_t NR_VALUES = (1ull << fbits + 1);
	for (size_t i = 0; i < NR_VALUES; ++i) {
		if (i == 0) a.setzero(); else a.setnormal();
		a.setbits(i);
		ConversionType v = ConversionType(a);
		nut = v;
		if (v != float(nut)) {
			++nrOfFailures;
			cout << setw(10) << i << " : " << to_binary(a) << " != " << to_binary(nut) << '\n';
		}
	}
	cout << (nrOfFailures ? "FAIL\n" : "PASS\n");
	return nrOfFailures;
}

// conditional compile flags
#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	print_cmd_line(argc, argv);

	//bool bReportIndividualTestCases = true;
	int nrOfFailedTestCases = 0;

	std::string tag = "blocktriple conversion validation: ";

#if MANUAL_TESTING

	float f;
	f = 511.875f;
	cout << to_binary(f, true) << '\n';
	cout << convert<12,float>(f) << '\n';
	cout << convert<11, float>(f) << '\n';
	cout << convert<10, float>(f) << '\n';
	cout << convert<9, float>(f) << '\n';
	cout << convert<8, float>(f) << '\n';

	cout << "convert floats\n";
	f = 1.0f;
	for (int i = 0; i < 10; ++i) {
		cout << convert<12, float>(f) << '\n';
		f *= 2.0f;
	}
	cout << "rounding floats\n";
	cout << convert<1, float>(15.0f) << '\n'; // 16
	cout << convert<2, float>(15.0f) << '\n'; // 16
	cout << convert<3, float>(15.0f) << '\n'; // 15
	cout << convert<4, float>(15.0f) << '\n'; // 15
	cout << convert<5, float>(15.0f) << '\n'; // 15
	
	///////////////////////////////////////////////////
	cout << "convert doubles\n";
	double d;
	d = 1.0;
	for (int i = 0; i < 10; ++i) {
		cout << convert<12, double>(d) << '\n';
		d *= 2.0;
	}
	cout << "rounding doubles\n";
	cout << convert<1, double>(15.0) << '\n'; // 16
	cout << convert<2, double>(15.0) << '\n'; // 16
	cout << convert<3, double>(15.0) << '\n'; // 15
	cout << convert<4, double>(15.0) << '\n'; // 15
	cout << convert<5, double>(15.0) << '\n'; // 15

	///////////////////////////////////////////////////
	cout << "convert long long with nbits = 10\n";
	for (long long i = 1; i < 257; i *= 2) {
		cout << convert<10, long long>(-i) << '\n';
	}
	for (long long i = 1; i < 257; i *= 2) {
		cout << convert<10, long long>(i) << '\n';
	}
	{
		constexpr long long maxpos = std::numeric_limits<long long>::max();
		cout << convert<10, long long>(maxpos) << " : " << maxpos << " : " << to_binary(maxpos, 64, true) << '\n';
		cout << convert<10, long long>(-maxpos) << " : " << -maxpos << '\n';
		float fmaxpos = float(maxpos);
		cout << convert<10, float>(fmaxpos) << " : " << fmaxpos << '\n';
	}

	cout << "convert unsigned long long with nbits = 32\n";
	for (unsigned long long i = 1; i < 257; i *= 2) {
		cout << convert<32, unsigned long long>(i) << '\n';
	}
	{
		constexpr unsigned long long maxpos = std::numeric_limits<unsigned long long>::max();
		cout << convert<10, unsigned long long>(maxpos) << " : " << maxpos << " : " << to_binary(maxpos, 64, true) << '\n';
		float fmaxpos = float(maxpos);
		cout << convert<10, float>(fmaxpos) << " : " << fmaxpos << '\n';
	}

	///////////////////////////////////////////////////
	cout << "rounding signed integers\n";
	long l = 0xFFF;
	cout << to_binary(l, 16) << " : " << l << '\n';
	cout << convert<16, long>(l) << '\n';
	cout << convert<13, long>(l) << '\n';
	cout << convert<12, long>(l) << '\n';
	cout << convert<11, long>(l) << '\n';
	cout << convert<8, long>(l) << '\n';

#if STRESS_TESTING

	// manual exhaustive test

#endif

#else  // !MANUAL_TESTING

	cout << tag << endl;

	nrOfFailedTestCases += VerifyConversion<5, float>();
	nrOfFailedTestCases += VerifyConversion<9, float>();
	nrOfFailedTestCases += VerifyConversion<12, float>();

	nrOfFailedTestCases += VerifyConversion<5, double>();
	nrOfFailedTestCases += VerifyConversion<9, double>();
	nrOfFailedTestCases += VerifyConversion<12, double>();

	for (int i = 1; i < 257; i *= 2) {
		blocktriple<9, uint8_t> b = i;
		float f = float(i);
		blocktriple<9, uint8_t> nut = f;
		if (f != float(nut)) {
			++nrOfFailedTestCases;
		}
	}

	cout << tag << ((0 == nrOfFailedTestCases) ? "PASS\n" : "FAIL\n");

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
