// reduce.cpp: integer<>::reduce(a, b, r) against native and identity oracles
//
// Guards issue #1750: reduce() carried a block-digit Knuth division of its own that was wrong for
// every divisor wider than one block, and could trap with SIGFPE: it normalized by the leading
// zeros of the signed divisor rather than its magnitude, shifted a block by its full width when
// no normalization was needed, and assumed 32-bit blocks. Its only caller in the tree, einteger's
// decimal printing, divides by a single-block power of ten, which is why nothing showed. Several
// blocks now go through idiv, the long division behind / and %.
//
// The oracles do not divide with the code under test:
//  - widths a native integer holds: the native quotient and remainder, for every block type,
//    which reaches both the native single-block path and the multi-block path;
//  - any width: a == q * b + r with |r| < |b|, r zero or of a's sign, for one-block and
//    multi-block divisors of both signs;
//  - r and *this aliasing the operands.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <limits>
#include <random>
#include <string>
#include <universal/number/integer/integer.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

using namespace sw::universal;

// q and r as reduce() returns them; integer is trivially constructible, so start them at zero
template<typename Integer>
void Reduce(const Integer& a, const Integer& b, Integer& q, Integer& r) {
	q.clear();
	r.clear();
	q.reduce(a, b, r);
}

// n random reductions against the native quotient and remainder; every third divisor is short
template<typename Integer, typename Native>
int VerifyAgainstNative(const std::string& tag, int n, bool reportTestCases) {
	std::mt19937_64 rng(1750);
	int nrOfFailedTests = 0;
	for (int i = 0; i < n; ++i) {
		Native a = static_cast<Native>(rng());
		Native b = static_cast<Native>(rng());
		if (i % 3 == 1) b = static_cast<Native>(b >> (rng() % (8 * sizeof(Native))));
		if (b == 0 || a == 0) continue;
		if (a == std::numeric_limits<Native>::min() && b == Native(-1)) continue;   // overflows natively
		Integer q, r;
		Reduce(Integer(a), Integer(b), q, r);
		if (static_cast<Native>(q) != static_cast<Native>(a / b) || static_cast<Native>(r) != static_cast<Native>(a % b)) {
			++nrOfFailedTests;
			if (reportTestCases && nrOfFailedTests < 10) std::cerr << "FAIL: " << tag << ' ' << int64_t(a) << " reduce " << int64_t(b) << '\n';
		}
	}
	return nrOfFailedTests;
}

// a == q * b + r, |r| < |b|, r zero or of a's sign, formed one bit wider than twice the width;
// divisors of one block and of several, of both signs
template<unsigned nbits, typename BlockType>
int VerifyIdentity(int n, bool reportTestCases) {
	using Integer = integer<nbits, BlockType, IntegerNumberType::IntegerNumber>;
	using Wide = integer<2 * nbits + 2, BlockType, IntegerNumberType::IntegerNumber>;
	constexpr unsigned bitsInBlock = sizeof(BlockType) * 8u;
	std::mt19937_64 rng(17500 + nbits + bitsInBlock);
	int nrOfFailedTests = 0;
	for (int i = 0; i < n; ++i) {
		Integer a, b;
		a.clear();
		b.clear();
		for (unsigned k = 0; k + 1 < nbits; ++k) a.setbit(k, (rng() & 1u) != 0);
		// alternate one-block and multi-block divisors
		const unsigned bbits = (i % 2 == 0) ? (bitsInBlock < nbits ? bitsInBlock - 1u : nbits / 2u) : (nbits / 2u + 1u);
		for (unsigned k = 0; k < bbits; ++k) b.setbit(k, (rng() & 1u) != 0);
		b.setbit(bbits - 1u);
		if ((i / 2) % 2 == 1) b = -b;
		if (i % 5 == 0) a = -a;
		Integer q, r;
		Reduce(a, b, q, r);
		const Wide wa(a), wb(b), wq(q), wr(r);
		const Wide absr = wr.sign() ? -wr : wr;
		const Wide absb = wb.sign() ? -wb : wb;
		const bool ok = (wq * wb + wr == wa) && (absr < absb) && (r.iszero() || r.sign() == a.sign());
		if (!ok) {
			++nrOfFailedTests;
			if (reportTestCases && nrOfFailedTests < 10) std::cerr << "FAIL: integer<" << nbits << "> reduce identity, case " << i << '\n';
		}
	}
	return nrOfFailedTests;
}

