// fractional.cpp: test suite runner for the posit fmod, remainder and frac
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

// fmod and remainder are correctly rounded and frac is exact (#1855). A posit's precision tapers away
// from 1, so the exact x - n y need not be a posit: fmod(1 + 2^-27, minpos) at 32 bits has to round.
// Up to 32 bits every posit is a double, and std::fmod and std::remainder are exact in double, so
// Posit(std::fmod(double(x), double(y))) rounds the exact result once: a correct reference. Past
// 32 bits the references are exact by construction: operands of a narrow posit, which the wide one
// holds exactly, with an exact result; integers of up to 58 bits, which a double cannot hold, modulo
// small integers, in integer arithmetic; and 2^k mod m for a large power of two and a small integer m,
// whose quotient has up to k bits.

namespace {

	using namespace sw::universal;

	template<typename Posit>
	Posit Make(std::uint64_t encoding) {
		Posit p;
		p.setbits(encoding);
		return p;
	}

	template<typename Posit>
	int Report(bool reportTestCases, const char* fn, const Posit& x, const Posit& y, const Posit& got, const Posit& want) {
		if (reportTestCases) {
			std::cerr << "FAIL " << fn << '(' << to_binary(x) << ", " << to_binary(y) << ") = " << to_binary(got) << " want " << to_binary(want) << '\n';
		}
		return 1;
	}

	// x and y against the double references; NaR operands and y = 0 give NaR
	template<typename Posit>
	int VerifyPair(const Posit& x, const Posit& y, bool reportTestCases) {
		int fails = 0;
		const Posit f = fmod(x, y), r = remainder(x, y);
		if (x.isnar() || y.isnar() || y.iszero()) {
			if (!f.isnar()) fails += Report(reportTestCases, "fmod", x, y, f, Posit(SpecificValue::nar));
			if (!r.isnar()) fails += Report(reportTestCases, "remainder", x, y, r, Posit(SpecificValue::nar));
			return fails;
		}
		const double dx = double(x), dy = double(y);
		const Posit wf(std::fmod(dx, dy)), wr(std::remainder(dx, dy));
		if (f != wf) fails += Report(reportTestCases, "fmod", x, y, f, wf);
		if (r != wr) fails += Report(reportTestCases, "remainder", x, y, r, wr);
		return fails;
	}

	template<typename Posit>
	int VerifyFrac(const Posit& x, bool reportTestCases) {
		const Posit got = frac(x);
		if (x.isnar()) return got.isnar() ? 0 : 1;
		const double d = double(x);
		const Posit want(d - std::trunc(d));
		if (got == want) return 0;
		if (reportTestCases) std::cerr << "FAIL frac(" << to_binary(x) << ") = " << to_binary(got) << " want " << to_binary(want) << '\n';
		return 1;
	}

	// every pair of encodings
	template<typename Posit>
	int VerifyExhaustive(bool reportTestCases) {
		constexpr std::uint64_t N = 1ull << Posit::nbits;
		int fails = 0;
		for (std::uint64_t i = 0; i < N; ++i) {
			const Posit x = Make<Posit>(i);
			fails += VerifyFrac(x, reportTestCases);
			for (std::uint64_t j = 0; j < N; ++j) fails += VerifyPair(x, Make<Posit>(j), reportTestCases);
		}
		return fails;
	}

	// random pairs of a posit of at most 32 bits; half of them with y near minpos or maxpos, where the
	// taper makes the exact result inexact
	template<typename Posit>
	int VerifyRandom(unsigned nrSamples, std::uint64_t seed, bool reportTestCases) {
		constexpr unsigned nbits = Posit::nbits;
		constexpr std::uint64_t mask = (nbits == 64) ? ~0ull : ((1ull << nbits) - 1u);
		std::mt19937_64 rng(seed);
		int fails = 0;
		for (unsigned s = 0; s < nrSamples; ++s) {
			const Posit x = Make<Posit>(rng() & mask);
			std::uint64_t ey = rng() & mask;
			if (s & 1u) {   // a long regime: a few fraction bits, far from 1
				const unsigned keep = 1u + static_cast<unsigned>(rng() % (nbits / 2u));
				ey = (ey & ((1ull << keep) - 1u)) | ((rng() & 1u) ? (((1ull << (nbits - 1u)) - 1u) & ~((1ull << keep) - 1u)) : 0u);
				if (ey == 0) ey = 1;
			}
			fails += VerifyPair(x, Make<Posit>(ey), reportTestCases);
			fails += VerifyFrac(x, reportTestCases);
		}
		return fails;
	}

