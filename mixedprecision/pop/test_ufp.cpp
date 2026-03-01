// test_ufp.cpp: validate UFP (unit in the first place) computation
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
//
// Tests compute_ufp against known values and validates integration
// with range_analyzer.

#include <universal/utility/directives.hpp>
#include <universal/mixedprecision/ufp.hpp>
#include <universal/utility/range_analyzer.hpp>
#include <iostream>
#include <string>
#include <cmath>
#include <vector>
#include <limits>

namespace sw { namespace universal {

int TestUfpBasic() {
	int nrOfFailedTestCases = 0;

	// Powers of 2
	struct test_case { double value; int expected_ufp; };
	std::vector<test_case> cases = {
		{   1.0,   0 },
		{   2.0,   1 },
		{   4.0,   2 },
		{   8.0,   3 },
		{  16.0,   4 },
		{   0.5,  -1 },
		{  0.25,  -2 },
		{ 0.125,  -3 },
		{ 1024.0, 10 },
	};

	for (const auto& tc : cases) {
		int result = compute_ufp(tc.value);
		if (result != tc.expected_ufp) {
			std::cerr << "FAIL: compute_ufp(" << tc.value << ") = " << result
			          << ", expected " << tc.expected_ufp << std::endl;
			++nrOfFailedTestCases;
		}
	}

	// Non-powers of 2
	// ufp(3.0) = floor(log2(3)) = 1
	if (compute_ufp(3.0) != 1) {
		std::cerr << "FAIL: compute_ufp(3.0) = " << compute_ufp(3.0) << ", expected 1" << std::endl;
		++nrOfFailedTestCases;
	}

	// ufp(7.0) = floor(log2(7)) = 2
	if (compute_ufp(7.0) != 2) {
		std::cerr << "FAIL: compute_ufp(7.0) = " << compute_ufp(7.0) << ", expected 2" << std::endl;
		++nrOfFailedTestCases;
	}

	// ufp(0.3) = floor(log2(0.3)) = -2
	if (compute_ufp(0.3) != -2) {
		std::cerr << "FAIL: compute_ufp(0.3) = " << compute_ufp(0.3) << ", expected -2" << std::endl;
		++nrOfFailedTestCases;
	}

	return nrOfFailedTestCases;
}

int TestUfpNegative() {
	int nrOfFailedTestCases = 0;

	// Negative values should give same ufp as positive
	if (compute_ufp(-1.0) != 0) {
		std::cerr << "FAIL: compute_ufp(-1.0) expected 0, got " << compute_ufp(-1.0) << std::endl;
		++nrOfFailedTestCases;
	}
	if (compute_ufp(-8.0) != 3) {
		std::cerr << "FAIL: compute_ufp(-8.0) expected 3, got " << compute_ufp(-8.0) << std::endl;
		++nrOfFailedTestCases;
	}

	return nrOfFailedTestCases;
}

int TestUfpSpecialValues() {
	int nrOfFailedTestCases = 0;

	// Zero should return sentinel
	int ufp_zero = compute_ufp(0.0);
	if (ufp_zero != std::numeric_limits<int>::min()) {
		std::cerr << "FAIL: compute_ufp(0.0) should return INT_MIN, got " << ufp_zero << std::endl;
		++nrOfFailedTestCases;
	}

	return nrOfFailedTestCases;
}

int TestUfpRange() {
	int nrOfFailedTestCases = 0;

	// Test ufp from range [lo, hi]
	int ufp = compute_ufp(-3.0, 10.0);
	// max(|-3|, |10|) = 10, ufp(10) = 3
	if (ufp != 3) {
		std::cerr << "FAIL: compute_ufp(-3, 10) expected 3, got " << ufp << std::endl;
		++nrOfFailedTestCases;
	}

	ufp = compute_ufp(-100.0, 50.0);
	// max(100, 50) = 100, ufp(100) = 6
	if (ufp != 6) {
		std::cerr << "FAIL: compute_ufp(-100, 50) expected 6, got " << ufp << std::endl;
		++nrOfFailedTestCases;
	}

	return nrOfFailedTestCases;
}

int TestUfpRangeAnalyzerIntegration() {
	int nrOfFailedTestCases = 0;

	// Create a range_analyzer, observe some values, check ufp
	range_analyzer<double> ra;
	ra.observe(1.0);
	ra.observe(8.5);
	ra.observe(0.5);
	ra.observe(-3.0);

	int ufp = ufp_from_analyzer(ra);
	// maxScale should be floor(log2(8.5)) = 3
	if (ufp != 3) {
		std::cerr << "FAIL: ufp_from_analyzer expected 3, got " << ufp << std::endl;
		++nrOfFailedTestCases;
	}

	// Larger values
	range_analyzer<double> ra2;
	ra2.observe(1024.0);
	ra2.observe(0.001);
	int ufp2 = ufp_from_analyzer(ra2);
	// maxScale = floor(log2(1024)) = 10
	if (ufp2 != 10) {
		std::cerr << "FAIL: ufp_from_analyzer (large) expected 10, got " << ufp2 << std::endl;
		++nrOfFailedTestCases;
	}

	return nrOfFailedTestCases;
}

}} // namespace sw::universal

#define TEST_CASE(name, func) \
	do { \
		int fails = func; \
		if (fails) { \
			std::cout << name << ": FAIL (" << fails << " errors)" << std::endl; \
			nrOfFailedTestCases += fails; \
		} else { \
			std::cout << name << ": PASS" << std::endl; \
		} \
	} while(0)

int main()
try {
	using namespace sw::universal;

	int nrOfFailedTestCases = 0;

	std::cout << "POP UFP Computation Tests\n";
	std::cout << std::string(40, '=') << "\n\n";

	TEST_CASE("UFP basic values", TestUfpBasic());
	TEST_CASE("UFP negative values", TestUfpNegative());
	TEST_CASE("UFP special values", TestUfpSpecialValues());
	TEST_CASE("UFP from range", TestUfpRange());
	TEST_CASE("UFP range_analyzer integration", TestUfpRangeAnalyzerIntegration());

	std::cout << "\n";
	if (nrOfFailedTestCases == 0) {
		std::cout << "All UFP tests PASSED\n";
	} else {
		std::cout << nrOfFailedTestCases << " test(s) FAILED\n";
	}

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (const char* msg) {
	std::cerr << "Caught exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
