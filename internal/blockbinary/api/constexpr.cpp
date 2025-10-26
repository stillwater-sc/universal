//  constexpr.cpp : compile-time tests for constexpr of blockbinary type
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/long_double.hpp>
#include <iostream>

#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "blockbinary constexpr compile-time validation";
	std::string test_tag    = "constexpr";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	{
		constexpr blockbinary<8, uint8_t> b8_1b(0x5555);
		constexpr blockbinary<8, uint16_t> b8_2b(0x5555);
		constexpr blockbinary<8, uint32_t> b8_4b(0x5555);

		std::cout << b8_1b << '\n' << b8_2b << '\n' << b8_4b << '\n';
	}

	{
		constexpr blockbinary<16, uint8_t> b16_2(0x5555);
		constexpr blockbinary<16, uint16_t> b16_1(0x5555);
		constexpr blockbinary<16, uint32_t> b16_4b(0x5555);

		std::cout << b16_1 << '\n' << b16_2 << '\n' << b16_4b << '\n';
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
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
