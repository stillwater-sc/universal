// truncate.cpp: test suite runner for truncation functions trunc, round, floor, and ceil
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// use default number system library configuration
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/cfloat_test_suite_mathlib.hpp>

// Exhaustive verifier: for every encoding of a small cfloat, compare the native
// trunc/floor/ceil/round against the representable cfloat of the std result.
// Valid because these types have <= 53 significand bits, so float/double are an
// exact reference. (The wide-precision case, where double is NOT exact, is
// covered by VerifyWideTruncation below -- the regression #1026 was about.)
template<typename TestType>
int VerifyFloor(bool reportTestCases) {
	using namespace sw::universal;
	constexpr size_t nbits = TestType::nbits;
	constexpr size_t NR_VALUES = (1ull << nbits);
	int nrOfFailedTestCases = 0;
	TestType a;
	for (size_t i = 0; i < NR_VALUES; ++i) {
		a.setbits(i);
		if (a.isnan() || a.isinf()) continue;
		TestType l1 = floor(a);
		TestType l2 = TestType(std::floor(float(a)));
		if (l1 != l2) {
			std::cout << to_binary(a) << " : floor(" << float(a) << ") = " << l2 << " vs result " << l1 << '\n';
			++nrOfFailedTestCases;
			if (reportTestCases) ReportOneInputFunctionError("floor", "floor", a, l1, l2);
		}
	}
	return nrOfFailedTestCases;
}

template<typename TestType>
int VerifyCeil(bool reportTestCases) {
	using namespace sw::universal;
	constexpr size_t nbits = TestType::nbits;
	constexpr size_t NR_VALUES = (1ull << nbits);
	int nrOfFailedTestCases = 0;
	TestType a;
	for (size_t i = 0; i < NR_VALUES; ++i) {
		a.setbits(i);
		if (a.isnan() || a.isinf()) continue;
		TestType l1 = ceil(a);
		TestType l2 = TestType(std::ceil(float(a)));
		if (l1 != l2) {
			std::cout << to_binary(a) << " : ceil(" << float(a) << ") = " << l2 << " vs result " << l1 << '\n';
			++nrOfFailedTestCases;
			if (reportTestCases) ReportOneInputFunctionError("ceil", "ceil", a, l1, l2);
		}
	}
	return nrOfFailedTestCases;
}

template<typename TestType>
int VerifyTrunc(bool reportTestCases) {
	using namespace sw::universal;
	constexpr size_t nbits = TestType::nbits;
	constexpr size_t NR_VALUES = (1ull << nbits);
	int nrOfFailedTestCases = 0;
	TestType a;
	for (size_t i = 0; i < NR_VALUES; ++i) {
		a.setbits(i);
		if (a.isnan() || a.isinf()) continue;
		TestType l1 = trunc(a);
		TestType l2 = TestType(std::trunc(float(a)));
		if (l1 != l2) {
			std::cout << to_binary(a) << " : trunc(" << float(a) << ") = " << l2 << " vs result " << l1 << '\n';
			++nrOfFailedTestCases;
			if (reportTestCases) ReportOneInputFunctionError("trunc", "trunc", a, l1, l2);
		}
	}
	return nrOfFailedTestCases;
}

template<typename TestType>
int VerifyRound(bool reportTestCases) {
	using namespace sw::universal;
	constexpr size_t nbits = TestType::nbits;
	constexpr size_t NR_VALUES = (1ull << nbits);
	int nrOfFailedTestCases = 0;
	TestType a;
	for (size_t i = 0; i < NR_VALUES; ++i) {
		a.setbits(i);
		if (a.isnan() || a.isinf()) continue;
		TestType l1 = round(a);
		TestType l2 = TestType(std::round(float(a)));  // ties away from zero, matches our round
		if (l1 != l2) {
			std::cout << to_binary(a) << " : round(" << float(a) << ") = " << l2 << " vs result " << l1 << '\n';
			++nrOfFailedTestCases;
			if (reportTestCases) ReportOneInputFunctionError("round", "round", a, l1, l2);
		}
	}
	return nrOfFailedTestCases;
}

