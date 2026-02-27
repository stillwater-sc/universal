// assignment.cpp: verify assignment and conversion of dfloat decimal floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/dfloat/dfloat.hpp>
#include <universal/verification/test_suite.hpp>

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

	std::string test_suite = "dfloat<> assignment and conversion validation";
	std::string test_tag = "dfloat<> assignment";
	bool reportTestCases = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

#if REGRESSION_LEVEL_1

	// Test 1: Integer assignment round-trip for decimal32
	std::cout << "+---------    Integer assignment round-trip (decimal32)\n";
	{
		using Decimal32 = dfloat<7, 6, DecimalEncoding::BID, uint32_t>;
		int values[] = { 0, 1, -1, 2, -2, 10, -10, 42, -42, 100, 999, -999, 9999999, -9999999 };
		for (int v : values) {
			Decimal32 a(v);
			int back = static_cast<int>(double(a));
			if (back != v) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: decimal32(" << v << ") round-trip = " << back << '\n';
			}
		}
	}

	// Test 2: Floating-point assignment round-trip
	std::cout << "+---------    Floating-point assignment round-trip (decimal32)\n";
	{
		using Decimal32 = dfloat<7, 6, DecimalEncoding::BID, uint32_t>;
		double values[] = { 0.0, 1.0, -1.0, 0.5, -0.5, 0.25, 0.125, 42.0, -42.0, 1e6, -1e6 };
		for (double v : values) {
			Decimal32 a(v);
			double back = double(a);
			if (back != v) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: decimal32(" << v << ") round-trip = " << back << '\n';
			}
		}
	}

	// Test 3: Key decimal property: 0.1 is representable exactly
	std::cout << "+---------    Decimal exactness: 0.1 * 10 == 1.0\n";
	{
		using Decimal32 = dfloat<7, 6, DecimalEncoding::BID, uint32_t>;
		Decimal32 tenth(0.1);
		Decimal32 sum(0.0);
		for (int i = 0; i < 10; ++i) sum += tenth;
		double result = double(sum);
		if (result != 1.0) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: 10 * 0.1 = " << result << " (expected 1.0)\n";
		}
	}

	// Test 4: Assignment from unsigned types
	std::cout << "+---------    Unsigned type assignment\n";
	{
		using Decimal32 = dfloat<7, 6, DecimalEncoding::BID, uint32_t>;
		unsigned values[] = { 0u, 1u, 10u, 255u, 1000u, 9999999u };
		for (unsigned v : values) {
			Decimal32 a(v);
			unsigned back = static_cast<unsigned>(double(a));
			if (back != v) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: decimal32(unsigned " << v << ") round-trip = " << back << '\n';
			}
		}
	}

	// Test 5: SpecificValue constructor
	std::cout << "+---------    SpecificValue constructor\n";
	{
		using Decimal32 = dfloat<7, 6, DecimalEncoding::BID, uint32_t>;
		Decimal32 pz(SpecificValue::zero);
		if (!pz.iszero()) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: SpecificValue::zero is not zero\n";
		}
		Decimal32 pinf(SpecificValue::infpos);
		if (!pinf.isinf()) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: SpecificValue::infpos is not inf\n";
		}
		Decimal32 pnan(SpecificValue::qnan);
		if (!pnan.isnan()) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: SpecificValue::qnan is not nan\n";
		}
		Decimal32 mp(SpecificValue::maxpos);
		if (double(mp) <= 0) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: SpecificValue::maxpos is not positive\n";
		}
	}

#endif

#if REGRESSION_LEVEL_2
	// Test 6: DPD encoding assignment round-trip
	std::cout << "+---------    DPD encoding assignment round-trip\n";
	{
		using DpdDecimal32 = dfloat<7, 6, DecimalEncoding::DPD, uint32_t>;
		int values[] = { 0, 1, -1, 42, -42, 100, 999, -999, 9999999, -9999999 };
		for (int v : values) {
			DpdDecimal32 a(v);
			int back = static_cast<int>(double(a));
			if (back != v) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: dpd decimal32(" << v << ") round-trip = " << back << '\n';
			}
		}
	}
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
	std::cerr << "caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
