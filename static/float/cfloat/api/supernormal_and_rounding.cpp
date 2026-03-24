// supernormal_and_rounding.cpp: regression tests for integer-to-cfloat
// rounding accuracy
//
// Copyright (C) 2026 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under
// an MIT Open Source license.
//
// This test validates bug fixes in convert_unsigned/signed_integer's round<>:
//
// 1. The sticky bit mask used (shift-2) instead of (shift-1), missing one bit
//    position from the sticky calculation. For E4M3 with 32-bit int input,
//    this produced incorrect round-to-even decisions (e.g., integer 101
//    rounded to 96 instead of 104).
//
// 2. When rounding overflowed the fraction (e.g., 0b111 + 1 = 0b1000), the
//    code did raw >>= 1 which left non-zero bits in the fraction. The correct
//    result after overflow to the next exponent is fraction = 0 (e.g., integer
//    31 became 48 instead of 32).
//
// 3. convert_unsigned/signed_integer did not check whether the exponent
//    exceeded the encoding range after rounding. For small formats, integers
//    beyond maxpos produced garbage bit patterns.
#include <universal/utility/directives.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_suite.hpp>

// Integer-to-cfloat rounding accuracy test.
// The conversion through the integer path should match the conversion through
// double for all values within the representable range.
template<size_t nbits, size_t es, typename bt,
	bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
int TestIntegerConversionRounding() {
	using namespace sw::universal;
	int fails = 0;

	using Cfloat = cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>;

	// determine the upper bound: clamp the double BEFORE narrowing to int
	Cfloat mp;
	mp.maxpos();
	double maxValD = static_cast<double>(mp);
	if (maxValD > 10000.0) maxValD = 10000.0;
	if (maxValD < 1.0) maxValD = 1.0;
	int maxVal = static_cast<int>(maxValD);

	// test positive signed integer values
	for (int v = 1; v <= maxVal; ++v) {
		Cfloat from_int, from_dbl;
		from_int = v;                          // integer conversion path
		from_dbl = static_cast<double>(v);     // IEEE-754 conversion path

		if (from_int != from_dbl) {
			std::cerr << "FAIL: cfloat<" << nbits << "," << es
			          << "> integer " << v << ": int path=" << from_int
			          << " (" << to_binary(from_int) << ")"
			          << " double path=" << from_dbl
			          << " (" << to_binary(from_dbl) << ")\n";
			++fails;
		}
	}

	// test negative signed integer values
	for (int v = -1; v >= -maxVal; --v) {
		Cfloat from_int, from_dbl;
		from_int = v;
		from_dbl = static_cast<double>(v);

		if (from_int != from_dbl) {
			std::cerr << "FAIL: cfloat<" << nbits << "," << es
			          << "> integer " << v << ": int path=" << from_int
			          << " (" << to_binary(from_int) << ")"
			          << " double path=" << from_dbl
			          << " (" << to_binary(from_dbl) << ")\n";
			++fails;
		}
	}

	// test unsigned integer values
	unsigned maxUVal = static_cast<unsigned>(maxVal);
	for (unsigned v = 1; v <= maxUVal; ++v) {
		Cfloat from_uint, from_dbl;
		from_uint = v;                         // unsigned integer conversion path
		from_dbl = static_cast<double>(v);     // IEEE-754 conversion path

		if (from_uint != from_dbl) {
			std::cerr << "FAIL: cfloat<" << nbits << "," << es
			          << "> unsigned " << v << ": uint path=" << from_uint
			          << " double path=" << from_dbl << "\n";
			++fails;
		}
	}

	return fails;
}

// Regression testing guards
#define MANUAL_TESTING 0
#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "cfloat<> integer-to-cfloat rounding regression";
	std::string test_tag    = "integer_rounding";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1

	// =========================================================
	// Integer-to-cfloat rounding accuracy (standard configs)
	// =========================================================
	std::cout << "integer rounding (standard)      : ";
	{
		int fails = 0;
		// noSubnormals, noMaxExpValues, notSaturating (fff)
		fails += TestIntegerConversionRounding<8, 2, uint8_t, false, false, false>();
		fails += TestIntegerConversionRounding<8, 3, uint8_t, false, false, false>();
		fails += TestIntegerConversionRounding<8, 4, uint8_t, false, false, false>();
		fails += TestIntegerConversionRounding<8, 5, uint8_t, false, false, false>();
		fails += TestIntegerConversionRounding<10, 4, uint8_t, false, false, false>();
		fails += TestIntegerConversionRounding<12, 5, uint8_t, false, false, false>();
		fails += TestIntegerConversionRounding<16, 5, uint8_t, false, false, false>();
		nrOfFailedTestCases += fails;
		std::cout << (fails == 0 ? "PASS" : "FAIL") << '\n';
	}

	// =========================================================
	// Integer-to-cfloat rounding accuracy (subsuper configs)
	// =========================================================
	std::cout << "integer rounding (subsuper)       : ";
	{
		int fails = 0;
		// hasSubnormals, hasMaxExpValues, notSaturating (ttf)
		fails += TestIntegerConversionRounding<8, 2, uint8_t, true, true, false>();
		fails += TestIntegerConversionRounding<8, 3, uint8_t, true, true, false>();
		fails += TestIntegerConversionRounding<8, 4, uint8_t, true, true, false>();
		fails += TestIntegerConversionRounding<8, 5, uint8_t, true, true, false>();
		fails += TestIntegerConversionRounding<10, 4, uint8_t, true, true, false>();
		fails += TestIntegerConversionRounding<12, 5, uint8_t, true, true, false>();
		fails += TestIntegerConversionRounding<16, 5, uint8_t, true, true, false>();
		nrOfFailedTestCases += fails;
		std::cout << (fails == 0 ? "PASS" : "FAIL") << '\n';
	}

#endif

#if REGRESSION_LEVEL_2

	// wider formats
	std::cout << "integer rounding wider formats    : ";
	{
		int fails = 0;
		fails += TestIntegerConversionRounding<32, 8, uint8_t, false, false, false>();
		fails += TestIntegerConversionRounding<32, 8, uint8_t, true, true, false>();
		nrOfFailedTestCases += fails;
		std::cout << (fails == 0 ? "PASS" : "FAIL") << '\n';
	}

#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif // MANUAL_TESTING
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
