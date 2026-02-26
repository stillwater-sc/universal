// attributes.cpp: attribute tests for arbitrary configuration classic floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// Configure the cfloat template environment
// first: enable general or specialized configurations
#define CFLOAT_FAST_SPECIALIZATION
// second: enable/disable arithmetic exceptions
#define CFLOAT_THROW_ARITHMETIC_EXCEPTION 1
// third: enable support for native literals in logic and arithmetic operations
#define CFLOAT_ENABLE_LITERALS 1
// minimum set of include files to reflect source code dependencies
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_reporters.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "classic floating-point attribute functions";
	std::string test_tag    = "attributes";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	/////////////////////////////////////////////////////////////////////////////////////
	//// cfloat attribute functions

	{
		constexpr size_t nbits = 16;
		constexpr size_t es = 2;
		using BlockType = std::uint16_t;

		blocktriple<nbits - 1ull - es, BlockTripleOperator::REP, BlockType> v(1.0f);
		for (int i = 0; i < 10; ++i) {
			v.setscale(i);
			std::cout << "blocktriple : " << to_triple(v) << " : " << v << '\n';
		}
		std::cout << '\n';
	}

	{
		std::cout << "Dynamic ranges of symmetric classic floating-point arithmetic types\n";
		std::cout << dynamic_range< cfloat<  8, 2> >() << '\n';
		std::cout << dynamic_range< cfloat< 16, 5> >() << '\n';
		std::cout << dynamic_range< cfloat< 32, 8> >() << '\n';
		std::cout << dynamic_range< cfloat< 64,11> >() << '\n';
		std::cout << dynamic_range< cfloat<128,15> >() << '\n';
		std::cout << '\n';
	}

	{
		std::cout << "Dynamic ranges of different specializations of a 32-bit classic floating-point\n";
		std::cout << minmax_range< cfloat<32, 8, std::uint32_t, false, false, false> >() << '\n';
		std::cout << minmax_range< cfloat<32, 8, std::uint32_t, true, false, false> >() << '\n';
		std::cout << minmax_range< cfloat<32, 8, std::uint32_t, false, true, false> >() << '\n';
		std::cout << minmax_range< cfloat<32, 8, std::uint32_t, true, true, false> >() << '\n';
	}
	{
		std::cout << "Dynamic ranges of different specializations of a 32-bit classic floating-point\n";
		std::cout << cfloat_range< cfloat<32, 8, std::uint32_t, false, false, false> >() << '\n';
		std::cout << cfloat_range< cfloat<32, 8, std::uint32_t, true, false, false> >() << '\n';
		std::cout << cfloat_range< cfloat<32, 8, std::uint32_t, false, true, false> >() << '\n';
		std::cout << cfloat_range< cfloat<32, 8, std::uint32_t, true, true, false> >() << '\n';
	}
	{
		std::cout << "Dynamic ranges of different specializations of a 32-bit classic floating-point\n";
		std::cout << symmetry_range< cfloat<32, 8, std::uint32_t, false, false, false> >() << '\n';
		std::cout << symmetry_range< cfloat<32, 8, std::uint32_t, true, false, false> >() << '\n';
		std::cout << symmetry_range< cfloat<32, 8, std::uint32_t, false, true, false> >() << '\n';
		std::cout << symmetry_range< cfloat<32, 8, std::uint32_t, true, true, false> >() << '\n';
	}

	{
		std::cout << "Number traits\n";
		numberTraits< cfloat<32, 8, std::uint32_t, false, false, false> >(std::cout);  // FP32
		numberTraits< cfloat<32, 8, std::uint32_t, true, false, false> >(std::cout);   // IEEE-754
		std::cout << '\n';
	}

	{
		std::cout << "Comparitive Number traits\n";
		compareNumberTraits< cfloat<8, 2>, cfloat<8, 4> >(std::cout);
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
Dynamic ranges of different specializations of a 32-bit classic floating-point
cfloat< 32,   8, unsigned int,  noSubnormals,  noSupernormals, notSaturating> : min   1.17549e-38     max   3.40282e+38
cfloat< 32,   8, unsigned int, hasSubnormals,  noSupernormals, notSaturating> : min    1.4013e-45     max   3.40282e+38
cfloat< 32,   8, unsigned int,  noSubnormals, hasMaxExpValues, notSaturating> : min   1.17549e-38     max   6.80565e+38
cfloat< 32,   8, unsigned int, hasSubnormals, hasMaxExpValues, notSaturating> : min    1.4013e-45     max   6.80565e+38

Dynamic ranges of different specializations of a 32-bit classic floating-point
cfloat< 32,   8, unsigned int,  noSubnormals,  noSupernormals, notSaturating> : [ -3.40282e+38 ... -1.17549e-38 0 1.17549e-38 ... 3.40282e+38 ]
cfloat< 32,   8, unsigned int, hasSubnormals,  noSupernormals, notSaturating> : [ -3.40282e+38 ... -1.4013e-45 0 1.4013e-45 ... 3.40282e+38 ]
cfloat< 32,   8, unsigned int,  noSubnormals, hasMaxExpValues, notSaturating> : [ -6.80565e+38 ... -1.17549e-38 0 1.17549e-38 ... 6.80565e+38 ]
cfloat< 32,   8, unsigned int, hasSubnormals, hasMaxExpValues, notSaturating> : [ -6.80565e+38 ... -1.4013e-45 0 1.4013e-45 ... 6.80565e+38 ]

Dynamic ranges of different specializations of a 32-bit classic floating-point
cfloat< 32,   8, unsigned int,  noSubnormals,  noSupernormals, notSaturating> : [         -3.40282e+38,                   -0       0                    -0,          3.40282e+38]
cfloat< 32,   8, unsigned int, hasSubnormals,  noSupernormals, notSaturating> : [         -3.40282e+38,          -1.4013e-45       0            1.4013e-45,          3.40282e+38]
cfloat< 32,   8, unsigned int,  noSubnormals, hasMaxExpValues, notSaturating> : [         -6.80565e+38,                   -0       0                    -0,          6.80565e+38]
cfloat< 32,   8, unsigned int, hasSubnormals, hasMaxExpValues, notSaturating> : [         -6.80565e+38,          -1.4013e-45       0            1.4013e-45,          6.80565e+38]


 */