// division_edges.cpp: blockbinary division, remainder and the string constructor at the edges
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The QA sweep of #2029 (tracking issue #2052) found that:
//  - /= and %= at nbits == limb width cast through the SIGNED native type whatever the
//    encoding: Unsigned 200 / 2 was 228 and 200 % 7 was 0, and INT32_MIN / -1 raised SIGFPE;
//  - x / 0 and x % 0 silently returned 0 with no way to raise, while urdiv threw a string;
//  - urdiv<8,4>(0, 3) returned uninitialised bits;
//  - blockbinary(const std::string&) ignored its argument.
// Every case below failed (or crashed) before the fix.
#define BLOCKBINARY_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/utility/directives.hpp>
#include <iostream>
#include <climits>
#include <cstdint>
#include <string>
#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

using namespace sw::universal;

// every pair of an nbits == limb width Unsigned blockbinary against native unsigned arithmetic
template<typename Limb>
int VerifyUnsignedFullLimbDivision(bool reportTestCases) {
	using U = blockbinary<8 * sizeof(Limb), Limb, BinaryNumberType::Unsigned>;
	const uint64_t samples[] = { 1, 2, 3, 7, 100, 127, 128, 129, 200, 255,
		uint64_t(Limb(~Limb(0))), uint64_t(Limb(~Limb(0)) / 2u), uint64_t(Limb(~Limb(0)) / 2u + 1u) };
	int nrOfFailedTests = 0;
	for (uint64_t x : samples) {
		for (uint64_t y : samples) {
			const Limb a = static_cast<Limb>(x), b = static_cast<Limb>(y);
			U q(0), r(0), d(0);
			q.setbits(a); r.setbits(a); d.setbits(b);
			q /= d;
			r %= d;
			if (q.to_ull() != uint64_t(Limb(a / b)) || r.to_ull() != uint64_t(Limb(a % b))) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: unsigned " << uint64_t(a) << " / " << uint64_t(b) << " = " << q.to_ull() << " r " << r.to_ull() << '\n';
			}
		}
	}
	return nrOfFailedTests;
}

// Unsigned at a width that is not a limb width goes through longdivision
int VerifyUnsignedLongDivision(bool reportTestCases) {
	using U = blockbinary<12, uint8_t, BinaryNumberType::Unsigned>;
	int nrOfFailedTests = 0;
	for (uint64_t x : { 0ull, 1ull, 5ull, 2047ull, 2048ull, 4000ull, 4095ull }) {
		for (uint64_t y : { 1ull, 3ull, 64ull, 2048ull, 4095ull }) {
			U q(0), r(0), d(0);
			q.setbits(x); r.setbits(x); d.setbits(y);
			q /= d;
			r %= d;
			if (q.to_ull() != x / y || r.to_ull() != x % y) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: unsigned<12> " << x << " / " << y << " = " << q.to_ull() << " r " << r.to_ull() << '\n';
			}
		}
	}
	return nrOfFailedTests;
}

// Signed at nbits == limb width: truncation toward zero, and maxneg / -1 wraps without trapping
template<typename Limb>
int VerifySignedFullLimbDivision(bool reportTestCases) {
	constexpr unsigned nbits = 8 * sizeof(Limb);
	using S = blockbinary<nbits, Limb, BinaryNumberType::Signed>;
	using Native = std::make_signed_t<Limb>;
	const Native maxneg = std::numeric_limits<Native>::min();
	const Native samples[] = { maxneg, Native(maxneg + 1), Native(-7), Native(-2), Native(-1), Native(1), Native(2), Native(7), std::numeric_limits<Native>::max() };
	int nrOfFailedTests = 0;
	for (Native x : samples) {
		for (Native y : samples) {
			S q(static_cast<long long>(x)), r(static_cast<long long>(x)), d(static_cast<long long>(y));
			q /= d;
			r %= d;
			long long qref, rref;
			if (x == maxneg && y == Native(-1)) {
				qref = static_cast<long long>(maxneg);   // the quotient 2^(nbits-1) wraps to maxneg
				rref = 0;
			}
			else {
				qref = static_cast<long long>(x) / static_cast<long long>(y);
				rref = static_cast<long long>(x) % static_cast<long long>(y);
			}
			if (q.to_sll() != qref || r.to_sll() != rref) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: signed<" << nbits << "> " << +x << " / " << +y << " = " << q.to_sll() << " r " << r.to_sll() << '\n';
			}
		}
	}
	return nrOfFailedTests;
}

