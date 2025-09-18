//  constexpr.cpp : compile-time tests for constexpr of blocksignificant type
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/long_double.hpp>
#include <iostream>

#include <universal/internal/blocksignificand/blocksignificand.hpp>
#include <universal/verification/test_suite.hpp>

template<typename blocksignificant>
void ConstexprBlockConstructor(uint64_t pattern) {
	constexpr blocksignificant bf(pattern);
	std::cout << to_binary(bf) << " : " << bf << '\n';
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
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "blocksignificant storage class constexpr compile-time testing";
	std::string test_tag    = "constexpr";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING


	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	{
		constexpr blocksignificant<8, uint8_t> b8_1w( 0x21, 5 );  // == 0b001.0'0001  = 1.03125
		constexpr blocksignificant<8, uint16_t> b8_2b( 0x21, 5 ); // == 0b001.0'0001  = 1.03125
		constexpr blocksignificant<8, uint32_t> b8_4b( 0x21, 5 ); // == 0b001.0'0001  = 1.03125
		std::cout << to_binary(b8_1w, true) << " : " << b8_1w << '\n';
		std::cout << to_binary(b8_2b, true) << " : " << b8_2b << '\n';
		std::cout << to_binary(b8_4b, true) << " : " << b8_4b << '\n';
	}

	{
		constexpr blocksignificant<12, uint8_t>  b12_1w(0x210, 9); // == 0b001.0'0001'0000  = 1.03125
		constexpr blocksignificant<12, uint16_t> b12_2b(0x210, 9); // == 0b001.0'0001'0000  = 1.03125
		constexpr blocksignificant<12, uint32_t> b12_4b(0x210, 9); // == 0b001.0'0001'0000  = 1.03125
		std::cout << to_binary(b12_1w, true) << " : " << b12_1w << '\n';
		std::cout << to_binary(b12_2b, true) << " : " << b12_2b << '\n';
		std::cout << to_binary(b12_4b, true) << " : " << b12_4b << '\n';
	}

	{
		constexpr blocksignificant<16, uint8_t> b16_2b( 0xff, 13 );  // subnormal
		constexpr blocksignificant<16, uint16_t> b16_1w( 0x2001, 13 );
		constexpr blocksignificant<16, uint32_t> b16_4b( 0x2001, 13 );

		std::cout << to_binary(b16_2b, true) << " : " << b16_2b << '\n';
		std::cout << to_binary(b16_1w, true) << " : " << b16_1w << '\n';
		std::cout << to_binary(b16_4b, true) << " : " << b16_4b << '\n';
	}

	{
		constexpr blocksignificant<32, uint8_t> b32_4b( 0xff, 29 );
		constexpr blocksignificant<32, uint16_t> b32_2w( 0x2001, 29 );
		constexpr blocksignificant<32, uint32_t> b32_1w( 0x30000001, 29 ); // == 1.5

		std::cout << to_binary(b32_4b, true) << " : " << b32_4b << '\n';
		std::cout << to_binary(b32_2w, true) << " : " << b32_2w << '\n';
		std::cout << to_binary(b32_1w, true) << " : " << b32_1w << '\n';
	}

	{
		constexpr blocksignificant<32, uint8_t> bf(0xAAAA'AAAA'5AAA'AAAA, 29);
		std::cout << to_binary(bf, true) << " : " << bf << '\n';
	}
	{
		constexpr blocksignificant<32, uint16_t> bf(0xAAAA'AAAA'5AAA'AAAA, 29);
		std::cout << to_binary(bf, true) << " : " << bf << '\n';
	}
	{
		constexpr blocksignificant<32, uint32_t> bf(0xAAAA'AAAA'5AAA'AAAA, 29);
		std::cout << to_binary(bf, true) << " : " << bf << '\n';
	}
	{
		constexpr blocksignificant<32, uint64_t> bf(0xAAAA'AAAA'5AAA'AAAA, 29);
		std::cout << to_binary(bf, true) << " : " << bf << '\n';
	}
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

	return EXIT_SUCCESS;
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
