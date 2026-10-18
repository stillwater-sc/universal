// sqrt_correct_rounding.cpp: cfloat sqrt is correctly rounded at every width (#1725)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// cfloat formats of up to 52 fraction bits took the host's root, cfloat(std::sqrt(double(a))),
// which rounds twice: correct only while the double carries 2p + 2 bits of a p-bit
// significand (fbits <= 24).  cfloat<48,8> misrounded 9 arguments in 10^5 and cfloat<60,8> a
// quarter of them; wider formats ended a Newton iteration without proving the last ulp.
//
// None of the oracles here uses cfloat arithmetic:
//   - fbits <= 51: r is the correctly rounded root of a exactly when a lies between the
//     squares of the midpoints to r's neighbours.  A midpoint then has at most 53 bits, a
//     double, and its square is exact in double-double.  Both sides are first scaled by a
//     power of four, exactly, so a subnormal square cannot underflow.
//   - fp64: IEEE 754 requires std::sqrt to be correctly rounded.
//   - cfloat<128,15>: encodings from mpmath (tools/generators/cfloat_sqrt_gen.py).
#include <universal/utility/directives.hpp>
#include <cmath>
#include <cstdint>
#include <random>
#include <string>

#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/dd/dd.hpp>
#include <universal/verification/cfloat_sqrt_vectors.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

using namespace sw::universal;

// r == sqrt(a) correctly rounded, for a finite a > 0, judged in double-double
template<typename Cfloat>
bool is_correctly_rounded(const Cfloat& a, const Cfloat& r) {
	static_assert(Cfloat::fbits <= 51, "a midpoint must fit a double");
	if (!r.isnormal() && !r.isdenormal()) return false;
	const double ad = double(a), rd = double(r);
	Cfloat down(r), up(r);
	--down;
	++up;
	const int s = std::ilogb(rd);                       // scale r near 1 and a near 1, exactly
	const dd A(std::ldexp(ad, -2 * s));
	const double lo = std::ldexp(0.5 * (double(down) + rd), -s);   // exact: 53 bits at most
	if (A < dd(lo) * dd(lo)) return false;
	if (up.isinf() || up.isnan()) return true;          // r is maxpos
	const double hi = std::ldexp(0.5 * (rd + double(up)), -s);
	return !(A > dd(hi) * dd(hi));
}

// every positive encoding of a narrow format, or a random sample of a wider one
template<typename Cfloat>
int VerifyAgainstMidpoints(bool reportTestCases, unsigned long long samples = 0) {
	constexpr unsigned nbits = Cfloat::nbits;
	int fails = 0;
	auto one = [&](const Cfloat& a) {
		if (a.isnan() || a.isinf() || a.iszero() || a.isneg()) return;
		const Cfloat r = sqrt(a);
		if (!is_correctly_rounded(a, r)) {
			if (fails < 4 && reportTestCases) std::cerr << "FAIL: " << type_tag(a) << " sqrt(" << to_binary(a) << ") = " << to_binary(r) << '\n';
			++fails;
		}
	};
	if constexpr (nbits <= 20) {
		if (samples == 0) {                               // exhaustive
			for (unsigned long long bits = 0; bits < (1ull << (nbits - 1)); ++bits) {
				Cfloat a;
				a.setbits(bits);
				one(a);
			}
			return fails;
		}
	}
	{
		std::mt19937_64 rng(1725);
		for (unsigned long long i = 0; i < samples; ++i) {
			Cfloat a;
			a.setbits(rng() & ((nbits >= 64) ? ~0ull : ((1ull << (nbits - 1)) - 1)));
			one(a);
		}
	}
	return fails;
}

// fp64: the host's std::sqrt is correctly rounded by IEEE 754
int VerifyFp64(bool reportTestCases, unsigned long long samples) {
	std::mt19937_64 rng(1725);
	int fails = 0;
	for (unsigned long long i = 0; i < samples; ++i) {
		const std::uint64_t bits = rng() & 0x7FFF'FFFF'FFFF'FFFFull;
		fp64 a;
		a.setbits(bits);
		if (a.isnan() || a.isinf() || a.iszero()) continue;
		const double want = std::sqrt(double(a));
		const fp64 r = sqrt(a);
		if (double(r) != want) {
			if (fails < 4 && reportTestCases) std::cerr << "FAIL: fp64 sqrt(" << double(a) << ") = " << double(r) << " vs " << want << '\n';
			++fails;
		}
	}
	return fails;
}

