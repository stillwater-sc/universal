// rounding_and_stepping.cpp: bfloat16 conversions round once, ++/-- step along the value line,
// the NaN types, numeric_limits and the integer casts (#2106)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Each check here failed before #2106:
//   - a double and an integer wider than 24 bits went through static_cast<float> and were rounded
//     twice: bfloat16(1.0 + 2^-8 + 2^-40) was 0x3F80 instead of 0x3F81, and
//     bfloat16((1LL<<30) + (1LL<<22) + 1) 0x4E80 instead of 0x4E81;
//   - ++(-0) gave 0x7FFF, a NaN; --0xFFFF wrapped to 0; ++ from maxpos went into inf;
//   - setnan(NAN_TYPE_QUIET) wrote 0x7F81, which isnan(NAN_TYPE_QUIET) calls signalling;
//   - numeric_limits: is_bounded false, has_denorm denorm_absent;
//   - the integer casts cast the float: UB for NaN, inf and every value outside the target.
// References: the exact source value rounded half-to-even to 8 significant bits by integer
// arithmetic on its binary expansion, and the float, which holds every bfloat16 value exactly.
#include <universal/utility/directives.hpp>
#include <universal/number/bfloat16/bfloat16.hpp>
#include <universal/verification/test_suite.hpp>

#include <climits>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <limits>
#include <random>
#include <string>

namespace {

	using sw::universal::bfloat16;

	bfloat16 from_bits(unsigned b) {
		bfloat16 x;
		x.setbits(b);
		return x;
	}

	// the bfloat16 encoding of the positive value m * 2^E (m > 0), rounded half-to-even once
	unsigned reference_encoding(std::uint64_t m, int E) {
		int p = 63;
		while (((m >> p) & 1u) == 0u) --p;
		int scale = p + E;                        // the value is in [2^scale, 2^(scale+1))
		int keep_lsb = scale - 7;                 // the weight of the last of 8 significant bits
		if (scale < -126) keep_lsb = -133;        // subnormal: a fixed last-bit weight of 2^-133
		// q = floor(value / 2^keep_lsb), with the round bit and the sticky bit below it
		const int shift = keep_lsb - E;           // bits of m below the kept ones
		std::uint64_t q = 0;
		bool round = false, sticky = false;
		if (shift <= 0) q = m << (-shift);
		else if (shift > 64) { q = 0; round = false; sticky = true; }
		else {
			q = (shift == 64) ? 0 : (m >> shift);
			round = ((m >> (shift - 1)) & 1u) != 0;
			sticky = (shift - 1 > 0) && ((m & ((std::uint64_t(1) << (shift - 1)) - 1u)) != 0);
		}
		if (round && (sticky || (q & 1u))) ++q;
		// q * 2^keep_lsb as an encoding: a normal has q in [2^7, 2^8], a subnormal q < 2^7
		if (keep_lsb == -133 && q < 128u) return static_cast<unsigned>(q);   // subnormal (or minnormal reached below)
		int bexp = keep_lsb + 7 + 127;            // the biased exponent of q's leading bit
		if (q >= 256u) { q >>= 1; ++bexp; }      // the rounding carried into the next binade
		// a subnormal that rounded up to 2^-126 already has bexp = -133 + 7 + 127 = 1
		if (bexp >= 255) return 0x7F80u;          // overflow: inf
		return static_cast<unsigned>((bexp << 7) | (q & 0x7Fu));
	}

	int VerifyFromDouble(std::mt19937_64& rng, unsigned nrSamples, bool reportTestCases) {
		int fails = 0;
		auto one = [&](double x) {
			std::uint64_t bits = 0;
			std::memcpy(&bits, &x, sizeof(bits));
			const bool neg = (bits >> 63) != 0;
			const std::uint64_t raw = (bits >> 52) & 0x7FFu, frac = bits & ((std::uint64_t(1) << 52) - 1u);
			if (raw == 0x7FFu || (raw == 0 && frac == 0)) return;
			const std::uint64_t m = (raw == 0) ? frac : ((std::uint64_t(1) << 52) | frac);
			const int E = (raw == 0) ? -1074 : static_cast<int>(raw) - 1075;
			const unsigned expected = reference_encoding(m, E) | (neg ? 0x8000u : 0u);
			const unsigned got = bfloat16(x).bits();
			if (got != expected) {
				++fails;
				if (reportTestCases && fails < 10) std::cerr << "FAIL bfloat16(" << x << ") = 0x" << std::hex << got << ", expected 0x" << expected << std::dec << '\n';
			}
		};
		one(1.0 + std::ldexp(1.0, -8) + std::ldexp(1.0, -40));
		one(1.0 + std::ldexp(1.0, -8) - std::ldexp(1.0, -40));
		one(-(1.0 + std::ldexp(1.0, -8) + std::ldexp(1.0, -40)));
		one(std::ldexp(1.5, -133) + std::ldexp(1.0, -170));   // a subnormal tie broken below float
		one(511.0 * std::ldexp(1.0, -134));                   // rounds up twice, to 2^-125 = 0x0100 (CodeRabbit, #2110)
		for (unsigned k = 0; k < nrSamples; ++k) {
			// a bfloat16 tie with a tiny perturbation of either sign, past float's precision
			const double base = double(from_bits(static_cast<unsigned>(rng() & 0x7F7Fu)));
			const double tie = base + std::ldexp(base, -8);
			one(tie + std::ldexp(tie, -30 - static_cast<int>(rng() % 20u)));
			one(tie - std::ldexp(tie, -30 - static_cast<int>(rng() % 20u)));
			// and a random double in bfloat16's range
			double x = 0.0;
			const std::uint64_t r = (rng() & 0x800FFFFFFFFFFFFFull) | ((std::uint64_t(1023 - 140 + rng() % 270u)) << 52);
			std::memcpy(&x, &r, sizeof(x));
			one(x);
		}
		return fails;
	}

