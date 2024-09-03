// bit_manipulation.cpp : test runner for bit manipulation of abitrary precision fixed-size integers
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <iomanip>
#include <string>
// configure the integer arithmetic class
#define INTEGER_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/integer/integer.hpp>
#include <universal/verification/integer_test_suite.hpp>

/*
   The goal of the arbitrary integers is to provide a constrained big integer type
   that enables fast computation with exceptions for overflow, so that the type
   can be used for forward error analysis studies.
*/

namespace sw { namespace universal {

	// TODO: we need to add a type_trait to integer<> so that we can SFINAE like this:
	// 	template<typename UnsignedInteger,
	//           typename = typename std::enable_if< std::is_unsigned<UnsignedInteger>::value, UnsignedInteger >::type >
	//  int VerifyNLZ(bool reportTestCases) { .. }

	template<unsigned nbits, typename BlockType>
	int VerifyFindMsb(bool reportTestCases) {
		using Integer = sw::universal::integer<nbits, BlockType>;
		int nrOfFailedTests = 0;

		Integer a(0);
		int msb = findMsb(a);
		if (reportTestCases) std::cout << to_binary(a, true) << " : msb at " << msb << '\n';
		if (msb != -1) ++nrOfFailedTests;
		a.setbit(0u);
		for (unsigned i = 0; i < nbits; ++i) {
			msb = findMsb(a);
			if (reportTestCases) std::cout << to_binary(a, true) << " : msb at " << msb << '\n';
			if (msb != static_cast<int>(i)) ++nrOfFailedTests;
			a <<= 1;
		}

		return nrOfFailedTests;
	}

} } // namespace sw::universal

// test the nlz method which returns the shift required to move the leading non-zero into the most significant bit position of the type
void TestNLZ() {
	using namespace sw::universal;
	{
		uint8_t a = 0x1;
		for (uint32_t i = 0; i < 8; ++i) {
			int shift = nlz(a);
			std::cout << " shift = " << shift << " : " << to_binary(a, true, 8) << '\n';
			a <<= 1;
		}
	}

	{
		uint16_t a = 0x1;
		for (uint32_t i = 0; i < 16; ++i) {
			int shift = nlz(a);
			std::cout << " shift = " << shift << " : " << to_binary(a, true, 16) << '\n';
			a <<= 1;
		}
	}
	{
		uint32_t a = 0x1;
		for (uint32_t i = 0; i < 32; ++i) {
			int shift = nlz(a);
			std::cout << " shift = " << shift << " : " << to_binary(a, true, 32) << '\n';
			a <<= 1;
		}
	}
	{
		uint64_t a = 0x1;
		for (uint32_t i = 0; i < 64; ++i) {
			int shift = nlz(a);
			std::cout << " shift = " << shift << " : " << to_binary(a, true, 64) << '\n';
			a <<= 1;
		}
	}
}

template<size_t nbits, typename BlockType>
void TestSignBitMask() {
	sw::universal::integer<nbits, BlockType> a{};
	std::cout << std::right << std::setw(50) << type_tag(a) << '\n';
	std::cout << "EXACT_FIT           : " << (a.EXACT_FIT ? "yes\n" : "no\n");
	std::cout << "bitsInBlock         : " << a.bitsInBlock << '\n';
	std::cout << "bitSurplus          : " << a.bitSurplus << '\n';
	std::cout << "bitsInMSU           : " << a.bitsInMSU << '\n';
	std::cout << "signBitShift        : " << a.signBitShift << '\n';
	std::cout << "SIGN_BIT_MASK       : " << sw::universal::to_binary(a.SIGN_BIT_MASK, a.bitsInBlock) << '\n';
	std::cout << "SIGN_EXTENTION_BITS : " << sw::universal::to_binary(a.SIGN_EXTENTION_BITS, a.bitsInBlock) << '\n';
	std::cout << "MSU_MASK            : " << sw::universal::to_binary(a.MSU_MASK, a.bitsInBlock) << '\n';
}

void TestBitMasks() {
	TestSignBitMask<3, uint8_t>();
	TestSignBitMask<4, uint8_t>();
	TestSignBitMask<5, uint8_t>();
	TestSignBitMask<6, uint8_t>();
	TestSignBitMask<7, uint8_t>();
	TestSignBitMask<8, uint8_t>();
	TestSignBitMask<9, uint8_t>();
	TestSignBitMask<10, uint8_t>();
	TestSignBitMask<11, uint8_t>();
	TestSignBitMask<12, uint8_t>();

	TestSignBitMask<12, uint16_t>();
	TestSignBitMask<16, uint16_t>();
	TestSignBitMask<28, uint32_t>();
	TestSignBitMask<32, uint32_t>();
	TestSignBitMask<56, uint64_t>();
	TestSignBitMask<60, uint64_t>();
	TestSignBitMask<64, uint64_t>();
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
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "Integer bit manipulation verification";
	std::string test_tag    = "bit manipulators";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	{
		using Integer = integer<16, uint16_t>;
		constexpr Integer a(SpecificValue::maxpos), b(SpecificValue::maxneg);
		int i = int(b);
		std::cout << i << '\n';
		std::cout << b << '\n';
		Integer c;
		c = a + b;
		std::cout << a << " + " << b << " = " << c << '\n';
		std::cout << to_binary(a, true) << " + " << to_binary(b, true) << " = " << to_binary(c) << '\n';
	}

	TestNLZ();  // TODO: we should have a native int

	TestBitMasks();

	nrOfFailedTestCases += ReportTestResult(VerifyFindMsb< 40, uint64_t>(reportTestCases), "integer< 40, uint64_t>", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	test_tag = "findMsb";
	nrOfFailedTestCases += ReportTestResult(VerifyFindMsb<  4, uint8_t >(reportTestCases), "integer<  4, uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyFindMsb<  8, uint8_t >(reportTestCases), "integer<  8, uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyFindMsb< 12, uint8_t >(reportTestCases), "integer< 12, uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyFindMsb< 20, uint16_t>(reportTestCases), "integer< 20, uint16_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyFindMsb< 40, uint16_t>(reportTestCases), "integer< 40, uint16_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyFindMsb< 40, uint32_t>(reportTestCases), "integer< 40, uint32_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyFindMsb< 40, uint64_t>(reportTestCases), "integer< 40, uint64_t>", test_tag);
#endif

#if REGRESSION_LEVEL_2
	
#endif

#if REGRESSION_LEVEL_3
	
#endif

#if	REGRESSION_LEVEL_4
	
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
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
