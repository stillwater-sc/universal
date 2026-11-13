// edges.cpp: blockfraction shifts, comparisons, setbits and to_double at the edges
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The QA sweep of #2029 (tracking issue #2052) found that:
//  - operator<<= did not null the bits shifted past nbits, so iszero() and == (which read
//    the raw blocks) saw them;
//  - operator< subtracted and tested the top bit, wrong whenever the operands were more than
//    half the range apart, although a blockfraction is an unsigned entity (div uses it);
//  - setbits() put the value in the MOST significant block for uint64_t limbs, and in the
//    least significant block for every other limb width;
//  - to_double() held the integer weight in an unsigned, truncating weights past 2^31.
// Every case below failed before the fix.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <cstdint>
#include <universal/internal/blockfraction/blockfraction.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

using namespace sw::universal;

template<unsigned nbits, typename bt>
uint64_t raw_bits(const blockfraction<nbits, bt>& a) {
	uint64_t v = 0;
	for (unsigned i = 0; i < nbits && i < 64; ++i) if (a.test(i)) v |= (1ull << i);
	return v;
}

// shifts against the native shift of the same bits, masked to nbits
template<unsigned nbits, typename bt>
int VerifyShifts(bool reportTestCases) {
	static_assert(nbits < 64, "the uint64_t reference holds fewer than 64 bits");
	constexpr uint64_t mask = (1ull << nbits) - 1ull;
	const uint64_t patterns[] = { 1ull, 1ull << (nbits - 1), mask, mask / 3, 0x5A5A5A5A5A5A5A5Aull & mask };
	int nrOfFailedTests = 0;
	for (uint64_t p : patterns) {
		for (int s = 0; s <= static_cast<int>(nbits) + 1; ++s) {
			blockfraction<nbits, bt> l, r;
			l.setradix(0); r.setradix(0);
			l.setbits(p); r.setbits(p);
			l <<= s;
			r >>= s;
			const uint64_t lref = (s >= 64 ? 0ull : (p << s) & mask);
			const uint64_t rref = (s >= 64 ? 0ull : (p >> s));
			if (raw_bits(l) != lref || l.iszero() != (lref == 0) || raw_bits(r) != rref || r.iszero() != (rref == 0)) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: blockfraction<" << nbits << "> 0x" << std::hex << p << " shift " << std::dec << s << '\n';
			}
		}
	}
	return nrOfFailedTests;
}

// shifting every bit out keeps the radix point, like every other shift
template<unsigned nbits, typename bt>
int VerifyShiftOutKeepsRadix(bool reportTestCases) {
	int nrOfFailedTests = 0;
	for (int s : { static_cast<int>(nbits), static_cast<int>(nbits) + 5 }) {
		blockfraction<nbits, bt> l, r;
		l.setradix(3); r.setradix(3);
		l.setbits(0x5); r.setbits(0x5);
		l <<= s;
		r >>= s;
		l.setbits(8); r.setbits(8);   // 8 at radix 3 is 1.0
		if (l.radix() != 3 || r.radix() != 3 || l.to_double() != 1.0 || r.to_double() != 1.0) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: blockfraction<" << nbits << "> shift by " << s << " changed the radix to " << l.radix() << '/' << r.radix() << '\n';
		}
	}
	return nrOfFailedTests;
}

// every pair of an nbits blockfraction ordered as unsigned numbers
template<unsigned nbits, typename bt>
int VerifyUnsignedOrder(bool reportTestCases) {
	static_assert(nbits <= 10, "exhaustive pairs");
	int nrOfFailedTests = 0;
	for (uint64_t i = 0; i < (1ull << nbits); ++i) {
		for (uint64_t j = 0; j < (1ull << nbits); ++j) {
			blockfraction<nbits, bt> a, b;
			a.setradix(0); b.setradix(0);
			a.setbits(i); b.setbits(j);
			if ((a < b) != (i < j) || (a <= b) != (i <= j) || (a > b) != (i > j) || (a >= b) != (i >= j)) {
				++nrOfFailedTests;
				if (reportTestCases && nrOfFailedTests < 10) std::cerr << "FAIL: blockfraction<" << nbits << "> order of " << i << " and " << j << '\n';
			}
		}
	}
	return nrOfFailedTests;
}