	int VerifyFromIntegers(std::mt19937_64& rng, unsigned nrSamples, bool reportTestCases) {
		int fails = 0;
		auto check = [&](unsigned got, std::uint64_t mag, bool neg, const char* what) {
			const unsigned expected = reference_encoding(mag, 0) | (neg ? 0x8000u : 0u);
			if (got == expected) return;
			++fails;
			if (reportTestCases && fails < 10) std::cerr << "FAIL " << what << ' ' << (neg ? "-" : "") << mag << " = 0x" << std::hex << got << ", expected 0x" << expected << std::dec << '\n';
		};
		const std::uint64_t cases[] = { (1ull << 30) + (1ull << 22) + 1u, (1ull << 30) + (1ull << 22), (1ull << 30) + (1ull << 22) - 1u,
		                                (1ull << 62) + (1ull << 54) + 1u, ULLONG_MAX, (1ull << 24) + 1u, 16777217u };
		for (std::uint64_t v : cases) {
			check(bfloat16(static_cast<unsigned long long>(v)).bits(), v, false, "unsigned long long");
			if (v <= static_cast<std::uint64_t>(LLONG_MAX)) check(bfloat16(-static_cast<long long>(v)).bits(), v, true, "long long");
		}
		check(bfloat16(LLONG_MIN).bits(), 9223372036854775808ull, true, "LLONG_MIN");
		for (unsigned k = 0; k < nrSamples; ++k) {
			const std::uint64_t v = (rng() >> (rng() % 40u)) | 1u;
			check(bfloat16(static_cast<unsigned long long>(v)).bits(), v, false, "unsigned long long");
			if (v <= static_cast<std::uint64_t>(LLONG_MAX)) check(bfloat16(static_cast<long long>(v)).bits(), v, false, "long long");
			const unsigned u = static_cast<unsigned>(v);
			if (u != 0u) check(bfloat16(u).bits(), u, false, "unsigned");
		}
		return fails;
	}

	// ++ and -- against the value line: the next encoding up or down, stopping at maxpos and
	// maxneg, with NaN and inf left in place and -0 stepping like +0
	int VerifyStepping(bool reportTestCases) {
		int fails = 0;
		for (unsigned b = 0; b < 0x10000u; ++b) {
			const bfloat16 x = from_bits(b);
			bfloat16 up = x, down = x;
			++up;
			--down;
			unsigned eu = b, ed = b;
			if (!(x.isnan() || x.isinf())) {
				const float f = float(x);
				// the next bfloat16 up: the smallest encoding whose value exceeds f
				if (b == 0x7F7Fu) eu = b;
				else if (f == 0.0f) eu = 0x0001u;
				else if (b & 0x8000u) eu = (b == 0x8001u) ? 0x0000u : b - 1u;
				else eu = b + 1u;
				if (b == 0xFF7Fu) ed = b;
				else if (f == 0.0f) ed = 0x8001u;
				else if (b & 0x8000u) ed = b + 1u;
				else ed = b - 1u;
				// the step is along the value line: up is above f and down below it
				if (!(float(from_bits(eu)) > f || eu == b) || !(float(from_bits(ed)) < f || ed == b)) ++fails;
			}
			if (up.bits() != eu || down.bits() != ed) {
				++fails;
				if (reportTestCases && fails < 10) std::cerr << "FAIL ++/-- 0x" << std::hex << b << ": 0x" << up.bits() << " / 0x" << down.bits()
				                                              << ", expected 0x" << eu << " / 0x" << ed << std::dec << '\n';
			}
		}
		return fails;
	}

