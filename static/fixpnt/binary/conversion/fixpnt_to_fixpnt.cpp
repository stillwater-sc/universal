// fixpnt_to_fixpnt.cpp: test suite for fixpnt cross-type conversions
// with different radix point positions (issue #357)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/long_double.hpp>
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

#endif

#if REGRESSION_LEVEL_2

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
