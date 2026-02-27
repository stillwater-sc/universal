// assignment.cpp: verify assignment and conversion of hfloat hexadecimal floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/hfloat/hfloat.hpp>
#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "hfloat<> assignment and conversion validation";
	int nrOfFailedTestCases = 0;
	bool reportTestCases = true;

	using HfloatShort = hfloat<6, 7, uint32_t>;

	std::cout << test_suite << '\n';

	// Test 1: Integer assignment round-trip
	std::cout << "+---------    Integer assignment round-trip\n";
	{
		int values[] = { 0, 1, -1, 2, -2, 10, -10, 42, -42, 100, -100, 255, 1000 };
		for (int v : values) {
			HfloatShort a(v);
			int back = static_cast<int>(double(a));
			if (back != v) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: hfloat_short(" << v << ") round-trip = " << back << '\n';
			}
		}
	}

	// Test 2: Floating-point assignment round-trip
	std::cout << "+---------    Floating-point assignment round-trip\n";
	{
		double values[] = { 0.0, 1.0, -1.0, 0.5, -0.5, 0.25, -0.25, 0.125, 42.0, -42.0, 256.0 };
		for (double v : values) {
			HfloatShort a(v);
			double back = double(a);
			if (back != v) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: hfloat_short(" << v << ") round-trip = " << back << '\n';
			}
		}
	}

	// Test 3: Powers of 2 (naturally representable in hex float)
	std::cout << "+---------    Powers of 2\n";
	{
		for (int exp = -10; exp <= 10; ++exp) {
			double v = std::ldexp(1.0, exp);
			HfloatShort a(v);
			double back = double(a);
			if (back != v) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: hfloat_short(2^" << exp << " = " << v << ") = " << back << '\n';
			}
		}
	}

	// Test 4: Powers of 16 (the radix)
	std::cout << "+---------    Powers of 16 (hex radix)\n";
	{
		for (int exp = -4; exp <= 4; ++exp) {
			double v = std::pow(16.0, exp);
			HfloatShort a(v);
			double back = double(a);
			double err = std::fabs(back - v);
			if (err > v * 1e-6) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: hfloat_short(16^" << exp << " = " << v << ") = " << back << '\n';
			}
		}
	}

	// Test 5: SpecificValue constructor - no NaN, no inf
	std::cout << "+---------    SpecificValue constructor (no NaN, no inf)\n";
	{
		HfloatShort zero(SpecificValue::zero);
		if (!zero.iszero()) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: SpecificValue::zero not zero\n";
		}
		HfloatShort nan(SpecificValue::qnan);
		if (!nan.iszero()) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: SpecificValue::qnan should map to zero for hfloat\n";
		}
		HfloatShort inf(SpecificValue::infpos);
		// infpos maps to maxpos
		double dmax = double(HfloatShort(SpecificValue::maxpos));
		double dinf = double(inf);
		if (dinf != dmax) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: SpecificValue::infpos should map to maxpos for hfloat\n";
		}
	}

	// Test 6: Unsigned type assignment
	std::cout << "+---------    Unsigned type assignment\n";
	{
		unsigned values[] = { 0u, 1u, 10u, 255u, 1000u };
		for (unsigned v : values) {
			HfloatShort a(v);
			unsigned back = static_cast<unsigned>(double(a));
			if (back != v) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: hfloat_short(unsigned " << v << ") round-trip = " << back << '\n';
			}
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
	std::cerr << "caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
