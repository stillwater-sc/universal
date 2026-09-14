// show_limbs.cpp: regression tests for einteger's limb introspection
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under
// an MIT Open Source license.
//
// Issue #1456: einteger::showLimbs() passed to_binary(number, bNibbleMarker, nbits) its
// arguments out of order, so the limb width landed in bNibbleMarker and true landed in
// nbits, and every limb printed as its least significant bit ("0b1 0b1 0b1 ...").

#include <universal/utility/directives.hpp>
#include <cstdint>
#include <iostream>
#include <random>
#include <string>
#include <universal/number/einteger/einteger.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

// the binary text of one limb, built independently of to_binary: "0b" and every bit,
// with a nibble marker between groups of four
template<typename BlockType>
std::string LimbText(std::uint64_t limb) {
	constexpr int bits = static_cast<int>(sizeof(BlockType) * 8);
	std::string s = "0b";
	for (int i = bits - 1; i >= 0; --i) {
		s += ((limb >> i) & 1u) ? '1' : '0';
		if (i > 0 && i % 4 == 0) s += '\'';
	}
	return s;
}

// showLimbs() and showLimbValues() of einteger<BlockType>(value) against the limbs of
// |value|, most significant first
template<typename BlockType>
int VerifyShowLimbs(std::int64_t value, bool reportTestCases) {
	constexpr unsigned bits = sizeof(BlockType) * 8;
	std::uint64_t magnitude = value < 0 ? (0u - static_cast<std::uint64_t>(value)) : static_cast<std::uint64_t>(value);
	std::string expectedBinary, expectedValues;
	do {
		const std::uint64_t limb = (bits == 64) ? magnitude : (magnitude & ((std::uint64_t(1) << bits) - 1u));
		std::string v = std::to_string(limb);
		if (v.size() < 5) v.insert(0, 5 - v.size(), ' ');  // showLimbValues pads to 5 columns
		expectedBinary = LimbText<BlockType>(limb) + (expectedBinary.empty() ? "" : " ") + expectedBinary;
		expectedValues = v + (expectedValues.empty() ? "" : ", ") + expectedValues;
		magnitude = (bits == 64) ? 0 : (magnitude >> bits);
	} while (magnitude != 0);

	einteger<BlockType> a(value);
	int nrOfFailedTestCases = 0;
	if (a.showLimbs() != expectedBinary) {
		++nrOfFailedTestCases;
		if (reportTestCases)
			std::cerr << "FAIL: showLimbs(" << value << ") = " << a.showLimbs() << ", expected " << expectedBinary
			          << '\n';
	}
	if (a.showLimbValues() != expectedValues) {
		++nrOfFailedTestCases;
		if (reportTestCases)
			std::cerr << "FAIL: showLimbValues(" << value << ") = " << a.showLimbValues() << ", expected "
			          << expectedValues << '\n';
	}
	return nrOfFailedTestCases;
}

// random nonzero values of both signs and every magnitude up to 62 bits
template<typename BlockType>
int VerifyRandomShowLimbs(unsigned samples, bool reportTestCases) {
	std::mt19937_64 rng(1456u + sizeof(BlockType));
	int nrOfFailedTestCases = 0;
	for (unsigned k = 0; k < samples; ++k) {
		const unsigned width = 1u + static_cast<unsigned>(rng() % 62u);
		std::int64_t v = static_cast<std::int64_t>(rng() >> (64u - width)) | 1;  // nonzero
		if (rng() % 2) v = -v;
		const int fails = VerifyShowLimbs<BlockType>(v, reportTestCases);
		// without reportTestCases, show the first failing value only
		if (fails && nrOfFailedTestCases == 0 && !reportTestCases) VerifyShowLimbs<BlockType>(v, true);
		nrOfFailedTestCases += fails;
	}
	return nrOfFailedTestCases;
}

}} // namespace sw::universal

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

	std::string test_suite  = "einteger limb introspection";
	std::string test_tag    = "showLimbs";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	einteger<std::uint8_t> a(-1234567890123LL);
	std::cout << a.showLimbs() << '\n' << a.showLimbValues() << '\n';
	nrOfFailedTestCases +=
	    ReportTestResult(VerifyShowLimbs<std::uint8_t>(-1234567890123LL, true), "einteger<uint8_t>", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	{
		// the case reported in #1456: limbs 1, 31, 113, 251, 4, 203
		einteger<std::uint8_t> a(-1234567890123LL);
		const std::string expected = "0b0000'0001 0b0001'1111 0b0111'0001 0b1111'1011 0b0000'0100 0b1100'1011";
		int fails = (a.showLimbs() != expected) ? 1 : 0;
		if (fails) std::cerr << "FAIL: showLimbs = " << a.showLimbs() << ", expected " << expected << '\n';
		nrOfFailedTestCases += ReportTestResult(fails, "einteger<uint8_t>", "reported case");
	}
	nrOfFailedTestCases +=
	    ReportTestResult(VerifyRandomShowLimbs<std::uint8_t>(1000, reportTestCases), "einteger<uint8_t>", test_tag);
	nrOfFailedTestCases +=
	    ReportTestResult(VerifyRandomShowLimbs<std::uint16_t>(1000, reportTestCases), "einteger<uint16_t>", test_tag);
	nrOfFailedTestCases +=
	    ReportTestResult(VerifyRandomShowLimbs<std::uint32_t>(1000, reportTestCases), "einteger<uint32_t>", test_tag);
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
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
