// native_rounding.cpp: fixpnt to float and double is correctly rounded past the target's precision (#2111)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <iomanip>
#include <cstdint>
#include <cmath>

#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/verification/test_reporters.hpp>

namespace sw { namespace universal {

// to_native added the bits one at a time in the target type, which rounded at every step once
// nbits passed 24 (float) or 53 (double): fixpnt<64,0,Modulo,uint64_t> was 1 ulp off for 13% of
// float and 8% of double conversions. The reference converts the raw value as an int64, which the
// hardware rounds once, to nearest even, and scales it by 2^-rbits, which is exact.
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt, typename Real>
int VerifyNativeRounding(unsigned long long nrSamples, bool reportTestCases) {
	static_assert(nbits <= 64, "the reference holds the raw value in an int64");
	using F = fixpnt<nbits, rbits, arithmetic, bt>;
	int nrOfFailedTestCases = 0;
	std::uint64_t x = 0x9E3779B97F4A7C15ull;
	auto check = [&](std::uint64_t bits) {
		if constexpr (nbits < 64) bits &= (std::uint64_t(1) << nbits) - 1;
		F f;
		f.setbits(bits);
		long long raw = static_cast<long long>(bits);
		if constexpr (nbits < 64) {
			if (bits & (std::uint64_t(1) << (nbits - 1))) raw = static_cast<long long>(bits | ~((std::uint64_t(1) << nbits) - 1));
		}
		const Real expected = std::ldexp(static_cast<Real>(raw), -static_cast<int>(rbits));
		const Real got = Real(f);
		if (got != expected) {
			++nrOfFailedTestCases;
			if (reportTestCases && nrOfFailedTestCases < 10) {
				std::cout << "FAIL: " << to_binary(f) << " -> " << std::setprecision(20) << got << ", expected " << expected << '\n';
			}
		}
	};
	for (unsigned long long i = 0; i < nrSamples; ++i) {
		x ^= x << 13; x ^= x >> 7; x ^= x << 17;   // xorshift64
		check(x);
		check(x >> (x & 63));                       // smaller magnitudes as well
	}
	// ties at the target's precision: a 1 just below it, with and without a sticky bit
	constexpr unsigned digits = static_cast<unsigned>(std::numeric_limits<Real>::digits);
	if constexpr (nbits > digits + 2) {
		for (unsigned top = digits + 1; top + 1 < nbits; ++top) {
			const std::uint64_t base = std::uint64_t(1) << top;
			const std::uint64_t half = std::uint64_t(1) << (top - digits);
			check(base | half);                  // tie, even: rounds down
			check(base | half | (half << 1));    // tie, odd: rounds up
			check(base | half | 1u);             // above the tie: rounds up
			check(~(base | half));               // negative
		}
	}
	return nrOfFailedTestCases;
}

}} // namespace sw::universal

// Regression testing guards
#define MANUAL_TESTING 0
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

	std::string test_suite  = "fixpnt to native floating-point rounding (#2111)";
	std::string test_tag    = "to_native";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	fixpnt<64, 0, Modulo, std::uint64_t> f;
	f.setbits(0x0000000001000001ull);
	std::cout << to_binary(f) << " : " << float(f) << '\n';
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult((VerifyNativeRounding<64, 0, Modulo, std::uint64_t, float>(20000, reportTestCases)), test_tag, "fixpnt<64,0,Modulo,uint64_t> -> float");
	nrOfFailedTestCases += ReportTestResult((VerifyNativeRounding<64, 0, Modulo, std::uint64_t, double>(20000, reportTestCases)), test_tag, "fixpnt<64,0,Modulo,uint64_t> -> double");
	nrOfFailedTestCases += ReportTestResult((VerifyNativeRounding<32, 16, Saturate, std::uint32_t, float>(20000, reportTestCases)), test_tag, "fixpnt<32,16,Saturate,uint32_t> -> float");
	nrOfFailedTestCases += ReportTestResult((VerifyNativeRounding<64, 32, Saturate, std::uint32_t, double>(20000, reportTestCases)), test_tag, "fixpnt<64,32,Saturate,uint32_t> -> double");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult((VerifyNativeRounding<64, 60, Modulo, std::uint64_t, float>(100000, reportTestCases)), test_tag, "fixpnt<64,60,Modulo,uint64_t> -> float");
	nrOfFailedTestCases += ReportTestResult((VerifyNativeRounding<56, 7, Modulo, std::uint8_t, double>(100000, reportTestCases)), test_tag, "fixpnt<56,7,Modulo,uint8_t> -> double");
	nrOfFailedTestCases += ReportTestResult((VerifyNativeRounding<40, 20, Saturate, std::uint16_t, float>(100000, reportTestCases)), test_tag, "fixpnt<40,20,Saturate,uint16_t> -> float");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult((VerifyNativeRounding<64, 0, Modulo, std::uint64_t, double>(2000000, reportTestCases)), test_tag, "fixpnt<64,0,Modulo,uint64_t> -> double, 2M");
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