// the quotient or the remainder may be stored over an operand
template<unsigned nbits, typename BlockType>
int VerifyAliasing(bool reportTestCases) {
	using Integer = integer<nbits, BlockType, IntegerNumberType::IntegerNumber>;
	std::mt19937_64 rng(175000 + nbits);
	int nrOfFailedTests = 0;
	for (int i = 0; i < 200; ++i) {
		Integer a, b;
		a.clear();
		b.clear();
		for (unsigned k = 0; k + 1 < nbits; ++k) a.setbit(k, (rng() & 1u) != 0);
		for (unsigned k = 0; k < nbits / 2u; ++k) b.setbit(k, (rng() & 1u) != 0);
		b.setbit(0);
		Integer q, r;
		Reduce(a, b, q, r);
		Integer x(a), y(b);
		x.reduce(x, b, y);   // quotient over the dividend, remainder over the divisor
		Integer s(a), t;
		t.clear();
		t.reduce(s, b, s);   // remainder over the dividend
		if (x != q || y != r || t != q || s != r) {
			++nrOfFailedTests;
			if (reportTestCases && nrOfFailedTests < 5) std::cerr << "FAIL: integer<" << nbits << "> aliased reduce\n";
		}
	}
	return nrOfFailedTests;
}

}  // namespace

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

	std::string test_suite  = "integer reduce";
	std::string test_tag    = "reduce";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyIdentity<128, std::uint32_t>(1000, true), "integer<128,uint32_t>", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

#if REGRESSION_LEVEL_1
	// single block: reduce's native path
	nrOfFailedTestCases += ReportTestResult(VerifyAgainstNative<integer< 8, std::uint8_t >, std::int8_t >("integer<8,uint8_t>",   20000, reportTestCases), "integer<8,uint8_t>",   "native int8_t");
	nrOfFailedTestCases += ReportTestResult(VerifyAgainstNative<integer<16, std::uint16_t>, std::int16_t>("integer<16,uint16_t>", 20000, reportTestCases), "integer<16,uint16_t>", "native int16_t");
	// several blocks
	nrOfFailedTestCases += ReportTestResult(VerifyAgainstNative<integer<64, std::uint8_t >, std::int64_t>("integer<64,uint8_t>",  20000, reportTestCases), "integer<64,uint8_t>",  "native int64_t");
	nrOfFailedTestCases += ReportTestResult(VerifyAgainstNative<integer<64, std::uint16_t>, std::int64_t>("integer<64,uint16_t>", 20000, reportTestCases), "integer<64,uint16_t>", "native int64_t");
	nrOfFailedTestCases += ReportTestResult(VerifyAgainstNative<integer<64, std::uint32_t>, std::int64_t>("integer<64,uint32_t>", 20000, reportTestCases), "integer<64,uint32_t>", "native int64_t");
	nrOfFailedTestCases += ReportTestResult(VerifyAgainstNative<integer<32, std::uint8_t >, std::int32_t>("integer<32,uint8_t>",  20000, reportTestCases), "integer<32,uint8_t>",  "native int32_t");
	nrOfFailedTestCases += ReportTestResult(VerifyIdentity<128, std::uint8_t >(4000, reportTestCases), "integer<128,uint8_t>",  "a == q*b + r");
	nrOfFailedTestCases += ReportTestResult(VerifyIdentity<128, std::uint16_t>(4000, reportTestCases), "integer<128,uint16_t>", "a == q*b + r");
	nrOfFailedTestCases += ReportTestResult(VerifyIdentity<128, std::uint32_t>(4000, reportTestCases), "integer<128,uint32_t>", "a == q*b + r");
	nrOfFailedTestCases += ReportTestResult(VerifyIdentity<192, std::uint64_t>(4000, reportTestCases), "integer<192,uint64_t>", "a == q*b + r");
	nrOfFailedTestCases += ReportTestResult(VerifyAliasing<128, std::uint32_t>(reportTestCases), "integer<128,uint32_t>", "aliased operands");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyIdentity<1000, std::uint32_t>(2000, reportTestCases), "integer<1000,uint32_t>", "a == q*b + r");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyAgainstNative<integer<64, std::uint32_t>, std::int64_t>("integer<64,uint32_t>", 1000000, reportTestCases), "integer<64,uint32_t>", "native int64_t");
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
	std::cerr << "caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
