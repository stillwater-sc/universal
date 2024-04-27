// attributes.cpp: attribute tests for fixed-sized arbitrary configuration fixed-points
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// Configure the fixpnt template environment
// first: enable general or specialized configurations
#define FIXPNT_FAST_SPECIALIZATION
// second: enable/disable arithmetic exceptions
#define FIXPNT_THROW_ARITHMETIC_EXCEPTION 1
// third: enable support for native literals in logic and arithmetic operations
#define FIXPNT_ENABLE_LITERALS 1
// minimum set of include files to reflect source code dependencies
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/verification/test_reporters.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "fixpnt attribute functions";
	std::string test_tag    = "attributes";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	/////////////////////////////////////////////////////////////////////////////////////
	//// fixpnt attribute functions

	{
		std::cout << "Dynamic ranges of different fixpnt configurations\n";
		std::cout << dynamic_range< fixpnt<  8,  4> >() << '\n';
		std::cout << dynamic_range< fixpnt< 16,  8> >() << '\n';
		std::cout << dynamic_range< fixpnt< 32, 16> >() << '\n';
		std::cout << dynamic_range< fixpnt< 64, 32> >() << '\n';
		std::cout << dynamic_range< fixpnt<128, 64> >() << '\n';
		std::cout << dynamic_range< fixpnt<256,128> >() << '\n';
	}

	{
		std::cout << "Dynamic ranges of different fixpnt configurations\n";
		std::cout << minmax_range< fixpnt<  8,  4> >() << '\n';
		std::cout << minmax_range< fixpnt< 16,  8> >() << '\n';
		std::cout << minmax_range< fixpnt< 32, 16> >() << '\n';
		std::cout << minmax_range< fixpnt< 64, 32> >() << '\n';
		std::cout << minmax_range< fixpnt<128, 64> >() << '\n';
		std::cout << minmax_range< fixpnt<256,128> >() << '\n';
	}

	{
		std::cout << "Dynamic ranges of different fixpnt configurations\n";
		std::cout << symmetry_range< fixpnt<  8,  4> >() << '\n';
		std::cout << symmetry_range< fixpnt< 16,  8> >() << '\n';
		std::cout << symmetry_range< fixpnt< 32, 16> >() << '\n';
		std::cout << symmetry_range< fixpnt< 64, 32> >() << '\n';
		std::cout << symmetry_range< fixpnt<128, 64> >() << '\n';
		std::cout << symmetry_range< fixpnt<256,128> >() << '\n';
	}

	{
		std::cout << "Number traits\n";
		numberTraits< fixpnt<32, 16> >(std::cout);
		std::cout << '\n';
	}

	{
		std::cout << "Comparitive Number traits\n";
		compareNumberTraits< fixpnt<24, 12>, fixpnt<24, 16> >(std::cout);
		std::cout << '\n';
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Uncaught universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Uncaught universal internal exception: " << err.what() << std::endl;
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
