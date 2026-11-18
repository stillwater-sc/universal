// exact_reference.cpp: the exact reference of an lns in the random-operand verifiers (#2125)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cstdint>
#include <iostream>
#include <random>
#include <string>
#include <type_traits>
#include <universal/number/lns/lns.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/test_suite_randoms.hpp>

/*
   lns rounds in the log domain: a result's exponent is the integer nearest to
   z = 2^rbits * log2|X|, ties to even. Its wide configurations (lns<64,59>: 59 fraction bits of
   the log) checked against double, which cannot hold them. The exact reference
   (lns_rounding_oracle.hpp) reads the exponent off the encoding, computes * / and sqrt exactly,
   and + - exp exp2 in efloat with Ziv's retry to a 2048-bit cap; a result still undecided there
   is counted and reported, neither pass nor fail. This test holds it to

   - lns arithmetic over every pair of 8-bit encodings, saturating and wrapping, + - * / and sqrt;
   - lns exp and exp2 over every 8-bit encoding, where they do not underflow double;
   - double's reference where double holds the results (lns<32,27>, lns<16,8>), except x/0,
     where double's reference is wrong: its inf converts to maxpos, and lns gives NaN;
   - exp2 at an exact tie (z = 1/2), which no precision decides and the reference rounds exactly;
   - the undecided count, through a tie that Ziv's retry cannot leave;
   - the defaults.

   Since #2128, lns + - exp exp2 are correctly rounded at every width (lns_correct_rounding.hpp);
   they used to route through double and were wrong past about 45 fraction bits. This test also
   holds them to the reference at lns<48,40>, lns<64,59> and lns<64,63> on operands close in
   magnitude, where the correction term matters, checks that exp underflows monotonically, and
   cross-checks the efloat path of lns wider than 64 bits against the 64-bit path.
*/