// setbits() puts the value in the low bits, whatever the limb width
template<typename bt>
int VerifySetbits(bool reportTestCases) {
	blockfraction<128, bt> a;
	a.setradix(120);
	a.setbits(5);
	int nrOfFailedTests = 0;
	if (!a.test(0) || a.test(1) || !a.test(2) || a.msb() != 2 || a.to_double() != 5.0 / 1329227995784915872903807060280344576.0) {  // 5 * 2^-120
		++nrOfFailedTests;
		if (reportTestCases) std::cerr << "FAIL: blockfraction<128," << sizeof(bt) * 8 << "-bit limbs>::setbits(5) msb " << a.msb() << '\n';
	}
	return nrOfFailedTests;
}

// to_double() integer weights past 2^31
int VerifyWideToDouble(bool reportTestCases) {
	int nrOfFailedTests = 0;
	blockfraction<64, uint32_t> a;
	a.setradix(0);
	a.setbit(40);
	if (a.to_double() != 1099511627776.0) {   // 2^40
		++nrOfFailedTests;
		if (reportTestCases) std::cerr << "FAIL: blockfraction<64> 2^40 to_double = " << a.to_double() << '\n';
	}
	blockfraction<48, uint16_t> b;
	b.setradix(3);
	b.setbit(45);
	b.setbit(1);
	if (b.to_double() != 4398046511104.0 + 0.25) {   // 2^42 + 2^-2
		++nrOfFailedTests;
		if (reportTestCases) std::cerr << "FAIL: blockfraction<48> to_double = " << b.to_double() << '\n';
	}
	blockfraction<1100, uint8_t> c;   // more than 1024 bits above the radix
	c.setradix(0);
	c.setbit(0);
	if (c.to_double() != 1.0) {
		++nrOfFailedTests;
		if (reportTestCases) std::cerr << "FAIL: blockfraction<1100> 1 to_double = " << c.to_double() << '\n';
	}
	return nrOfFailedTests;
}

} // namespace

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

	std::string test_suite  = "blockfraction edges";
	std::string test_tag    = "edges";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	nrOfFailedTestCases += VerifyShifts<12, uint8_t>(true);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyShifts<12, uint8_t>(reportTestCases), "blockfraction<12,uint8_t>", "shifts");
	nrOfFailedTestCases += ReportTestResult(VerifyShifts<20, uint16_t>(reportTestCases), "blockfraction<20,uint16_t>", "shifts");
	nrOfFailedTestCases += ReportTestResult(VerifyShifts<40, uint32_t>(reportTestCases), "blockfraction<40,uint32_t>", "shifts");
	nrOfFailedTestCases += ReportTestResult(VerifyShifts<60, uint8_t>(reportTestCases), "blockfraction<60,uint8_t>", "shifts");
	nrOfFailedTestCases += ReportTestResult(VerifyUnsignedOrder<8, uint8_t>(reportTestCases), "blockfraction<8,uint8_t>", "unsigned order");
	nrOfFailedTestCases += ReportTestResult(VerifyUnsignedOrder<10, uint8_t>(reportTestCases), "blockfraction<10,uint8_t>", "unsigned order");
	nrOfFailedTestCases += ReportTestResult(VerifySetbits<uint8_t>(reportTestCases), "blockfraction<128,uint8_t>", "setbits");
	nrOfFailedTestCases += ReportTestResult(VerifySetbits<uint16_t>(reportTestCases), "blockfraction<128,uint16_t>", "setbits");
	nrOfFailedTestCases += ReportTestResult(VerifySetbits<uint32_t>(reportTestCases), "blockfraction<128,uint32_t>", "setbits");
	nrOfFailedTestCases += ReportTestResult(VerifySetbits<uint64_t>(reportTestCases), "blockfraction<128,uint64_t>", "setbits");
	nrOfFailedTestCases += ReportTestResult(VerifyWideToDouble(reportTestCases), "blockfraction", "wide to_double");
	nrOfFailedTestCases += ReportTestResult(VerifyShiftOutKeepsRadix<12, uint8_t>(reportTestCases), "blockfraction<12,uint8_t>", "shift out keeps radix");
	nrOfFailedTestCases += ReportTestResult(VerifyShiftOutKeepsRadix<40, uint32_t>(reportTestCases), "blockfraction<40,uint32_t>", "shift out keeps radix");
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
