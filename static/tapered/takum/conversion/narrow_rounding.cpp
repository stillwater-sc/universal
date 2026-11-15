// narrow_rounding.cpp: takum conversion rounds the bit string, as the takum specification does
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Hunhold's rounding function (arXiv:2404.18603, Algorithm 1) and his reference implementations
// (the VHDL codec of arXiv:2408.10594 and the C library libtakum) round the lossless ENCODING: the
// bit string is cut to n bits and rounded to nearest on what was cut, ties to the even string,
// saturating to minpos / maxpos instead of reaching 0 or NaR.  When a layout has no trailing
// field and stores only the top bits of the characteristic -- takum<8,3>, takum<10,3> -- the cut
// bits are characteristic bits, so rounding is in exponent space, not to the nearest value
// (#2079): takum<8,3>(3.709e-68) is pattern 2, although pattern 1 is nearer in value.
//
// The reference below is that bit-string rounding, applied to the encodings of a wider takum with
// the same rbits.  Takum encodings are prefixes of one another, so every value of the wide takum
// is exactly a double here and its bit string is exactly the wide encoding followed by zeros:
// rounding it to the narrow width is libtakum's width reduction, an integer operation on the
// pattern that shares nothing with the codec under test.  The wide grid holds every narrow
// encoding, every midpoint between neighbours (the ties), and the values in between.
#include <universal/utility/directives.hpp>
#define TAKUM_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/takum/takum.hpp>
#include <universal/verification/test_suite.hpp>

#include <cstdint>
#include <iostream>
#include <string>

namespace {

// libtakum's TAKUM_INTERNAL_CONVERT_REDUCE: round the two's complement string of `wide` bits to
// `narrow` bits, to nearest with ties to even, then turn a result of 0 or NaR that the input was
// not into the nearest finite non-zero value of the input's sign
std::uint64_t reduce_bit_string(std::uint64_t in, unsigned wide, unsigned narrow) {
	const unsigned cut = wide - narrow;
	const std::uint64_t narrowMask = (std::uint64_t(1) << narrow) - 1u;
	const std::uint64_t wideMask   = (std::uint64_t(1) << wide) - 1u;
	const std::uint64_t roundBit = std::uint64_t(1) << (cut - 1u);
	const std::uint64_t below    = (std::uint64_t(1) << cut) - 1u;
	std::uint64_t out = in >> cut;
	const bool roundUp = (in & roundBit) && (((in & below) != roundBit) || (out & 1u));
	if (roundUp) ++out;
	out &= narrowMask;
	const std::uint64_t narrowNaR = std::uint64_t(1) << (narrow - 1u);
	const std::uint64_t wideNaR   = std::uint64_t(1) << (wide - 1u);
	const bool negative = (in & wideNaR) != 0;
	const bool inZero = (in & wideMask) == 0, inNaR = (in & wideMask) == wideNaR;
	if (out == 0 && !inZero)              out = negative ? narrowMask : 1u;                      // -minpos : +minpos
	else if (out == narrowNaR && !inNaR)  out = negative ? (narrowNaR + 1u) : (narrowNaR - 1u);  // maxneg : maxpos
	return out;
}

template<unsigned narrow, unsigned wide, unsigned rbits>
int VerifyBitStringRounding(bool reportTestCases) {
	using namespace sw::universal;
	using Narrow = takum<narrow, rbits, std::uint16_t>;
	using Wide   = takum<wide, rbits, std::uint32_t>;
	static_assert(wide <= 24, "an exhaustive sweep of the wide encodings");
	int nrOfFailedTests = 0;
	const std::uint64_t wideNaR = std::uint64_t(1) << (wide - 1u);
	for (std::uint64_t w = 1; w < (std::uint64_t(1) << wide); ++w) {
		if (w == wideNaR) continue;
		Wide x;
		x.setbits(w);
		const double v = double(x);
		if (Wide(v).raw_bits() != w) {   // the wide value must be an exact double, or the reference is moot
			if (reportTestCases) std::cerr << "FAIL: wide encoding 0x" << std::hex << w << std::dec << " is not exact in a double\n";
			++nrOfFailedTests;
			continue;
		}
		const Narrow n(v);
		const std::uint64_t expected = reduce_bit_string(w, wide, narrow);
		if (n.raw_bits() != expected) {
			if (nrOfFailedTests < 8 && reportTestCases) {
				std::cerr << "FAIL: takum<" << narrow << ',' << rbits << ">(" << v << ") = 0x" << std::hex << n.raw_bits()
				          << ", the bit string 0x" << w << " rounds to 0x" << expected << std::dec << '\n';
			}
			++nrOfFailedTests;
		}
	}
	return nrOfFailedTests;
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
#define REGRESSION_LEVEL_2 0
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "takum conversion rounds the bit string";
	std::string test_tag    = "narrow rounding";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	std::cout << "takum<8,3>(3.709e-68) = 0x" << std::hex << takum<8, 3>(3.709e-68).raw_bits() << std::dec << '\n';
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors

#else

#if REGRESSION_LEVEL_1
	{
		// the sweep's example: a layout with no trailing field rounds the characteristic.  Pattern 1 is
		// the nearer value, pattern 2 the nearer bit string.
		const auto t = takum<8, 3>(3.709e-68);
		if (t.raw_bits() != 2u) {
			std::cerr << "FAIL: takum<8,3>(3.709e-68) = 0x" << std::hex << t.raw_bits() << std::dec << ", expected pattern 2\n";
			++nrOfFailedTestCases;
		}
	}
	// no trailing field, dropped characteristic bits (r > maxCharBits)
	nrOfFailedTestCases += ReportTestResult(VerifyBitStringRounding< 8, 16, 3>(reportTestCases), "takum<8,3>",  "from takum<16,3>");
	nrOfFailedTestCases += ReportTestResult(VerifyBitStringRounding<10, 18, 3>(reportTestCases), "takum<10,3>", "from takum<18,3>");
	// trailing fields: the bit string and the value agree there
	nrOfFailedTestCases += ReportTestResult(VerifyBitStringRounding<12, 20, 3>(reportTestCases), "takum<12,3>", "from takum<20,3>");
	nrOfFailedTestCases += ReportTestResult(VerifyBitStringRounding< 8, 16, 2>(reportTestCases), "takum<8,2>",  "from takum<16,2>");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyBitStringRounding< 9, 20, 3>(reportTestCases), "takum<9,3>",  "from takum<20,3>");
	nrOfFailedTestCases += ReportTestResult(VerifyBitStringRounding<14, 22, 1>(reportTestCases), "takum<14,1>", "from takum<22,1>");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyBitStringRounding<16, 24, 3>(reportTestCases), "takum<16,3>", "from takum<24,3>");
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
