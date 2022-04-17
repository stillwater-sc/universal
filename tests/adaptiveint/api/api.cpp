// api.cpp: application programming interface tests for adaptiveint
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// minimum set of include files to reflect source code dependencies
// Configure the cfloat template environment
// second: enable/disable arithmetic exceptions
#define ADAPTIVEINT_THROW_ARITHMETIC_EXCEPTION 0
// third: enable trace conversion
#define TRACE_CONVERSION 0
#include <universal/number/adaptiveint/adaptiveint.hpp>
#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "adaptiveint Application Programming Interface tests";
	int nrOfFailedTestCases = 0;

	// default behavior
	std::cout << "Default adaptiveint expands and contracts as needed\n";
	{
		using Integer = adaptiveint<std::uint32_t>;

		Integer a(0xFFFF'FFFF), b(0), c(1);
		std::cout << type_tag(a) << '\n';
		c = a + b;
		std::cout << "c = " << to_binary(c) << '\n';
		c = a + c;
		std::cout << "c = " << to_binary(c) << '\n';
		c = c - a;
		std::cout << "c = " << to_binary(c) << '\n';
		c = c * b;
		std::cout << "c = " << to_binary(c) << '\n';
		std::cout << "---\n";
	}

	return EXIT_SUCCESS;

	// set bit patterns
	std::cout << "set bit patterns API\n";
	{
		constexpr size_t nbits = 16;
		constexpr size_t es = 5;
		using Integer = adaptiveint<std::uint32_t>;

		Integer a;
		std::cout << type_tag(a) << '\n';

		a.setbits(0x0000);
		std::cout << to_binary(a) << " : " << a << '\n';

		a.setbits(0xAAAA);
		std::cout << to_binary(a) << " : " << a << '\n';

		a.assign(std::string("0b1.01010.1010'1010'10"));
		std::cout << to_binary(a) << " : " << a << '\n';

		a.assign(std::string("0b1.01010.10'1010'1010"));
		std::cout << to_binary(a) << " : " << a << '\n';
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
