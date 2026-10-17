// arithmetic.cpp: lindex + - * / against the mpmath oracle (#1695)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Every expected result is the correctly rounded encoding from
// tools/generators/lindex_arithmetic_gen.py, which evaluates each operation exactly in the
// psi domain at 600 bits.  The test compares encodings for equality: no round trips, and
// nothing through double, which cannot carry a level-5 value or measure an error in psi.
//
//   lindex<8,3>   every one of the 65536 signed pairs, per operation.  The oracle gives
//                 the result for each pair of magnitudes; the signed result follows from
//                 the sign rules, which this test applies on its own.
//   sampled       lindex<12,3> and <16,3> (double working precision), <32,3> (double with
//                 the dd_cascade retry near midpoints), <64,3> (dd_cascade) and <16,2>,
//                 where lbits = 2 is not closed and products saturate
#include <universal/utility/directives.hpp>
#include <cstdint>
#include <random>
#include <string>
#include <universal/number/lindex/lindex.hpp>
#include <universal/verification/lindex_arithmetic_vectors.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

using namespace sw::universal;
namespace lv = sw::universal::lindex_verification;

int hexdigit(char c) { return (c <= '9') ? (c - '0') : (c - 'a' + 10); }
std::uint64_t table8(const char* const (&table)[128], std::uint64_t a, std::uint64_t b) {
	const char* row = table[a];
	return static_cast<std::uint64_t>(hexdigit(row[2 * b]) * 16 + hexdigit(row[2 * b + 1]));
}

enum class Op { add, sub, mul, div };
const char* name(Op op) {
	switch (op) { case Op::add: return "+"; case Op::sub: return "-"; case Op::mul: return "*"; default: return "/"; }
}

template<typename L>
L apply(Op op, const L& x, const L& y) {
	switch (op) {
	case Op::add: return x + y;
	case Op::sub: return x - y;
	case Op::mul: return x * y;
	default:      return x / y;
	}
}

// lindex<8,3>: the expected encoding of x op y from the magnitude tables and the sign rules
std::uint64_t expected8(Op op, std::uint64_t ex, std::uint64_t ey) {
	constexpr std::uint64_t nar = 0x80;
	if (ex == nar || ey == nar) return nar;
	const bool sx = (ex & 0x80) != 0;
	const std::uint64_t ma = sx ? ((0x100 - ex) & 0xFF) : ex;
	bool sy = (ey & 0x80) != 0;
	const std::uint64_t mb = sy ? ((0x100 - ey) & 0xFF) : ey;
	auto with_sign = [](std::uint64_t mag, bool negative) { return (negative && mag != 0) ? ((0x100 - mag) & 0xFF) : mag; };
	switch (op) {
	case Op::sub:
		sy = !sy;
		[[fallthrough]];
	case Op::add:
		if (mb == 0) return ex;
		if (ma == 0) return with_sign(mb, sy);
		if (sx == sy) return with_sign(table8(lv::lindex8_add, ma, mb), sx);
		if (ma == mb) return 0;
		return with_sign(table8(lv::lindex8_sub, ma, mb), ma > mb ? sx : sy);
	case Op::mul:
		return with_sign(table8(lv::lindex8_mul, ma, mb), sx != sy);
	case Op::div:
	default:
		if (mb == 0) return nar;
		return with_sign(table8(lv::lindex8_div, ma, mb), sx != sy);
	}
}

int VerifyExhaustive8(Op op, bool reportTestCases) {
	using L = lindex<8, 3>;
	int fails = 0;
	L x, y;
	for (std::uint64_t i = 0; i < 256; ++i) {
		x.setbits(i);
		for (std::uint64_t j = 0; j < 256; ++j) {
			y.setbits(j);
			const L z = apply(op, x, y);
			const std::uint64_t e = expected8(op, i, j);
			if (z.raw_bits() != e) {
				++fails;
				if (reportTestCases && fails < 25) {
					std::cerr << "FAIL: " << to_hex(x) << ' ' << name(op) << ' ' << to_hex(y) << " = " << to_hex(z)
					          << ", expected 0x" << std::hex << e << std::dec << '\n';
				}
			}
		}
	}
	return fails;
}

template<unsigned nbits, unsigned lbits, std::size_t N>
int VerifySampled(const lv::ArithmeticVector (&vectors)[N], bool reportTestCases) {
	using L = lindex<nbits, lbits>;
	int fails = 0;
	for (const auto& v : vectors) {
		L x, y;
		x.setbits(v.x);
		y.setbits(v.y);
		const std::uint64_t expected[4] = { v.sum, v.difference, v.product, v.quotient };
		for (Op op : { Op::add, Op::sub, Op::mul, Op::div }) {
			const L z = apply(op, x, y);
			if (z.raw_bits() != expected[static_cast<int>(op)]) {
				++fails;
				if (reportTestCases) {
					std::cerr << "FAIL: " << type_tag(x) << ' ' << to_hex(x) << ' ' << name(op) << ' ' << to_hex(y) << " = " << to_hex(z)
					          << ", expected 0x" << std::hex << expected[static_cast<int>(op)] << std::dec << '\n';
				}
			}
		}
	}
	return fails;
}

