// ranges.cpp: testing dynamic ranges of decimal floating-point number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// Configure the dfloat template environment
#define DFLOAT_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/dfloat/dfloat.hpp>
#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "dfloat<> range tests";
	std::string test_tag    = "dfloat<> ranges";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// dynamic range
	std::cout << "+---------    Dynamic range\n";
	{
		dfloat<7, 6> d32;
		std::cout << dynamic_range(d32) << '\n';

		dfloat<16, 8> d64;
		std::cout << dynamic_range(d64) << '\n';

		dfloat<34, 12> d128;
		std::cout << dynamic_range(d128) << '\n';
	}

	// numeric_limits
	std::cout << "+---------    numeric_limits\n";
	{
		using Real = decimal32;
		std::cout << std::setprecision(7);
		std::cout << "decimal32 radix     : " << std::numeric_limits<Real>::radix << '\n';
		std::cout << "decimal32 digits    : " << std::numeric_limits<Real>::digits << '\n';
		std::cout << "decimal32 digits10  : " << std::numeric_limits<Real>::digits10 << '\n';
		std::cout << "decimal32 is_exact  : " << std::numeric_limits<Real>::is_exact << '\n';
		std::cout << "decimal32 max       : " << std::numeric_limits<Real>::max() << '\n';
		std::cout << "decimal32 min       : " << std::numeric_limits<Real>::min() << '\n';
	}
	{
		using Real = decimal64;
		std::cout << std::setprecision(16);
		std::cout << "decimal64 radix     : " << std::numeric_limits<Real>::radix << '\n';
		std::cout << "decimal64 digits    : " << std::numeric_limits<Real>::digits << '\n';
		std::cout << "decimal64 digits10  : " << std::numeric_limits<Real>::digits10 << '\n';
		std::cout << "decimal64 is_exact  : " << std::numeric_limits<Real>::is_exact << '\n';
		std::cout << "decimal64 max       : " << std::numeric_limits<Real>::max() << '\n';
		std::cout << "decimal64 min       : " << std::numeric_limits<Real>::min() << '\n';
	}
	{
		using Real = decimal128;
		std::cout << std::setprecision(34);
		std::cout << "decimal128 radix    : " << std::numeric_limits<Real>::radix << '\n';
		std::cout << "decimal128 digits   : " << std::numeric_limits<Real>::digits << '\n';
		std::cout << "decimal128 digits10 : " << std::numeric_limits<Real>::digits10 << '\n';
		std::cout << "decimal128 is_exact : " << std::numeric_limits<Real>::is_exact << '\n';
		std::cout << "decimal128 max      : " << std::numeric_limits<Real>::max() << '\n';
		std::cout << "decimal128 min      : " << std::numeric_limits<Real>::min() << '\n';
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