	// a wide posit: the operands of a narrow one, which it holds exactly, with an exact result
	template<typename Wide, typename Narrow>
	int VerifyEmbedded(unsigned nrSamples, std::uint64_t seed, bool reportTestCases) {
		std::mt19937_64 rng(seed);
		int fails = 0;
		for (unsigned s = 0; s < nrSamples; ++s) {
			const Narrow nx = Make<Narrow>(rng() & ((1ull << Narrow::nbits) - 1u));
			const Narrow ny = Make<Narrow>(rng() & ((1ull << Narrow::nbits) - 1u));
			if (nx.isnar() || ny.isnar() || ny.iszero()) continue;
			const double dx = double(nx), dy = double(ny);
			const Wide x(dx), y(dy);
			const double ef = std::fmod(dx, dy), er = std::remainder(dx, dy);
			// the exact results have at most the narrow posit's bits at its scales, so they are wide posits
			const Wide f = fmod(x, y), r = remainder(x, y), t = frac(x);
			if (double(f) != ef) fails += Report(reportTestCases, "fmod", x, y, f, Wide(ef));
			if (double(r) != er) fails += Report(reportTestCases, "remainder", x, y, r, Wide(er));
			if (double(t) != dx - std::trunc(dx)) fails += Report(reportTestCases, "frac", x, y, t, Wide(dx - std::trunc(dx)));
		}
		return fails;
	}

	// 2^k mod m and 2^k remainder m: a quotient of up to k bits, and an exact small integer result
	template<typename Posit>
	int VerifyLargeQuotients(int maxk, bool reportTestCases) {
		int fails = 0;
		Posit power(2);   // 2^k, k = 1, 8, 15, ...: a power of 2 times 2^7 stays exact while it is a posit
		for (int k = 1; k <= maxk; k += 7, power *= Posit(128)) {
			const Posit x = power;
			// near maxpos the regime cuts off exponent bits, and not every power of 2 is a posit
			if (x.scale() != k) break;
			for (std::uint64_t m : { 3ull, 5ull, 7ull, 10ull, 12ull, 1000003ull, 4294967291ull }) {
				std::uint64_t p = 1;   // 2^k mod m
				for (int i = 0; i < k; ++i) p = (p * 2u) % m;
				const Posit y(static_cast<double>(m));
				const Posit wantf(static_cast<double>(p));
				// remainder: p, or p - m when p > m/2; a tie (2p = m) is left to the integer test below
				const bool tie = (2u * p == m);
				const double rem = (2u * p > m) ? static_cast<double>(p) - static_cast<double>(m) : static_cast<double>(p);
				const Posit f = fmod(x, y), r = remainder(x, y);
				if (f != wantf) fails += Report(reportTestCases, "fmod", x, y, f, wantf);
				if (!tie && r != Posit(rem)) fails += Report(reportTestCases, "remainder", x, y, r, Posit(rem));
				const Posit nf = fmod(-x, y);
				if (nf != -wantf) fails += Report(reportTestCases, "fmod", -x, y, nf, -wantf);
			}
		}
		return fails;
	}