// identities that hold exactly on encodings: x - x = 0, x / x = 1, x * 1 = x / 1 = x,
// x + 0 = x, commutativity, and the sign symmetries of round-to-nearest-even
template<unsigned nbits, unsigned lbits>
int VerifyIdentities(unsigned nrSamples, bool reportTestCases) {
	using L = lindex<nbits, lbits>;
	constexpr std::uint64_t mask = (nbits == 64) ? ~0ull : ((1ull << nbits) - 1ull);
	std::mt19937_64 rng(16950);
	L one(1.0), zero(0);
	int fails = 0;
	auto check = [&](bool ok, const char* what, const L& x, const L& y) {
		if (!ok) {
			++fails;
			if (reportTestCases && fails < 25) std::cerr << "FAIL: " << type_tag(x) << ' ' << what << ' ' << to_hex(x) << ' ' << to_hex(y) << '\n';
		}
	};
	for (unsigned k = 0; k < nrSamples; ++k) {
		L x, y;
		x.setbits(rng() & mask);
		y.setbits(rng() & mask);
		if (x.isnar() || y.isnar()) continue;
		check((x - x).iszero(), "x - x", x, x);
		if (!x.iszero()) check(x / x == one, "x / x", x, x);
		check(x * one == x && x / one == x && x + zero == x, "x * 1, x / 1, x + 0", x, y);
		check(x + y == y + x && x * y == y * x, "commutative", x, y);
		check(-(x + y) == (-x) + (-y) && x - y == -(y - x), "sign symmetry of +", x, y);
		check((-(x * y) == (-x) * y && -(x / y) == x / (-y)) || y.iszero(), "sign symmetry of * /", x, y);
	}
	return fails;
}

// NaR propagates, division by zero gives NaR, and closure: lbits = 3 never saturates a
// product of two encodings below the top level, while lbits = 2 does
int VerifySpecialCases(bool reportTestCases) {
	using L = lindex<16, 3>;
	int fails = 0;
	auto check = [&](bool ok, const char* what) { if (!ok) { ++fails; if (reportTestCases) std::cerr << "FAIL: " << what << '\n'; } };
	const L nar(SpecificValue::nar), two(2.0), zero(0), maxpos(SpecificValue::maxpos);
	check((nar + two).isnar() && (two - nar).isnar() && (nar * zero).isnar() && (zero / nar).isnar(), "NaR propagates");
	check((two / zero).isnar() && zero.reciprocal().isnar(), "x / 0 is NaR");
	check((zero / two).iszero() && (zero * maxpos).iszero(), "0 / x, 0 * x");
	check(maxpos * maxpos == maxpos && maxpos + maxpos == maxpos, "maxpos saturates");
	// the reciprocal underflows into level 0: 1/X rounds to 0 from about 2^13 up at fbits 12
	check(L(1.0e5).reciprocal().iszero() && !L(1000.0).reciprocal().iszero(), "reciprocal underflow");
	check(two.reciprocal() == L(1.0) / two, "reciprocal is 1 / x");
	// mixed operands with a double convert the double first
	check(two * 0.5 == two * L(0.5) && 1.0 / two == two.reciprocal(), "mixed with double");
	return fails;
}

}  // anonymous namespace

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
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "lindex arithmetic against the mpmath oracle (#1695)";
	std::string test_tag    = "arithmetic";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive8(Op::mul, reportTestCases), "lindex<8,3>", "multiplication");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive8(Op::add, reportTestCases), "lindex<8,3>", "addition, every pair");
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive8(Op::sub, reportTestCases), "lindex<8,3>", "subtraction, every pair");
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive8(Op::mul, reportTestCases), "lindex<8,3>", "multiplication, every pair");
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive8(Op::div, reportTestCases), "lindex<8,3>", "division, every pair");
	nrOfFailedTestCases += ReportTestResult(VerifySampled<12, 3>(lv::arithmetic_12_3, reportTestCases), "lindex<12,3>", "+ - * /, sampled");
	nrOfFailedTestCases += ReportTestResult(VerifySampled<16, 3>(lv::arithmetic_16_3, reportTestCases), "lindex<16,3>", "+ - * /, sampled");
	nrOfFailedTestCases += ReportTestResult(VerifySampled<32, 3>(lv::arithmetic_32_3, reportTestCases), "lindex<32,3>", "+ - * /, sampled");
	nrOfFailedTestCases += ReportTestResult(VerifySampled<64, 3>(lv::arithmetic_64_3, reportTestCases), "lindex<64,3>", "+ - * /, sampled");
	nrOfFailedTestCases += ReportTestResult(VerifySampled<16, 2>(lv::arithmetic_16_2, reportTestCases), "lindex<16,2>", "+ - * /, sampled");
	nrOfFailedTestCases += ReportTestResult(VerifySpecialCases(reportTestCases), "lindex<16,3>", "special cases");
	nrOfFailedTestCases += ReportTestResult((VerifyIdentities<16, 3>(2000, reportTestCases)), "lindex<16,3>", "identities");
	nrOfFailedTestCases += ReportTestResult((VerifyIdentities<64, 3>(200, reportTestCases)), "lindex<64,3>", "identities");
#endif

#if REGRESSION_LEVEL_2
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
	// every operation is a chain of transcendental evaluations: the larger sweeps live here
	nrOfFailedTestCases += ReportTestResult((VerifyIdentities<12, 3>(200000, reportTestCases)), "lindex<12,3>", "identities");
	nrOfFailedTestCases += ReportTestResult((VerifyIdentities<32, 3>(100000, reportTestCases)), "lindex<32,3>", "identities");
	nrOfFailedTestCases += ReportTestResult((VerifyIdentities<64, 3>(20000, reportTestCases)), "lindex<64,3>", "identities");
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
