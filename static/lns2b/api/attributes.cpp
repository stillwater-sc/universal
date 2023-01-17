// attributes.cpp: attribute tests for fixed-size arbitrary configuration 2-base logarithmic floating-point
//
// Copyright (C) 2017-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// Configure the lns2b template environment
// first: enable general or specialized configurations
#define LNS2B_FAST_SPECIALIZATION
// second: enable/disable arithmetic exceptions
#define LNS2B_THROW_ARITHMETIC_EXCEPTION 1
// third: enable support for native literals in logic and arithmetic operations
#define LNS2B_ENABLE_LITERALS 1
// minimum set of include files to reflect source code dependencies
#include <universal/number/lns2b/lns2b.hpp>
#include <universal/verification/test_reporters.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "2-base logarithmic floating-point attribute functions";
	std::string test_tag    = "attributes";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	/////////////////////////////////////////////////////////////////////////////////////
	//// lns2b attribute functions

	{
		std::cout << "Dynamic ranges of logarithmic floating-point arithmetic types\n";
		std::cout << dynamic_range< lns2b<  8, 6> >() << '\n';
		std::cout << dynamic_range< lns2b< 16,10> >() << '\n';
		std::cout << dynamic_range< lns2b< 32,22> >() << '\n';
		std::cout << '\n';
	}

	{
		std::cout << "Dynamic ranges of different logarithmic floating-point\n";
		std::cout << minmax_range< lns2b< 8, 6> >() << '\n';
		std::cout << minmax_range< lns2b<16,10> >() << '\n';
		std::cout << minmax_range< lns2b<24,16> >() << '\n';
		std::cout << minmax_range< lns2b<32,22> >() << '\n';
		std::cout << '\n';
	}
	
	{
		std::cout << "Dynamic ranges of different logarithmic floating-point\n";
		std::cout << symmetry_range< lns2b< 8, 6> >() << '\n';
		std::cout << symmetry_range< lns2b<16,10> >() << '\n';
		std::cout << symmetry_range< lns2b<24,16> >() << '\n';
		std::cout << symmetry_range< lns2b<32,22> >() << '\n';
		std::cout << '\n';
	}

	{
		std::cout << "Number traits\n";
//		numberTraits< lns2b<32, 16> >(std::cout);
		std::cout << '\n';
	}

	{
		std::cout << "Comparitive Number traits\n";
//		compareNumberTraits< lns2b<24, 12>, lns2b<32, 16> >(std::cout);
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

/*
2-base logarithmic floating-point attribute functions: report test cases
Dynamic ranges of logarithmic floating-point arithmetic types
lns2b<  8,   6, unsigned char, Saturating>                                       : minexp scale 0              maxexp scale 0              minimum 1.0842e-19       maximum 3
lns2b< 16,  10, unsigned char, Saturating>                                       : minexp scale -16            maxexp scale 15             minimum 0                maximum 6.17673e+14
lns2b< 32,  22, unsigned char, Saturating>                                       : minexp scale -256           maxexp scale 255            minimum 0                maximum inf

Dynamic ranges of different logarithmic floating-point
lns2b<  8,   6, unsigned char, Saturating>                                       : min 1.0842e-19        max 3
lns2b< 16,  10, unsigned char, Saturating>                                       : min 0                 max 6.17673e+14
lns2b< 24,  16, unsigned char, Saturating>                                       : min 0                 max inf
lns2b< 32,  22, unsigned char, Saturating>                                       : min 0                 max inf

Dynamic ranges of different logarithmic floating-point
lns2b<  8,   6, unsigned char, Saturating>                                       : [ -3 ... -1.0842e-19  0  1.0842e-19 ... 3]
lns2b< 16,  10, unsigned char, Saturating>                                       : [ -6.17673e+14 ... -0  0  0 ... 6.17673e+14]
lns2b< 24,  16, unsigned char, Saturating>                                       : [ -inf ... -0  0  0 ... inf]
lns2b< 32,  22, unsigned char, Saturating>                                       : [ -inf ... -0  0  0 ... inf]
*/
