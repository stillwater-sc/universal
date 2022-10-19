// ulp.cpp: testing ulp values and algebra for generalized posit configurations
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
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
void TestULP() 
{
	using namespace sw::universal;

	using Posit = posit<nbits, es, bt>;
	Posit a(1.0f);
	std::cout << typeid(a).name() << '\n';
	double da(1.0);
	std::cout << "posit  at 1.0  : " << to_binary(a) << " : ULP : " << to_binary(ulp(a)) << " : value : " << a << '\n';
	std::cout << "double at 1.0  : " << to_binary(da) << " : ULP : " << to_binary(ulp(da)) << " : value : " << da << '\n';

	a = std::numeric_limits< Posit >::epsilon();
	std::cout << "posit epsilon : " << to_binary(a) << " : " << a << '\n';
}

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "generalized posit ULP tests";
	std::string test_tag    = "ulp";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	std::cout << "classic floating-point ULP tests\n";

	TestULP<8, 2, uint8_t>();     // quarter precision
	TestULP<16, 2, uint16_t>();   // half precision
	TestULP<32, 2, uint32_t>();   // single precision
	TestULP<64, 2, uint32_t>();   // double precision
	TestULP<128, 2, uint32_t>();  // quad precision
	TestULP<256, 2, uint32_t>();  // octo precision

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
