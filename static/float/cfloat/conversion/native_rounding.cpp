// native_rounding.cpp: cfloat to float and double rounds once, to nearest even
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// to_native summed the fraction bit by bit in the target type, which rounds at every bit past the
// target's precision and then again: cfloat<40,8,uint32_t> 1.5 + 2^-24 + 2^-27 gave 0x1.8p+0, not
// 0x1.800002p+0, and double rounded the same way for fbits > 52 (#2087).
//
// The reference decodes the raw encoding itself (sign, exponent field, fraction) into an exact
// value and rounds it once with the hardware:
//   - to float: the value fits a double exactly (fbits <= 52), and a double -> float conversion is
//     one rounding, subnormal results included;
//   - to double, fbits > 52: the significand is split into two parts that are each exact in double,
//     and their double sum is one rounding of the exact value (normal results only).
#include <universal/utility/directives.hpp>
#define CFLOAT_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_suite.hpp>

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <random>
#include <string>

namespace {

// the fraction bits [lo, lo + n) of a cfloat, n <= 64, read with at()
template<typename Cfloat>
uint64_t raw_field(const Cfloat& c, unsigned lo, unsigned n) {
	uint64_t f{ 0 };
	for (unsigned i = 0; i < n; ++i) if (c.at(lo + i)) f |= (uint64_t(1) << i);
	return f;
}

// fill a cfloat's encoding with random bits, keeping the exponent field in [emin, emax]
template<typename Cfloat>
Cfloat random_encoding(std::mt19937_64& rng, uint64_t emin, uint64_t emax) {
	Cfloat c;
	for (unsigned i = 0; i < Cfloat::nbits; ++i) c.setbit(i, (rng() & 1u) != 0);
	const uint64_t e = emin + rng() % (emax - emin + 1u);
	for (unsigned i = 0; i < Cfloat::es; ++i) c.setbit(Cfloat::fbits + i, ((e >> i) & 1u) != 0);
	return c;
}

// the exact value of a cfloat whose significand fits a double (fbits <= 52)
template<typename Cfloat>
double exact_double(const Cfloat& c) {
	static_assert(Cfloat::fbits <= 52, "the significand must fit a double");
	constexpr int bias = (1 << (Cfloat::es - 1)) - 1;
	const uint64_t e = raw_field(c, Cfloat::fbits, Cfloat::es);
	const uint64_t f = raw_field(c, 0, Cfloat::fbits);
	double v = (e == 0) ? std::ldexp(double(f), 1 - bias - int(Cfloat::fbits))
	                    : std::ldexp(double((uint64_t(1) << Cfloat::fbits) | f), int(e) - bias - int(Cfloat::fbits));
	return c.at(Cfloat::nbits - 1) ? -v : v;
}

// cfloat -> float against float(exact double), over random encodings with exponents in [emin, emax]
template<typename Cfloat>
int VerifyToFloat(unsigned samples, uint64_t emin, uint64_t emax, bool reportTestCases) {
	int fails = 0;
	std::mt19937_64 rng(2087);
	for (unsigned k = 0; k < samples; ++k) {
		const Cfloat c = random_encoding<Cfloat>(rng, emin, emax);
		if (c.isnan() || c.isinf()) continue;
		if (!Cfloat::hasSubnormals && raw_field(c, Cfloat::fbits, Cfloat::es) == 0) continue;
		const float ref = static_cast<float>(exact_double(c));
		const float got = float(c);
		if (got != ref || std::signbit(got) != std::signbit(ref)) {
			if (reportTestCases && fails < 10) std::printf("FAIL: %s -> %a, expected %a\n", to_binary(c).c_str(), double(got), double(ref));
			++fails;
		}
	}
	return fails;
}

// float ties and near-ties: the bits below float's precision are 10..0, 10..01 and 01..1
template<typename Cfloat>
int VerifyFloatTies(unsigned samples, bool reportTestCases) {
	static_assert(Cfloat::fbits > 24u, "needs fraction bits below float's precision");
	constexpr unsigned drop = Cfloat::fbits - 23u;   // fraction bits float cannot keep at a normal result
	constexpr int bias = (1 << (Cfloat::es - 1)) - 1;
	int fails = 0;
	std::mt19937_64 rng(2088);
	const uint64_t half = uint64_t(1) << (drop - 1u);
	const uint64_t tails[3] = { half, half | 1u, half - 1u };
	for (unsigned k = 0; k < samples; ++k) {
		Cfloat c;
		c.setbits(0);
		const uint64_t kept = rng() & ((uint64_t(1) << 23) - 1u);
		const uint64_t e = uint64_t(bias) + (rng() % 16u);          // results in float's normal range
		for (uint64_t tail : tails) {
			const uint64_t f = (kept << drop) | tail;
			for (unsigned i = 0; i < Cfloat::fbits; ++i) c.setbit(i, ((f >> i) & 1u) != 0);
			for (unsigned i = 0; i < Cfloat::es; ++i) c.setbit(Cfloat::fbits + i, ((e >> i) & 1u) != 0);
			const float ref = static_cast<float>(exact_double(c));
			if (float(c) != ref) {
				if (reportTestCases && fails < 10) std::printf("FAIL: %s -> %a, expected %a\n", to_binary(c).c_str(), double(float(c)), double(ref));
				++fails;
			}
		}
	}
	return fails;
}

// cfloat -> double for fbits > 52, against the double sum of two exact parts
template<typename Cfloat>
int VerifyToDouble(unsigned samples, bool reportTestCases) {
	static_assert(Cfloat::fbits > 52 && Cfloat::fbits <= 84, "two parts of at most 53 and 32 bits");
	constexpr int bias = (1 << (Cfloat::es - 1)) - 1;
	constexpr unsigned loBits = 32;
	int fails = 0;
	std::mt19937_64 rng(2089);
	for (unsigned k = 0; k < samples; ++k) {
		// exponents that keep both parts and the result in double's normal range
		const Cfloat c = random_encoding<Cfloat>(rng, uint64_t(bias - 900), uint64_t(bias + 900));
		if (c.isnan() || c.isinf()) continue;
		const uint64_t e = raw_field(c, Cfloat::fbits, Cfloat::es);
		const uint64_t lo = raw_field(c, 0, loBits);
		const uint64_t hi = raw_field(c, loBits, Cfloat::fbits - loBits) | (uint64_t(1) << (Cfloat::fbits - loBits));
		const int scale = int(e) - bias - int(Cfloat::fbits);
		double ref = std::ldexp(double(hi), scale + int(loBits)) + std::ldexp(double(lo), scale);   // one rounding
		if (c.at(Cfloat::nbits - 1)) ref = -ref;
		const double got = double(c);
		if (got != ref) {
			if (reportTestCases && fails < 10) std::printf("FAIL: %s -> %a, expected %a\n", to_binary(c).c_str(), got, ref);
			++fails;
		}
	}
	return fails;
}

}  // namespace

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

	std::string test_suite  = "cfloat to native rounds once";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	cfloat<40, 8, uint32_t> a(1.5);
	a += cfloat<40, 8, uint32_t>(std::ldexp(1.0, -24));
	a += cfloat<40, 8, uint32_t>(std::ldexp(1.0, -27));
	std::printf("1.5 + 2^-24 + 2^-27 -> %a\n", double(float(a)));
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors

