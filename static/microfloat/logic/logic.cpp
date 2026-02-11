// logic.cpp: comparison operator tests for microfloat types
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
int VerifyLogicOperators() {
	int nrOfFailedTestCases = 0;

	MicrofloatType a(1.0f), b(2.0f), c(1.0f);

	// equality
	if (!(a == c)) { ++nrOfFailedTestCases; std::cerr << "FAIL: 1.0 == 1.0\n"; }
	if (a == b) { ++nrOfFailedTestCases; std::cerr << "FAIL: 1.0 != 2.0\n"; }

	// inequality
	if (!(a != b)) { ++nrOfFailedTestCases; std::cerr << "FAIL: 1.0 != 2.0\n"; }
	if (a != c) { ++nrOfFailedTestCases; std::cerr << "FAIL: 1.0 == 1.0 (!=)\n"; }

	// less than
	if (!(a < b)) { ++nrOfFailedTestCases; std::cerr << "FAIL: 1.0 < 2.0\n"; }
	if (b < a) { ++nrOfFailedTestCases; std::cerr << "FAIL: !(2.0 < 1.0)\n"; }

	// greater than
	if (!(b > a)) { ++nrOfFailedTestCases; std::cerr << "FAIL: 2.0 > 1.0\n"; }
	if (a > b) { ++nrOfFailedTestCases; std::cerr << "FAIL: !(1.0 > 2.0)\n"; }

	// less than or equal
	if (!(a <= c)) { ++nrOfFailedTestCases; std::cerr << "FAIL: 1.0 <= 1.0\n"; }
	if (!(a <= b)) { ++nrOfFailedTestCases; std::cerr << "FAIL: 1.0 <= 2.0\n"; }

	// greater than or equal
	if (!(a >= c)) { ++nrOfFailedTestCases; std::cerr << "FAIL: 1.0 >= 1.0\n"; }
	if (!(b >= a)) { ++nrOfFailedTestCases; std::cerr << "FAIL: 2.0 >= 1.0\n"; }

	// NaN comparisons
	if constexpr (MicrofloatType::hasNaN) {
		MicrofloatType nan_val;
		nan_val.setnan();
		if (nan_val == nan_val) { ++nrOfFailedTestCases; std::cerr << "FAIL: NaN == NaN should be false\n"; }
		if (!(nan_val != nan_val)) { ++nrOfFailedTestCases; std::cerr << "FAIL: NaN != NaN should be true\n"; }
		if (nan_val < a) { ++nrOfFailedTestCases; std::cerr << "FAIL: NaN < 1.0 should be false\n"; }
		if (nan_val > a) { ++nrOfFailedTestCases; std::cerr << "FAIL: NaN > 1.0 should be false\n"; }
	}

	// zero comparisons
	MicrofloatType pos_zero, neg_zero;
	pos_zero.setzero();
	neg_zero.setbits(MicrofloatType::sign_mask); // -0
	if (!(pos_zero == neg_zero)) { ++nrOfFailedTestCases; std::cerr << "FAIL: +0 == -0\n"; }

	return nrOfFailedTestCases;
}

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "microfloat logic operator tests";
	int nrOfFailedTestCases = 0;

	std::cout << "e2m1 logic tests\n";
	nrOfFailedTestCases += VerifyLogicOperators<e2m1>();

	std::cout << "e2m3 logic tests\n";
	nrOfFailedTestCases += VerifyLogicOperators<e2m3>();

	std::cout << "e3m2 logic tests\n";
	nrOfFailedTestCases += VerifyLogicOperators<e3m2>();

	std::cout << "e4m3 logic tests\n";
	nrOfFailedTestCases += VerifyLogicOperators<e4m3>();

	std::cout << "e5m2 logic tests\n";
	nrOfFailedTestCases += VerifyLogicOperators<e5m2>();

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
