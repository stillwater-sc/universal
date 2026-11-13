// edges.cpp: blocksignificand fraction_ull, to_double and ordering at the edges
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The QA sweep of #2029 (tracking issue #2052) found that:
//  - fraction_ull() shifted a uint64_t by 64 - radixPoint: UB for a radix of 0 or past 64;
//  - to_double() called std::pow, so it was not usable in a constant expression on clang;
//  - to_double() always read the MSB as a 2's complement sign: the MUL/DIV/SQRT layouts hold
//    an unsigned magnitude whose MSB is an overflow bit (1.5 * 1.5 read back -1.19);
//  - operator< subtracted and tested the sign, which overflows for operands more than half
//    the range apart.
// Every case below failed (or was UB) before the fix.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <cstdint>
#include <universal/internal/blocksignificand/blocksignificand.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

using namespace sw::universal;

int VerifyFractionUll(bool reportTestCases) {
	int nrOfFailedTests = 0;
	auto check = [&](uint64_t got, uint64_t expected, const char* label) {
		if (got != expected) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: " << label << " = 0x" << std::hex << got << " expected 0x" << expected << std::dec << '\n';
		}
	};
	blocksignificand<16, uint8_t> a(0xB5A5ull, 0);   // radix 0: no fraction bits
	check(a.fraction_ull(), 0, "fraction_ull radix 0");
	blocksignificand<16, uint8_t> b(0xB5A5ull, 8);
	check(b.fraction_ull(), 0xA5, "fraction_ull radix 8");
	blocksignificand<80, uint32_t> c(0xFFFF'FFFF'FFFF'FFFFull, 70);   // a radix past 64 keeps all 64 bits
	check(c.fraction_ull(), 0xFFFF'FFFF'FFFF'FFFFull, "fraction_ull radix 70");
	blocksignificand<80, uint32_t> d(0x0123'4567'89AB'CDEFull, 64);
	check(d.fraction_ull(), 0x0123'4567'89AB'CDEFull, "fraction_ull radix 64");
	// uint64_t limbs: the fraction bits live in the low limb, not in the MSU
	blocksignificand<80, uint64_t> e(0x0123'4567'89AB'CDEFull, 64);
	check(e.fraction_ull(), 0x0123'4567'89AB'CDEFull, "fraction_ull radix 64, uint64_t limbs");
	blocksignificand<80, uint64_t> f(0xFFFF'0000'FFFF'0000ull, 20);
	check(f.fraction_ull(), 0x000F'0000ull, "fraction_ull radix 20, uint64_t limbs");
	return nrOfFailedTests;
}

int VerifyToDouble(bool reportTestCases) {
	int nrOfFailedTests = 0;
	auto check = [&](double got, double expected, const char* label) {
		if (got != expected) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: " << label << " = " << got << " expected " << expected << '\n';
		}
	};
	// 10 bits, radix 8: 0b10.0100'0000 is the product 1.5 * 1.5 in a MUL layout
	blocksignificand<10, uint8_t> product(0b10'0100'0000ull, 8);
	check(product.to_double(false), 2.25, "unsigned 2.25");
	check(product.to_double(true), -1.75, "2's complement of the same bits");
	check(product.to_double(), -1.75, "the default reads 2's complement");
	blocksignificand<10, uint8_t> one(0b01'0000'0000ull, 8);
	check(one.to_double(false), 1.0, "unsigned 1.0");
	check(one.to_double(true), 1.0, "2's complement 1.0");
	// an integer weight past bit 63 is finite: 2^64 is exact in a double
	blocksignificand<65, uint8_t> wide(0, 0);
	wide.setbit(64);
	check(wide.to_double(false), 18446744073709551616.0, "unsigned 2^64");
	blocksignificand<80, uint32_t> wider(0, 2);
	wider.setbit(78);
	wider.setbit(1);
	check(wider.to_double(false), 75557863725914323419136.0 + 0.5, "unsigned 2^76 + 2^-1");
	// more than 1024 bits above the radix: the unset high bits must not make the weight inf
	blocksignificand<1100, uint8_t> huge(1, 0);
	check(huge.to_double(false), 1.0, "unsigned 1 in 1100 bits");
	check(huge.to_double(true), 1.0, "2's complement 1 in 1100 bits");
	huge.setbit(1023);
	check(huge.to_double(false), 8.98846567431158e307 * 1.0 + 1.0, "unsigned 2^1023 + 1");
	return nrOfFailedTests;
}

// usable in a constant expression: std::pow is not constexpr on clang
constexpr double constexpr_to_double() {
	blocksignificand<10, uint8_t> v(0b01'1000'0000ull, 8);   // 1.5
	return v.to_double();
}
static_assert(constexpr_to_double() == 1.5, "blocksignificand::to_double in a constant expression");

// the raw constructor drops bits above nbits, so they cannot take part in an ordering
int VerifyPaddingBits(bool reportTestCases) {
	int nrOfFailedTests = 0;
	blocksignificand<10, uint8_t> a(0x400ull, 0), b(1ull, 0);   // bit 10 is outside the 10 bits
	if (!a.iszero() || !(a < b) || a == b) {
		++nrOfFailedTests;
		if (reportTestCases) std::cerr << "FAIL: blocksignificand<10>(0x400) kept a bit above nbits\n";
	}
	return nrOfFailedTests;
}

// 2's complement order, every pair of 8 bits
int VerifyOrder(bool reportTestCases) {
	int nrOfFailedTests = 0;
	for (int i = -128; i < 128; ++i) {
		for (int j = -128; j < 128; ++j) {
			blocksignificand<8, uint8_t> a(static_cast<uint64_t>(static_cast<uint8_t>(i)), 0);
			blocksignificand<8, uint8_t> b(static_cast<uint64_t>(static_cast<uint8_t>(j)), 0);
			if ((a < b) != (i < j) || (a <= b) != (i <= j) || (a > b) != (i > j) || (a >= b) != (i >= j)) {
				++nrOfFailedTests;
				if (reportTestCases && nrOfFailedTests < 10) std::cerr << "FAIL: blocksignificand<8> order of " << i << " and " << j << '\n';
			}
		}
	}
	return nrOfFailedTests;
}

} // namespace

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
#define REGRESSION_LEVEL_2 0
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "blocksignificand edges";
	std::string test_tag    = "edges";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	nrOfFailedTestCases += VerifyToDouble(true);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyFractionUll(reportTestCases), "blocksignificand", "fraction_ull");
	nrOfFailedTestCases += ReportTestResult(VerifyToDouble(reportTestCases), "blocksignificand", "to_double");
	nrOfFailedTestCases += ReportTestResult(VerifyOrder(reportTestCases), "blocksignificand<8,uint8_t>", "2's complement order");
	nrOfFailedTestCases += ReportTestResult(VerifyPaddingBits(reportTestCases), "blocksignificand<10,uint8_t>", "bits above nbits");
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
	std::cerr << msg << std::endl;
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
