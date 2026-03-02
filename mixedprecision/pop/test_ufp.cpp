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

#define VERIFY(cond, msg) do { if (!(cond)) { std::cerr << "FAIL: " << msg << std::endl; ++nrOfFailedTestCases; } } while(0)
#define VERIFY_UFP(val, expected) VERIFY(compute_ufp(val) == (expected), "compute_ufp(" << val << ") = " << compute_ufp(val) << ", expected " << (expected))

namespace sw { namespace universal {

int TestUfpBasic() {
	int nrOfFailedTestCases = 0;
	struct test_case { double value; int expected_ufp; };
	std::vector<test_case> cases = {
		{   1.0,   0 }, {   2.0,   1 }, {   4.0,   2 }, {   8.0,   3 },
		{  16.0,   4 }, {   0.5,  -1 }, {  0.25,  -2 }, { 0.125,  -3 },
		{ 1024.0, 10 },
	};
	for (const auto& tc : cases) {
		VERIFY(compute_ufp(tc.value) == tc.expected_ufp, "compute_ufp(" << tc.value << ") = " << compute_ufp(tc.value) << ", expected " << tc.expected_ufp);
	}
	VERIFY_UFP(3.0, 1);   // floor(log2(3)) = 1
	VERIFY_UFP(7.0, 2);   // floor(log2(7)) = 2
	VERIFY_UFP(0.3, -2);  // floor(log2(0.3)) = -2
	return nrOfFailedTestCases;
}

int TestUfpNegative() {
	int nrOfFailedTestCases = 0;
	VERIFY_UFP(-1.0, 0);
	VERIFY_UFP(-8.0, 3);
	return nrOfFailedTestCases;
}

int TestUfpSpecialValues() {
	int nrOfFailedTestCases = 0;
	VERIFY(compute_ufp(0.0) == std::numeric_limits<int>::min(), "compute_ufp(0.0) should return INT_MIN, got " << compute_ufp(0.0));
	return nrOfFailedTestCases;
}

int TestUfpRange() {
	int nrOfFailedTestCases = 0;
	VERIFY(compute_ufp(-3.0, 10.0) == 3, "compute_ufp(-3, 10) expected 3, got " << compute_ufp(-3.0, 10.0));
	VERIFY(compute_ufp(-100.0, 50.0) == 6, "compute_ufp(-100, 50) expected 6, got " << compute_ufp(-100.0, 50.0));
	return nrOfFailedTestCases;
}

int TestUfpRangeAnalyzerIntegration() {
	int nrOfFailedTestCases = 0;
	range_analyzer<double> ra;
	ra.observe(1.0); ra.observe(8.5); ra.observe(0.5); ra.observe(-3.0);
	VERIFY(ufp_from_analyzer(ra) == 3, "ufp_from_analyzer expected 3, got " << ufp_from_analyzer(ra));
	range_analyzer<double> ra2;
	ra2.observe(1024.0); ra2.observe(0.001);
	VERIFY(ufp_from_analyzer(ra2) == 10, "ufp_from_analyzer (large) expected 10, got " << ufp_from_analyzer(ra2));
	return nrOfFailedTestCases;
}

int TestUfpFloat() {
	int nrOfFailedTestCases = 0;
	VERIFY(compute_ufp(1.0f) == 0, "compute_ufp(1.0f) expected 0, got " << compute_ufp(1.0f));
	VERIFY(compute_ufp(8.0f) == 3, "compute_ufp(8.0f) expected 3, got " << compute_ufp(8.0f));
	VERIFY(compute_ufp(0.5f) == -1, "compute_ufp(0.5f) expected -1, got " << compute_ufp(0.5f));
	VERIFY(compute_ufp(0.0f) == std::numeric_limits<int>::min(), "compute_ufp(0.0f) expected INT_MIN");
	return nrOfFailedTestCases;
}

int TestUfpLargeValues() {
	int nrOfFailedTestCases = 0;
	VERIFY_UFP(1073741824.0, 30);       // 2^30
	VERIFY_UFP(1125899906842624.0, 50); // 2^50
	return nrOfFailedTestCases;
}

int TestUfpVerySmall() {
	int nrOfFailedTestCases = 0;
	VERIFY(compute_ufp(std::ldexp(1.0, -100)) == -100, "compute_ufp(2^-100) expected -100");
	VERIFY(compute_ufp(std::ldexp(1.0, -500)) == -500, "compute_ufp(2^-500) expected -500");
	return nrOfFailedTestCases;
}

int TestUfpNegativeZero() {
	int nrOfFailedTestCases = 0;
	VERIFY(compute_ufp(-0.0) == std::numeric_limits<int>::min(), "compute_ufp(-0.0) expected INT_MIN");
	return nrOfFailedTestCases;
}

int TestUfpNegativeRange() {
	int nrOfFailedTestCases = 0;
	VERIFY(compute_ufp(-100.0, -10.0) == 6, "compute_ufp(-100, -10) expected 6, got " << compute_ufp(-100.0, -10.0));
	return nrOfFailedTestCases;
}

int TestUfpZeroRange() {
	int nrOfFailedTestCases = 0;
	VERIFY(compute_ufp(0.0, 0.0) == std::numeric_limits<int>::min(), "compute_ufp(0, 0) expected INT_MIN");
	return nrOfFailedTestCases;
}

}} // namespace sw::universal

#define TEST_CASE(name, func) do { int f_ = func; if (f_) { std::cout << name << ": FAIL (" << f_ << " errors)\n"; nrOfFailedTestCases += f_; } else { std::cout << name << ": PASS\n"; } } while(0)

int main()
try {
	using namespace sw::universal;
	int nrOfFailedTestCases = 0;
	std::cout << "POP UFP Computation Tests\n" << std::string(40, '=') << "\n\n";

	TEST_CASE("UFP basic values", TestUfpBasic());
	TEST_CASE("UFP negative values", TestUfpNegative());
	TEST_CASE("UFP special values", TestUfpSpecialValues());
	TEST_CASE("UFP from range", TestUfpRange());
	TEST_CASE("UFP range_analyzer integration", TestUfpRangeAnalyzerIntegration());
	TEST_CASE("UFP float overload", TestUfpFloat());
	TEST_CASE("UFP large values", TestUfpLargeValues());
	TEST_CASE("UFP very small values", TestUfpVerySmall());
	TEST_CASE("UFP negative zero", TestUfpNegativeZero());
	TEST_CASE("UFP negative range", TestUfpNegativeRange());
	TEST_CASE("UFP zero range", TestUfpZeroRange());

	std::cout << "\n" << (nrOfFailedTestCases == 0 ? "All UFP tests PASSED" : std::to_string(nrOfFailedTestCases) + " test(s) FAILED") << "\n";
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (const char* msg) { std::cerr << "Caught exception: " << msg << std::endl; return EXIT_FAILURE; }
catch (...) { std::cerr << "Caught unknown exception" << std::endl; return EXIT_FAILURE; }
