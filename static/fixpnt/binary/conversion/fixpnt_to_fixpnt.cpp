// fixpnt_to_fixpnt.cpp: test suite for fixpnt cross-type conversions
// with different radix point positions (issue #357)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <iomanip>

#define FIXPNT_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/verification/test_reporters.hpp>

namespace sw { namespace universal {

// Verify fixpnt-to-fixpnt conversion preserves value when radix points differ.
// Exhaustively tests all values of the source type.
template<unsigned src_nbits, unsigned src_rbits, unsigned dst_nbits, unsigned dst_rbits, bool arithmetic, typename bt>
int VerifyFixpntToFixpntConversion(bool reportTestCases) {
	using SrcType = fixpnt<src_nbits, src_rbits, arithmetic, bt>;
	using DstType = fixpnt<dst_nbits, dst_rbits, arithmetic, bt>;
	constexpr unsigned NR_VALUES = (1u << src_nbits);
	int nrOfFailedTestCases = 0;

	SrcType src;
	for (unsigned i = 0; i < NR_VALUES; ++i) {
		src.setbits(i);
		double srcValue = double(src);

		DstType dst = src;  // cross-type conversion under test
		double dstValue = double(dst);

		// Compute expected: convert srcValue through double into DstType
		DstType expected(srcValue);
		double expectedValue = double(expected);

		if (dstValue != expectedValue) {
			++nrOfFailedTestCases;
			if (reportTestCases) {
				std::cout << "FAIL: fixpnt<" << src_nbits << "," << src_rbits << "> -> fixpnt<" << dst_nbits << "," << dst_rbits << ">: "
					<< "src = " << to_binary(src) << " (" << srcValue << ") "
					<< "got = " << to_binary(dst) << " (" << dstValue << ") "
					<< "expected = " << to_binary(expected) << " (" << expectedValue << ")\n";
			}
		}
	}
	return nrOfFailedTestCases;
}

// An exact integer reference (#2111), independent of the conversion through double: the source
// raw value s is s * 2^-src_rbits, so the target raw value is s * 2^(dst_rbits - src_rbits)
// rounded to nearest, ties to even; then Saturate clamps to [maxneg, maxpos] and Modulo keeps the
// low dst_nbits. Compares encodings, exhaustively over the source.
template<unsigned src_nbits, unsigned src_rbits, unsigned dst_nbits, unsigned dst_rbits, bool arithmetic, typename bt>
int VerifyExactFixpntToFixpnt(bool reportTestCases) {
	static_assert(src_nbits <= 16 && dst_nbits <= 16, "the reference works in 64-bit integers");
	using SrcType = fixpnt<src_nbits, src_rbits, arithmetic, bt>;
	using DstType = fixpnt<dst_nbits, dst_rbits, arithmetic, bt>;
	int nrOfFailedTestCases = 0;
	for (unsigned i = 0; i < (1u << src_nbits); ++i) {
		SrcType src;
		src.setbits(i);
		const long long s = (i & (1u << (src_nbits - 1))) ? static_cast<long long>(i) - (1ll << src_nbits) : static_cast<long long>(i);
		long long q;
		if constexpr (dst_rbits >= src_rbits) {
			q = s * (1ll << (dst_rbits - src_rbits));
		}
		else {
			constexpr unsigned shift = src_rbits - dst_rbits;
			const long long div = 1ll << shift;
			long long fl = s >= 0 ? s / div : -((-s + div - 1) / div);   // floor
			const long long rem = s - fl * div;                          // in [0, div)
			if (2 * rem > div || (2 * rem == div && (fl & 1))) ++fl;
			q = fl;
		}
		const long long maxpos = (1ll << (dst_nbits - 1)) - 1, maxneg = -(1ll << (dst_nbits - 1));
		if constexpr (arithmetic == Saturate) {
			if (q > maxpos) q = maxpos;
			if (q < maxneg) q = maxneg;
		}
		const unsigned long long expectedBits = static_cast<unsigned long long>(q) & ((1ull << dst_nbits) - 1);
		DstType dst;
		dst = src;
		DstType expected;
		expected.setbits(expectedBits);
		if (!(dst == expected)) {
			++nrOfFailedTestCases;
			if (reportTestCases && nrOfFailedTestCases < 10) {
				std::cout << "FAIL: fixpnt<" << src_nbits << "," << src_rbits << "> " << to_binary(src) << " (" << src << ") -> fixpnt<"
					<< dst_nbits << "," << dst_rbits << ">: " << to_binary(dst) << " (" << dst << "), expected " << to_binary(expected) << " (" << expected << ")\n";
			}
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
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "fixed-point cross-type conversion (issue #357)";
	std::string test_tag    = "fixpnt-to-fixpnt";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// Quick demonstration of the bug scenario from issue #357
	{
		fixpnt<8, 4> a = 4.25;    // 0100.0100
		fixpnt<8, 2> b;
		b = a;
		std::cout << "fixpnt<8,4> a = " << to_binary(a) << " = " << a << '\n';
		std::cout << "fixpnt<8,2> b = " << to_binary(b) << " = " << b << " (expected 4.25)\n";
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1

	// Same rbits (baseline: radix points aligned)
	nrOfFailedTestCases += ReportTestResult(
		(VerifyFixpntToFixpntConversion<4, 2, 8, 2, Modulo, uint8_t>(reportTestCases)),
		test_tag, "fixpnt<4,2> -> fixpnt<8,2> (expand, same rbits)");
	nrOfFailedTestCases += ReportTestResult(
		(VerifyFixpntToFixpntConversion<8, 4, 4, 4, Modulo, uint8_t>(reportTestCases)),
		test_tag, "fixpnt<8,4> -> fixpnt<4,4> (shrink, same rbits)");

	// Source has more fraction bits (right-shift to align)
	nrOfFailedTestCases += ReportTestResult(
		(VerifyFixpntToFixpntConversion<8, 6, 8, 4, Modulo, uint8_t>(reportTestCases)),
		test_tag, "fixpnt<8,6> -> fixpnt<8,4> (same size, src_rbits > dst_rbits)");
	nrOfFailedTestCases += ReportTestResult(
		(VerifyFixpntToFixpntConversion<4, 3, 8, 1, Modulo, uint8_t>(reportTestCases)),
		test_tag, "fixpnt<4,3> -> fixpnt<8,1> (expand, src_rbits > dst_rbits)");
	nrOfFailedTestCases += ReportTestResult(
		(VerifyFixpntToFixpntConversion<8, 4, 4, 2, Modulo, uint8_t>(reportTestCases)),
		test_tag, "fixpnt<8,4> -> fixpnt<4,2> (shrink, src_rbits > dst_rbits)");

	// Source has fewer fraction bits (left-shift to align)
	nrOfFailedTestCases += ReportTestResult(
		(VerifyFixpntToFixpntConversion<8, 2, 8, 6, Modulo, uint8_t>(reportTestCases)),
		test_tag, "fixpnt<8,2> -> fixpnt<8,6> (same size, src_rbits < dst_rbits)");
	nrOfFailedTestCases += ReportTestResult(
		(VerifyFixpntToFixpntConversion<4, 1, 8, 4, Modulo, uint8_t>(reportTestCases)),
		test_tag, "fixpnt<4,1> -> fixpnt<8,4> (expand, src_rbits < dst_rbits)");
	nrOfFailedTestCases += ReportTestResult(
		(VerifyFixpntToFixpntConversion<8, 2, 4, 3, Modulo, uint8_t>(reportTestCases)),
		test_tag, "fixpnt<8,2> -> fixpnt<4,3> (shrink, src_rbits < dst_rbits)");

	// The specific scenario from issue #357
	nrOfFailedTestCases += ReportTestResult(
		(VerifyFixpntToFixpntConversion<8, 4, 8, 2, Modulo, uint8_t>(reportTestCases)),
		test_tag, "fixpnt<8,4> -> fixpnt<8,2> (issue #357 scenario)");

	// against the exact reference, both arithmetic modes (#2111: Saturate wrapped, e.g.
	// fixpnt<16,8>(100) -> fixpnt<8,4> was 4, and fixpnt<12,6> -> fixpnt<8,3> got half the values wrong)
	nrOfFailedTestCases += ReportTestResult((VerifyExactFixpntToFixpnt<12, 6, 8, 3, Saturate, uint8_t>(reportTestCases)), test_tag, "fixpnt<12,6> -> fixpnt<8,3> Saturate, exact");
	nrOfFailedTestCases += ReportTestResult((VerifyExactFixpntToFixpnt<12, 6, 8, 3, Modulo, uint8_t>(reportTestCases)), test_tag, "fixpnt<12,6> -> fixpnt<8,3> Modulo, exact");
	nrOfFailedTestCases += ReportTestResult((VerifyExactFixpntToFixpnt<16, 8, 8, 4, Saturate, uint8_t>(reportTestCases)), test_tag, "fixpnt<16,8> -> fixpnt<8,4> Saturate, exact");
	nrOfFailedTestCases += ReportTestResult((VerifyExactFixpntToFixpnt<8, 4, 8, 6, Saturate, uint8_t>(reportTestCases)), test_tag, "fixpnt<8,4> -> fixpnt<8,6> Saturate, exact");
	nrOfFailedTestCases += ReportTestResult((VerifyExactFixpntToFixpnt<10, 2, 8, 2, Saturate, uint8_t>(reportTestCases)), test_tag, "fixpnt<10,2> -> fixpnt<8,2> Saturate, exact");
	// a right shift of nbits or more: fixpnt<8,8>(-0.5) -> fixpnt<8,0> is a tie that rounds to even, 0 (it gave -1)
	nrOfFailedTestCases += ReportTestResult((VerifyExactFixpntToFixpnt<8, 8, 8, 0, Modulo, uint8_t>(reportTestCases)), test_tag, "fixpnt<8,8> -> fixpnt<8,0> Modulo, exact");
	nrOfFailedTestCases += ReportTestResult((VerifyExactFixpntToFixpnt<8, 8, 8, 0, Saturate, uint8_t>(reportTestCases)), test_tag, "fixpnt<8,8> -> fixpnt<8,0> Saturate, exact");
	nrOfFailedTestCases += ReportTestResult((VerifyExactFixpntToFixpnt<8, 8, 4, 0, Saturate, uint8_t>(reportTestCases)), test_tag, "fixpnt<8,8> -> fixpnt<4,0> Saturate, exact");

#endif

#if REGRESSION_LEVEL_2
	// exhaustive Saturate and Modulo conversions between 8- to 12-bit configurations (#2111)
	nrOfFailedTestCases += ReportTestResult((VerifyExactFixpntToFixpnt<12, 4, 8, 6, Saturate, uint8_t>(reportTestCases)), test_tag, "fixpnt<12,4> -> fixpnt<8,6> Saturate, exact");
	nrOfFailedTestCases += ReportTestResult((VerifyExactFixpntToFixpnt<12, 8, 10, 3, Saturate, uint16_t>(reportTestCases)), test_tag, "fixpnt<12,8> -> fixpnt<10,3> Saturate, exact");
	nrOfFailedTestCases += ReportTestResult((VerifyExactFixpntToFixpnt<10, 9, 12, 2, Saturate, uint16_t>(reportTestCases)), test_tag, "fixpnt<10,9> -> fixpnt<12,2> Saturate, exact");
	nrOfFailedTestCases += ReportTestResult((VerifyExactFixpntToFixpnt<12, 0, 9, 0, Saturate, uint8_t>(reportTestCases)), test_tag, "fixpnt<12,0> -> fixpnt<9,0> Saturate, exact");
	nrOfFailedTestCases += ReportTestResult((VerifyExactFixpntToFixpnt<12, 4, 8, 6, Modulo, uint8_t>(reportTestCases)), test_tag, "fixpnt<12,4> -> fixpnt<8,6> Modulo, exact");
	nrOfFailedTestCases += ReportTestResult((VerifyExactFixpntToFixpnt<12, 8, 10, 3, Modulo, uint16_t>(reportTestCases)), test_tag, "fixpnt<12,8> -> fixpnt<10,3> Modulo, exact");

	// Broader coverage with different bit widths
	nrOfFailedTestCases += ReportTestResult(
		(VerifyFixpntToFixpntConversion<8, 4, 12, 6, Modulo, uint8_t>(reportTestCases)),
		test_tag, "fixpnt<8,4> -> fixpnt<12,6> (expand both)");
	nrOfFailedTestCases += ReportTestResult(
		(VerifyFixpntToFixpntConversion<8, 4, 12, 2, Modulo, uint8_t>(reportTestCases)),
		test_tag, "fixpnt<8,4> -> fixpnt<12,2> (expand, fewer rbits)");
	nrOfFailedTestCases += ReportTestResult(
		(VerifyFixpntToFixpntConversion<8, 2, 12, 8, Modulo, uint8_t>(reportTestCases)),
		test_tag, "fixpnt<8,2> -> fixpnt<12,8> (expand, more rbits)");
	nrOfFailedTestCases += ReportTestResult(
		(VerifyFixpntToFixpntConversion<8, 1, 4, 3, Modulo, uint8_t>(reportTestCases)),
		test_tag, "fixpnt<8,1> -> fixpnt<4,3> (shrink, more rbits)");
	nrOfFailedTestCases += ReportTestResult(
		(VerifyFixpntToFixpntConversion<8, 6, 4, 1, Modulo, uint8_t>(reportTestCases)),
		test_tag, "fixpnt<8,6> -> fixpnt<4,1> (shrink, fewer rbits)");

	// Edge cases: all fraction bits or all integer bits
	nrOfFailedTestCases += ReportTestResult(
		(VerifyFixpntToFixpntConversion<4, 4, 8, 0, Modulo, uint8_t>(reportTestCases)),
		test_tag, "fixpnt<4,4> -> fixpnt<8,0> (all frac -> all int)");
	nrOfFailedTestCases += ReportTestResult(
		(VerifyFixpntToFixpntConversion<4, 0, 8, 4, Modulo, uint8_t>(reportTestCases)),
		test_tag, "fixpnt<4,0> -> fixpnt<8,4> (all int -> mixed)");

#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::fixpnt_arithmetic_exception& err) {
	std::cerr << "Uncaught fixpnt arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::fixpnt_internal_exception& err) {
	std::cerr << "Uncaught fixpnt internal exception: " << err.what() << std::endl;
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