namespace sw { namespace universal {

using lnsWide   = lns<64, 59, std::uint32_t, Behavior::Wrapping>;
using lnsNarrow = lns<32, 27, std::uint32_t, Behavior::Wrapping>;

static_assert(has_exact_rounding<lnsWide>);
static_assert(std::is_same_v<default_reference_t<lnsWide>, exact_reference>);          // 60 digits
static_assert(std::is_same_v<default_reference_t<lns<32, 20>>, exact_reference>);      // its range reaches 2^1023
static_assert(std::is_same_v<default_reference_t<lnsNarrow>, double>);                  // double holds it
static_assert(!valid_reference_v<lnsWide, double>);

namespace lns_exact_reference {

template<typename Lns>
Lns apply(char op, const Lns& a, const Lns& b) {
	switch (op) {
	case '+': return a + b;
	case '-': return a - b;
	case '*': return a * b;
	default:  return a / b;
	}
}

template<typename Lns>
int VerifyExhaustive(bool reportTestCases) {
	constexpr unsigned N = 1u << Lns::nbits;
	int fails = 0;
	Lns a, b;
	for (unsigned i = 0; i < N; ++i) {
		a.setbits(i);
		for (unsigned j = 0; j < N; ++j) {
			b.setbits(j);
			for (char op : { '+', '-', '*', '/' }) {
				Lns ref;
				if (!exact_rounding<Lns>::binary(op, a, b, ref)) {   // an 8-bit lns sum is never undecided
					++fails;
					if (reportTestCases) std::cerr << "FAIL no reference for " << to_binary(a) << ' ' << op << ' ' << to_binary(b) << '\n';
					continue;
				}
				const Lns result = apply(op, a, b);
				if (!reference_equal(result, ref)) {
					++fails;
					if (reportTestCases && fails < 8) ReportBinaryArithmeticError("FAIL", std::string(1, op), a, b, result, ref);
				}
			}
		}
		Lns ref;
		exact_rounding<Lns>::sqrt(a, ref);
		const Lns root = sqrt(a);
		if (!reference_equal(root, ref)) {
			++fails;
			if (reportTestCases && fails < 8) ReportUnaryArithmeticError("FAIL", "sqrt", a, root, ref);
		}
	}
	return fails;
}

// exp and exp2 of every encoding
template<typename Lns>
int VerifyExhaustiveExponentials(bool reportTestCases) {
	constexpr unsigned N = 1u << Lns::nbits;
	int fails = 0;
	Lns a;
	for (unsigned i = 0; i < N; ++i) {
		a.setbits(i);
		for (char f : { 'e', '2' }) {
			Lns ref;
			if (!exact_rounding<Lns>::function(f, a, ref)) continue;
			const Lns result = (f == 'e') ? exp(a) : exp2(a);
			if (!reference_equal(result, ref)) {
				++fails;
				if (reportTestCases && fails < 8) ReportUnaryArithmeticError("FAIL", f == 'e' ? "exp" : "exp2", a, result, ref);
			}
		}
	}
	return fails;
}

// where double holds the results, the exact reference and double's reference agree, except at
// x/0: double's inf converts to maxpos, where lns, and the exact reference, give NaN
template<typename Lns>
int VerifyAgainstDoubleReference(int trials, bool reportTestCases) {
	static_assert(double_reference_holds<Lns>);
	std::mt19937_64 eng(0x2125u + Lns::nbits);
	int fails = 0;
	for (int t = 0; t < trials; ++t) {
		Lns a, b;
		a.setbits(eng());
		b.setbits(eng());
		if (a.isnan() || b.isnan()) continue;
		for (RandomsOp op : { RandomsOp::OPCODE_ADD, RandomsOp::OPCODE_SUB, RandomsOp::OPCODE_MUL, RandomsOp::OPCODE_DIV }) {
			if (op == RandomsOp::OPCODE_DIV && b.iszero()) continue;
			Lns c, exact, viaDouble;
			if (!executeBinary<Lns, exact_reference>(op, double(a), double(b), a, b, c, exact)) continue;
			if (!executeBinary<Lns, double>(op, double(a), double(b), a, b, c, viaDouble)) continue;
			if (!reference_equal(exact, viaDouble)) {
				++fails;
				if (reportTestCases) std::cerr << "FAIL op " << int(op) << ' ' << to_binary(a) << ' ' << to_binary(b) << " exact " << to_binary(exact) << " double " << to_binary(viaDouble) << '\n';
			}
		}
	}
	return fails;
}

// exp2(2^-4) at rbits = 3 is 2^(1/16): z = 2^3 * 1/16 = 1/2 exactly, a tie, which rounds to the
// even exponent 0, the value 1
int VerifyExactTie(bool reportTestCases) {
	using L = lns<8, 3, std::uint8_t>;
	const L x(0.0625);   // 2^-4, exactly an lns<8,3> value
	L ref;
	const unsigned long long before = exact_rounding<L>::undecided();
	const bool decided = exact_rounding<L>::function('2', x, ref);
	int fails = 0;
	if (!decided || exact_rounding<L>::undecided() != before) { ++fails; if (reportTestCases) std::cerr << "FAIL exp2(2^-4) left undecided\n"; }
	if (!(ref == L(1.0))) { ++fails; if (reportTestCases) std::cerr << "FAIL exp2(2^-4) = " << to_binary(ref) << ", the tie goes to the even exponent: 1\n"; }
	return fails;
}

// a value Ziv's retry cannot leave -- z exactly on a tie, at any precision -- is counted undecided
// and gives no reference
int VerifyUndecidedCounted(bool reportTestCases) {
	using namespace lns_oracle;
	const unsigned long long before = undecidedCount();
	E n(99.0);
	auto onTie = [](unsigned W) { E z(2.5); z.set_precision(W); return z; };
	auto margin = [](unsigned W, const E&) { return ldexp(E(1.0), -static_cast<int>(W)); };
	const decision d = decide(onTie, margin, 64u, n);
	int fails = 0;
	if (d != decision::undecided) { ++fails; if (reportTestCases) std::cerr << "FAIL a tie was not undecided\n"; }
	if (undecidedCount() != before + 1) { ++fails; if (reportTestCases) std::cerr << "FAIL the undecided count did not move\n"; }
	if (n != E(99.0)) { ++fails; if (reportTestCases) std::cerr << "FAIL an undecided result wrote n\n"; }
	auto offTie = [](unsigned W) { E z(2.25); z.set_precision(W); return z; };
	if (decide(offTie, margin, 64u, n) != decision::decided || n != E(2.0)) { ++fails; if (reportTestCases) std::cerr << "FAIL 2.25 did not decide to 2\n"; }

	// a value that leaves the tie only at the cap: the precisions tried end at the cap itself,
	// and the decision runs there (222, 444, 888, 1776 and then 2048 bits)
	auto atCap = [](unsigned W) {
		E z(2.5);
		z.set_precision(W);
		return (W >= zivCap) ? z + ldexp(E(1.0), -1900) : z;
	};
	if (decide(atCap, margin, 222u, n) != decision::decided || n != E(3.0)) { ++fails; if (reportTestCases) std::cerr << "FAIL 2.5 + 2^-1900 was not decided at the cap\n"; }

	// a margin below double's range: 2^-1150 off a tie with a bound of 2^(50 - W) is undecided at
	// 1200 bits and decided at the cap; a margin in double would be 0 at 1200 and decide too early
	unsigned lastW = 0;
	auto nearTie = [&lastW](unsigned W) {
		lastW = W;
		E z(2.5);
		z.set_precision(W);
		return z + ldexp(E(1.0), -1150);
	};
	auto wideMargin = [](unsigned W, const E&) { return ldexp(E(1.0), 50 - static_cast<int>(W)); };
	if (decide(nearTie, wideMargin, 1200u, n) != decision::decided || n != E(3.0) || lastW != zivCap) {
		++fails;
		if (reportTestCases) std::cerr << "FAIL 2.5 + 2^-1150 decided at " << lastW << " bits, not at the cap\n";
	}
	return fails;
}

// lns of 64 bits: exponents far outside int64, and the rbits = 63 doubling, are handled exactly
// before anything is narrowed (#2129 review)
template<typename Lns>
Lns withExponent(int64_t k, bool negative = false) {
	Lns v;
	lns_oracle::encode<Lns>(k, negative, v);
	return v;
}

int VerifyWideExponents(bool reportTestCases) {
	int fails = 0;
	auto expect = [&](const char* what, bool ok) { if (!ok) { ++fails; if (reportTestCases) std::cerr << "FAIL " << what << '\n'; } };
	{   // 1 + (-(2^(-2^-59))): |X| ~ 2^-59 ln2, z ~ -3.4e19, below int64
		using S = lns<64, 59, std::uint32_t, Behavior::Saturating>;
		using W = lns<64, 59, std::uint32_t, Behavior::Wrapping>;
		S rs;
		expect("lns<64,59> saturating 1 - (1 - 2^-59 ln2) has a reference", exact_rounding<S>::binary('+', withExponent<S>(0), withExponent<S>(-1, true), rs));
		expect("lns<64,59> saturating 1 - (1 - 2^-59 ln2) is zero", rs.iszero());
		W rw;
		expect("lns<64,59> wrapping 1 - (1 - 2^-59 ln2) has a reference", exact_rounding<W>::binary('+', withExponent<W>(0), withExponent<W>(-1, true), rw));
		expect("lns<64,59> wrapping 1 - (1 - 2^-59 ln2) is a number", !rw.isnan());
	}
	{   // 1 + 1 at rbits = 63: z = 2^63, past int64
		using S = lns<64, 63, std::uint32_t, Behavior::Saturating>;
		using W = lns<64, 63, std::uint32_t, Behavior::Wrapping>;
		S rs;
		exact_rounding<S>::binary('+', withExponent<S>(0), withExponent<S>(0), rs);
		expect("lns<64,63> saturating 1 + 1 is maxpos", rs == S(SpecificValue::maxpos));
		W rw;
		exact_rounding<W>::binary('+', withExponent<W>(0), withExponent<W>(0), rw);
		expect("lns<64,63> wrapping 1 + 1 wraps to exponent 0, the value 1", lns_oracle::exponentOf(rw) == 0 && !rw.sign() && !rw.iszero());
	}
	{   // exp2 of 2^-(2^32) at lns<64,0>: z = 2^-(2^32), rounds to exponent 0, the value 1
		using L = lns<64, 0, std::uint32_t, Behavior::Saturating>;
		L r;
		exact_rounding<L>::function('2', withExponent<L>(-(int64_t(1) << 32)), r);
		expect("lns<64,0> exp2(2^-(2^32)) is 1", lns_oracle::exponentOf(r) == 0 && !r.iszero());
	}
	{   // wrapping exp2(8) at lns<8,3>: z = 64, which wraps onto the zero encoding, as lns gives
		using L = lns<8, 3, std::uint8_t, Behavior::Wrapping>;
		const L x(8.0);
		L r;
		expect("lns<8,3> wrapping exp2(8) has a reference", exact_rounding<L>::function('2', x, r));
		expect("lns<8,3> wrapping exp2(8) is the zero encoding", r.iszero());
		expect("lns<8,3> wrapping exp2(8) agrees with lns", reference_equal(exp2(x), r));
	}
	{   // a wrapping exponential with 1101 integer bits: decided at the cap, its margin far past double's range
		using L = lns<16, 0, std::uint16_t, Behavior::Wrapping>;
		const unsigned long long before = exact_rounding<L>::undecided() + exact_rounding<L>::unreferenced();
		L r;
		const bool referenced = exact_rounding<L>::function('e', withExponent<L>(1100), r);   // e^(2^1100)
		expect("lns<16,0> wrapping exp(2^1100) has a reference", referenced);
		expect("lns<16,0> wrapping exp(2^1100) is neither undecided nor unreferenced", exact_rounding<L>::undecided() + exact_rounding<L>::unreferenced() == before);
	}
	{   // a wrapping exponential with more integer bits than the cap can round: counted, no reference
		using L = lns<16, 0, std::uint16_t, Behavior::Wrapping>;
		const unsigned long long before = exact_rounding<L>::unreferenced();
		L r;
		const bool referenced = exact_rounding<L>::function('e', withExponent<L>(4000), r);   // e^(2^4000)
		expect("lns<16,0> wrapping exp(2^4000) has no reference", !referenced);
		expect("lns<16,0> wrapping exp(2^4000) is counted unreferenced", exact_rounding<L>::unreferenced() == before + 1);
	}
	return fails;
}

// + and - on operands within (rbits + 2) * 2^rbits exponent units of each other, where the
// correction term log2(1 +- 2^-d) decides the result: correctly rounded at every width (#2128)
template<typename Lns>
int VerifyCloseOperands(int trials, bool reportTestCases) {
	std::mt19937_64 eng(0x2128u + Lns::nbits + Lns::rbits);
	constexpr unsigned rb = Lns::rbits;
	const uint64_t span = (uint64_t(rb) + 2u) << (rb > 56u ? 56u : rb);
	int fails = 0;
	for (int t = 0; t < trials; ++t) {
		Lns a;
		a.setbits(eng());
		if (a.isnan() || a.iszero()) { --t; continue; }
		const int64_t ka = lns_oracle::exponentOf(a);
		const int64_t kb = ka - static_cast<int64_t>(eng() % span) * ((eng() & 1u) ? 1 : -1);
		Lns b;
		lns_oracle::encode<Lns>(kb, (eng() & 1u) != 0, b);
		if (b.isnan() || b.iszero()) { --t; continue; }
		for (char op : { '+', '-' }) {
			Lns ref;
			if (!exact_rounding<Lns>::binary(op, a, b, ref)) continue;   // undecided: counted and reported
			const Lns result = (op == '+') ? a + b : a - b;
			if (!reference_equal(result, ref)) {
				++fails;
				if (reportTestCases && fails < 4) ReportBinaryArithmeticError("FAIL", std::string(1, op), a, b, result, ref);
			}
		}
	}
	return fails;
}

template<typename Lns>
int VerifyRandomExponentials(int trials, bool reportTestCases) {
	std::mt19937_64 eng(0x2128u);
	int fails = 0;
	for (int t = 0; t < trials; ++t) {
		Lns x;
		x.setbits(eng());
		for (char f : { 'e', '2' }) {
			Lns ref;
			if (!exact_rounding<Lns>::function(f, x, ref)) continue;
			const Lns result = (f == 'e') ? exp(x) : exp2(x);
			if (!reference_equal(result, ref)) {
				++fails;
				if (reportTestCases && fails < 4) ReportUnaryArithmeticError("FAIL", f == 'e' ? "exp" : "exp2", x, result, ref);
			}
		}
	}
	return fails;
}

// exp(x) is monotone in x down to the underflow: once it is zero, it stays zero (the old shim gave
// minpos where double underflowed, and zero just above it)
template<typename Lns>
int VerifyExpUnderflowMonotone(bool reportTestCases) {
	int fails = 0;
	Lns previous(1.0);
	bool seenZero = false;
	for (double x = -1.0; x > -1.0e6; x *= 1.5) {
		const Lns e = exp(Lns(x));
		if (seenZero && !e.iszero()) {
			++fails;
			if (reportTestCases) std::cerr << "FAIL exp(" << x << ") = " << to_binary(e) << " after a zero\n";
		}
		if (e > previous) {
			++fails;
			if (reportTestCases) std::cerr << "FAIL exp(" << x << ") grew\n";
		}
		seenZero = seenZero || e.iszero();
		previous = e;
	}
	if (!seenZero) { ++fails; if (reportTestCases) std::cerr << "FAIL exp never underflowed to zero\n"; }
	return fails;
}

// the efloat path of an lns wider than 64 bits against the 64-bit path, on values both hold:
// lns<80,16> and lns<24,16> share their grid, and a sum that stays in lns<24,16>'s range is the same
int VerifyWidePathAgainst64(int trials, bool reportTestCases) {
	using Narrow = lns<24, 16, std::uint32_t>;
	using Wide   = lns<80, 16, std::uint32_t>;
	std::mt19937_64 eng(0x2128u);
	int fails = 0, compared = 0;
	for (int t = 0; t < trials; ++t) {
		Narrow a, b;
		a.setbits(eng() & 0x7FFFFFu);   // positive values
		b.setbits(eng() & 0x7FFFFFu);
		if (a.iszero() || b.iszero() || a.isnan() || b.isnan()) continue;
		const int64_t ka = lns_oracle::exponentOf(a), kb = lns_oracle::exponentOf(b);
		const int64_t limit = int64_t(1) << 20;   // keep the results inside lns<24,16>'s field of 2^22
		if (ka > limit || kb > limit || ka < -limit || kb < -limit) continue;
		Wide wa, wb;
		lns_rounding::encode(sw::universal::lns_rounding::E(static_cast<long long>(ka)), false, wa);
		lns_rounding::encode(sw::universal::lns_rounding::E(static_cast<long long>(kb)), false, wb);
		for (char op : { '+', '-' }) {
			const Narrow rn = (op == '+') ? a + b : a - b;
			const Wide   rw = (op == '+') ? wa + wb : wa - wb;
			if (rn.iszero() || rw.iszero()) {
				if (rn.iszero() != rw.iszero()) { ++fails; if (reportTestCases) std::cerr << "FAIL zero mismatch\n"; }
				continue;
			}
			++compared;
			if (lns_oracle::exponentOf(rn) != static_cast<int64_t>(double(lns_rounding::exponentOf(rw))) || rn.sign() != rw.sign()) {
				++fails;
				if (reportTestCases && fails < 4) std::cerr << "FAIL lns<80,16> " << op << " differs from lns<24,16>: " << to_binary(rn) << '\n';
			}
		}
	}
	return fails + (compared == 0 ? 1 : 0);
}

}  // namespace lns_exact_reference
}}  // namespace sw::universal

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
	using namespace sw::universal::lns_exact_reference;

	std::string test_suite  = "lns verifier exact reference";
	std::string test_tag    = "exact reference";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<lns<8, 3, std::uint8_t>>(reportTestCases), "lns<8,3>", test_tag);
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<lns<8, 0, std::uint8_t, Behavior::Saturating>>(reportTestCases), "lns<8,0,sat>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<lns<8, 3, std::uint8_t, Behavior::Saturating>>(reportTestCases), "lns<8,3,sat>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<lns<8, 6, std::uint8_t, Behavior::Saturating>>(reportTestCases), "lns<8,6,sat>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<lns<8, 0, std::uint8_t, Behavior::Wrapping>>(reportTestCases), "lns<8,0,wrap>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<lns<8, 3, std::uint8_t, Behavior::Wrapping>>(reportTestCases), "lns<8,3,wrap>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<lns<8, 7, std::uint8_t, Behavior::Wrapping>>(reportTestCases), "lns<8,7,wrap>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyExhaustiveExponentials<lns<8, 0, std::uint8_t, Behavior::Saturating>>(reportTestCases), "lns<8,0,sat>", "exp, exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustiveExponentials<lns<8, 3, std::uint8_t, Behavior::Saturating>>(reportTestCases), "lns<8,3,sat>", "exp, exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustiveExponentials<lns<8, 3, std::uint8_t, Behavior::Wrapping>>(reportTestCases), "lns<8,3,wrap>", "exp, exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustiveExponentials<lns<8, 5, std::uint8_t, Behavior::Saturating>>(reportTestCases), "lns<8,5,sat>", "exp, exp2");

	nrOfFailedTestCases += ReportTestResult(VerifyAgainstDoubleReference<lnsNarrow>(5000, reportTestCases), "lns<32,27>", "exact vs double");
	nrOfFailedTestCases += ReportTestResult(VerifyAgainstDoubleReference<lns<16, 8, std::uint16_t>>(5000, reportTestCases), "lns<16,8>", "exact vs double");

	nrOfFailedTestCases += ReportTestResult(VerifyExactTie(reportTestCases), "lns<8,3>", "exp2 exact tie");
	nrOfFailedTestCases += ReportTestResult(VerifyUndecidedCounted(reportTestCases), "lns", "undecided counted");
	nrOfFailedTestCases += ReportTestResult(VerifyWideExponents(reportTestCases), "lns<64,x>", "wide exponents");

	// correctly rounded at every width (#2128)
	nrOfFailedTestCases += ReportTestResult(VerifyCloseOperands<lns<32, 20, std::uint32_t>>(2000, reportTestCases), "lns<32,20>", "close + -");
	nrOfFailedTestCases += ReportTestResult(VerifyCloseOperands<lns<48, 40, std::uint32_t>>(1000, reportTestCases), "lns<48,40>", "close + -");
	nrOfFailedTestCases += ReportTestResult(VerifyCloseOperands<lnsWide>(500, reportTestCases), "lns<64,59>", "close + -");
	nrOfFailedTestCases += ReportTestResult(VerifyCloseOperands<lns<64, 63, std::uint32_t>>(300, reportTestCases), "lns<64,63>", "close + -");
	nrOfFailedTestCases += ReportTestResult(VerifyRandomExponentials<lnsWide>(500, reportTestCases), "lns<64,59>", "exp, exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyRandomExponentials<lns<16, 8, std::uint16_t>>(2000, reportTestCases), "lns<16,8>", "exp, exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExpUnderflowMonotone<lns<16, 8, std::uint16_t>>(reportTestCases), "lns<16,8>", "exp underflow");
	nrOfFailedTestCases += ReportTestResult(VerifyWidePathAgainst64(2000, reportTestCases), "lns<80,16>", "wide path");

	// the verifiers with their default reference, exact for lns<64,59>
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<lnsWide>(false, RandomsOp::OPCODE_MUL, 1000), "lns<64,59>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<lnsWide>(false, RandomsOp::OPCODE_DIV, 1000), "lns<64,59>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyUnaryOperatorThroughRandoms<lnsWide>(false, RandomsOp::OPCODE_SQRT, 1000), "lns<64,59>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<lnsWide>(false, RandomsOp::OPCODE_ADD, 1000), "lns<64,59>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<lnsWide>(false, RandomsOp::OPCODE_SUB, 1000), "lns<64,59>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyElementaryFunctionThroughRandoms<lnsWide>(false, RandomsOp::OPCODE_EXP2, 500), "lns<64,59>", "exp2");
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
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
