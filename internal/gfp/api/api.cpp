//  api.cpp : test suite runner for the class interface of the simplified floating-point type
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/long_double.hpp>
#include <universal/internal/gfp/gfp.hpp>
#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "gfp API validation";
	std::string test_tag    = "API";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	/////////////////         construction


	{
		gfp<uint32_t> a, b, c;
		a = 1.0e0f;
		std::cout << to_binary(a) << '\n';
		b = 1.0e0f;
		c = a + b;
		std::cout << a << " * " << b << " = " << c << '\n';
	}

	{
		gfp<uint64_t> a, b, c;
		a = 1.0e0;
		std::cout << to_binary(a) << '\n';
		b = 1.0e0;
		c = a + b;
		std::cout << a << " * " << b << " = " << c << '\n';
	}

	{
		gfp<uint64_t> a, b, c;
		a.set(false, 0, 0xf'ffff'ffff);
		b.set(false, 0, 0x1'ffff'ffff);
		std::cout << to_hex(b.significant()) << '\n';
		c = a * b;
		std::cout << a << " * " << b << " = " << c << '\n';
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << '\n';
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
