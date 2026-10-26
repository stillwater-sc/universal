// sqrt.cpp: fixpnt sqrt is correctly rounded (#1725)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// fixpnt had two square roots: the umbrella header selected a Babylonian iteration that
// stopped on an epsilon test, and math/sqrt.hpp on its own selected fixpnt(std::sqrt(double(a))).
// Neither was the correctly rounded root.  A fixpnt value is N 2^-rbits, and its correctly
// rounded root is R 2^-rbits with R the integer nearest sqrt(N 2^rbits).
//
// The oracle works in integer<>, independent of the implementation's integer square root:
// R is that nearest integer exactly when (2R - 1)^2 <= 4 N 2^rbits <= (2R + 1)^2 (no tie can
// occur), or R is maxpos for a root a format without an integer bit cannot hold.
#include <universal/utility/directives.hpp>
#include <cerrno>
#include <cmath>
#include <cstdint>
#include <random>
#include <string>

#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/number/integer/integer.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

using namespace sw::universal;
using Big = integer<320, std::uint32_t>;

// the raw value N of a nonnegative fixpnt
template<typename Fixed>
Big raw_of(const Fixed& v) {
	Big n(0);
	for (unsigned i = Fixed::nbits - 1u; i-- > 0;) {
		n <<= 1;
		if (v.at(i)) n += Big(1);
	}
	return n;
}

template<typename Fixed>
int VerifySqrt(bool reportTestCases, unsigned long long samples = 0) {
	constexpr unsigned nbits = Fixed::nbits, rbits = Fixed::rbits;
	int fails = 0;
	unsigned long long checked = 0;
	const Big maxraw = raw_of(Fixed(SpecificValue::maxpos));
	auto one = [&](const Fixed& a) {
		if (a.isneg() || a.iszero()) return;
		++checked;
		const Fixed r = sqrt(a);
		const Big N = raw_of(a), R = raw_of(r);
		Big M4 = N;
		M4 <<= static_cast<int>(rbits + 2u);                     // 4 N 2^rbits
		const Big lo = Big(2) * R - Big(1), hi = Big(2) * R + Big(1);
		bool ok = !r.isneg() && (lo * lo <= M4) && (M4 <= hi * hi);
		if (!ok && R == maxraw && hi * hi < M4) ok = true;      // saturated: the root is beyond maxpos
		if (!ok) {
			if (reportTestCases && fails < 4) std::cerr << "FAIL: " << type_tag(a) << " sqrt(" << to_binary(a) << ") = " << to_binary(r) << '\n';
			++fails;
		}
	};
	if constexpr (nbits <= 20) {
		if (samples == 0) {
			for (unsigned long long bits = 0; bits < (1ull << nbits); ++bits) {
				Fixed a;
				a.setbits(bits);
				one(a);
			}
			return checked > 0 ? fails : 1;
		}
	}
	std::mt19937_64 rng(1725);
	for (unsigned long long i = 0; i < samples; ++i) {
		Fixed a(0);
		for (unsigned b = 0; b < nbits; b += 64) {
			const std::uint64_t word = rng();
			for (unsigned k = 0; k < 64 && b + k < nbits; ++k) a.setbit(b + k, (word >> k) & 1u);
		}
		one(a);
	}
	return checked > 0 ? fails : 1;
}

// a negative argument: EDOM and 0, or fixpnt_negative_sqrt_arg under
// FIXPNT_THROW_ARITHMETIC_EXCEPTION
template<typename Fixed>
int VerifyNegative(bool reportTestCases) {
#if FIXPNT_THROW_ARITHMETIC_EXCEPTION
	try { (void)sqrt(Fixed(-2.0)); }
	catch (const fixpnt_negative_sqrt_arg&) { return 0; }
	if (reportTestCases) std::cerr << "FAIL: " << type_tag(Fixed()) << " sqrt(-2) did not throw fixpnt_negative_sqrt_arg\n";
	return 1;
#else
	errno = 0;
	const Fixed r = sqrt(Fixed(-2.0));
	if (r.iszero() && errno == EDOM) return 0;
	if (reportTestCases) std::cerr << "FAIL: " << type_tag(r) << " sqrt(-2) = " << double(r) << " errno " << errno << '\n';
	return 1;
#endif
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

	std::string test_suite  = "fixpnt sqrt correct rounding (#1725)";
	std::string test_tag    = "sqrt";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult((VerifySqrt<fixpnt<16, 8, Modulo, std::uint16_t>>(reportTestCases)), "fixpnt<16,8>", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	// exhaustive, both arithmetic behaviours, and formats with no integer bit or no fraction
	nrOfFailedTestCases += ReportTestResult((VerifySqrt<fixpnt<8, 4, Modulo, std::uint8_t>>(reportTestCases)), "fixpnt<8,4>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifySqrt<fixpnt<8, 8, Modulo, std::uint8_t>>(reportTestCases)), "fixpnt<8,8>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifySqrt<fixpnt<12, 0, Modulo, std::uint8_t>>(reportTestCases)), "fixpnt<12,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifySqrt<fixpnt<12, 11, Saturate, std::uint8_t>>(reportTestCases)), "fixpnt<12,11> saturating", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifySqrt<fixpnt<16, 8, Modulo, std::uint16_t>>(reportTestCases)), "fixpnt<16,8>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifySqrt<fixpnt<16, 12, Saturate, std::uint8_t>>(reportTestCases)), "fixpnt<16,12> saturating", test_tag);
	// sampled: past what a double carries, and past 64 bits
	nrOfFailedTestCases += ReportTestResult((VerifySqrt<fixpnt<32, 16, Modulo, std::uint32_t>>(reportTestCases, 20000)), "fixpnt<32,16>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifySqrt<fixpnt<64, 32, Modulo, std::uint32_t>>(reportTestCases, 20000)), "fixpnt<64,32>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifySqrt<fixpnt<80, 60, Modulo, std::uint8_t>>(reportTestCases, 5000)), "fixpnt<80,60>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifySqrt<fixpnt<128, 64, Modulo, std::uint32_t>>(reportTestCases, 2000)), "fixpnt<128,64>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNegative<fixpnt<16, 8, Modulo, std::uint16_t>>(reportTestCases), "fixpnt<16,8> negative", test_tag);
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
