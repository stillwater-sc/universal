// ulp.cpp: testing ulp values and algebra for classic floating-point cfloat configurations
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// Configure the cfloat template environment
// first: enable general or specialized configurations
#define CFLOAT_FAST_SPECIALIZATION
// second: enable/disable arithmetic exceptions
#define CFLOAT_THROW_ARITHMETIC_EXCEPTION 1
// third: enable native literals in logic and arithmetic operations
#define CFLOAT_ENABLE_LITERALS 1

// minimum set of include files to reflect source code dependencies
#include <universal/number/cfloat/cfloat_impl.hpp>
#include <universal/number/cfloat/numeric_limits.hpp>
// type manipulators such as pretty printers
#include <universal/number/cfloat/manipulators.hpp>
#include <universal/number/cfloat/mathlib.hpp>

template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
void TestULP() 
{
	using namespace sw::universal;

	using Cfloat = cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>;
	Cfloat a(1.0f);
	std::cout << typeid(a).name() << '\n';
	double da(1.0);
	std::cout << "cfloat at 1.0  : " << to_binary(a) << " : ULP : " << to_binary(ulp(a)) << " : value : " << a << '\n';
	std::cout << "double at 1.0  : " << to_binary(da) << " : ULP : " << to_binary(ulp(da)) << " : value : " << da << '\n';

	a = std::numeric_limits< Cfloat >::epsilon();
	std::cout << "cfloat epsilon : " << to_binary(a) << " : " << a << '\n';
}

// conditional compile flags
#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main()
try {
	using namespace sw::universal;

	int nrOfFailedTestCases = 0;

	std::cout << "classic floating-point ULP tests\n";

	constexpr bool hasSubnormals = true;
	constexpr bool hasSupernormals = true;
	constexpr bool isSaturating = true;
	TestULP<8, 2, uint8_t, !hasSubnormals, !hasSupernormals, !isSaturating>();     // quarter precision
	TestULP<16, 5, uint16_t, hasSubnormals, !hasSupernormals, !isSaturating>();    // half precision
	TestULP<32, 8, uint32_t, hasSubnormals, !hasSupernormals, !isSaturating>();    // single precision
	TestULP<64, 11, uint64_t, hasSubnormals, !hasSupernormals, !isSaturating>();   // double precision
//	TestULP<128, 15, uint32_t, hasSubnormals, !hasSupernormals, !isSaturating>();  // quad precision

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
