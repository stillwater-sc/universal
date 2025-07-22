// misc.cpp : miscellaneous tests for abitrary fixed-size integers
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

#include <iostream>
#include <string>
// configure the integer arithmetic class
#define INTEGER_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/integer/integer.hpp>
// is representable
#include <math/functions/isrepresentable.hpp>
#include <universal/verification/test_status.hpp>
#include <universal/verification/test_suite.hpp>

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
	using namespace sw::universal;

	std::cout << "\nTestSizeof\n";
	bool pass = true;
	using int8 = integer<8, uint8_t>;
	using int64 = integer<64, uint32_t>;
	using int128 = integer<128, uint32_t>;
	using int1024 = integer<1024, uint32_t>;

	int8 a{};
	int64 k{};
	int128 m{};
	int1024 o{};

	constexpr int WIDTH = 30;
	std::cout << std::setw(WIDTH) << type_tag(a) << "  size in bytes " << a.nbits / 8 << '\n';
	std::cout << std::setw(WIDTH) << type_tag(k) << "  size in bytes " << k.nbits / 8 << '\n';
	std::cout << std::setw(WIDTH) << type_tag(m) << "  size in bytes " << m.nbits / 8 << '\n';
	std::cout << std::setw(WIDTH) << type_tag(o) << "  size in bytes " << o.nbits / 8 << '\n';
	if constexpr (a.nbits >> 3 != sizeof(a)) pass = false;
	if constexpr (k.nbits >> 3 != sizeof(k)) pass = false;
	if constexpr (m.nbits >> 3 != sizeof(m)) pass = false;
	if constexpr (o.nbits >> 3 != sizeof(o)) pass = false;

	std::cout << (pass ? "PASS" : "FAIL") << '\n';
}

void TestConversion() {
	using namespace sw::universal;

	std::cout << "\nTestConversion\n";

	integer<128, uint32_t> i1, i2;

	bool pass = true;
	constexpr int iconst = 123456789;
	i1 = iconst;
	int64_t ll = int64_t(i1);
	std::cout << "integer  " << i1 << '\n';
	if (iconst != ll) pass = false;
	i2 = 1.23456789e8;
	std::cout << "double   " << i2 << " TBD\n";

	// TODO
	//integer<128, uint32_t> i3;
	//i3.parse("123456789");

	std::cout << (pass ? "PASS\n" : "FAIL\n");
}

void TestFindMsb() {
	using namespace sw::universal;

	std::cout << "\nTestFindMsb\n";
	bool pass = true;
	integer<32, uint32_t> a = 0xD5555555;
	int golden_ref[] = { 31, 30, 28, 26, 24, 22, 20, 18, 16, 14, 12, 10, 8, 6, 4, 2, 0, -1 };
	for (int i = 0; i < int(sizeof(golden_ref)/sizeof(int)); ++i) {
		int msb = findMsb(a);
		std::cout << "msb of " << to_binary(a) << " is " << msb << '\n';
		if (msb >= 0) a.setbit(static_cast<unsigned>(msb), false);
		if (msb != golden_ref[i]) pass = false;
	}

	std::cout << (pass ? "PASS\n" : "FAIL\n");
}

// enumerate a couple ratios to test representability
void ReproducibilityTestSuite() {
	for (int i = 0; i < 30; i += 3) {
		for (int j = 0; j < 70; j += 7) {
			sw::universal::reportRepresentability(i, j);
		}
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
#define REGRESSION_LEVEL_2 0
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "integer class API ";
	std::string test_tag    = "integer";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	using int1024 = integer<1024, uint32_t>;
	int1024 a, b, c, zero(0);

	a = 1024;
	b = 2;
	c = a * a * a;
	std::cout << "1K ^ 2 = " << ipow(a, b) << " reference : " << 1024 * 1024 << '\n';
	long long oneK = 1024;
	long long oneM = oneK * oneK;
	long long oneG = oneK * oneM;
	std::cout << "1G ^ 2 = " << ipow(c, b) << " reference : " << (oneG * oneG) << " diff : " << (ipow(c, b) - (oneG * oneG)) << '\n';
	std::cout << "1G  = " << c << '\n';
	std::cout << "2G  = " <<  2 * c << '\n';
	std::cout << "4G  = " <<  4 * c << '\n';
	std::cout << "8G  = " <<  8 * c << '\n';
	std::cout << "16G = " << 16 * c << '\n';

	{
		constexpr unsigned nbits = 128;
		integer<nbits, std::uint32_t> d;
		d.clear();
		d.setbit(nbits - 1);
		std::cout << "maxneg = " << to_binary(d) << '\n';
		for (unsigned i = 0; i < nbits; ++i) {
			std::cout << d << " : " << to_triple(d) << '\n';
			d /= 2;
		}
	}

	std::cout << '\n';

	std::cout << "a fast zero value: " << zero << '\n';

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else // !MANUAL_TESTING

#if REGRESSION_LEVEL_1
	TestSizeof();
	TestConversion();
	TestFindMsb();
	ReproducibilityTestSuite();
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
	std::cerr << msg << std::endl;
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
