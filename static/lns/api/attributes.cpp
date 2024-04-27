// attributes.cpp: attribute tests for fixed-size arbitrary configuration logarithmic floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// Configure the lns template environment
// first: enable general or specialized configurations
#define LNS_FAST_SPECIALIZATION
// second: enable/disable arithmetic exceptions
#define LNS_THROW_ARITHMETIC_EXCEPTION 1
// third: enable support for native literals in logic and arithmetic operations
#define LNS_ENABLE_LITERALS 1
// minimum set of include files to reflect source code dependencies
#include <universal/number/lns/lns.hpp>
#include <universal/verification/test_reporters.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "logarithmic floating-point attribute functions";
	std::string test_tag    = "attributes";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	/////////////////////////////////////////////////////////////////////////////////////
	//// lns attribute functions

	{
		std::cout << "\nDynamic ranges of logarithmic floating-point arithmetic types\n";
		std::cout << dynamic_range< lns<  8, 2> >() << '\n';
		std::cout << dynamic_range< lns< 16, 5> >() << '\n';
		std::cout << dynamic_range< lns< 32, 8> >() << '\n';
	}

	{
		std::cout << "\nMinmax of logarithmic floating-point\n";
		std::cout << minmax_range< lns< 8, 4> >() << '\n';
		std::cout << minmax_range< lns<16, 8> >() << '\n';
		std::cout << minmax_range< lns<24,12> >() << '\n';
		std::cout << minmax_range< lns<32,16> >() << '\n';
	}
	
	{
		std::cout << "\nDynamic ranges of logarithmic floating-point\n";
		std::cout << symmetry_range< lns< 8, 4> >() << '\n';
		std::cout << symmetry_range< lns<16, 8> >() << '\n';
		std::cout << symmetry_range< lns<24,12> >() << '\n';
		std::cout << symmetry_range< lns<32,16> >() << '\n';
	}

	{
		std::cout << "\nSpecific logarithmic floating-point range function\n";
		std::cout << lns_range(lns<7, 3>()) << '\n';
	}

	{
		std::cout << "\nlns sign() function\n";
		lns<7, 3> a;

		a.setbits(0x7f);
		std::cout << std::setw(45) << type_tag(a) << " : " << to_binary(a) << " : " << a << " : " << (sign(a) ? "sign = 1" : "sign = 0") << '\n';
	}

	{
		std::cout << "\nNumber traits\n";
		numberTraits< lns<32, 16> >(std::cout);
	}

	{
		std::cout << "\nComparitive Number traits\n";
		compareNumberTraits< lns<24, 12>, lns<32, 16> >(std::cout);
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

/*

Dynamic ranges of logarithmic floating-point arithmetic types
lns<  8,   2, Saturating, unsigned char>: minpos scale        -16     maxpos scale         15
[-55109 ... -1.81459e-05, 0, 1.81459e-05 ... 55109]
[0b1.01111.11 ... 0b1.10000.01, 0, 0b0.10000.01 ... 0b0.01111.11]

lns< 16,   5, Saturating, unsigned char>: minpos scale       -512     maxpos scale        511
[-1.31205e+154 ... -7.62166e-155, 0, 7.62166e-155 ... 1.31205e+154]
[0b1.0111111111.11111 ... 0b1.1000000000.00001, 0, 0b0.1000000000.00001 ... 0b0.0111111111.11111]

lns< 32,   8, Saturating, unsigned char>: minpos scale   -4194304     maxpos scale    4194303
[-inf ... -0, 0, 0 ... inf]
[0b1.01111111111111111111111.11111111 ... 0b1.10000000000000000000000.00000001, 0, 0b0.10000000000000000000000.00000001 ... 0b0.01111111111111111111111.11111111]


Dynamic ranges of different logarithmic floating-point
lns<  8,   4, Saturating, unsigned char> : min     0.0652671     max       15.3217
lns< 16,   8, Saturating, unsigned char> : min   5.43571e-20     max   1.83969e+19
lns< 24,  12, Saturating, unsigned char> : min  5.56363e-309     max  1.79739e+308
lns< 32,  16, Saturating, unsigned char> : min             0     max           inf
Dynamic ranges of different logarithmic floating-point
lns<  8,   4, Saturating, unsigned char> : [             -15.3217,           -0.0652671       0             0.0652671,              15.3217]
lns< 16,   8, Saturating, unsigned char> : [         -1.83969e+19,         -5.43571e-20       0           5.43571e-20,          1.83969e+19]
lns< 24,  12, Saturating, unsigned char> : [        -1.79739e+308,        -5.56363e-309       0          5.56363e-309,         1.79739e+308]
lns< 32,  16, Saturating, unsigned char> : [                 -inf,                   -0       0                     0,                  inf]

*/
