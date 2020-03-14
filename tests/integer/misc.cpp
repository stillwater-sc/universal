//  misc.cpp : miscellaneous tests for abitrary precision integers
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>
// configure the integer arithmetic class
#define INTEGER_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/integer/integer.hpp>
#include <universal/integer/numeric_limits.hpp>
#include <universal/integer/integer_functions.hpp>
// is representable
#include <universal/functions/isrepresentable.hpp>
// test helpers, such as, ReportTestResults
#include "../utils/test_helpers.hpp"

/*
   The goal of the arbitrary integers is to provide a constrained big integer type
   that enables fast computation with exceptions for overflow, so that the type
   can be used for forward error analysis studies.
*/

/*
// integer power function, for efficiency
template<size_t nbits, typename BlockType>
integer<nbits, BlockType> ipow(integer<nbits, BlockType> base, integer<nbits, BlockType> exp) {
	integer<nbits, BlockType> result = 1;
	for (;;) {
		if (exp.isodd()) {    // if (exp & 1)
			result *= base;
		}
		exp >>= 1;
		if (exp.iszero()) {   // if (!exp)
			break;
		}
		base *= base;
	}

	return result;
}*/

void TestSizeof() {
	using namespace std;
	using namespace sw::unum;

	cout << endl << "TestSizeof" << endl;
	bool pass = true;
	using int8 = integer<8, uint8_t>;
	using int64 = integer<64, uint32_t>;
	using int128 = integer<128, uint32_t>;
	using int1024 = integer<1024, uint32_t>;

	int8 a;
	int64 k;
	int128 m;
	int1024 o;

	constexpr int WIDTH = 30;
	cout << setw(WIDTH) << typeid(a).name() << "  size in bytes " << a.nrBytes << endl;
	cout << setw(WIDTH) << typeid(k).name() << "  size in bytes " << k.nrBytes << endl;
	cout << setw(WIDTH) << typeid(m).name() << "  size in bytes " << m.nrBytes << endl;
	cout << setw(WIDTH) << typeid(o).name() << "  size in bytes " << o.nrBytes << endl;
	if (a.nrBytes != sizeof(a)) pass = false;
	if (k.nrBytes != sizeof(k)) pass = false;
	if (m.nrBytes != sizeof(m)) pass = false;
	if (o.nrBytes != sizeof(o)) pass = false;

	cout << (pass ? "PASS" : "FAIL") << endl;
}

void TestConversion() {
	using namespace std;
	using namespace sw::unum;

	cout << endl << "TestConversion" << endl;

	integer<128, uint32_t> i1, i2, i3;

	bool pass = true;
	constexpr int iconst = 123456789;
	i1 = iconst;
	int64_t ll = int64_t(i1);
	cout << "integer  " << i1 << endl;
	if (iconst != ll) pass = false;
	i2 = 1.23456789e8;
	cout << "double   " << i2 << " TBD " << endl;
	//i3.parse("123456789");

	cout << (pass ? "PASS" : "FAIL") << endl;
}

void TestFindMsb() {
	using namespace std;
	using namespace sw::unum;

	cout << endl << "TestFindMsb" << endl;
	bool pass = true;
	integer<32, uint32_t> a = 0xD5555555;
	int golden_ref[] = { 31, 30, 28, 26, 24, 22, 20, 18, 16, 14, 12, 10, 8, 6, 4, 2, 0, -1 };
	for (int i = 0; i < int(sizeof(golden_ref)/sizeof(int)); ++i) {
		int msb = findMsb(a);
		cout << "msb of " << to_binary(a) << " is " << msb << endl;
		if (msb >= 0) a.reset(msb);
		if (msb != golden_ref[i]) pass = false;
	}

	cout << (pass ? "PASS" : "FAIL") << endl;
}

// enumerate a couple ratios to test representability
void ReproducibilityTestSuite() {
	for (int i = 0; i < 30; i += 3) {
		for (int j = 0; j < 70; j += 7) {
			sw::unum::reportRepresentability(i, j);
		}
	}
}


#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main()
try {
	using namespace std;
	using namespace sw::unum;

	std::string tag = "Miscellaneous integer tests failed";

#if MANUAL_TESTING


	using int1024 = integer<1024, uint32_t>;
	int1024 a, b, c;

	a = 1024;
	b = 2;
	c = a * a * a;
	cout << "1K ^ 2 = " << ipow(a, b) << " reference : " << 1024 * 1024 << endl;
	long long oneK = 1024;
	long long oneM = oneK * oneK;
	long long oneG = oneK * oneM;
	cout << "1G ^ 2 = " << ipow(c, b) << " reference : " << (oneG * oneG) << " diff : " << (ipow(c, b) - (oneG * oneG)) << endl;
	cout << "1G  = " << c << endl;
	cout << "2G  = " <<  2 * c << endl;
	cout << "4G  = " <<  4 * c << endl;
	cout << "8G  = " <<  8 * c << endl;
	cout << "16G = " << 16 * c << endl;
	cout << "done" << endl;

#else // !MANUAL_TESTING

	std::cout << "Miscellaneous integer function verfication" << std::endl;

	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	TestSizeof();
	TestConversion();
	TestFindMsb();
	ReproducibilityTestSuite();

#if STRESS_TESTING

#endif // STRESS_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << msg << '\n';
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
