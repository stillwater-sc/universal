// manipulators.cpp: manipulator tests for arbitrary configuration posit types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// Configure the posit template environment
// first: enable general or specialized configurations
#define POSIT_FAST_SPECIALIZATION
// second: enable/disable arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
// third: enable support for native literals in logic and arithmetic operations
#define POSIT_ENABLE_LITERALS 1
// fourth: enable/disable error-free serialization I/O
#define POSIT_ERROR_FREE_IO_FORMAT 0
// minimum set of include files to reflect source code dependencies
#include <universal/number/posit/posit.hpp>
#include <universal/verification/test_reporters.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "generalized posit manipulator functions";
	std::string test_tag    = "manipulators";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	/////////////////////////////////////////////////////////////////////////////////////
	//// posit manipulators

	{
		// report the type
		posit<8, 2, std::uint8_t> p8;
		std::cout << type_tag(p8) << '\n';
	}

	// verify NaR representation across display functions (issue #559)
	{
		int start = nrOfFailedTestCases;
		posit<8, 2> p8;  p8.setnar();
		posit<16, 2> p16; p16.setnar();
		posit<32, 2> p32; p32.setnar();
		// to_binary: NaR has sign=1, regime=all zeros (nbits-1 bits), empty exponent and fraction
		if (to_binary(p8)  != "0b1.0000000..") ++nrOfFailedTestCases;
		if (to_binary(p16) != "0b1.000000000000000..") ++nrOfFailedTestCases;
		if (to_binary(p32) != "0b1.0000000000000000000000000000000..") ++nrOfFailedTestCases;
		// verify nibbleMarker is honored
		if (to_binary(p8, true)  != "0b1.000'0000..") ++nrOfFailedTestCases;
		if (to_binary(p16, true) != "0b1.000'0000'0000'0000..") ++nrOfFailedTestCases;
		// to_triple: NaR has no meaningful triple decomposition
		if (to_triple(p8)  != "(nar)") ++nrOfFailedTestCases;
		if (to_triple(p16) != "(nar)") ++nrOfFailedTestCases;
		if (to_triple(p32) != "(nar)") ++nrOfFailedTestCases;
		// color_print: NaR shows decoded fields (sign=1, regime=all zeros)
		{
			std::string cp = color_print(p8);
			std::string stripped;
			for (size_t i = 0; i < cp.size(); ++i) {
				if (cp[i] == '\033') { while (i < cp.size() && cp[i] != 'm') ++i; }
				else stripped += cp[i];
			}
			if (stripped != "10000000") ++nrOfFailedTestCases;
		}
		if (nrOfFailedTestCases - start > 0) {
			std::cout << "FAIL: NaR representation in display functions (issue #559)\n";
			std::cout << "  posit<8,2>  to_binary:     " << to_binary(p8)  << '\n';
			std::cout << "  posit<16,2> to_binary:     " << to_binary(p16) << '\n';
			std::cout << "  posit<32,2> to_binary:     " << to_binary(p32) << '\n';
			std::cout << "  posit<8,2>  w/ marker:     " << to_binary(p8, true)  << '\n';
			std::cout << "  posit<16,2> w/ marker:     " << to_binary(p16, true) << '\n';
			std::cout << "  posit<8,2>  to_triple:     " << to_triple(p8) << '\n';
			std::cout << "  posit<8,2>  color_print:   " << color_print(p8) << '\n';
		}
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_internal_exception& err) {
	std::cerr << "Uncaught posit internal exception: " << err.what() << std::endl;
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
