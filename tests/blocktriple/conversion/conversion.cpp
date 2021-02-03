// conversion.cpp: test suite runner for blocktriple conversions
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#if defined(_MSC_VER)
#pragma warning(disable : 4514) // 'sw::universal::scale': unreferenced inline function has been removed
#pragma warning(disable : 4515) // 'sw::universal::findMostSignificantBit': unreferenced inline function has been removed
#pragma warning(disable : 4820) // 'sw::universal::blocktriple<32,uint32_t>': '3' bytes padding added after data member 'sw::universal::blocktriple<32,uint32_t>::_sign'
#pragma warning(disable : 5045) // Compiler will insert Spectre mitigation for memory load if /Qspectre switch specified
#endif
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>

// minimum set of include files to reflect source code dependencies
#include <universal/native/integers.hpp>
#include <universal/internal/blocktriple/blocktriple.hpp>

template<size_t nbits, typename Ty>
std::string convert(Ty f) {
	std::stringstream s;
	sw::universal::blocktriple<nbits> a(f);
	s << std::setw(30) << sw::universal::to_binary(a) << " : " << a;
	return s.str();
}

// conditional compile flags
#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	for (int i = 0; i < argc; ++i) std::cout << argv[0] << ' ';
	std::cout << std::endl;

	//bool bReportIndividualTestCases = true;
	int nrOfFailedTestCases = 0;

	std::string tag = "conversion: ";

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
	cout << convert<3, float>(15.0f) << '\n'; // 16
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
	cout << convert<3, double>(15.f) << '\n'; // 16
	cout << convert<4, double>(15.0) << '\n'; // 15
	cout << convert<5, double>(15.0) << '\n'; // 15

	///////////////////////////////////////////////////
	cout << "convert long long with nbits = 10\n";
	for (long long i = 1; i < 1025; i *= 2) {
		cout << convert<10, long long>(i) << '\n';
	}
	cout << "convert unsigned long long with nbits = 32\n";
	for (unsigned long long i = 1; i < 1025; i *= 2) {
		cout << convert<32, unsigned long long>(i) << '\n';
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

	cout << "blocktriple conversion validation" << endl;


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
