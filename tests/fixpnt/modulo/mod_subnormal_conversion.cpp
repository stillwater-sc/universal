// mod_subnormal_conversion.cpp: test suite runner for subnormal IEEE-754 floating-point to fixed-point 
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
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

// minimum set of include files to reflect source code dependencies
#include <universal/number/fixpnt/fixpnt_impl.hpp>
#include <universal/number/fixpnt/manipulators.hpp>
#include <universal/number/fixpnt/math_functions.hpp>
#include <universal/verification/fixpnt_test_suite.hpp>

/*

IEEE-754 has subnormal numbers that a fixed-point needs to be able to pick up.

The exponent values 0x00 and 0xFF are encoding special cases.

Exponent     |     fraction = 0     |      fraction ≠ 0      |      Equation
0x00         |       zero           |    subnormal number    |    (-1)sign * 0.fraction * 2^-126
0x01...0xFE  |                normal value                   |    (-1)sign * 1.fraction * 2^(exponent - 127)
0xFF         |    ±infinity         |    NaN(quiet, signalling)

The minimum positive normal value is 2−126 ≈ 1.18 × 10−38.
The minimum positive(subnormal) value is 2−149 ≈ 1.4 × 10−45.
*/

template<size_t nbits, size_t rbits>
void TestDenormalizedNumberConversions() {
	using namespace std;
	using namespace sw::universal;

	// minimum positive normal value of a single precision float == 2^-126
	float minpos_normal = 1.1754943508222875079687365372222e-38;
	cout << to_binary(minpos_normal) << endl;
	float minpos_subnormal = 1.4012984643248170709237295832899e-45;
	cout << to_binary(minpos_subnormal) << endl;

	fixpnt<nbits, rbits> a;
	float f = minpos_normal;
	for (int i = 0; i < 16; ++i) {
		f *= 0.5f;
		a = 1;
		cout << to_float(a) << endl;
//		cout << setw(10) << f << ' ' << to_binary(f) << ' ' << to_binary(a) << ' ' << a << endl;
	}
}

/*

IEEE-754 has subnormal numbers that a fixed-point needs to be able to pick up.

The exponent values 0x000 and 0x7FF are encoding special cases.

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
		a.set_raw_bits(i);
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
		a.set_raw_bits(i);
		double f = double(a);
		ostr << to_binary(a) << " | " << to_triple(a) << " | " << std::setw(15) << a << " | " << std::setw(15) << f << std::endl;
	}
}

// conditional compile flags
#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	// bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	std::string tag = "conversion of IEEE-754 subnormals: ";

#if MANUAL_TESTING
	// minpos_subnormal value
	float multiplier = 1.4012984643248170709237295832899e-45;
	for (int i = 0; i < 149 - 127; ++i) {
		multiplier *= 2.0f;
	}
	std::cout << to_binary(multiplier) << std::endl;
	float minpos_normal = 1.1754943508222875079687365372222e-38;
	std::cout << to_binary(minpos_normal) << std::endl;

	// minimum positive normal value of a double precision float == 2^-1022
	double dbl_minpos_normal = 2.2250738585072013830902327173324e-308;
	cout << to_binary(dbl_minpos_normal) << endl;
	double dbl_minpos_subnormal = 4.940656458412465441765687928622e-324;
	cout << to_binary(dbl_minpos_subnormal) << endl;

	//FloatGenerateFixedPointValues<8, 4>();
	//DoubleGenerateFixedPointValues<8, 4>();

	// can't use the regular exhaustive test suites for these very large fixed-points
	// nrOfFailedTestCases = ReportTestResult(VerifyAssignment<256, 150, Modular, uint32_t, float>(bReportIndividualTestCases), tag, "fixpnt<4,0, Modular, uint32_t>");


#if STRESS_TESTING

	// manual stress test

#endif

#else
	cout << "Fixed-point modular assignment validation" << endl;


#if STRESS_TESTING

	// regression stress test

#endif  // STRESS_TESTING

#endif  // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
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