// x / 0 and x % 0 raise blockbinary_divide_by_zero when BLOCKBINARY_THROW_ARITHMETIC_EXCEPTION is 1
int VerifyDivideByZero(bool reportTestCases) {
	int nrOfFailedTests = 0;
#if UNIVERSAL_HAS_EXCEPTIONS
	auto expectThrow = [&](auto op, const char* label) {
		bool caught = false;
		try { op(); }
		catch (const blockbinary_divide_by_zero&) { caught = true; }
		if (!caught) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: " << label << " did not raise blockbinary_divide_by_zero\n";
		}
	};
	expectThrow([] { blockbinary<8, uint8_t> a(5), z(0); a /= z; }, "blockbinary<8> / 0");
	expectThrow([] { blockbinary<8, uint8_t> a(5), z(0); a %= z; }, "blockbinary<8> % 0");
	expectThrow([] { blockbinary<20, uint8_t> a(5), z(0); a /= z; }, "blockbinary<20> / 0");
	expectThrow([] { blockbinary<8, uint8_t, BinaryNumberType::Unsigned> a(5), z(0); a %= z; }, "unsigned blockbinary<8> % 0");
	expectThrow([] { blockbinary<8, uint8_t> a(5), z(0); auto q = urdiv<8, 4>(a, z); (void)q; }, "urdiv by 0");
#else
	(void)reportTestCases;
#endif
	return nrOfFailedTests;
}

// urdiv of a zero dividend is exactly zero, with every bit initialised
int VerifyUrdivZeroDividend(bool reportTestCases) {
	int nrOfFailedTests = 0;
	for (long long d : { 1ll, 3ll, -3ll, 127ll, -128ll }) {
		auto q = urdiv<8, 4>(blockbinary<8, uint8_t>(0), blockbinary<8, uint8_t>(d));
		if (!q.iszero()) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: urdiv<8,4>(0, " << d << ") is not zero\n";
		}
	}
	return nrOfFailedTests;
}

int VerifyStringConstructor(bool reportTestCases) {
	int nrOfFailedTests = 0;
	auto check = [&](const std::string& s, long long expected) {
		blockbinary<12, uint8_t> a(s);
		if (a.to_sll() != expected) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: blockbinary<12>(\"" << s << "\") = " << a.to_sll() << " expected " << expected << '\n';
		}
	};
	check("0b0000'0001'0101", 21);
	check("0b1111'1111'1111", -1);       // the sign bit of a Signed blockbinary<12>
	check("0b1000'0000'0000", -2048);
	check("10101", 21);                  // no prefix
	check("0b1'0000'0000'0011", 3);      // bits above nbits are dropped
	check("0b0102", 0);                  // not a bit string
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

	std::string test_suite  = "blockbinary division edges";
	std::string test_tag    = "division";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	nrOfFailedTestCases += VerifyUnsignedFullLimbDivision<uint8_t>(true);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyUnsignedFullLimbDivision<uint8_t>(reportTestCases), "blockbinary<8,uint8_t,Unsigned>", "full-limb / %");
	nrOfFailedTestCases += ReportTestResult(VerifyUnsignedFullLimbDivision<uint16_t>(reportTestCases), "blockbinary<16,uint16_t,Unsigned>", "full-limb / %");
	nrOfFailedTestCases += ReportTestResult(VerifyUnsignedFullLimbDivision<uint32_t>(reportTestCases), "blockbinary<32,uint32_t,Unsigned>", "full-limb / %");
	nrOfFailedTestCases += ReportTestResult(VerifyUnsignedFullLimbDivision<uint64_t>(reportTestCases), "blockbinary<64,uint64_t,Unsigned>", "full-limb / %");
	nrOfFailedTestCases += ReportTestResult(VerifyUnsignedLongDivision(reportTestCases), "blockbinary<12,uint8_t,Unsigned>", "long division");
	nrOfFailedTestCases += ReportTestResult(VerifySignedFullLimbDivision<uint8_t>(reportTestCases), "blockbinary<8,uint8_t>", "full-limb / %");
	nrOfFailedTestCases += ReportTestResult(VerifySignedFullLimbDivision<uint16_t>(reportTestCases), "blockbinary<16,uint16_t>", "full-limb / %");
	nrOfFailedTestCases += ReportTestResult(VerifySignedFullLimbDivision<uint32_t>(reportTestCases), "blockbinary<32,uint32_t>", "full-limb / %");
	nrOfFailedTestCases += ReportTestResult(VerifySignedFullLimbDivision<uint64_t>(reportTestCases), "blockbinary<64,uint64_t>", "full-limb / %");
	nrOfFailedTestCases += ReportTestResult(VerifyDivideByZero(reportTestCases), "blockbinary", "divide by zero");
	nrOfFailedTestCases += ReportTestResult(VerifyUrdivZeroDividend(reportTestCases), "urdiv<8,4>", "zero dividend");
	nrOfFailedTestCases += ReportTestResult(VerifyStringConstructor(reportTestCases), "blockbinary<12>", "string constructor");
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
