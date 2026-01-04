//  api.cpp : test suite runner for blockfraction application programming interface tests
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/long_double.hpp>
#include <iostream>

#include <universal/native/integers.hpp>
#include <universal/internal/blockfraction/blockfraction.hpp>
#include <universal/verification/test_suite.hpp>

/*
A blockfraction is a 1's complement binary encoding with a radix point that is aligned
with the hidden bit of the fraction encoding in a floating-point representation.

The main goal of the blockfraction abstraction is to support arbitrary floating-point 
number systems with a high-quality, high-performance, binary-to-decimal string conversion.

*/


int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "blockfraction API examples";
	std::string test_tag    = "API";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	{
		std::cout << "Construction\n";
		blockfraction<26, uint32_t> sp; // default creates a 26 bit fraction of the format .fffff, that is, radix point after bit 26
		std::cout << to_binary(sp, true) << " : " << sp << '\n';
		sp.setradix(25); // bring the radix point in to 0.fffff
		std::cout << to_binary(sp, true) << " : " << sp << '\n';
		sp.setradix(24); // bring the radix point in to 00.fffff
		std::cout << to_binary(sp, true) << " : " << sp << '\n';
		sp.setradix(23); // bring the radix point in to 000.fffff
		std::cout << to_binary(sp, true) << " : " << sp << '\n';

		sp.setbit(22); // with radix at bit 23, set value to 0.5
		std::cout << to_binary(sp, true) << " : " << sp << '\n';
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
