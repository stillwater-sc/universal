// rounding.cpp: e8m0 conversions against an independent reference, run time against constant evaluation,
// and the integer cast (#2106)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// e8m0 rounds v to the nearest power of two in the log domain: up from 2^E exactly when v >= 2^(E + 1/2).
// Each check here failed before #2106:
//   - at constant evaluation every float subnormal was flushed to encoding 0, while the run-time path
//     rounded it in the log domain;
//   - a double was first rounded to float, which can carry a value just below 2^(E + 1/2) onto the
//     float above it, and the run-time path used std::round(std::log2(float)), whose rounding can do
//     the same;
//   - (int)e8m0(maxpos) cast 2^127 to int: UB, INT_MIN on x86.
// The reference decides v >= 2^(E + 1/2) exactly: with v = M * 2^(E - 52) for the 53-bit significand
// M, the test is M^2 >= 2^105, done in integer<128>.
#include <universal/utility/directives.hpp>
#include <universal/number/e8m0/e8m0.hpp>
#include <universal/number/integer/integer.hpp>
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

	using sw::universal::e8m0;
	using U128 = sw::universal::integer<128, std::uint32_t, sw::universal::IntegerNumberType::IntegerNumber>;

	// the encoding e8m0 must give for a double
	unsigned reference_encoding(double v) {
		if (v != v) return 0xFFu;
		if (std::isinf(v)) return 0xFEu;              // either sign saturates to maxpos, as SpecificValue::infneg does
		if (v <= 0.0) return 0u;
		std::uint64_t bits = 0;
		std::memcpy(&bits, &v, sizeof(bits));
		const int raw = static_cast<int>((bits >> 52) & 0x7FFu);
		if (raw == 0) return 0u;                      // a subnormal double: below 2^-1022
		const std::uint64_t M = (std::uint64_t(1) << 52) | (bits & ((std::uint64_t(1) << 52) - 1u));
		U128 m2(static_cast<long long>(M));
		m2 *= U128(static_cast<long long>(M));
		U128 two105(1);
		two105 <<= 105;
		const int E = raw - 1023;
		int biased = E + 127 + ((m2 >= two105) ? 1 : 0);
		if (biased < 0) biased = 0;
		if (biased > 254) biased = 254;
		return static_cast<unsigned>(biased);
	}

	int VerifyDoubles(std::mt19937_64& rng, unsigned nrSamples, bool reportTestCases) {
		int fails = 0;
		auto one = [&](double v) {
			const unsigned expected = reference_encoding(v);
			const unsigned got = e8m0(v).bits();
			if (got == expected) return;
			++fails;
			if (reportTestCases && fails < 10) std::cerr << "FAIL e8m0(" << v << ") = " << got << ", expected " << expected << '\n';
		};
		for (double v : { 0.0, -0.0, -1.0, -std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity(),
		                  std::numeric_limits<double>::quiet_NaN(), 1.0, 1.5, 0.75, std::ldexp(1.0, -127), std::ldexp(1.0, -128),
		                  std::ldexp(1.0, 127), std::ldexp(1.0, 128), std::numeric_limits<double>::max(), 5e-324 }) {
			one(v);
		}
		// the doubles on either side of every midpoint 2^(E + 1/2) in e8m0's range, and around them
		for (int E = -130; E <= 128; ++E) {
			const double mid = std::ldexp(std::sqrt(2.0), E);   // within an ulp of the irrational midpoint
			double lo = mid, hi = mid;
			for (int k = 0; k < 3; ++k) {
				one(lo);
				one(hi);
				lo = std::nextafter(lo, 0.0);
				hi = std::nextafter(hi, std::numeric_limits<double>::infinity());
			}
		}
		for (unsigned k = 0; k < nrSamples; ++k) {
			double v = 0.0;
			const std::uint64_t r = (rng() & 0x000FFFFFFFFFFFFFull) | ((std::uint64_t(1023 - 160 + rng() % 320u)) << 52);
			std::memcpy(&v, &r, sizeof(v));
			one(v);
		}
		return fails;
	}

	int VerifyFloats(bool reportTestCases) {
		int fails = 0;
		// every float subnormal binade and both sides of its midpoint, and every normal binade
		for (unsigned b = 1; b < 0x00800000u; b = b * 2u + 1u) {
			for (unsigned d : { b, b + 1u, (b >> 1) + 1u }) {
				float f = 0.0f;
				std::memcpy(&f, &d, sizeof(f));
				if (e8m0(f).bits() != reference_encoding(static_cast<double>(f))) {
					++fails;
					if (reportTestCases && fails < 10) std::cerr << "FAIL e8m0(float " << f << ")\n";
				}
			}
		}
		for (int E = -149; E <= 127; ++E) {
			const float f = std::ldexp(1.0f, E) * 1.41421354f;      // the float just above sqrt(2) * 2^E
			if (e8m0(f).bits() != reference_encoding(static_cast<double>(f))) ++fails;
			const float g = std::nextafter(f, 0.0f);
			if (e8m0(g).bits() != reference_encoding(static_cast<double>(g))) ++fails;
		}
		return fails;
	}

	// constant evaluation gives what run time gives
	constexpr unsigned cx_denorm = e8m0(std::numeric_limits<float>::denorm_min()).bits();
	constexpr unsigned cx_sub    = e8m0(1.0e-39f).bits();
	constexpr unsigned cx_mid    = e8m0(1.41421354f).bits();
	constexpr unsigned cx_below  = e8m0(1.41421342f).bits();
	constexpr unsigned cx_double = e8m0(1.4142135623730951).bits();
	constexpr unsigned cx_sub_up = e8m0(8.8162076e-39f).bits();   // 1.5 * 2^-127, a float subnormal: encoding 1

	int VerifyConstexpr(bool reportTestCases) {
		int fails = 0;
		volatile float vden = std::numeric_limits<float>::denorm_min(), vsub = 1.0e-39f, vmid = 1.41421354f, vbelow = 1.41421342f;
		volatile double vd = 1.4142135623730951;
		if (cx_denorm != e8m0(float(vden)).bits()) ++fails;
		if (cx_sub    != e8m0(float(vsub)).bits()) ++fails;
		if (cx_mid    != e8m0(float(vmid)).bits()) ++fails;
		if (cx_below  != e8m0(float(vbelow)).bits()) ++fails;
		if (cx_double != e8m0(double(vd)).bits()) ++fails;
		if (cx_sub_up != 1u) ++fails;
		if (cx_sub != reference_encoding(1.0e-39) && cx_sub != reference_encoding(static_cast<double>(1.0e-39f))) ++fails;
		if (fails && reportTestCases) std::cerr << "FAIL constexpr and run time disagree\n";
		return fails;
	}

	// the library's integer-cast rule (#2104): truncate, saturate to max(), NaN gives INT_MIN
	int VerifyIntegerCast(bool reportTestCases) {
		int fails = 0;
		for (unsigned b = 0; b < 256u; ++b) {
			e8m0 x;
			x.setbits(b);
			int expected = 0;
			if (b == 0xFFu) expected = INT_MIN;
			else {
				const int E = static_cast<int>(b) - 127;
				expected = (E < 0) ? 0 : ((E >= 31) ? INT_MAX : (1 << E));
			}
			if (int(x) != expected) {
				++fails;
				if (reportTestCases && fails < 10) std::cerr << "FAIL int(e8m0 " << b << ") = " << int(x) << ", expected " << expected << '\n';
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

	std::string test_suite  = "e8m0 rounding against an exact reference (#2106)";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors

#else

#if REGRESSION_LEVEL_1
	std::mt19937_64 rng(2106);
	nrOfFailedTestCases += ReportTestResult(VerifyDoubles(rng, 5000, reportTestCases), "e8m0", "double -> e8m0");
	nrOfFailedTestCases += ReportTestResult(VerifyFloats(reportTestCases), "e8m0", "float -> e8m0, subnormals and midpoints");
	nrOfFailedTestCases += ReportTestResult(VerifyConstexpr(reportTestCases), "e8m0", "constexpr == run time");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerCast(reportTestCases), "e8m0", "int cast over every encoding");
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
