// truncate.cpp: test suite runner for truncation functions trunc, round, floor, and ceil
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// use default number system library configuration
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/verification/fixpnt_test_suite_mathlib.hpp>

template<typename TestType>
int VerifyFloor(bool reportTestCases) {
	using namespace sw::universal;
	constexpr size_t nbits = TestType::nbits;
	constexpr size_t NR_VALUES = (1 << nbits);
	int nrOfFailedTestCases = 0;

	TestType a;
	for (size_t i = 0; i < NR_VALUES; ++i) {
		a.setbits(i);
		auto l1 = sw::universal::floor(a);
		// generate the reference
		float f = float(a);
		auto l2 = std::floor(f);
		if (l1 != l2) {
			++nrOfFailedTestCases;
			if (reportTestCases) ReportOneInputFunctionError("floor", "floor", a, TestType(l1), TestType(l2));
		}
	}
	return nrOfFailedTestCases;
}

template<typename TestType>
int VerifyCeil(bool reportTestCases) {
	using namespace sw::universal;
	constexpr size_t nbits = TestType::nbits;
	constexpr size_t NR_VALUES = (1 << nbits);
	int nrOfFailedTestCases = 0;

	TestType a;
	for (size_t i = 0; i < NR_VALUES; ++i) {
		a.setbits(i);
		auto l1 = sw::universal::ceil(a);
		// generate the reference
		float f = float(a);
		auto l2 = std::ceil(f);
		if (l1 != l2) {             // TODO: fix float to int64 comparison
			++nrOfFailedTestCases;
			if (reportTestCases) ReportOneInputFunctionError("ceil", "ceil", a, TestType(l1), TestType(l2));
		}
	}
	return nrOfFailedTestCases;
}

// trunc, round, floor, ceil, frac, min and max on values past double's 53 bits (#2111: they went
// through double, so max(2^60 + 1, 2^60) was 2^60 and floor(2^60 + 1.5) lost its low bits). The
// reference builds the expected encoding: in fixpnt<nbits, 4>, the integer n plus f/16 has raw bits
// n * 16 + f.
template<bool arithmetic>
int VerifyWideTruncation(bool reportTestCases) {
	using namespace sw::universal;
	using F = fixpnt<72, 4, arithmetic, std::uint32_t>;
	int nrOfFailedTestCases = 0;
	auto make = [](long long n, unsigned f) {   // n + f/16, n of either sign, f in [0, 16)
		F r;
		r.setbits(0);
		F one;
		one.setbits(0);
		// n * 16 + f, assembled in the fixpnt so it is exact past 64 bits: (n << 4) | f
		const bool neg = n < 0;
		unsigned long long m = neg ? static_cast<unsigned long long>(-n) : static_cast<unsigned long long>(n);
		for (unsigned i = 0; i < 64; ++i) if ((m >> i) & 1ull) r.setbit(i + 4, true);
		if (neg) r = -r;
		for (unsigned i = 0; i < 4; ++i) if ((f >> i) & 1u) one.setbit(i, true);
		return r + one;
	};
	auto check = [&](const F& got, const F& expected, const char* label) {
		if (got == expected) return;
		++nrOfFailedTestCases;
		if (reportTestCases) std::cout << "FAIL: " << label << ": " << to_binary(got) << ", expected " << to_binary(expected) << '\n';
	};
	const long long big = (1ll << 60) + 1;   // past 53 bits
	const F x = make(big, 8);                 // 2^60 + 1.5
	check(trunc(x), make(big, 0), "trunc(2^60 + 1.5)");
	check(floor(x), make(big, 0), "floor(2^60 + 1.5)");
	check(ceil(x),  make(big + 1, 0), "ceil(2^60 + 1.5)");
	check(round(x), make(big + 1, 0), "round(2^60 + 1.5)");   // half away from zero
	check(frac(x),  make(0, 8), "frac(2^60 + 1.5)");
	const F y = make(-big - 1, 8);            // -(2^60 + 1.5) = -(2^60 + 2) + 0.5
	check(trunc(y), make(-big, 0), "trunc(-(2^60 + 1.5))");
	check(floor(y), make(-big - 1, 0), "floor(-(2^60 + 1.5))");
	check(ceil(y),  make(-big, 0), "ceil(-(2^60 + 1.5))");
	check(round(y), make(-big - 1, 0), "round(-(2^60 + 1.5))");
	check(frac(y),  -make(0, 8), "frac(-(2^60 + 1.5))");
	const F z = make(-3, 8);                   // -2.5
	check(frac(z), -make(0, 8), "frac(-2.5)");  // the sign of x: -0.5 (it was +0.5)
	check(round(z), make(-3, 0), "round(-2.5)");
	check(round(make(2, 4)), make(2, 0), "round(2.25)");
	const F a = make(big, 0), b = make(big - 1, 0);
	check(max(a, b), a, "max(2^60 + 1, 2^60)");
	check(max(b, a), a, "max(2^60, 2^60 + 1)");
	check(min(a, b), b, "min(2^60 + 1, 2^60)");
	check(min(b, a), b, "min(2^60, 2^60 + 1)");
	return nrOfFailedTestCases;
}

