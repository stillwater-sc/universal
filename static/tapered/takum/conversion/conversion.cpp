// conversion.cpp : test suite runner for conversion operators to takum numbers
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/takum/takum.hpp>
#include <universal/verification/test_suite.hpp>

// Verify that every n-bit takum roundtrips correctly through double conversion:
// for each bit pattern, decode to double, re-encode, and check we get the same bits.
template<unsigned nbits>
int VerifyRoundtrip(bool reportTestCases) {
	using namespace sw::universal;
	using Takum = takum<nbits>;
	int nrOfFailedTestCases = 0;
	const unsigned NR_VALUES = (1u << nbits);

	for (unsigned i = 0; i < NR_VALUES; ++i) {
		Takum original;
		original.setbits(i);

		if (original.isnar()) continue;  // NaR has no double roundtrip

		double d = double(original);
		Takum roundtripped(d);

		if (original.raw_bits() != roundtripped.raw_bits()) {
			++nrOfFailedTestCases;
			if (reportTestCases) {
				std::cerr << "FAIL: bits " << to_binary(original) << " = " << d
				          << " -> " << to_binary(roundtripped) << " = " << double(roundtripped) << '\n';
			}
		}
	}
	return nrOfFailedTestCases;
}

// Verify monotonicity: for all consecutive bit patterns (as signed 2's complement),
// the decoded values should be non-decreasing.
template<unsigned nbits>
int VerifyMonotonicity(bool reportTestCases) {
	using namespace sw::universal;
	using Takum = takum<nbits>;
	int nrOfFailedTestCases = 0;
	const unsigned NR_VALUES = (1u << nbits);

	Takum prev;
	prev.setbits(0);  // start at zero

	for (unsigned i = 1; i < NR_VALUES; ++i) {
		Takum curr;
		curr.setbits(i);

		if (curr.isnar() || prev.isnar()) {
			prev = curr;
			continue;
		}

		// Skip NaR boundary
		if (prev.isnar()) { prev = curr; continue; }

		double d_prev = double(prev);
		double d_curr = double(curr);

		if (d_curr < d_prev) {
			++nrOfFailedTestCases;
			if (reportTestCases) {
				std::cerr << "FAIL monotonicity: bits[" << (i-1) << "]=" << d_prev
				          << " > bits[" << i << "]=" << d_curr << '\n';
			}
		}

		prev = curr;
	}
	return nrOfFailedTestCases;
}

// Verify that encoding specific double values gives the correct result
int VerifySpecificValues(bool reportTestCases) {
	using namespace sw::universal;
	int nrOfFailedTestCases = 0;

	{
		// takum<16>: 1.0 should encode to DR=8 (D=1, R=000), c=0, f=0
		// magnitude = 0b01000_00000000000 = 0x4000
		takum<16> t(1.0);
		if (t.raw_bits() != 0x4000) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: takum<16>(1.0) = " << to_binary(t) << " expected 0x4000\n";
		}
	}
	{
		// takum<16>: -1.0 should be two's complement of 1.0 = ~0x4000 + 1 = 0xC000
		takum<16> t(-1.0);
		if (t.raw_bits() != 0xC000) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: takum<16>(-1.0) = " << to_binary(t) << " expected 0xC000\n";
		}
	}
	{
		// takum<16>: 2.0 should encode to DR=9 (D=1, R=001), c=1, f=0
		// C_bits = 1 - 1 = 0, magnitude = 0b01001_0_0000000000 = 0x4800
		takum<16> t(2.0);
		if (t.raw_bits() != 0x4800) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: takum<16>(2.0) = " << to_binary(t) << " expected 0x4800\n";
		}
	}
	{
		// takum<16>: 0.5 should encode to DR=7 (D=0, R=111), c=-1, f=0
		// r=0, p=11, C_bits=0, M_bits=0
		// magnitude = 0b00111_00000000000 = 0x3800
		takum<16> t(0.5);
		if (t.raw_bits() != 0x3800) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: takum<16>(0.5) = " << to_binary(t) << " expected 0x3800\n";
		}
	}
	{
		// takum<16>: zero should be 0x0000
		takum<16> t(0.0);
		if (t.raw_bits() != 0x0000) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: takum<16>(0.0) = " << to_binary(t) << " expected 0x0000\n";
		}
	}
	{
		// takum<16>: NaR should be 0x8000
		takum<16> t(SpecificValue::nar);
		if (t.raw_bits() != 0x8000) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: takum<16>(NaR) = " << to_binary(t) << " expected 0x8000\n";
		}
	}

	return nrOfFailedTestCases;
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

	std::string test_suite  = "takum conversion validation";
	std::string test_tag    = "conversion";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug


	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifySpecificValues(reportTestCases), "takum<16> specific values", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyRoundtrip<8>(reportTestCases), "takum<8> roundtrip", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMonotonicity<8>(reportTestCases), "takum<8> monotonicity", test_tag);
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyRoundtrip<10>(reportTestCases), "takum<10> roundtrip", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMonotonicity<10>(reportTestCases), "takum<10> monotonicity", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyRoundtrip<12>(reportTestCases), "takum<12> roundtrip", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMonotonicity<12>(reportTestCases), "takum<12> monotonicity", test_tag);
#endif

#if REGRESSION_LEVEL_3

#endif

#if REGRESSION_LEVEL_4

#endif // REGRESSION_LEVEL_4

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::takum_arithmetic_exception& err) {
	std::cerr << "Uncaught takum arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::takum_internal_exception& err) {
	std::cerr << "Uncaught takum internal exception: " << err.what() << std::endl;
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
