// attributes.cpp: attribute tests for Google Brain floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// Configure the bfloat and cfloat template environment
// enable/disable arithmetic exceptions
#define BFLOAT_THROW_ARITHMETIC_EXCEPTION 1
#define CFLOAT_THROW_ARITHMETIC_EXCEPTION 1
// enable support for native literals in logic and arithmetic operations
#define BFLOAT_ENABLE_LITERALS 1
#define CFLOAT_ENABLE_LITERALS 1
#include <universal/number/bfloat/bfloat.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_reporters.hpp>

template<typename Real>
void NumericalLimits() {
	Real a;
	std::cout << "minpos : " << to_binary(a.minpos()) << " : " << a << '\n';
	std::cout << "maxpos : " << to_binary(a.maxpos()) << " : " << a << '\n';
	std::cout << "maxneg : " << to_binary(a.maxneg()) << " : " << a << '\n';
	std::cout << "minneg : " << to_binary(a.minneg()) << " : " << a << '\n';
	Real epsilon = std::numeric_limits<Real>::epsilon();
	std::cout << "epsilon: " << to_binary(epsilon) << " : " << epsilon << '\n';
}

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "Google Brain Float attribute functions";
	std::string test_tag    = "attributes";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	/////////////////////////////////////////////////////////////////////////////////////
	//// bfloat attribute functions

	{
//		NumericalLimits<bfloat_t>();
//		NumericalLimits<bfloat16>();
	}

	{
		std::cout << "Dynamic ranges of Google Brain Floats\n";
		std::cout << dynamic_range< bfloat_t >() << '\n';  // this is an equivalent cfloat
		std::cout << dynamic_range< bfloat16 >() << '\n';
		std::cout << '\n';
	}

	{
		std::cout << "Dynamic ranges of different specializations of a 16-bit brain floating-point\n";
		std::cout << minmax_range< bfloat_t >() << '\n';
		std::cout << minmax_range< bfloat16 >() << '\n';
	}
	{
		std::cout << "Dynamic ranges of different specializations of a 16-bit brain floating-point\n";
		std::cout << cfloat_range< bfloat_t >() << '\n';
		std::cout << bfloat_range< bfloat16 >() << '\n';
	}
	{
		std::cout << "Dynamic ranges of different specializations of a 16-bit brain floating-point\n";
		std::cout << symmetry_range< bfloat_t >() << '\n';
		std::cout << symmetry_range< bfloat16 >() << '\n';
	}

	{
		std::cout << "Number traits\n";
		numberTraits< bfloat_t >(std::cout);   // cfloat emulation
		numberTraits< bfloat16 >(std::cout);   // fp32 IEEE-754 emulation
		std::cout << '\n';
	}

	{
		bfloat16 a(SpecificValue::qnan);
		std::cout << to_binary(a) << " : " << a << '\n';
	}
	{
		std::cout << "Comparitive Number traits\n";
		compareNumberTraits< bfloat_t, bfloat16 >(std::cout);
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
cfloat< 32,   8, unsigned int,  noSubnormals, hasSupernormals, notSaturating> : min   1.17549e-38     max   6.80565e+38
cfloat< 32,   8, unsigned int, hasSubnormals, hasSupernormals, notSaturating> : min    1.4013e-45     max   6.80565e+38

Dynamic ranges of different specializations of a 32-bit classic floating-point
cfloat< 32,   8, unsigned int,  noSubnormals,  noSupernormals, notSaturating> : [ -3.40282e+38 ... -1.17549e-38 0 1.17549e-38 ... 3.40282e+38 ]
cfloat< 32,   8, unsigned int, hasSubnormals,  noSupernormals, notSaturating> : [ -3.40282e+38 ... -1.4013e-45 0 1.4013e-45 ... 3.40282e+38 ]
cfloat< 32,   8, unsigned int,  noSubnormals, hasSupernormals, notSaturating> : [ -6.80565e+38 ... -1.17549e-38 0 1.17549e-38 ... 6.80565e+38 ]
cfloat< 32,   8, unsigned int, hasSubnormals, hasSupernormals, notSaturating> : [ -6.80565e+38 ... -1.4013e-45 0 1.4013e-45 ... 6.80565e+38 ]

Dynamic ranges of different specializations of a 32-bit classic floating-point
cfloat< 32,   8, unsigned int,  noSubnormals,  noSupernormals, notSaturating> : [         -3.40282e+38,                   -0       0                    -0,          3.40282e+38]
cfloat< 32,   8, unsigned int, hasSubnormals,  noSupernormals, notSaturating> : [         -3.40282e+38,          -1.4013e-45       0            1.4013e-45,          3.40282e+38]
cfloat< 32,   8, unsigned int,  noSubnormals, hasSupernormals, notSaturating> : [         -6.80565e+38,                   -0       0                    -0,          6.80565e+38]
cfloat< 32,   8, unsigned int, hasSubnormals, hasSupernormals, notSaturating> : [         -6.80565e+38,          -1.4013e-45       0            1.4013e-45,          6.80565e+38]


 */