// Wide-precision verifier (issue #1026): a cfloat with > 53 significand bits has
// integers that a double cannot represent. We build a large integer N exactly
// (2^60 + 8, well within a 113-bit significand) plus exactly-representable
// fractions, and check trunc/floor/ceil/round against cfloat-constructed
// references -- NO double round-trip, which the buggy implementation relied on.
template<typename Wide>
int VerifyWideTruncation(bool reportTestCases) {
	using namespace sw::universal;
	(void)reportTestCases;
	int fail = 0;
	const Wide N  = Wide(1152921504606846976.0) + Wide(8);  // 2^60 + 8 (exact)
	const Wide N1 = N + Wide(1);                             // 2^60 + 9
	const Wide h(0.5), q(0.25), t3(0.75);

	struct Case { Wide x, efloor, eceil, etrunc, eround; const char* tag; };
	const Case cases[] = {
		{ N,        N,   N,  N,  N,   "N" },
		{ N + h,    N,   N1, N,  N1,  "N+0.5" },
		{ N + q,    N,   N1, N,  N,   "N+0.25" },
		{ N + t3,   N,   N1, N,  N1,  "N+0.75" },
		{ -(N + h), -N1, -N, -N, -N1, "-(N+0.5)" },
		{ -(N + q), -N1, -N, -N, -N,  "-(N+0.25)" },
		{ -(N + t3),-N1, -N, -N, -N1, "-(N+0.75)" },
	};
	for (const auto& c : cases) {
		if (floor(c.x) != c.efloor) { std::cout << "wide floor " << c.tag << " FAIL: " << floor(c.x) << " vs " << c.efloor << '\n'; ++fail; }
		if (ceil (c.x) != c.eceil ) { std::cout << "wide ceil "  << c.tag << " FAIL: " << ceil(c.x)  << " vs " << c.eceil  << '\n'; ++fail; }
		if (trunc(c.x) != c.etrunc) { std::cout << "wide trunc " << c.tag << " FAIL: " << trunc(c.x) << " vs " << c.etrunc << '\n'; ++fail; }
		if (round(c.x) != c.eround) { std::cout << "wide round " << c.tag << " FAIL: " << round(c.x) << " vs " << c.eround << '\n'; ++fail; }
	}
	return fail;
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

	std::string test_suite  = "cfloat<> mathlib truncation validation";
	std::string test_tag    = "truncation";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug
	nrOfFailedTestCases += ReportTestResult(VerifyFloor< cfloat<8, 2, uint8_t> >(reportTestCases), "floor", "cfloat<8,2>");
	nrOfFailedTestCases += ReportTestResult(VerifyCeil < cfloat<8, 2, uint8_t> >(reportTestCases), "ceil ", "cfloat<8,2>");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;  // ignore failures in manual testing mode
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyFloor< cfloat<8, 2, uint8_t> >(reportTestCases), "floor", "cfloat<8,2>");
	nrOfFailedTestCases += ReportTestResult(VerifyCeil < cfloat<8, 2, uint8_t> >(reportTestCases), "ceil ", "cfloat<8,2>");
	nrOfFailedTestCases += ReportTestResult(VerifyTrunc< cfloat<8, 2, uint8_t> >(reportTestCases), "trunc", "cfloat<8,2>");
	nrOfFailedTestCases += ReportTestResult(VerifyRound< cfloat<8, 2, uint8_t> >(reportTestCases), "round", "cfloat<8,2>");

	nrOfFailedTestCases += ReportTestResult(VerifyFloor< half >(reportTestCases), "floor", "half");
	nrOfFailedTestCases += ReportTestResult(VerifyCeil < half >(reportTestCases), "ceil ", "half");
	nrOfFailedTestCases += ReportTestResult(VerifyTrunc< half >(reportTestCases), "trunc", "half");
	nrOfFailedTestCases += ReportTestResult(VerifyRound< half >(reportTestCases), "round", "half");

	nrOfFailedTestCases += ReportTestResult(VerifyFloor< bfloat_t >(reportTestCases), "floor", "bfloat_t");
	nrOfFailedTestCases += ReportTestResult(VerifyCeil < bfloat_t >(reportTestCases), "ceil ", "bfloat_t");
	nrOfFailedTestCases += ReportTestResult(VerifyTrunc< bfloat_t >(reportTestCases), "trunc", "bfloat_t");
	nrOfFailedTestCases += ReportTestResult(VerifyRound< bfloat_t >(reportTestCases), "round", "bfloat_t");

	// issue #1026: > 53-bit significand, where the old double-based impl was wrong
	nrOfFailedTestCases += ReportTestResult(VerifyWideTruncation< cfloat<128, 15, std::uint32_t, true, false, false> >(reportTestCases), "wide trunc/floor/ceil/round", "cfloat<128,15>");

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