#else

#if REGRESSION_LEVEL_1
	{
		// the issue's value: float rounds 1.5 + 2^-24 + 2^-27 up, past the tie
		cfloat<40, 8, uint32_t> a(1.5);
		a += cfloat<40, 8, uint32_t>(std::ldexp(1.0, -24));
		a += cfloat<40, 8, uint32_t>(std::ldexp(1.0, -27));
		if (float(a) != 0x1.800002p+0f) {
			std::printf("FAIL: float(cfloat<40,8,uint32_t>(1.5 + 2^-24 + 2^-27)) = %a, expected 0x1.800002p+0\n", double(float(a)));
			++nrOfFailedTestCases;
		}
	}
	// float's normal range, and its subnormals (es = 11 reaches below 2^-149)
	nrOfFailedTestCases += ReportTestResult(VerifyToFloat<cfloat<40, 8, uint32_t>>(20000, 1, 254, reportTestCases),                       "cfloat<40,8,uint32_t>",        "to float");
	nrOfFailedTestCases += ReportTestResult(VerifyToFloat<cfloat<40, 11, uint8_t, true, false, false>>(20000, 1023 - 160, 1023 - 110, reportTestCases), "cfloat<40,11,uint8_t,t,f,f>", "to float subnormal");
	nrOfFailedTestCases += ReportTestResult(VerifyToFloat<cfloat<48, 8, uint16_t, true, false, false>>(20000, 0, 254, reportTestCases),    "cfloat<48,8,uint16_t,t,f,f>",  "to float");
	nrOfFailedTestCases += ReportTestResult(VerifyToFloat<cfloat<36, 8, uint8_t, false, true, false>>(20000, 1, 255, reportTestCases),     "cfloat<36,8,uint8_t,f,t,f>",   "to float");
	nrOfFailedTestCases += ReportTestResult(VerifyFloatTies<cfloat<40, 8, uint32_t>>(5000, reportTestCases),                              "cfloat<40,8,uint32_t>",        "float ties");
	nrOfFailedTestCases += ReportTestResult(VerifyFloatTies<cfloat<48, 8, uint8_t, true, false, false>>(5000, reportTestCases),           "cfloat<48,8,uint8_t,t,f,f>",   "float ties");
	// double past 52 fraction bits
	nrOfFailedTestCases += ReportTestResult(VerifyToDouble<cfloat<72, 11, uint32_t>>(20000, reportTestCases),                             "cfloat<72,11,uint32_t>",       "to double");
	nrOfFailedTestCases += ReportTestResult(VerifyToDouble<cfloat<80, 11, uint16_t, true, false, false>>(20000, reportTestCases),         "cfloat<80,11,uint16_t,t,f,f>", "to double");
	nrOfFailedTestCases += ReportTestResult(VerifyToDouble<cfloat<96, 15, uint64_t, true, false, false>>(20000, reportTestCases),         "cfloat<96,15,uint64_t,t,f,f>", "to double");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyToFloat<cfloat<40, 8, uint32_t>>(500000, 1, 254, reportTestCases),   "cfloat<40,8,uint32_t>",  "to float");
	nrOfFailedTestCases += ReportTestResult(VerifyToDouble<cfloat<72, 11, uint32_t>>(500000, reportTestCases),         "cfloat<72,11,uint32_t>", "to double");
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
