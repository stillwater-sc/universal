// ulp.cpp: testing ulp values and algebra for fixed-size arbitrary logarithmic number configurations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// Configure the posit template environment
// first: enable general or specialized configurations
#define LNS_FAST_SPECIALIZATION
// second: enable/disable arithmetic exceptions
#define LNS_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/lns/lns.hpp>
#include <universal/verification/test_suite.hpp>

template<size_t nbits, size_t rbits, typename bt>
void TestULP(sw::universal::lns<nbits, rbits, bt> v = 1.0f) {
	using namespace sw::universal;

	lns<nbits, rbits, bt> a(v), ulpAt(ulp(a));
	std::cout << type_tag(a) << " at " << std::setw(15) << a << " : " << to_binary(a) << " : ULP : " << to_binary(ulpAt) << " : " << ulpAt << '\n';
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

	std::string test_suite  = "lns ULP tests";
	std::string test_tag    = "ulp";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	{
		TestULP<8, 4, uint8_t>();     // quarter precision
		TestULP<12, 6, uint16_t>();   // 
		TestULP<16, 8, uint16_t>();   // half precision
		TestULP<32, 16, uint32_t>();  // single precision
	}

	{
		std::cout << "\n8-bit lns ULPs\n";
		constexpr size_t nbits = 8;
		constexpr size_t rbits = 4;
		lns<nbits, rbits, std::uint16_t> eps = std::numeric_limits< lns<nbits, rbits, std::uint16_t> >::epsilon();
		std::cout << "lns<8,4> epsilon : " << to_binary(eps) << " : " << eps << '\n';
		for (float base = 0.25; base < 16.0f; base *= 2.0f) {
			TestULP<nbits, rbits, std::uint16_t>(base);
		}
	}

	{
		std::cout << "\n16-bit lns ULPs\n";
		constexpr size_t nbits = 16;
		constexpr size_t rbits = 8;
		lns<nbits, rbits, std::uint16_t> eps = std::numeric_limits< lns<nbits, rbits, std::uint16_t> >::epsilon();
		std::cout << "lns<16,8> epsilon : " << to_binary(eps) << " : " << eps << '\n';
		for (float base = 1.0f; base < 1.0e20f; base *= 1.0e1f) {
			TestULP<nbits, rbits, std::uint16_t>(base);
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
