// sat_subnormal_conversion.cpp: test suite runner for subnormal IEEE-754 floating-point to fixed-point 
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <iomanip>

// Configure the fixpnt template environment
// first: enable general or specialized fixed-point configurations
#define FIXPNT_FAST_SPECIALIZATION
// second: enable/disable fixpnt arithmetic exceptions
#define FIXPNT_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/verification/fixpnt_test_suite.hpp>

/*

IEEE-754 has subnormal numbers that a fixed-point needs to be able to pick up.

The stored exponents 0x00 and 0xFF are interpreted specially.

Exponent     |     fraction = 0     |      fraction ≠ 0      |      Equation
0x00         |       zero           |    subnormal number    |    (-1)sign * 0.fraction * 2^-126
0x01...0xFE  |                normal value                   |    (-1)sign * 1.fraction * 2^(exponent - 127)
0xFF         |    ±infinity         |    NaN(quiet, signalling)

The minimum positive normal value is 2−126 ≈ 1.18 × 10−38.
The minimum positive(subnormal) value is 2−149 ≈ 1.4 × 10−45.
*/

void TestDenormalizedNumberConversions() {
	using namespace sw::universal;

	constexpr unsigned nbits = 151;
	constexpr unsigned rbits = 149;
	// minimum positive normal value of a single precision float == 2^-126
	float minpos_normal = 1.1754943508222875079687365372222e-38;
	std::cout << to_binary(minpos_normal) << '\n';
	float minpos_subnormal = 1.4012984643248170709237295832899e-45;
	std::cout << to_binary(minpos_subnormal) << '\n';

	fixpnt<nbits, rbits> a;
	a.setbits(0x1);
	float f = minpos_subnormal;
	for (int i = 0; i < 23; ++i) {
		std::cout << std::setw(10) << f << ' ' << to_binary(f) << ' ' << to_binary(a) << ' ' << a << '\n';
		f *= 2.0f;
		a *= 2;
	}
}

/*

IEEE-754 has subnormal numbers that a fixed-point needs to be able to pick up.

The stored exponents 0x000 and 0x7FF are interpreted specially.

Exponent      |     fraction = 0     |      fraction ≠ 0      |      Equation
0x000         |       zero           |    subnormal number    |    (-1)^sign * 0.fraction * 2^-1022
0x001...0x7FE |                normal value                   |    (-1)^sign * 1.fraction * 2^(exponent - 1023)
0x7FF         |    ±infinity         |    NaN(quiet, signalling)

The minimum positive normal value is 2−1022 ≈ 2.22e-308.
The minimum positive(subnormal) value is 2−1074 ≈ 1.4 × 10−45.
*/

template<size_t nbits, size_t rbits>
void FloatGenerateFixedPointValues(std::ostream& ostr = std::cout) {
	constexpr size_t NR_TEST_CASES = (size_t(1) << nbits);
	sw::universal::fixpnt<nbits, rbits> a;
	ostr << "  fixpnt<" << nbits << "," << rbits << ">\n";
	for (size_t i = 0; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		float f = float(a);
		ostr << to_binary(a) << " | " << to_triple(a) << " | " << std::setw(15) << a << " | " << std::setw(15) << f << std::endl;
	}
}

template<size_t nbits, size_t rbits>
void DoubleGenerateFixedPointValues(std::ostream& ostr = std::cout) {
	constexpr size_t NR_TEST_CASES = (size_t(1) << nbits);
	sw::universal::fixpnt<nbits, rbits> a;
	ostr << "  fixpnt<" << nbits << "," << rbits << ">\n";
	for (size_t i = 0; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		double f = double(a);
		ostr << to_binary(a) << " | " << to_triple(a) << " | " << std::setw(15) << a << " | " << std::setw(15) << f << std::endl;
	}
}

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
//#undef REGRESSION_LEVEL_OVERRIDE
#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "Fixed-point saturating subnormal conversion";
	std::string test_tag    = "conversion of IEEE-754 subnormals";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	// minpos_subnormal value
	float multiplier = 1.4012984643248170709237295832899e-45;
	for (int i = 0; i < 149 - 127; ++i) {
		multiplier *= 2.0f;
	}
	std::cout << to_binary(multiplier) << std::endl;
	float minpos_normal = 1.1754943508222875079687365372222e-38;
	std::cout << to_binary(minpos_normal) << std::endl;

	// minimum positive normal value of a single precision float == 2^-1022
	double dbl_minpos_normal = 2.2250738585072013830902327173324e-308;
	std::cout << to_binary(dbl_minpos_normal) << std::endl;
	double dbl_minpos_subnormal = 4.940656458412465441765687928622e-324;
	std::cout << to_binary(dbl_minpos_subnormal) << std::endl;

	FloatGenerateFixedPointValues<8, 4>();
	DoubleGenerateFixedPointValues<8, 4>();

	// can't use the regular exhaustive test suites for these very large fixed-points
	// nrOfFailedTestCases = ReportTestResult(ValidateAssignment<256, 150, Modular, uint32_t, float>(reportTestCases), tag, "fixpnt<4,0, Modular, uint32_t>");
	
	TestDenormalizedNumberConversions();

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

std::cout << "Fixed-point saturating subnormal conversion : TODO\n";

#if REGRESSION_LEVEL_1

#endif

#if REGRESSION_LEVEL_2

#endif

#if REGRESSION_LEVEL_3

#endif

#if REGRESSION_LEVEL_4

#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::fixpnt_arithmetic_exception& err) {
	std::cerr << "Uncaught fixpnt arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::fixpnt_internal_exception& err) {
	std::cerr << "Uncaught fixpnt internal exception: " << err.what() << std::endl;
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
