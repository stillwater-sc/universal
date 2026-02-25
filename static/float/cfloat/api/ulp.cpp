// ulp.cpp: testing ulp values and algebra for classic floating-point cfloat configurations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// Configure the cfloat template environment
// first: enable general or specialized configurations
#define CFLOAT_FAST_SPECIALIZATION
// second: enable/disable arithmetic exceptions
#define CFLOAT_THROW_ARITHMETIC_EXCEPTION 1
// third: enable native literals in logic and arithmetic operations
#define CFLOAT_ENABLE_LITERALS 1
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_suite.hpp>

template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
void TestULP(sw::universal::cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> v = 1.0f) {
	using namespace sw::universal;

	cfloat < nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> a(v), ulpAt(ulp(a));
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

	std::string test_suite  = "classic floating-point ULP tests";
	std::string test_tag    = "ulp";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	constexpr bool hasSubnormals = true;
	constexpr bool noSupernormals = false;
	constexpr bool notSaturating = false;

	{
		TestULP<8, 2, uint8_t, hasSubnormals, noSupernormals, notSaturating>();     // quarter precision
		TestULP<16, 5, uint16_t, hasSubnormals, noSupernormals, notSaturating>();   // half precision
		TestULP<32, 8, uint32_t, hasSubnormals, noSupernormals, notSaturating>();   // single precision
		TestULP<64, 11, uint32_t, hasSubnormals, noSupernormals, notSaturating>();   // double precision
		TestULP<128, 15, uint32_t, hasSubnormals, noSupernormals, notSaturating>();  // quad precision
		TestULP<256, 19, uint32_t, hasSubnormals, noSupernormals, notSaturating>();  // octo precision
	}

	{
		std::cout << "\nFP8 classic floating-point ULPs\n";
		constexpr size_t nbits = 8;
		constexpr size_t es = 2;
		using Scalar = cfloat<nbits, es, uint32_t, hasSubnormals, noSupernormals, notSaturating>;
		Scalar eps = std::numeric_limits< Scalar >::epsilon();
		std::cout << "FP8 epsilon : " << to_binary(eps) << " : " << eps << '\n';
		for (float base = 0.1f; base < 4.0f; base *= 2.0f) {
			TestULP<nbits, es, std::uint32_t, hasSubnormals, noSupernormals, notSaturating>(base);
		}
	}

	{
		std::cout << "\nhalf-precision FP16 classic floating-point ULPs\n";
		constexpr size_t nbits = 16;
		constexpr size_t es = 5;
		using Scalar = cfloat<nbits, es, uint32_t, hasSubnormals, noSupernormals, notSaturating>;
		Scalar eps = std::numeric_limits< Scalar >::epsilon();
		std::cout << "FP16 epsilon : " << to_binary(eps) << " : " << eps << '\n';
		for (float base = 1.0f; base < 1.0e4f; base *= 1.0e1f) {
			TestULP<nbits, es, std::uint32_t, hasSubnormals, noSupernormals, notSaturating>(base);
		}
	}

	{
		std::cout << "\nBFLOAT16: Brain floating-point ULPs\n";
		constexpr size_t nbits = 16;
		constexpr size_t es = 8;
		using Scalar = cfloat<nbits, es, uint32_t, hasSubnormals, noSupernormals, notSaturating>;
		Scalar eps = std::numeric_limits< Scalar >::epsilon();
		std::cout << "bfloat16 epsilon : " << to_binary(eps) << " : " << eps << '\n';
		for (float base = 1.0f; base < 1.0e10f; base *= 1.0e1f) {
			TestULP<nbits, es, std::uint32_t, hasSubnormals, noSupernormals, notSaturating>(base);
		}
	}
	{
		std::cout << "\n32-bit classic floating-point ULPs as baseline\n";
		constexpr size_t nbits = 32;
		constexpr size_t es = 8;
		using Scalar = cfloat<nbits, es, uint32_t, hasSubnormals, noSupernormals, notSaturating>;
		Scalar eps = std::numeric_limits< Scalar >::epsilon();
		std::cout << "cfloat epsilon : " << to_binary(eps) << " : " << eps << '\n';
		for (float base = 1.0f; base < 1.0e30f; base *= 1.0e3f) {
			TestULP<nbits, es, std::uint32_t, hasSubnormals, noSupernormals, notSaturating>(base);
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
