// long_division.cpp: integer and blockbinary division, a 32-bit word at a time, against native and identity oracles
//
// Guards issue #1744: multi-block integer division (idiv, behind / and %) and blockbinary's
// longdivision were shift-and-subtract, one quotient bit per full-width compare, subtract and
// shift. Both now divide magnitudes with limb::divide (Knuth's Algorithm D on 32-bit limbs), which
// reads and writes the number through 32-bit limbs whatever the block type.
//
// Two oracles, neither of which divides with the code under test:
//  - widths a native integer holds: the native quotient and remainder, for every block type;
//  - any width: the defining identity a == q * b + r with |r| < |b|, r zero or of a's sign, and
//    q truncated toward zero, plus the edge operands maxneg, -1, 1, equal operands and
//    single-limb divisors.
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
#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

using namespace sw::universal;

// n random quotients and remainders against the native ones; every third divisor is short
template<typename Integer, typename Native>
int VerifyAgainstNative(const std::string& tag, int n, bool reportTestCases) {
	std::mt19937_64 rng(1744);
	int nrOfFailedTests = 0;
	for (int i = 0; i < n; ++i) {
		Native a = static_cast<Native>(rng());
		Native b = static_cast<Native>(rng());
		if (i % 3 == 1) b = static_cast<Native>(b >> (rng() % (8 * sizeof(Native))));
		if (b == 0) continue;
		if constexpr (std::numeric_limits<Native>::is_signed) {
			if (a == std::numeric_limits<Native>::min() && b == Native(-1)) continue;   // overflows natively
		}
		if constexpr (!std::numeric_limits<Native>::is_signed) {
			if (a < b) continue;   // a whole number refuses a zero quotient
		}
		const Integer ia(a), ib(b);
		const Integer q = ia / ib;
		const Integer r = ia % ib;
		if (static_cast<Native>(q) != static_cast<Native>(a / b) || static_cast<Native>(r) != static_cast<Native>(a % b)) {
			++nrOfFailedTests;
			if (reportTestCases && nrOfFailedTests < 10) std::cerr << "FAIL: " << tag << ' ' << a << " / " << b << '\n';
		}
	}
	return nrOfFailedTests;
}

// an operand of kind: 0 maxneg, 1 -1, 2 1, 3 a single limb, otherwise all bits random
template<typename X>
void Operand(std::mt19937_64& rng, X& x, int kind) {
	x.clear();
	switch (kind) {
	case 0: x.setbit(X::nbits - 1); break;
	case 1: for (unsigned k = 0; k < X::nbits; ++k) x.setbit(k); break;
	case 2: x.setbit(0); break;
	case 3: for (unsigned k = 0; k < 24 && k < X::nbits; ++k) x.setbit(k, (rng() & 1u) != 0); x.setbit(0); break;
	default: for (unsigned k = 0; k < X::nbits; ++k) x.setbit(k, (rng() & 1u) != 0);
	}
}

template<typename X>
bool IsNegative(const X& x) { return x.at(X::nbits - 1); }

// q and r of a / b must satisfy a == q * b + r, |r| < |b|, and r zero or of a's sign. The
// products and magnitudes are formed one bit wider, so maxneg's magnitude does not wrap. The
// quotient maxneg / -1 does not fit and wraps to maxneg, as it always has: that pair is skipped.
template<typename X, typename Wide, typename Divide>
int VerifyIdentity(const std::string& tag, int n, Divide&& divide, bool reportTestCases) {
	std::mt19937_64 rng(17440 + X::nbits);
	int nrOfFailedTests = 0;
	for (int i = 0; i < n; ++i) {
		X a, b;
		Operand(rng, a, (i % 7 == 0) ? static_cast<int>(rng() % 4) : 9);
		Operand(rng, b, (i % 5 == 0) ? static_cast<int>(rng() % 4) : 9);
		if (i % 11 == 0) b = a;
		if (i % 13 == 0) { b.clear(); for (unsigned k = 0; k < X::nbits / 3; ++k) b.setbit(k, (rng() & 1u) != 0); }
		if (b.iszero()) b.setbit(0);
		X maxneg; maxneg.clear(); maxneg.setbit(X::nbits - 1);
		X minusone; minusone.clear(); for (unsigned k = 0; k < X::nbits; ++k) minusone.setbit(k);
		if (a == maxneg && b == minusone) continue;

		X q, r;
		divide(a, b, q, r);
		const Wide wa(a), wb(b), wq(q), wr(r);
		const Wide back = wq * wb + wr;
		Wide absr = IsNegative(wr) ? -wr : wr;
		Wide absb = IsNegative(wb) ? -wb : wb;
		const bool ok = (back == wa) && (absr < absb) && (r.iszero() || IsNegative(r) == IsNegative(a));
		if (!ok) {
			++nrOfFailedTests;
			if (reportTestCases && nrOfFailedTests < 10) std::cerr << "FAIL: " << tag << " identity, case " << i << '\n';
		}
	}
	return nrOfFailedTests;
}

