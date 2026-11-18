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

   Known: lns + and - (the default DoubleTripAddSub) and exp, exp2 route through double and are
   not correctly rounded past about 45 fraction bits (#2128).
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

// exp and exp2 of every encoding, for an lns whose exponentials do not underflow double: lns's
// exp shim gives minpos where double underflows to 0, and zero below lns's half-minpos otherwise
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
	int64_t k = 99;
	auto onTie = [](unsigned W) { E z(2.5); z.set_precision(W); return z; };
	auto margin = [](unsigned W, double) { return std::ldexp(1.0, -static_cast<int>(W)); };
	const bool decided = decide(onTie, margin, 64u, k);
	int fails = 0;
	if (decided) { ++fails; if (reportTestCases) std::cerr << "FAIL a tie was decided\n"; }
	if (undecidedCount() != before + 1) { ++fails; if (reportTestCases) std::cerr << "FAIL the undecided count did not move\n"; }
	if (k != 99) { ++fails; if (reportTestCases) std::cerr << "FAIL an undecided result wrote k\n"; }
	auto offTie = [](unsigned W) { E z(2.25); z.set_precision(W); return z; };
	if (!decide(offTie, margin, 64u, k) || k != 2) { ++fails; if (reportTestCases) std::cerr << "FAIL 2.25 did not decide to 2\n"; }
	return fails;
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

	nrOfFailedTestCases += ReportTestResult(VerifyExhaustiveExponentials<lns<8, 3, std::uint8_t, Behavior::Saturating>>(reportTestCases), "lns<8,3,sat>", "exp, exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustiveExponentials<lns<8, 5, std::uint8_t, Behavior::Saturating>>(reportTestCases), "lns<8,5,sat>", "exp, exp2");

	nrOfFailedTestCases += ReportTestResult(VerifyAgainstDoubleReference<lnsNarrow>(5000, reportTestCases), "lns<32,27>", "exact vs double");
	nrOfFailedTestCases += ReportTestResult(VerifyAgainstDoubleReference<lns<16, 8, std::uint16_t>>(5000, reportTestCases), "lns<16,8>", "exact vs double");

	nrOfFailedTestCases += ReportTestResult(VerifyExactTie(reportTestCases), "lns<8,3>", "exp2 exact tie");
	nrOfFailedTestCases += ReportTestResult(VerifyUndecidedCounted(reportTestCases), "lns", "undecided counted");

	// the verifiers with their default reference, exact for lns<64,59>: * / and sqrt are exact there
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<lnsWide>(false, RandomsOp::OPCODE_MUL, 1000), "lns<64,59>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<lnsWide>(false, RandomsOp::OPCODE_DIV, 1000), "lns<64,59>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyUnaryOperatorThroughRandoms<lnsWide>(false, RandomsOp::OPCODE_SQRT, 1000), "lns<64,59>", "sqrt");
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
