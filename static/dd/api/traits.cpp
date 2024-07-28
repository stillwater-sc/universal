// traits.cpp: tests for type and number traits for doubledouble (dd) floating-point type
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/dd/dd.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_reporters.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "bfloat traits";
	std::string test_tag    = "traits";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	/////////////////////////////////////////////////////////////////////////////////////
	//// bfloat type attribute functions

	{
		using Real = dd;
		bool isTrivial = bool(std::is_trivial<Real>());
		static_assert(std::is_trivial<Real>(), "dd should be trivial but failed the assertion");
		std::cout << (isTrivial ? "dd is trivial: PASS" : "dd failed trivial: FAIL") << '\n';

		bool isTriviallyConstructible = bool(std::is_trivially_constructible<Real>());
		static_assert(std::is_trivially_constructible<Real>(), "dd should be trivially constructible but failed the assertion");
		std::cout << (isTriviallyConstructible ? "dd is trivial constructible: PASS" : "dd failed trivial constructible: FAIL") << '\n';

		bool isTriviallyCopyable = bool(std::is_trivially_copyable<Real>());
		static_assert(std::is_trivially_copyable<Real>(), "dd should be trivially copyable but failed the assertion");
		std::cout << (isTriviallyCopyable ? "dd is trivially copyable: PASS" : "dd failed trivially copyable: FAIL") << '\n';

		bool isTriviallyCopyAssignable = bool(std::is_trivially_copy_assignable<Real>());
		static_assert(std::is_trivially_copy_assignable<Real>(), "dd should be trivially copy-assignable but failed the assertion");
		std::cout << (isTriviallyCopyAssignable ? "dd is trivially copy-assignable: PASS" : "dd failed trivially copy-assignable: FAIL") << '\n';
	}

	{
		std::cout << "Comparison of dynamic ranges of doubledouble and the standard classic floating-point configuration\n";
		std::cout << dynamic_range< double >() << '\n';
		std::cout << dynamic_range< dd >() << '\n';
		std::cout << symmetry_range< double >() << '\n';
		std::cout << symmetry_range< dd >() << '\n';
	}
	{
		std::cout << "Comparison of min/max values of doubledouble and the standard classic floating-point configurations\n";
		std::cout << minmax_range< quarter >() << '\n';
		std::cout << minmax_range< half >() << '\n';
		std::cout << minmax_range< single >() << '\n';
		std::cout << minmax_range< duble >() << '\n';

		std::cout << minmax_range< dd >() << '\n';
	}

	{
		std::cout << "Comparison of sampling ranges of doubledouble and the standard classic floating-point configurations\n";
		std::cout << symmetry_range< quarter >() << '\n';
		std::cout << symmetry_range< half >() << '\n';
		std::cout << symmetry_range< single >() << '\n';
		std::cout << symmetry_range< duble >() << '\n';

		std::cout << symmetry_range< dd >() << '\n';
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
cfloat<  8,   1, unsigned char, hasSubnormals, hasSupernormals, notSaturating> : minexp scale         -1     maxexp scale          1     minimum      0.03125     maximum      3.90625
cfloat<  8,   2, unsigned char, hasSubnormals, hasSupernormals, notSaturating> : minexp scale         -2     maxexp scale          2     minimum      0.03125     maximum        7.625
cfloat<  8,   3, unsigned char, hasSubnormals, hasSupernormals, notSaturating> : minexp scale         -4     maxexp scale          4     minimum     0.015625     maximum           29
cfloat<  8,   4, unsigned char, hasSubnormals, hasSupernormals, notSaturating> : minexp scale         -8     maxexp scale          8     minimum   0.00195312     maximum          416
cfloat<  8,   5, unsigned char, hasSubnormals, hasSupernormals, notSaturating> : minexp scale        -16     maxexp scale         16     minimum  1.52588e-05     maximum        81920
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
