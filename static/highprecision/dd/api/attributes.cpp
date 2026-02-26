// attributes.cpp: attribute tests for double-double (dd) floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// Configure the bfloat and cfloat template environment
// enable/disable arithmetic exceptions
#define DOUBLEDOUBLE_THROW_ARITHMETIC_EXCEPTION 1
#define CFLOAT_THROW_ARITHMETIC_EXCEPTION 1
// enable support for native literals in logic and arithmetic operations
#define DOUBLEDOUBLE_ENABLE_LITERALS 1
#define CFLOAT_ENABLE_LITERALS 1
#include <universal/number/dd/dd.hpp>
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

void ConstructExtremeValues() {
	using namespace sw::universal;

	auto oldPrec = std::cout.precision();

	// construct the doubledouble maxpos bit pattern
	using Cfloat = cfloat<64, 11, uint32_t, true, false, false>;
	Cfloat a, b(1.7976931348623157e+308);
	a.maxpos();
	std::cout << std::setprecision(25);
	std::cout << to_binary(a) << " : " << a << '\n';
	std::cout << to_binary(b) << " : " << b << '\n';
	// maxpos exponent is 0b111'1111'1110;
	// lo segment is scaled by 2^53
	int i = 0x7fE;
	std::cout << "exponent is " << i - Cfloat::EXP_BIAS << '\n';
	std::cout << "lo exponent is " << (i - Cfloat::EXP_BIAS - 53) << '\n';
	std::cout << to_binary(i - 53) << '\n'; // get the unbiased scaled exponent value
	// 111'1100'1001
	a.setbits(0x7C9F'FFFF'FFFF'FFFFull);
	std::cout << to_binary(a) << " : " << a << '\n';
	b = 1.9958403095347196e+292;
	std::cout << to_binary(b) << " : " << b << '\n';

	// construct the doubledouble minpos bit pattern
	a.minpos();
	b = 1.0;
	std::cout << to_binary(a) << " : " << a << '\n';
	std::cout << to_binary(b) << " : " << b << '\n';

	std::cout << std::setprecision(oldPrec);

}
int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "double-double attribute functions";
	std::string test_tag    = "attributes";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	auto defaultPrecision = std::cout.precision();

	/////////////////////////////////////////////////////////////////////////////////////
	//// doubledouble attribute functions

	using doubledouble = dd;
	using f117_11 = cfloat<117, 11, uint32_t, true, false, false>;
	using f118_11 = cfloat<118, 11, uint32_t, true, false, false>;

	{
		std::cout << "Number traits: numeric limits of doubledouble floats\n";
		numberTraits< doubledouble >(std::cout);   // doubledouble emulation
		numberTraits< f117_11 >(std::cout);   // cfloat emulation
		std::cout << '\n';
	}

	{
		std::cout << "extreme values of doubledouble floats\n";
		NumericalLimits<doubledouble>();
		NumericalLimits<f118_11>();

		dd a(SpecificValue::qnan);
		std::cout << to_binary(a) << " : " << a << '\n';
	}

	{
		std::cout << "Dynamic range of doubledouble floats\n";
		std::cout << dynamic_range< doubledouble >() << '\n';
		std::cout << dynamic_range< f118_11 >() << '\n';
		std::cout << '\n';
	}

	{
		std::cout << "Dynamic range of a doubledouble floating-point\n";
		std::cout << minmax_range< doubledouble >() << '\n';
		std::cout << minmax_range< f118_11 >() << '\n';
	}
	{
		std::cout << "Dynamic range of a doubledouble floating-point\n";
		std::cout << dd_range() << '\n';
	}
	{
		std::cout << "Dynamic range of a doubledouble floating-point\n";
		std::cout << symmetry_range< doubledouble >() << '\n';
		std::cout << symmetry_range< f118_11 >() << '\n';
	}

	{
		std::cout << "Comparitive Number traits\n";
		compareNumberTraits< doubledouble, f117_11 >(std::cout);
		compareNumberTraits< doubledouble, f118_11 >(std::cout);
		std::cout << '\n';
	}

	std::cout << std::setprecision(defaultPrecision);

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
