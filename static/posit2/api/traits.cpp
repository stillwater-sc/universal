// traits.cpp: tests for type and number traits for arbitrary configuration posit types
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
#include <universal/number/posit2/posit.hpp>
#include <universal/verification/test_reporters.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "generalized posit traits";
	std::string test_tag    = "traits";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	/////////////////////////////////////////////////////////////////////////////////////
	//// posit type attribute functions

	{
		using Real = posit<8, 2, std::uint8_t>;
		bool isTrivial = bool(std::is_trivial<Real>());
		static_assert(std::is_trivial<Real>(), "posit should be trivial but failed the assertion");
		std::cout << (isTrivial ? "posit is trivial: PASS" : "posit failed trivial: FAIL") << '\n';

		bool isTriviallyConstructible = bool(std::is_trivially_constructible<Real>());
		static_assert(std::is_trivially_constructible<Real>(), "posit should be trivially constructible but failed the assertion");
		std::cout << (isTriviallyConstructible ? "posit is trivial constructible: PASS" : "posit failed trivial constructible: FAIL") << '\n';

		bool isTriviallyCopyable = bool(std::is_trivially_copyable<Real>());
		static_assert(std::is_trivially_copyable<Real>(), "posit should be trivially copyable but failed the assertion");
		std::cout << (isTriviallyCopyable ? "posit is trivially copyable: PASS" : "posit failed trivially copyable: FAIL") << '\n';

		bool isTriviallyCopyAssignable = bool(std::is_trivially_copy_assignable<Real>());
		static_assert(std::is_trivially_copy_assignable<Real>(), "posit should be trivially copy-assignable but failed the assertion");
		std::cout << (isTriviallyCopyAssignable ? "posit is trivially copy-assignable: PASS" : "posit failed trivially copy-assignable: FAIL") << '\n';
	}

	{
		std::cout << "Dynamic ranges of different specializations of an 8-bit generalized posit\n";
		std::cout << dynamic_range< posit<8, 0> >() << '\n';
		std::cout << dynamic_range< posit<8, 1> >() << '\n';
		std::cout << dynamic_range< posit<8, 2> >() << '\n';
		std::cout << dynamic_range< posit<8, 3> >() << '\n';
		std::cout << dynamic_range< posit<8, 4> >() << '\n';
	}

	{
		std::cout << "Min/max values of the standard posit configurations\n";
		std::cout << minmax_range< posit<  8, 2> >() << '\n';
		std::cout << minmax_range< posit< 16, 2> >() << '\n';
		std::cout << minmax_range< posit< 32, 2> >() << '\n';
		std::cout << minmax_range< posit< 64, 2> >() << '\n';
		//std::cout << minmax_range< posit<128, 2> >() << '\n';   disabled as we do not have a valid decimal converter for multi-limb posits > 64 bits
		//std::cout << minmax_range< posit<256, 2> >() << '\n';
	}

	{
		std::cout << "Sampling ranges of the standard posit configurations\n";
		std::cout << symmetry_range< posit<  8, 2> >() << '\n';
		std::cout << symmetry_range< posit< 16, 2> >() << '\n';
		std::cout << symmetry_range< posit< 32, 2> >() << '\n';
		std::cout << symmetry_range< posit< 64, 2> >() << '\n';
		//std::cout << symmetry_range< posit<128, 2> >() << '\n';
		//std::cout << symmetry_range< posit<256, 2> >() << '\n';
	}

#ifdef LATER
	{
		std::cout << "Number traits of the standard posit configurations\n";
		numberTraits< posit< 8, 2, std::uint8_t> >(std::cout);
		numberTraits< posit<16, 2, std::uint16_t> >(std::cout);
		numberTraits< posit<32, 2, std::uint32_t> >(std::cout);
		numberTraits< posit<64, 2, std::uint64_t> >(std::cout);
	}
#endif

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


/*
Dynamic ranges of different specializations of an 8-bit generalized posit
sw::universal::posit<  8, 0, unsigned char> : minexp scale         -6     maxexp scale          6     minimum     0.015625     maximum           64
sw::universal::posit<  8, 1, unsigned char> : minexp scale        -12     maxexp scale         12     minimum  0.000244141     maximum         4096
sw::universal::posit<  8, 2, unsigned char> : minexp scale        -24     maxexp scale         24     minimum  5.96046e-08     maximum  1.67772e+07
sw::universal::posit<  8, 3, unsigned char> : minexp scale        -48     maxexp scale         48     minimum  3.55271e-15     maximum  2.81475e+14
sw::universal::posit<  8, 4, unsigned char> : minexp scale        -96     maxexp scale         96     minimum  1.26218e-29     maximum  7.92282e+28

Dynamic ranges of the standard posit configurations
sw::universal::posit<  8, 2, unsigned char> : min   5.96046e-08     max   1.67772e+07
sw::universal::posit< 16, 2, unsigned char> : min   1.38778e-17     max   7.20576e+16
sw::universal::posit< 32, 2, unsigned char> : min   7.52316e-37     max   1.32923e+36
sw::universal::posit< 64, 2, unsigned char> : min   2.21086e-75     max   4.52313e+74
sw::universal::posit<128, 2, unsigned char> : min  1.90934e-152     max  5.23742e+151
sw::universal::posit<256, 2, unsigned char> : min  1.42405e-306     max  7.02224e+305

Dynamic ranges of the standard posit configurations
sw::universal::posit<  8, 2, unsigned char> : [         -1.67772e+07,         -5.96046e-08       0           5.96046e-08,          1.67772e+07]
sw::universal::posit< 16, 2, unsigned char> : [         -7.20576e+16,         -1.38778e-17       0           1.38778e-17,          7.20576e+16]
sw::universal::posit< 32, 2, unsigned char> : [         -1.32923e+36,         -7.52316e-37       0           7.52316e-37,          1.32923e+36]
sw::universal::posit< 64, 2, unsigned char> : [         -4.52313e+74,         -2.21086e-75       0           2.21086e-75,          4.52313e+74]
sw::universal::posit<128, 2, unsigned char> : [        -5.23742e+151,        -1.90934e-152       0          1.90934e-152,         5.23742e+151]
sw::universal::posit<256, 2, unsigned char> : [        -7.02224e+305,        -1.42405e-306       0          1.42405e-306,         7.02224e+305]
 */
