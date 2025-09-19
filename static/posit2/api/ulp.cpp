// ulp.cpp: testing ulp values and algebra for generalized posit configurations
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
#include <universal/number/posit2/posit.hpp>

#include <universal/verification/test_suite.hpp>

template<size_t nbits, size_t es, typename bt>
void TestULP(sw::universal::posit<nbits, es, bt> v = 1.0f) {
	using namespace sw::universal;

	posit<nbits, es, bt> a(v), ulpAt(ulp(a));
	//std::cout << type_tag(a) << " at " << std::setw(15) << a << " : " << to_binary(a) << " : ULP : " << to_binary(ulpAt) << " : " << ulpAt << '\n';
	std::cout << type_tag(a) << " at " << std::setw(15) << to_binary(a) << " : ULP : " << to_binary(ulpAt) << '\n';
}

template<typename Real>
void TestNativeULP(Real v = 1.0f) {
	using namespace sw::universal;

	Real a(v), ulpAt(ulp(a));
	std::cout << type_tag(a) << " at " << std::setw(15) << a << " : " << to_binary(a) << " : ULP : " << to_binary(ulpAt) << " : " << ulpAt << '\n';
}

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "generalized posit ULP tests";
	std::string test_tag    = "ulp";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	{
		TestULP<8, 2, uint8_t>();     // quarter precision
		TestULP<16, 2, uint16_t>();   // half precision
		TestULP<32, 2, uint32_t>();   // single precision
		TestULP<64, 2, uint32_t>();   // double precision
		TestULP<128, 2, uint32_t>();  // quad precision
		TestULP<256, 2, uint32_t>();  // octo precision
	}

	{
		std::cout << "\n32-bit standard posit ULPs as baseline\n";
		posit<32, 2> eps = std::numeric_limits< posit<32, 2> >::epsilon();
		std::cout << "posit epsilon : " << to_binary(eps) << " : " << eps << '\n';
		for (float base = 1.0f; base < 1.0e30f; base *= 1.0e3f) {
			TestULP<32, 2, std::uint32_t>(base);
		}
	}

	{
		std::cout << "\nNative IEEE-754 single precision float ULPs to reference\n";
		constexpr float eps = std::numeric_limits< float >::epsilon();
		std::cout << "float epsilon : " << to_binary(eps) << " : " << eps << '\n';
		for (float base = 1.0f; base < 1.0e30f; base *= 1.0e3f) {
			TestNativeULP(base);
		}
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
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
