// sqrt.cpp: lns sqrt is correctly rounded in the exponent domain (#1725)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// An lns value is 2^(E 2^-rbits) and lns rounds in the exponent domain, ties to even.  The
// correctly rounded root therefore has the exponent E/2 rounded to an integer, ties to even.
// sqrt used to go through double twice (std::sqrt, then std::log2 in the conversion back),
// which capped the precision at a double's and decided every tie, an odd E, by noise.
//
// The oracle reads both exponents as integers from the raw encodings, so it shares nothing
// with the implementation's shift: 2 Er must lie within one of Ea, and when it misses by one
// (a tie) Er must be even.
#include <universal/utility/directives.hpp>
#include <cerrno>
#include <cmath>
#include <cstdint>
#include <random>
#include <string>

#include <universal/number/lns/lns.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

using namespace sw::universal;

// the signed exponent field, bits [0, nbits - 1), of an lns encoding
template<typename Lns>
std::int64_t exponent_of(const Lns& v) {
	constexpr unsigned ebits = Lns::nbits - 1u;
	static_assert(ebits <= 63, "the exponent must fit an int64");
	std::uint64_t u = 0;
	for (unsigned i = 0; i < ebits; ++i) if (v.at(i)) u |= (std::uint64_t(1) << i);
	if (v.at(ebits - 1u)) u |= ~std::uint64_t(0) << ebits;           // sign extend
	return static_cast<std::int64_t>(u);
}

template<typename Lns>
int VerifySqrt(bool reportTestCases, unsigned long long samples = 0) {
	constexpr unsigned nbits = Lns::nbits;
	int fails = 0;
	unsigned long long checked = 0;
	auto fail = [&](const Lns& a, const Lns& r, const char* why) {
		if (reportTestCases && fails < 4) std::cerr << "FAIL: " << type_tag(a) << " sqrt(" << to_binary(a) << ") = " << to_binary(r) << ": " << why << '\n';
		++fails;
	};
	auto one = [&](const Lns& a) {
		if (a.isnan() || a.iszero() || a.isneg()) return;
		++checked;
		const Lns r = sqrt(a);
		if (r.isnan() || r.iszero() || r.isneg()) { fail(a, r, "not a positive value"); return; }
		const std::int64_t Ea = exponent_of(a), Er = exponent_of(r);
		const std::int64_t miss = 2 * Er - Ea;
		if (miss < -1 || miss > 1) fail(a, r, "not the nearest exponent");
		else if (miss != 0 && (Er & 1)) fail(a, r, "a tie not rounded to even");
	};
	if constexpr (nbits <= 20) {
		if (samples == 0) {
			for (unsigned long long bits = 0; bits < (1ull << nbits); ++bits) {
				Lns a;
				a.setbits(bits);
				one(a);
			}
			return checked > 0 ? fails : 1;
		}
	}
	std::mt19937_64 rng(1725);
	for (unsigned long long i = 0; i < samples; ++i) {
		Lns a;
		a.setbits(rng() & ((nbits >= 64) ? ~0ull : ((1ull << nbits) - 1)));
		one(a);
	}
	return checked > 0 ? fails : 1;
}

// the special values: sqrt(0) = 0, sqrt(NaN) = NaN, and a negative argument is EDOM and a NaN,
// or lns_negative_sqrt_arg under LNS_THROW_ARITHMETIC_EXCEPTION
template<typename Lns>
int VerifySpecialValues(bool reportTestCases) {
	int fails = 0;
	auto check = [&](bool ok, const char* what) {
		if (!ok) { ++fails; if (reportTestCases) std::cerr << "FAIL: " << type_tag(Lns()) << ' ' << what << '\n'; }
	};
	Lns zero(0.0), nan;
	nan.setnan();
	check(sqrt(zero).iszero(), "sqrt(0) is not 0");
	check(sqrt(nan).isnan(), "sqrt(NaN) is not NaN");
#if LNS_THROW_ARITHMETIC_EXCEPTION
	bool thrown = false;
	try { (void)sqrt(Lns(-4.0)); } catch (const lns_negative_sqrt_arg&) { thrown = true; }
	check(thrown, "sqrt(-4) did not throw lns_negative_sqrt_arg");
#else
	errno = 0;
	const Lns r = sqrt(Lns(-4.0));
	check(r.isnan() && errno == EDOM, "sqrt(-4) is not NaN with EDOM");
#endif
	check(std::fabs(double(sqrt(Lns(16.0))) - 4.0) < 1.0e-6, "sqrt(16) is not 4");
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

	std::string test_suite  = "lns sqrt correct rounding (#1725)";
	std::string test_tag    = "sqrt";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult((VerifySqrt<lns<16, 8>>(reportTestCases)), "lns<16,8>", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	// exhaustive
	nrOfFailedTestCases += ReportTestResult((VerifySqrt<lns<8, 2, std::uint8_t>>(reportTestCases)), "lns<8,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifySqrt<lns<10, 4, std::uint8_t>>(reportTestCases)), "lns<10,4>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifySqrt<lns<12, 0, std::uint8_t>>(reportTestCases)), "lns<12,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifySqrt<lns<16, 8, std::uint16_t>>(reportTestCases)), "lns<16,8>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifySqrt<lns<16, 12, std::uint8_t, Behavior::Saturating>>(reportTestCases)), "lns<16,12> saturating", test_tag);
	// sampled: past what the double route could carry
	nrOfFailedTestCases += ReportTestResult((VerifySqrt<lns<32, 22, std::uint32_t>>(reportTestCases, 50000)), "lns<32,22>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifySqrt<lns<48, 40, std::uint16_t>>(reportTestCases, 50000)), "lns<48,40>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifySqrt<lns<64, 60, std::uint32_t>>(reportTestCases, 50000)), "lns<64,60>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySpecialValues<lns<16, 8, std::uint16_t>>(reportTestCases), "lns<16,8> special values", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySpecialValues<lns<32, 22, std::uint32_t>>(reportTestCases), "lns<32,22> special values", test_tag);
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