// exhaustive on a narrow type against <cmath>, which is exact there; frac against x - trunc(x)
template<typename TestType>
int VerifyNarrowTruncation(bool reportTestCases) {
	using namespace sw::universal;
	constexpr unsigned nbits = TestType::nbits;
	int nrOfFailedTestCases = 0;
	for (unsigned i = 0; i < (1u << nbits); ++i) {
		TestType a;
		a.setbits(i);
		const double v = double(a);
		auto check = [&](const TestType& got, double expected, const char* fn) {
			if (double(got) == double(TestType(expected))) return;
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "FAIL: " << fn << "(" << v << ") = " << double(got) << ", expected " << double(TestType(expected)) << '\n';
		};
		check(trunc(a), std::trunc(v), "trunc");
		check(round(a), std::round(v), "round");
		check(floor(a), std::floor(v), "floor");
		check(ceil(a),  std::ceil(v),  "ceil");
		check(frac(a),  v - std::trunc(v), "frac");
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

	std::string test_suite  = "fixpnt<> mathlib truncate verfication";
	std::string test_tag    = "truncate";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug

	nrOfFailedTestCases += ReportTestResult(VerifyFloor< fixpnt<8, 2, Saturate, uint8_t> >(reportTestCases), "floor", "fixpnt<8,2,Saturate,uint8_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyCeil < fixpnt<8, 2, Saturate, uint8_t> >(reportTestCases), "ceil", "fixpnt<8,2,Saturate,uint8_t>");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyFloor< fixpnt<8, 2, Saturate, uint8_t> >(reportTestCases), "floor", "fixpnt<8,2,Saturate,uint8_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyCeil < fixpnt<8, 2, Saturate, uint8_t> >(reportTestCases), "ceil", "fixpnt<8,2,Saturate,uint8_t>");

	nrOfFailedTestCases += ReportTestResult(VerifyFloor< fixpnt<16, 8, Saturate, uint8_t> >(reportTestCases), "floor", "fixpnt<16,8,Saturate,uint8_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyCeil < fixpnt<16, 8, Saturate, uint8_t> >(reportTestCases), "ceil", "fixpnt<16,8,Saturate,uint8_t>");

	nrOfFailedTestCases += ReportTestResult(VerifyWideTruncation<Modulo>(reportTestCases), "truncate", "fixpnt<72,4,Modulo> past 53 bits");
	nrOfFailedTestCases += ReportTestResult(VerifyWideTruncation<Saturate>(reportTestCases), "truncate", "fixpnt<72,4,Saturate> past 53 bits");
	nrOfFailedTestCases += ReportTestResult(VerifyNarrowTruncation< fixpnt<8, 3, Modulo, uint8_t> >(reportTestCases), "truncate", "fixpnt<8,3,Modulo> trunc/round/floor/ceil/frac");
	nrOfFailedTestCases += ReportTestResult(VerifyNarrowTruncation< fixpnt<10, 4, Saturate, uint8_t> >(reportTestCases), "truncate", "fixpnt<10,4,Saturate> trunc/round/floor/ceil/frac");

#endif

#if REGRESSION_LEVEL_2

#endif

#if REGRESSION_LEVEL_3

#endif

#if REGRESSION_LEVEL_4

#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::fixpnt_arithmetic_exception& err) {
	std::cerr << "Uncaught fixpnt arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
//catch (const sw::universal::fixpnt_quire_exception& err) {
//	std::cerr << "Uncaught fixpnt quire exception: " << err.what() << std::endl;
//	return EXIT_FAILURE;
//}
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
