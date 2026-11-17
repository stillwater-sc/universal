// integer_cast.cpp: fixpnt to native integer casts follow the library's integer-cast rule (#2104, #2111)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <iomanip>
#include <cstdint>
#include <limits>

#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/verification/test_reporters.hpp>

namespace sw { namespace universal {

// The rule, in both arithmetic modes: the value truncates toward zero; past the range it saturates
// to +/-max(), an unsigned type to [0, max() - 1], and a negative value cast to unsigned is 0.
// Before #2111 a signed cast floored ((int)-1.5 was -2) and an unsigned cast returned the raw
// encoding without the radix scale ((unsigned)fixpnt<8,4>(3.0) was 48).

// the reference: the raw two's complement value r is r * 2^-rbits; truncate it toward zero
template<typename Integer>
Integer ReferenceCast(long long raw, unsigned rbits) {
	const bool negative = raw < 0;
	const unsigned long long m = negative ? static_cast<unsigned long long>(-raw) : static_cast<unsigned long long>(raw);
	const unsigned long long t = m >> rbits;
	if (negative) {
		if constexpr (std::is_signed_v<Integer>) {
			const unsigned long long cap = static_cast<unsigned long long>(std::numeric_limits<Integer>::max());
			return static_cast<Integer>(-static_cast<long long>(t > cap ? cap : t));
		}
		else return Integer(0);
	}
	unsigned long long cap = static_cast<unsigned long long>(std::numeric_limits<Integer>::max());
	if constexpr (!std::is_signed_v<Integer>) cap -= 1;
	return static_cast<Integer>(t > cap ? cap : t);
}

// exhaustive over a narrow fixpnt
template<unsigned nbits, unsigned rbits, bool arithmetic, typename Integer>
int VerifyIntegerCast(bool reportTestCases) {
	using F = fixpnt<nbits, rbits, arithmetic, std::uint8_t>;
	int nrOfFailedTestCases = 0;
	for (unsigned i = 0; i < (1u << nbits); ++i) {
		F f;
		f.setbits(i);
		const long long raw = (i & (1u << (nbits - 1))) ? static_cast<long long>(i) - (1ll << nbits) : static_cast<long long>(i);
		const Integer expected = ReferenceCast<Integer>(raw, rbits);
		const Integer got = static_cast<Integer>(f);
		if (got != expected) {
			++nrOfFailedTestCases;
			if (reportTestCases && nrOfFailedTestCases < 10) {
				std::cout << "FAIL: cast of " << to_binary(f) << " (" << f << ") = " << +got << ", expected " << +expected << '\n';
			}
		}
	}
	return nrOfFailedTestCases;
}

template<typename Integer, typename F>
int CheckCast(const F& f, Integer expected, const char* label) {
	const Integer got = static_cast<Integer>(f);
	if (got == expected) return 0;
	std::cout << "FAIL: " << label << " = " << +got << ", expected " << +expected << '\n';
	return 1;
}

// the issue's cases, and wide layouts whose integer part is past 64 bits
int VerifyIntegerCastCases() {
	int nrOfFailedTestCases = 0;
	nrOfFailedTestCases += CheckCast<unsigned>(fixpnt<8, 4>(3.0), 3u, "(unsigned)fixpnt<8,4>(3.0)");
	nrOfFailedTestCases += CheckCast<unsigned>(fixpnt<8, 4>(-1.5), 0u, "(unsigned)fixpnt<8,4>(-1.5)");
	nrOfFailedTestCases += CheckCast<int>(fixpnt<8, 4>(-1.5), -1, "(int)fixpnt<8,4>(-1.5)");
	nrOfFailedTestCases += CheckCast<int>(fixpnt<8, 4, Saturate>(-1.5), -1, "(int)fixpnt<8,4,Saturate>(-1.5)");
	nrOfFailedTestCases += CheckCast<int>(fixpnt<8, 4>(-8.0), -8, "(int)fixpnt<8,4>(maxneg)");
	nrOfFailedTestCases += CheckCast<int>(fixpnt<8, 8>(-0.5), 0, "(int)fixpnt<8,8>(-0.5)");

	using W = fixpnt<96, 16, Modulo, std::uint32_t>;
	W big;
	big.setbits(0);
	big.setbit(16 + 70, true);   // 2^70
	nrOfFailedTestCases += CheckCast<long long>(big, std::numeric_limits<long long>::max(), "(long long)2^70");
	nrOfFailedTestCases += CheckCast<long long>(-big, -std::numeric_limits<long long>::max(), "(long long)-2^70");
	nrOfFailedTestCases += CheckCast<unsigned long long>(big, std::numeric_limits<unsigned long long>::max() - 1, "(unsigned long long)2^70");
	nrOfFailedTestCases += CheckCast<int>(big, std::numeric_limits<int>::max(), "(int)2^70");
	nrOfFailedTestCases += CheckCast<short>(-big, static_cast<short>(-std::numeric_limits<short>::max()), "(short)-2^70");

	using U = fixpnt<64, 0, Saturate, std::uint64_t>;
	U x;
	x.setbits(0x7FFFFFFFFFFFFFFFull);   // maxpos
	nrOfFailedTestCases += CheckCast<long long>(x, std::numeric_limits<long long>::max(), "(long long)fixpnt<64,0>(maxpos)");
	nrOfFailedTestCases += CheckCast<unsigned long long>(x, 0x7FFFFFFFFFFFFFFFull, "(unsigned long long)fixpnt<64,0>(maxpos)");
	nrOfFailedTestCases += CheckCast<unsigned>(x, std::numeric_limits<unsigned>::max() - 1, "(unsigned)fixpnt<64,0>(maxpos)");
	x.setbits(0x8000000000000000ull);   // maxneg, -2^63
	nrOfFailedTestCases += CheckCast<long long>(x, -std::numeric_limits<long long>::max(), "(long long)fixpnt<64,0>(maxneg)");
	using T = fixpnt<64, 32, Modulo, std::uint32_t>;
	T y(-12345.75);
	nrOfFailedTestCases += CheckCast<long>(y, -12345l, "(long)fixpnt<64,32>(-12345.75)");
	nrOfFailedTestCases += CheckCast<unsigned short>(T(54321.5), static_cast<unsigned short>(54321), "(unsigned short)fixpnt<64,32>(54321.5)");
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

	std::string test_suite  = "fixpnt integer casts (#2104, #2111)";
	std::string test_tag    = "integer cast";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	std::cout << "(int)fixpnt<8,4>(-1.5) = " << int(fixpnt<8, 4>(-1.5)) << '\n';
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerCastCases(), test_tag, "issue cases and wide layouts");
	nrOfFailedTestCases += ReportTestResult((VerifyIntegerCast<8, 4, Modulo, int>(reportTestCases)), test_tag, "fixpnt<8,4,Modulo> -> int");
	nrOfFailedTestCases += ReportTestResult((VerifyIntegerCast<8, 4, Saturate, int>(reportTestCases)), test_tag, "fixpnt<8,4,Saturate> -> int");
	nrOfFailedTestCases += ReportTestResult((VerifyIntegerCast<8, 4, Modulo, unsigned>(reportTestCases)), test_tag, "fixpnt<8,4,Modulo> -> unsigned");
	nrOfFailedTestCases += ReportTestResult((VerifyIntegerCast<8, 4, Saturate, unsigned>(reportTestCases)), test_tag, "fixpnt<8,4,Saturate> -> unsigned");
	nrOfFailedTestCases += ReportTestResult((VerifyIntegerCast<8, 8, Modulo, short>(reportTestCases)), test_tag, "fixpnt<8,8,Modulo> -> short");
	nrOfFailedTestCases += ReportTestResult((VerifyIntegerCast<8, 0, Modulo, unsigned short>(reportTestCases)), test_tag, "fixpnt<8,0,Modulo> -> unsigned short");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult((VerifyIntegerCast<12, 5, Modulo, long long>(reportTestCases)), test_tag, "fixpnt<12,5,Modulo> -> long long");
	nrOfFailedTestCases += ReportTestResult((VerifyIntegerCast<12, 5, Saturate, unsigned long long>(reportTestCases)), test_tag, "fixpnt<12,5,Saturate> -> unsigned long long");
	// a target narrower than the integer part saturates
	nrOfFailedTestCases += ReportTestResult((VerifyIntegerCast<20, 2, Modulo, short>(reportTestCases)), test_tag, "fixpnt<20,2,Modulo> -> short");
	nrOfFailedTestCases += ReportTestResult((VerifyIntegerCast<20, 2, Saturate, unsigned short>(reportTestCases)), test_tag, "fixpnt<20,2,Saturate> -> unsigned short");
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
