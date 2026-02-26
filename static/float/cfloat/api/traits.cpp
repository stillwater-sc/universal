// traits.cpp: tests for type and number traits for arbitrary configuration classic floating-point types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// Configure the posit template environment
// first: enable general or specialized configurations
#define CFLOAT_FAST_SPECIALIZATION
// second: enable/disable arithmetic exceptions
#define CFLOAT_THROW_ARITHMETIC_EXCEPTION 1
// third: enable support for native literals in logic and arithmetic operations
#define CFLOAT_ENABLE_LITERALS 1
// fourth: enable/disable error-free serialization I/O
#define CFLOAT_ERROR_FREE_IO_FORMAT 0
// minimum set of include files to reflect source code dependencies
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_reporters.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "cfloat traits";
	std::string test_tag    = "traits";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	/////////////////////////////////////////////////////////////////////////////////////
	//// cfloat type attribute functions

	{
		using Real = cfloat<8, 2, std::uint8_t>;
		bool isTrivial = bool(std::is_trivial<Real>());
		static_assert(std::is_trivial<Real>(), "cfloat should be trivial but failed the assertion");
		std::cout << (isTrivial ? "cfloat is trivial: PASS" : "cfloat failed trivial: FAIL") << '\n';

		bool isTriviallyConstructible = bool(std::is_trivially_constructible<Real>());
		static_assert(std::is_trivially_constructible<Real>(), "cfloat should be trivially constructible but failed the assertion");
		std::cout << (isTriviallyConstructible ? "cfloat is trivial constructible: PASS" : "cfloat failed trivial constructible: FAIL") << '\n';

		bool isTriviallyCopyable = bool(std::is_trivially_copyable<Real>());
		static_assert(std::is_trivially_copyable<Real>(), "cfloat should be trivially copyable but failed the assertion");
		std::cout << (isTriviallyCopyable ? "cfloat is trivially copyable: PASS" : "cfloat failed trivially copyable: FAIL") << '\n';

		bool isTriviallyCopyAssignable = bool(std::is_trivially_copy_assignable<Real>());
		static_assert(std::is_trivially_copy_assignable<Real>(), "cfloat should be trivially copy-assignable but failed the assertion");
		std::cout << (isTriviallyCopyAssignable ? "cfloat is trivially copy-assignable: PASS" : "cfloat failed trivially copy-assignable: FAIL") << '\n';
	}

	{
		std::cout << "Dynamic ranges of different specializations of an 8-bit classic floating-point\n";
		constexpr bool hasSubnormals = true;
		constexpr bool hasMaxExpValues = true;
		std::cout << dynamic_range< cfloat<8, 1, std::uint8_t, hasSubnormals, hasMaxExpValues> >() << '\n';
		std::cout << dynamic_range< cfloat<8, 2, std::uint8_t, hasSubnormals, hasMaxExpValues> >() << '\n';
		std::cout << dynamic_range< cfloat<8, 3, std::uint8_t, hasSubnormals, hasMaxExpValues> >() << '\n';
		std::cout << dynamic_range< cfloat<8, 4, std::uint8_t, hasSubnormals, hasMaxExpValues> >() << '\n';
		std::cout << dynamic_range< cfloat<8, 5, std::uint8_t, hasSubnormals, hasMaxExpValues> >() << '\n';
	}

	{
		std::cout << "Min/max values of the standard classic floating-point configurations\n";
		std::cout << minmax_range< quarter >() << '\n';
		std::cout << minmax_range< half >() << '\n';
		std::cout << minmax_range< single >() << '\n';
		std::cout << minmax_range< duble >() << '\n';
		std::cout << minmax_range< quad >() << '\n';
		std::cout << minmax_range< octo >() << '\n';
	}

	{
		std::cout << "Sampling ranges of the standard classic floating-point configurations\n";
		std::cout << symmetry_range< quarter >() << '\n';
		std::cout << symmetry_range< half >() << '\n';
		std::cout << symmetry_range< single >() << '\n';
		std::cout << symmetry_range< duble >() << '\n';
		std::cout << symmetry_range< quad >() << '\n';
		std::cout << symmetry_range< octo >() << '\n';
	}

	{
		std::cout << "Number traits native floating-point\n";
		numberTraits<float>(std::cout);
		std::cout << "Number traits Universal classic floating-point\n";
		numberTraits<single>(std::cout);

		std::cout << "First principles to derive the C++ numeric_limits<>::[min|max]_exponent value\n";
		// the C++ library specification of numeric::limits<> has a non-intuitive
		// interpretation of min_exponent and max_exponent.
		// Link: https://en.cppreference.com/w/cpp/types/numeric_limits
		// min_exponent:
		// one more than the smallest negative power of the radix that is a valid normalized floating-point value
		// max_exponent
		// one more than the largest integer power of the radix that is a valid finite floating-point value
		sw::universal::single sp;
		std::cout << "Smallest negative power of a single precision floating-point\n";
		std::cout << "C++ std::numeric_limits<float>::min_exponent : " << std::numeric_limits<float>::min_exponent << '\n';
		// 0b0.0000'0001.000'0000'0000'0000'0000'0000  smallest normal value of a single precision floating-point
		sp.setbits(0x00800000);
		std::cout << "binary pattern                               = " << to_binary(sp) << '\n';
		std::cout << "smallest normal value                        = " << sp << std::endl;
		std::cout << "scale of smallest normal value               = " << scale(sp) << '\n';
		std::cout << "one more than that                           = " << scale(sp) + 1 << '\n';
		std::cout << "std::numeric_limits<single>::min_exponent    = " << std::numeric_limits<single>::min_exponent << '\n';

		std::cout << '\n';
		std::cout << "Largest finite value of a single precision floating-point\n";
		std::cout << "C++ std::numeric_limits<float>::max_exponent = " << std::numeric_limits<float>::max_exponent << '\n';
		// 0b0.1111'1110.1111'1111'1111'1111'1111'1111  largest finite value of a single precision floating-point
		sp.setbits(0x7F7FFFFF);
		std::cout << "binary pattern                               = " << to_binary(sp) << '\n';
		std::cout << "largest finite value                         = " << sp << std::endl;
		std::cout << "scale of largest finite value                = " << scale(sp) << '\n';
		std::cout << "one more than that                           = " << scale(sp) + 1 << '\n';
		std::cout << "std::numeric_limits<single>::max_exponent    = " << std::numeric_limits<single>::max_exponent << '\n';
	}
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::cfloat_arithmetic_exception& err) {
	std::cerr << "Uncaught cfloat arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::cfloat_internal_exception& err) {
	std::cerr << "Uncaught cfloat internal exception: " << err.what() << std::endl;
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
generalized posit traits: report test cases
cfloat is trivial: PASS
poscfloatit is trivial constructible: PASS
cfloat is trivially copyable: PASS
cfloat is trivially copy-assignable: PASS
Dynamic ranges of different specializations of an 8-bit classic floating-point
cfloat<  8,   1, unsigned char, hasSubnormals, hasMaxExpValues, notSaturating> : minexp scale         -1     maxexp scale          1     minimum      0.03125     maximum      3.90625
cfloat<  8,   2, unsigned char, hasSubnormals, hasMaxExpValues, notSaturating> : minexp scale         -2     maxexp scale          2     minimum      0.03125     maximum        7.625
cfloat<  8,   3, unsigned char, hasSubnormals, hasMaxExpValues, notSaturating> : minexp scale         -4     maxexp scale          4     minimum     0.015625     maximum           29
cfloat<  8,   4, unsigned char, hasSubnormals, hasMaxExpValues, notSaturating> : minexp scale         -8     maxexp scale          8     minimum   0.00195312     maximum          416
cfloat<  8,   5, unsigned char, hasSubnormals, hasMaxExpValues, notSaturating> : minexp scale        -16     maxexp scale         16     minimum  1.52588e-05     maximum        81920
Dynamic ranges of the standard classic floating-point configurations
cfloat<  8,   2, unsigned int, hasSubnormals,  noSupernormals, notSaturating> : min       0.03125     max        3.9375
cfloat< 16,   5, unsigned int, hasSubnormals,  noSupernormals, notSaturating> : min   5.96046e-08     max         65504
cfloat< 32,   8, unsigned int, hasSubnormals,  noSupernormals, notSaturating> : min    1.4013e-45     max   3.40282e+38
cfloat< 64,  11, unsigned int, hasSubnormals,  noSupernormals, notSaturating> : min  4.94066e-324     max  1.79769e+308
cfloat<128,  15, unsigned int, hasSubnormals,  noSupernormals, notSaturating> : min             0     max           inf
cfloat<256,  19, unsigned int, hasSubnormals,  noSupernormals, notSaturating> : min             0     max           inf
Dynamic ranges of the standard posit configurations
cfloat<  8,   2, unsigned int, hasSubnormals,  noSupernormals, notSaturating> : [              -3.9375,             -0.03125       0               0.03125,               3.9375]
cfloat< 16,   5, unsigned int, hasSubnormals,  noSupernormals, notSaturating> : [               -65504,         -5.96046e-08       0           5.96046e-08,                65504]
cfloat< 32,   8, unsigned int, hasSubnormals,  noSupernormals, notSaturating> : [         -3.40282e+38,          -1.4013e-45       0            1.4013e-45,          3.40282e+38]
cfloat< 64,  11, unsigned int, hasSubnormals,  noSupernormals, notSaturating> : [        -1.79769e+308,        -4.94066e-324       0          4.94066e-324,         1.79769e+308]
cfloat<128,  15, unsigned int, hasSubnormals,  noSupernormals, notSaturating> : [                 -inf,                    0       0                     0,                  inf]
cfloat<256,  19, unsigned int, hasSubnormals,  noSupernormals, notSaturating> : [                 -inf,                    0       0                     0,                  inf]
generalized posit traits: PASS

*/
