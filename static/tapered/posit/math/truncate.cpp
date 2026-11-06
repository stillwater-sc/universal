// truncate.cpp: test suite runner for the posit trunc, round, floor, ceil and isinteger
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <cstdint>
#include <random>
#include <universal/number/posit/posit.hpp>
#include <universal/verification/test_suite.hpp>

// The four functions are exact, computed on the encoding (#1855). Up to 32 bits every posit is a
// double, so std::trunc, std::round, std::floor and std::ceil of double(x) are exact references, and
// so is their conversion back. Past that, the result is checked against what defines it: it is an
// integer (isinteger reads the library's own decoder, not the one these functions use), it lies on
// the right side of x, and it is less than 1 from x, or for round at most 1/2 with ties away from
// zero; for |x| >= 1 those differences are exact in posit arithmetic, and below 1 the answer is 0 or
// +-1 by a comparison. Together they leave one possible result.

namespace {

	using namespace sw::universal;

	template<typename Posit>
	Posit Make(std::uint64_t encoding) {
		Posit p;
		p.setbits(encoding);
		return p;
	}

	// a random encoding of a wide posit, block by block
	template<typename Posit>
	Posit RandomPosit(std::mt19937_64& rng) {
		using bt = typename Posit::BlockType;
		auto bits = Posit().bits();
		for (unsigned i = 0; i < bits.nrBlocks; ++i) bits.setblock(i, static_cast<bt>(rng()));
		Posit p;
		p.setbits(bits);
		return p;
	}

	template<typename Posit>
	int Report(bool reportTestCases, const char* fn, const Posit& x, const Posit& got, const Posit& want) {
		if (reportTestCases) {
			std::cerr << "FAIL " << fn << '(' << to_binary(x) << ") = " << to_binary(got) << " want " << to_binary(want) << '\n';
		}
		return 1;
	}

	// every encoding, against the double functions
	template<typename Posit>
	int VerifyExhaustive(bool reportTestCases) {
		constexpr unsigned nbits = Posit::nbits;
		int fails = 0;
		for (std::uint64_t i = 0; i < (1ull << nbits); ++i) {
			const Posit x = Make<Posit>(i);
			if (x.isnar()) {
				if (!trunc(x).isnar() || !round(x).isnar() || !floor(x).isnar() || !ceil(x).isnar()) ++fails;
				if (x.isinteger()) ++fails;
				continue;
			}
			const double d = double(x);
			if (trunc(x) != Posit(std::trunc(d))) fails += Report(reportTestCases, "trunc", x, trunc(x), Posit(std::trunc(d)));
			if (round(x) != Posit(std::round(d))) fails += Report(reportTestCases, "round", x, round(x), Posit(std::round(d)));
			if (floor(x) != Posit(std::floor(d))) fails += Report(reportTestCases, "floor", x, floor(x), Posit(std::floor(d)));
			if (ceil(x)  != Posit(std::ceil(d)))  fails += Report(reportTestCases, "ceil",  x, ceil(x),  Posit(std::ceil(d)));
			if (x.isinteger() != (std::floor(d) == d)) {
				++fails;
				if (reportTestCases) std::cerr << "FAIL isinteger(" << to_binary(x) << ")\n";
			}
		}
		return fails;
	}

	// random encodings of a posit of at most 32 bits, against the double functions
	template<typename Posit>
	int VerifyRandomNarrow(unsigned nrSamples, std::uint64_t seed, bool reportTestCases) {
		std::mt19937_64 rng(seed);
		int fails = 0;
		for (unsigned s = 0; s < nrSamples; ++s) {
			const Posit x = Make<Posit>(rng() & ((1ull << Posit::nbits) - 1u));
			if (x.isnar()) continue;
			const double d = double(x);
			if (trunc(x) != Posit(std::trunc(d))) fails += Report(reportTestCases, "trunc", x, trunc(x), Posit(std::trunc(d)));
			if (round(x) != Posit(std::round(d))) fails += Report(reportTestCases, "round", x, round(x), Posit(std::round(d)));
			if (floor(x) != Posit(std::floor(d))) fails += Report(reportTestCases, "floor", x, floor(x), Posit(std::floor(d)));
			if (ceil(x)  != Posit(std::ceil(d)))  fails += Report(reportTestCases, "ceil",  x, ceil(x),  Posit(std::ceil(d)));
			if (x.isinteger() != (std::floor(d) == d)) ++fails;
		}
		return fails;
	}

	// the defining properties at any width
	template<typename Posit>
	int VerifyProperties(const Posit& x, bool reportTestCases) {
		if (x.isnar() || x.iszero()) return 0;
		const Posit one(1), half(0.5), zero(0);
		int fails = 0;
		auto check = [&](bool ok, const char* fn, const Posit& got) {
			if (!ok) {
				++fails;
				if (reportTestCases) std::cerr << "FAIL " << fn << '(' << to_binary(x) << ") = " << to_binary(got) << '\n';
			}
		};
		const Posit t = trunc(x), r = round(x), f = floor(x), c = ceil(x);
		if (abs(x) < one) {   // the integers around x are 0 and +-1, and x - (+-1) need not be exact
			const bool neg = x.isneg();
			check(t.iszero(), "trunc", t);
			check(f == (neg ? -one : zero), "floor", f);
			check(c == (neg ? zero : one), "ceil", c);
			check(r == (abs(x) < half ? zero : (neg ? -one : one)), "round", r);
			check(!x.isinteger(), "isinteger", t);
			return fails;
		}
		check(t.isinteger() && abs(t) <= abs(x) && abs(x - t) < one && (t.iszero() || t.isneg() == x.isneg()), "trunc", t);
		check(f.isinteger() && f <= x && x - f < one, "floor", f);
		check(c.isinteger() && c >= x && c - x < one, "ceil", c);
		const Posit d = abs(x - r);
		check(r.isinteger() && d <= half && (d < half || abs(r) > abs(x)) && (r.iszero() || r.isneg() == x.isneg()), "round", r);
		check(x.isinteger() == (t == x), "isinteger", t);
		return fails;
	}

