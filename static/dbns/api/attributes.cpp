// attributes.cpp: attribute tests for fixed-size arbitrary configuration double base number systems
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// Configure the dbns template environment
// first: enable general or specialized configurations
#define DBNS_FAST_SPECIALIZATION
// second: enable/disable arithmetic exceptions
#define DBNS_THROW_ARITHMETIC_EXCEPTION 1
// third: enable support for native literals in logic and arithmetic operations
#define DBNS_ENABLE_LITERALS 1
#include <universal/number/dbns/dbns.hpp>
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
	//// dbns attribute functions

	{
		std::cout << "\nDynamic ranges of 2-base logarithmic floating-point arithmetic types\n";
		std::cout << dynamic_range< dbns<  8, 5> >() << '\n';    //  8 - 1 -  5 = 2 scale exponent bits: 3^##
		std::cout << dynamic_range< dbns< 16,12> >() << '\n';    // 16 - 1 - 12 = 3 scale exponent bits: 3^###
		std::cout << dynamic_range< dbns< 24,19> >() << '\n';    // 24 - 1 - 19 = 4 scale exponent bits: 3^####
		std::cout << dynamic_range< dbns< 32,26> >() << '\n';    // 32 - 1 - 26 = 5 scale exponent bits: 3^#####
	}

	{
		std::cout << "\nMinmax of 2-base logarithmic floating-point\n";
		std::cout << minmax_range< dbns< 8, 5> >() << '\n';
		std::cout << minmax_range< dbns<16,12> >() << '\n';
		std::cout << minmax_range< dbns<24,19> >() << '\n';
		std::cout << minmax_range< dbns<32,26> >() << '\n';
	}
	
	{
		std::cout << "\nValue ranges of 2-base logarithmic floating-point\n";
		std::cout << symmetry_range< dbns< 8, 5> >() << '\n';
		std::cout << symmetry_range< dbns<16,12> >() << '\n';
		std::cout << symmetry_range< dbns<24,19> >() << '\n';
		std::cout << symmetry_range< dbns<32,26> >() << '\n';
	}

	{
		std::cout << "\nSpecific 2-base logarithmic floating-point range function\n";
		std::cout << dbns_range(dbns<7, 3>()) << '\n';
	}

	{
		std::cout << "\n2-base lns sign() function\n";
		dbns<7, 3> a;

		a.setbits(0x7f);
		std::cout << std::setw(45) << type_tag(a) << " : " << to_binary(a) << " : " << a << " : " << (sign(a) ? "sign = 1" : "sign = 0") << '\n';
	}

	{
		std::cout << "\nNumber traits\n";
		numberTraits< dbns<8, 4> >(std::cout);
	}

	{
		std::cout << "\nComparitive Number traits\n";
		compareNumberTraits< dbns<10, 6>, dbns<12, 7> >(std::cout);
		threeWayCompareNumberTraits< float, dbns<10, 6>, dbns<12, 7> >(std::cout);
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

Dynamic ranges of 2-base logarithmic floating-point arithmetic types
dbns<  8,   5, unsigned char, Saturating>                                       : minexp scale -2             maxexp scale 1              minimum 4.65661e-10      maximum 27
dbns< 16,  12, unsigned char, Saturating>                                       : minexp scale -4             maxexp scale 3              minimum 0                maximum 2187
dbns< 24,  19, unsigned char, Saturating>                                       : minexp scale -8             maxexp scale 7              minimum 0                maximum 1.43489e+07
dbns< 32,  26, unsigned char, Saturating>                                       : minexp scale -16            maxexp scale 15             minimum 0                maximum 6.17673e+14

Minmax of 2-base logarithmic floating-point
dbns<  8,   5, unsigned char, Saturating>                                       : min 4.65661e-10       max 27
dbns< 16,  12, unsigned char, Saturating>                                       : min 0                 max 2187
dbns< 24,  19, unsigned char, Saturating>                                       : min 0                 max 1.43489e+07
dbns< 32,  26, unsigned char, Saturating>                                       : min 0                 max 6.17673e+14

Value ranges of 2-base logarithmic floating-point
dbns<  8,   5, unsigned char, Saturating>                                       : [ -27 ... -4.65661e-10  0  4.65661e-10 ... 27]
dbns< 16,  12, unsigned char, Saturating>                                       : [ -2187 ... -0  0  0 ... 2187]
dbns< 24,  19, unsigned char, Saturating>                                       : [ -1.43489e+07 ... -0  0  0 ... 1.43489e+07]
dbns< 32,  26, unsigned char, Saturating>                                       : [ -6.17673e+14 ... -0  0  0 ... 6.17673e+14]

Specific 2-base logarithmic floating-point range function
   dbns<  7,   3, unsigned char, Saturating> : [ -2187 ... -0.0078125 0 0.0078125 ... 2187 ]

2-base lns sign() function
   dbns<  7,   3, unsigned char, Saturating> : 0b1.111.111 : -17.0859 : sign = 1

Number traits
std::numeric_limits< class sw::universal::dbns<8,4,unsigned char> >
min exponent                                             -4
max exponent                                              3
radix                                                     2
radix digits                                              8
min                                             3.05176e-05
max                                                    2187
lowest                                                -2187
epsilon (1+1ULP-1)                                     0.25
round_error                                     6.10352e-05
smallest value                                  3.05176e-05
infinity                                               2187
quiet_NAN                                         -nan(ind)
signaling_NAN                                     -nan(ind)


Comparitive Number traits
comparing numeric_limits between class sw::universal::dbns<10,6,unsigned char> and class sw::universal::dbns<12,7,unsigned char>
				class sw::universal::dbns<10,6,unsigned char> vs class sw::universal::dbns<12,7,unsigned char>
min exponent                                -4 vs                             -8
max exponent                                 3 vs                              7
radix                                        2 vs                              2
radix digits                                10 vs                             15
min                                 1.0842e-19 vs                    5.87747e-39
max                                       2187 vs                    1.43489e+07
lowest                                   -2187 vs                   -1.43489e+07
epsilon                             0.00390625 vs                     0.00390625
round_error                        1.38778e-17 vs                    7.52316e-37
smallest value                      1.0842e-19 vs                    5.87747e-39
infinity                                  2187 vs                    1.43489e+07
quiet_NAN                            -nan(ind) vs                      -nan(ind)
signaling_NAN                        -nan(ind) vs                      -nan(ind)

*/
