// fractional.cpp: test suite runner for classification functions specialized for classic floats
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// use default number system configuration
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/cfloat_math_test_suite.hpp>

#define MANUAL_TESTING 1

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "cfloat<> mathlib fractional validation";
	std::string test_tag    = "fractional";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug

#define MY_DBL_MIN          2.2250738585072014e-308 // minpos value

	constexpr size_t nbits = 32;
	constexpr size_t es = 8;
	using bt = uint32_t;
	constexpr bool hasSubnormals = true;
	constexpr bool hasSupernormals = true;
	constexpr bool isSaturating = false;
	using Real = cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>;

	float fa(1.5), fb(2.25);
	Real a(fa), b(fb);

	std::cout << "IEEE-754 float reference\n";
	std::cout << "fmod      : " << fmod(fa, fb) << " : " << fa << " : " << fb << '\n';
	std::cout << "remainder : " << remainder(fa, fb) << " : " << fa << " : " << fb << '\n';
//	std::cout << "frac      : " << std::frac(fa) << " : " << fa << '\n';

	std::cout << "cfloat results\n";
	std::cout << "fmod      : " << fmod(a, b) << " : " << a << " : " << b << '\n';
	std::cout << "remainder : " << remainder(a, b) << " : " << a << " : " << b << '\n';
	std::cout << "frac      : " << frac(a) << " : " << a << '\n';

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
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