	template<typename Posit>
	int VerifyRandomWide(unsigned nrSamples, std::uint64_t seed, bool reportTestCases) {
		std::mt19937_64 rng(seed);
		int fails = 0;
		for (unsigned s = 0; s < nrSamples; ++s) fails += VerifyProperties(RandomPosit<Posit>(rng), reportTestCases);
		return fails;
	}

	// values near 1, where the binary point sits inside the fraction, and integers and half-integers past
	// 2^53, which a double cannot hold: k + {0, 1/4, 1/2, 3/4} at 2^m for m up to the fraction width
	template<typename Posit>
	int VerifyStructured(bool reportTestCases) {
		int fails = 0;
		const Posit one(1), quarter(0.25), half(0.5);
		Posit p = one;
		for (unsigned m = 0; m + 3u < Posit::fbits; ++m, p *= Posit(2)) {
			for (const Posit& base : { p, p + one, p - one }) {
				for (const Posit& offset : { Posit(0), quarter, half, half + quarter }) {
					const Posit x = base + offset;
					fails += VerifyProperties(x, reportTestCases);
					fails += VerifyProperties(-x, reportTestCases);
				}
			}
		}
		// an integer past 2^53 is its own floor (#1855: through double it lost its low bits)
		Posit big = one;
		for (int i = 0; i < 60 && big.isinteger(); ++i) big *= Posit(2);
		const Posit x = big + Posit(3);
		if (x.isinteger() && (floor(x) != x || trunc(x) != x || ceil(x) != x || round(x) != x)) {
			++fails;
			if (reportTestCases) std::cerr << "FAIL an integer past 2^53 is not its own floor: " << to_binary(x) << '\n';
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

	std::string test_suite  = "posit trunc, round, floor, ceil and isinteger";
	std::string test_tag    = "truncate";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	{
		using Posit = posit<64, 3>;
		Posit x(1);
		for (int i = 0; i < 56; ++i) x *= Posit(2);
		x += Posit(3);
		std::cout << to_binary(x) << " floor " << to_binary(floor(x)) << '\n';
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<posit< 2, 0>>(reportTestCases), "posit< 2,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<posit< 3, 1>>(reportTestCases), "posit< 3,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<posit< 4, 0>>(reportTestCases), "posit< 4,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<posit< 5, 2>>(reportTestCases), "posit< 5,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<posit< 8, 0>>(reportTestCases), "posit< 8,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<posit< 8, 1>>(reportTestCases), "posit< 8,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<posit< 8, 2>>(reportTestCases), "posit< 8,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<posit< 8, 5>>(reportTestCases), "posit< 8,5>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<posit<12, 1>>(reportTestCases), "posit<12,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<posit<16, 1>>(reportTestCases), "posit<16,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<posit<16, 2>>(reportTestCases), "posit<16,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyExhaustive<posit<16, 1, std::uint16_t>>(reportTestCases)), "posit<16,1,uint16_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyRandomNarrow<posit<32, 2>>(100000, 1, reportTestCases), "posit<32,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyRandomNarrow<posit<32, 2, std::uint32_t>>(20000, 2, reportTestCases)), "posit<32,2,uint32_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyStructured<posit<32, 2>>(reportTestCases), "posit<32,2> structured", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyStructured<posit<64, 3>>(reportTestCases), "posit<64,3> structured", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyStructured<posit<128, 4>>(reportTestCases), "posit<128,4> structured", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyRandomWide<posit<64, 3>>(20000, 3, reportTestCases), "posit<64,3>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyRandomWide<posit<64, 3, std::uint64_t>>(5000, 4, reportTestCases)), "posit<64,3,uint64_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyRandomWide<posit<80, 3>>(5000, 5, reportTestCases), "posit<80,3>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyRandomWide<posit<128, 4>>(5000, 6, reportTestCases), "posit<128,4>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyRandomWide<posit<256, 5>>(1000, 7, reportTestCases), "posit<256,5>", test_tag);
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<posit<20, 1>>(reportTestCases), "posit<20,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyRandomNarrow<posit<32, 3>>(1000000, 8, reportTestCases), "posit<32,3>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyRandomWide<posit<128, 4>>(100000, 9, reportTestCases), "posit<128,4>", test_tag);
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<posit<24, 2>>(reportTestCases), "posit<24,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyStructured<posit<256, 5>>(reportTestCases), "posit<256,5> structured", test_tag);
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyRandomNarrow<posit<32, 2>>(100000000, 10, reportTestCases), "posit<32,2>", test_tag);
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