// IEEE quad against mpmath
int VerifyQuad(bool reportTestCases) {
	using Quad = cfloat<128, 15, std::uint32_t, true, false, false>;
	int fails = 0;
	auto load = [](const std::uint64_t (&w)[2]) {
		Quad q;
		q.setblock(0, static_cast<std::uint32_t>(w[1]));
		q.setblock(1, static_cast<std::uint32_t>(w[1] >> 32));
		q.setblock(2, static_cast<std::uint32_t>(w[0]));
		q.setblock(3, static_cast<std::uint32_t>(w[0] >> 32));
		return q;
	};
	for (const auto& v : cfloat_sqrt_verification::quad_sqrt) {
		const Quad a = load(v.a), want = load(v.r);
		const Quad r = sqrt(a);
		if (!(r == want)) {
			if (fails < 4 && reportTestCases) std::cerr << "FAIL: quad sqrt(" << to_binary(a) << ")\n   got  " << to_binary(r) << "\n   want " << to_binary(want) << '\n';
			++fails;
		}
	}
	return fails;
}

}  // anonymous namespace

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

	std::string test_suite  = "cfloat sqrt correct rounding (#1725)";
	std::string test_tag    = "sqrt";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult((VerifyAgainstMidpoints<cfloat<60, 8, std::uint32_t, true, false, false>>(reportTestCases, 10000)), "cfloat<60,8>", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	// exhaustive: half, a bfloat16 layout, a narrow exponent whose roots go subnormal, no subnormals
	nrOfFailedTestCases += ReportTestResult((VerifyAgainstMidpoints<cfloat<16, 5, std::uint16_t, true, false, false>>(reportTestCases)), "cfloat<16,5>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyAgainstMidpoints<cfloat<16, 8, std::uint16_t, true, false, false>>(reportTestCases)), "cfloat<16,8>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyAgainstMidpoints<cfloat<16, 2, std::uint16_t, true, false, false>>(reportTestCases)), "cfloat<16,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyAgainstMidpoints<cfloat<16, 5, std::uint8_t, false, false, false>>(reportTestCases)), "cfloat<16,5> no subnormals", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyAgainstMidpoints<cfloat<20, 6, std::uint8_t, true, true, false>>(reportTestCases)), "cfloat<20,6> max-exp values", test_tag);
	// sampled: the double-rounding range 24 < fbits <= 51, then fp64
	nrOfFailedTestCases += ReportTestResult((VerifyAgainstMidpoints<cfloat<32, 8, std::uint32_t, true, false, false>>(reportTestCases, 20000)), "cfloat<32,8>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyAgainstMidpoints<cfloat<40, 8, std::uint32_t, true, false, false>>(reportTestCases, 20000)), "cfloat<40,8>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyAgainstMidpoints<cfloat<48, 8, std::uint32_t, true, false, false>>(reportTestCases, 20000)), "cfloat<48,8>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyAgainstMidpoints<cfloat<60, 8, std::uint32_t, true, false, false>>(reportTestCases, 20000)), "cfloat<60,8>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyAgainstMidpoints<cfloat<56, 5, std::uint8_t, true, false, false>>(reportTestCases, 20000)), "cfloat<56,5>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyFp64(reportTestCases, 20000), "fp64", test_tag);
	// wider than a double: Newton and the correction step
	nrOfFailedTestCases += ReportTestResult(VerifyQuad(reportTestCases), "cfloat<128,15>", test_tag);
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult((VerifyAgainstMidpoints<cfloat<60, 8, std::uint32_t, true, false, false>>(reportTestCases, 200000)), "cfloat<60,8>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyFp64(reportTestCases, 200000), "fp64", test_tag);
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
