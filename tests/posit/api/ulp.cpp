// ulp.cpp: testing ulp values and algebra for posit configurations
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// Configure the posit template environment
// first: enable general or specialized configurations
#define POSIT_FAST_SPECIALIZATION
// second: enable/disable arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
// third: enable native literals in logic and arithmetic operations
#define POSIT_ENABLE_LITERALS 1
#include <universal/number/posit/posit.hpp>
#include <universal/number/posit/numeric_limits.hpp>
#include <universal/number/posit/mathlib.hpp>

template<size_t nbits, size_t es>
void TestULP() 
{
	using namespace sw::universal;

	posit<nbits, es> a(1.0f);
	std::cout << type_tag(a) << '\n';
//	double da(1.0);
	std::cout << "posit at 1.0  : " << to_binary(a) << " : ULP : " << to_binary(ulp(a)) << '\n';
//	std::cout << "double at 1.0 : " << to_binary(da) << " : ULP : " << to_binary(ulp(da)) << '\n';

	a = std::numeric_limits< posit<nbits, es> >::epsilon();
	std::cout << "posit epsilon : " << to_binary(a) << " : " << a << '\n';
}

// conditional compile flags
#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main()
try {
	using namespace sw::universal;

	int nrOfFailedTestCases = 0;

	std::cout << "posit ULP tests\n";

#if MANUAL_TESTING
	TestULP<  8, 0>();
	TestULP< 16, 1>();
	TestULP< 32, 2>();
	TestULP< 64, 3>();
	TestULP<128, 4>();

#else 

#if REGRESSION_LEVEL_1
	TestULP< 8, 0>();
	TestULP< 8, 1>();
	TestULP< 8, 2>();
	TestULP<16, 2>();
	TestULP<32, 2>();
	TestULP<64, 2>();
#endif

#if REGRESSION_LEVEL_2

#endif

#if REGRESSION_LEVEL_3

#endif

#if REGRESSION_LEVEL_4
	TestULP<64, 3>();
#endif

#endif // MANUAL_TESTING

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