template<unsigned nbits, typename BlockType>
int VerifyIntegerIdentity(int n, bool reportTestCases) {
	using Integer = integer<nbits, BlockType, IntegerNumberType::IntegerNumber>;
	using Wide = integer<2 * nbits + 2, BlockType, IntegerNumberType::IntegerNumber>;
	return VerifyIdentity<Integer, Wide>("integer<" + std::to_string(nbits) + ">", n,
		[](const Integer& a, const Integer& b, Integer& q, Integer& r) { q = a / b; r = a % b; }, reportTestCases);
}

template<unsigned nbits, typename BlockType>
int VerifyBlockBinaryIdentity(int n, bool reportTestCases) {
	using BlockBinary = blockbinary<nbits, BlockType, BinaryNumberType::Signed>;
	using Wide = blockbinary<2 * nbits + 2, BlockType, BinaryNumberType::Signed>;
	return VerifyIdentity<BlockBinary, Wide>("blockbinary<" + std::to_string(nbits) + ">", n,
		[](const BlockBinary& a, const BlockBinary& b, BlockBinary& q, BlockBinary& r) {
			const auto result = longdivision(a, b);
			q = result.quo;
			r = result.rem;
		}, reportTestCases);
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

	std::string test_suite  = "integer and blockbinary long division";
	std::string test_tag    = "long division";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyIntegerIdentity<256, std::uint32_t>(1000, true), "integer<256,uint32_t>", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyAgainstNative<integer<64, std::uint8_t >, std::int64_t>("integer<64,uint8_t>",  20000, reportTestCases), "integer<64,uint8_t>",  "native int64_t");
	nrOfFailedTestCases += ReportTestResult(VerifyAgainstNative<integer<64, std::uint16_t>, std::int64_t>("integer<64,uint16_t>", 20000, reportTestCases), "integer<64,uint16_t>", "native int64_t");
	nrOfFailedTestCases += ReportTestResult(VerifyAgainstNative<integer<64, std::uint32_t>, std::int64_t>("integer<64,uint32_t>", 20000, reportTestCases), "integer<64,uint32_t>", "native int64_t");
	nrOfFailedTestCases += ReportTestResult(VerifyAgainstNative<integer<32, std::uint8_t >, std::int32_t>("integer<32,uint8_t>",  20000, reportTestCases), "integer<32,uint8_t>",  "native int32_t");
	nrOfFailedTestCases += ReportTestResult(VerifyAgainstNative<integer<64, std::uint32_t, IntegerNumberType::WholeNumber>, std::uint64_t>("whole<64,uint32_t>", 20000, reportTestCases), "integer<64,uint32_t,Whole>", "native uint64_t");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerIdentity< 77, std::uint16_t>(5000, reportTestCases), "integer<77,uint16_t>",  "a == q*b + r");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerIdentity<128, std::uint64_t>(5000, reportTestCases), "integer<128,uint64_t>", "a == q*b + r");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerIdentity<256, std::uint8_t >(2000, reportTestCases), "integer<256,uint8_t>",  "a == q*b + r");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockBinaryIdentity< 96, std::uint8_t >(5000, reportTestCases), "blockbinary<96,uint8_t>",   "a == q*b + r");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockBinaryIdentity<200, std::uint64_t>(2000, reportTestCases), "blockbinary<200,uint64_t>", "a == q*b + r");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerIdentity<1000, std::uint32_t>(2000, reportTestCases), "integer<1000,uint32_t>", "a == q*b + r");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockBinaryIdentity<300, std::uint32_t>(5000, reportTestCases), "blockbinary<300,uint32_t>", "a == q*b + r");
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