	// the integer-cast rule (#2104): truncate, saturate to +/-max (unsigned: [0, max - 1]), NaN -> NaR
	template<typename Integer>
	Integer reference_cast(float f) {
		using L = std::numeric_limits<Integer>;
		if (f != f) return L::is_signed ? L::min() : L::max();
		const bool neg = f < 0.0f;
		if (neg && !L::is_signed) return Integer(0);
		const double a = std::trunc(std::fabs(static_cast<double>(f)));
		const std::uint64_t cap = static_cast<std::uint64_t>(L::max()) - (L::is_signed ? 0u : 1u);
		std::uint64_t mag = (a >= std::ldexp(1.0, 64)) ? cap : static_cast<std::uint64_t>(a);
		if (mag > cap) mag = cap;
		return neg ? static_cast<Integer>(-static_cast<Integer>(mag)) : static_cast<Integer>(mag);
	}

	int VerifyIntegerCasts(bool reportTestCases) {
		int fails = 0;
		for (unsigned b = 0; b < 0x10000u; ++b) {
			const bfloat16 x = from_bits(b);
			const float f = float(x);
			const bool ok = (int(x) == reference_cast<int>(f)) && (static_cast<short>(x) == reference_cast<short>(f))
			             && (static_cast<signed char>(x) == reference_cast<signed char>(f)) && (static_cast<long long>(x) == reference_cast<long long>(f))
			             && (static_cast<unsigned>(x) == reference_cast<unsigned>(f)) && (static_cast<unsigned long long>(x) == reference_cast<unsigned long long>(f))
			             && (static_cast<unsigned short>(x) == reference_cast<unsigned short>(f));
			if (!ok) {
				++fails;
				if (reportTestCases && fails < 10) std::cerr << "FAIL integer casts of 0x" << std::hex << b << std::dec << " (" << f << ")\n";
			}
		}
		return fails;
	}

	int VerifyNaNAndLimits(bool reportTestCases) {
		using namespace sw::universal;
		int fails = 0;
		auto expect = [&](bool ok, const char* what) { if (!ok) { ++fails; if (reportTestCases) std::cerr << "FAIL " << what << '\n'; } };
		const bfloat16 q(SpecificValue::qnan), s(SpecificValue::snan);
		expect(q.isnan(NAN_TYPE_QUIET) && !q.isnan(NAN_TYPE_SIGNALLING), "qnan is quiet");
		expect(s.isnan(NAN_TYPE_SIGNALLING) && !s.isnan(NAN_TYPE_QUIET), "snan is signalling");
		expect(std::numeric_limits<bfloat16>::quiet_NaN().isnan(NAN_TYPE_QUIET), "quiet_NaN() is quiet");
		expect(std::numeric_limits<bfloat16>::signaling_NaN().isnan(NAN_TYPE_SIGNALLING), "signaling_NaN() is signalling");
		expect(std::numeric_limits<bfloat16>::is_bounded, "is_bounded");
		expect(std::numeric_limits<bfloat16>::has_denorm == std::denorm_present, "has_denorm");
		expect(std::numeric_limits<bfloat16>::denorm_min().bits() == 0x0001u, "denorm_min is 0x0001");
		return fails;
	}

	int VerifyRoundTrip(bool reportTestCases) {
		int fails = 0;
		for (unsigned b = 0; b < 0x10000u; ++b) {
			const bfloat16 x = from_bits(b);
			if (x.isnan()) {
				if (!bfloat16(float(x)).isnan() || !bfloat16(double(x)).isnan()) ++fails;
				continue;
			}
			if (bfloat16(float(x)).bits() != b || bfloat16(double(x)).bits() != b) {
				++fails;
				if (reportTestCases && fails < 10) std::cerr << "FAIL round trip of 0x" << std::hex << b << std::dec << '\n';
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

	std::string test_suite  = "bfloat16 rounding, stepping, NaN and casts (#2106)";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors

#else

#if REGRESSION_LEVEL_1
	std::mt19937_64 rng(2106);
	nrOfFailedTestCases += ReportTestResult(VerifyRoundTrip(reportTestCases), "bfloat16", "every encoding round trips");
	nrOfFailedTestCases += ReportTestResult(VerifyFromDouble(rng, 3000, reportTestCases), "bfloat16", "double rounds once");
	nrOfFailedTestCases += ReportTestResult(VerifyFromIntegers(rng, 3000, reportTestCases), "bfloat16", "integers round once");
	nrOfFailedTestCases += ReportTestResult(VerifyStepping(reportTestCases), "bfloat16", "++ and -- over every encoding");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerCasts(reportTestCases), "bfloat16", "integer casts over every encoding");
	nrOfFailedTestCases += ReportTestResult(VerifyNaNAndLimits(reportTestCases), "bfloat16", "NaN types and numeric_limits");
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
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
