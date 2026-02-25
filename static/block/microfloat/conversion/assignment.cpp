// assignment.cpp: test from_float / to_float round trips for microfloat types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

#define MICROFLOAT_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/microfloat/microfloat.hpp>
#include <universal/verification/test_suite.hpp>

template<typename MicrofloatType>
int VerifyAssignment() {
	int nrOfFailedTestCases = 0;

	// Test assignment from float and round-trip
	float testValues[] = { 0.0f, 1.0f, -1.0f, 0.5f, -0.5f, 2.0f, -2.0f, 0.25f };
	for (float fv : testValues) {
		MicrofloatType a(fv);
		float roundtrip = float(a);
		MicrofloatType b(roundtrip);
		// The round-trip should be idempotent
		if (a.bits() != b.bits()) {
			std::cerr << "FAIL: round-trip failed for " << fv
				<< " : a=" << sw::universal::to_binary(a) << "(" << float(a) << ")"
				<< " b=" << sw::universal::to_binary(b) << "(" << float(b) << ")\n";
			++nrOfFailedTestCases;
		}
	}

	// Test assignment from integer types
	{
		MicrofloatType a;
		a = 0; if (!a.iszero()) { ++nrOfFailedTestCases; std::cerr << "FAIL: assignment from int(0)\n"; }
		a = 1; if (float(a) != 1.0f) { ++nrOfFailedTestCases; std::cerr << "FAIL: assignment from int(1)\n"; }
		a = -1; if (float(a) != -1.0f) { ++nrOfFailedTestCases; std::cerr << "FAIL: assignment from int(-1)\n"; }
	}

	return nrOfFailedTestCases;
}

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "microfloat assignment tests";
	int nrOfFailedTestCases = 0;

	std::cout << "e2m1 assignment tests\n";
	nrOfFailedTestCases += VerifyAssignment<e2m1>();

	std::cout << "e2m3 assignment tests\n";
	nrOfFailedTestCases += VerifyAssignment<e2m3>();

	std::cout << "e3m2 assignment tests\n";
	nrOfFailedTestCases += VerifyAssignment<e3m2>();

	std::cout << "e4m3 assignment tests\n";
	nrOfFailedTestCases += VerifyAssignment<e4m3>();

	std::cout << "e5m2 assignment tests\n";
	nrOfFailedTestCases += VerifyAssignment<e5m2>();

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
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