	// N 2^-j modulo m 2^-j, for a 58-bit integer N and m of up to 20 bits. j puts x in [1, 16), where a
	// posit<64,3> has 58 fraction bits and a double 52, so the posit holds x exactly and a double does
	// not; y = m 2^-j lies near 2^-38, still with more fraction bits than m needs. The reference is
	// integer arithmetic; remainder takes the even quotient at a tie.
	template<typename Posit>
	int VerifyWideIntegers(unsigned nrSamples, std::uint64_t seed, bool reportTestCases) {
		constexpr unsigned nbits = 58u;   // bits of N
		std::mt19937_64 rng(seed);
		int fails = 0;
		for (unsigned s = 0; s < nrSamples; ++s) {
			const std::uint64_t N = (rng() >> (64u - nbits)) | (1ull << (nbits - 1u));
			const std::uint64_t m = 1u + (rng() >> 44);
			const int j = static_cast<int>(nbits - 1u) - static_cast<int>(rng() % 4u);
			// x = hi 2^(32-j) + lo 2^-j: each part exact, and their sum lands in [1, 16), where it is
			// exact too (summed at 2^57 first, it would round to the posit's 52 bits there)
			const Posit scale(std::ldexp(1.0, -j)), high(std::ldexp(1.0, 32 - j));   // posits: 2^-57 is above minpos
			const Posit x = Posit(static_cast<double>(N >> 32)) * high + Posit(static_cast<double>(N & 0xFFFF'FFFFull)) * scale;
			const Posit y = Posit(static_cast<double>(m)) * scale;
			const std::uint64_t q = N / m, p = N % m;
			const Posit wantf = Posit(static_cast<double>(p)) * scale;
			const bool down = (2u * p < m) || (2u * p == m && (q % 2u) == 0u);
			const Posit wantr = down ? wantf : -(Posit(static_cast<double>(m - p)) * scale);
			const Posit f = fmod(x, y), r = remainder(x, y);
			if (f != wantf) fails += Report(reportTestCases, "fmod", x, y, f, wantf);
			if (r != wantr) fails += Report(reportTestCases, "remainder", x, y, r, wantr);
			if (fmod(-x, y) != -wantf) fails += Report(reportTestCases, "fmod", -x, y, fmod(-x, y), -wantf);
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

	std::string test_suite  = "posit fmod, remainder and frac";
	std::string test_tag    = "fractional";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	{
		using Posit = posit<32, 2>;
		const Posit x(1.0 + std::ldexp(1.0, -27)), y(SpecificValue::minpos);
		std::cout << to_binary(fmod(x, y)) << " vs " << to_binary(Posit(std::fmod(double(x), double(y)))) << '\n';
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<posit<3, 1>>(reportTestCases), "posit<3,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<posit<4, 0>>(reportTestCases), "posit<4,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<posit<5, 2>>(reportTestCases), "posit<5,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<posit<6, 1>>(reportTestCases), "posit<6,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<posit<8, 0>>(reportTestCases), "posit<8,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<posit<8, 1>>(reportTestCases), "posit<8,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<posit<8, 2>>(reportTestCases), "posit<8,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyRandom<posit<16, 1>>(20000, 1, reportTestCases), "posit<16,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyRandom<posit<16, 2>>(20000, 2, reportTestCases), "posit<16,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyRandom<posit<24, 2>>(20000, 3, reportTestCases), "posit<24,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyRandom<posit<32, 2>>(20000, 4, reportTestCases), "posit<32,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyRandom<posit<32, 2, std::uint32_t>>(5000, 5, reportTestCases)), "posit<32,2,uint32_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyEmbedded<posit<64, 3>, posit<16, 1>>(10000, 6, reportTestCases)), "posit<64,3>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyEmbedded<posit<80, 3>, posit<16, 1>>(2000, 7, reportTestCases)), "posit<80,3>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyEmbedded<posit<128, 4>, posit<20, 2>>(5000, 8, reportTestCases)), "posit<128,4>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyEmbedded<posit<256, 5>, posit<20, 2>>(1000, 9, reportTestCases)), "posit<256,5>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyWideIntegers<posit<64, 3>>(20000, 14, reportTestCases), "posit<64,3> integers", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyWideIntegers<posit<128, 4>>(5000, 15, reportTestCases), "posit<128,4> integers", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLargeQuotients<posit<64, 3>>(480, reportTestCases), "posit<64,3> 2^k mod m", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLargeQuotients<posit<128, 4>>(1950, reportTestCases), "posit<128,4> 2^k mod m", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLargeQuotients<posit<256, 5>>(7800, reportTestCases), "posit<256,5> 2^k mod m", test_tag);
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<posit<10, 1>>(reportTestCases), "posit<10,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyRandom<posit<32, 2>>(1000000, 10, reportTestCases), "posit<32,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyRandom<posit<32, 3>>(1000000, 11, reportTestCases), "posit<32,3>", test_tag);
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<posit<12, 1>>(reportTestCases), "posit<12,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyEmbedded<posit<128, 4>, posit<24, 2>>(100000, 12, reportTestCases)), "posit<128,4>", test_tag);
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyRandom<posit<32, 2>>(20000000, 13, reportTestCases), "posit<32,2>", test_tag);
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
