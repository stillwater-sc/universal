// reference_type.cpp: the reference type of the random-operand verifiers (#2121)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cstdint>
#include <iostream>
#include <string>
#include <type_traits>
#include <universal/number/posit/posit.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/posit_test_suite_randoms.hpp>

/*
   The random-operand verifiers round a reference once to the type under test. With double as the
   reference that is right below 53 digits; posit<64,x> has up to 60, so its operands are already
   rounded when they become doubles, and division failed against it at regression level 4. The
   exact reference (posit_rounding_oracle.hpp) decodes the operands exactly and rounds by the posit
   rule itself. This test holds the exact reference to

   - posit arithmetic over every pair of posit<8,0> and posit<8,2> encodings, where the arithmetic is
     verified exhaustively elsewhere: ties, the taper, minpos and maxpos;
   - double's reference wherever double holds the results, posit<32,2> and posit<48,2>;
   - the special cases: NaR, division by zero, an exact zero, and the clamps at minpos and maxpos;

   and shows the double reference is wrong for posit<64,2> division while the exact one is not:
   the cases where the two disagree are the cases where the posit result is the exact one.
*/

namespace sw {
namespace universal {

// the defaults, and double refused where it cannot hold the results
static_assert(std::is_same_v<default_reference_t<posit<32, 2>>, double>);
static_assert(std::is_same_v<default_reference_t<posit<48, 2>>, double>);
static_assert(std::is_same_v<default_reference_t<posit<64, 2>>, exact_reference>);
static_assert(std::is_same_v<default_reference_t<posit<64, 4>>, exact_reference>);
static_assert(valid_reference_v<posit<32, 2>, double>);
static_assert(valid_reference_v<posit<32, 2>, exact_reference>);
static_assert(!valid_reference_v<posit<64, 2>, double>);
static_assert(valid_reference_v<posit<64, 2>, exact_reference>);
static_assert(!valid_reference_v<double, exact_reference>);   // no exact_rounding<double>
static_assert(!valid_reference_v<posit<32, 2>, float>);
// posit<32,6> has 24 digits but reaches 2^1920, past double's range
static_assert(std::is_same_v<default_reference_t<posit<32, 6>>, exact_reference>);
static_assert(!valid_reference_v<posit<32, 6>, double>);

namespace reference_type {

struct Rng {  // splitmix64: reproducible on every platform
	std::uint64_t state;
	std::uint64_t next() {
		std::uint64_t z = (state += 0x9E3779B97F4A7C15ull);
		z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
		z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
		return z ^ (z >> 31);
	}
};

template<typename Posit>
Posit apply(char op, const Posit& a, const Posit& b) {
	switch (op) {
	case '+': return a + b;
	case '-': return a - b;
	case '*': return a * b;
	default:  return a / b;
	}
}

template<typename Posit>
int CheckOne(char op, const Posit& a, const Posit& b, bool reportTestCases) {
	Posit ref;
	if (!exact_rounding<Posit>::binary(op, a, b, ref)) return 1;
	const Posit result = apply(op, a, b);
	if (result == ref) return 0;
	if (reportTestCases) ReportBinaryArithmeticError("FAIL", std::string(1, op), a, b, result, ref);
	return 1;
}

// every pair of encodings, every operator
template<typename Posit>
int VerifyExhaustive(bool reportTestCases) {
	constexpr unsigned N = 1u << Posit::nbits;
	int fails = 0;
	Posit a, b;
	for (unsigned i = 0; i < N; ++i) {
		a.setbits(i);
		for (unsigned j = 0; j < N; ++j) {
			b.setbits(j);
			for (char op : { '+', '-', '*', '/' }) fails += CheckOne(op, a, b, reportTestCases);
		}
		Posit ref;
		exact_rounding<Posit>::sqrt(a, ref);
		const Posit root = sqrt(a);
		if (root != ref) {
			++fails;
			if (reportTestCases) ReportUnaryArithmeticError("FAIL", "sqrt", a, root, ref);
		}
	}
	return fails;
}

// where double holds the results, the exact reference and double's reference are the same value
template<typename Posit>
int VerifyAgainstDoubleReference(int trials, bool reportTestCases) {
	static_assert(valid_reference_v<Posit, double>);
	Rng rng{ 0x2121ull + Posit::nbits };
	int fails = 0;
	for (int t = 0; t < trials; ++t) {
		Posit a, b;
		a.setbits(rng.next());
		b.setbits(rng.next());
		for (int opcode : { OPCODE_ADD, OPCODE_SUB, OPCODE_MUL, OPCODE_DIV }) {
			Posit c, exact, viaDouble;
			double dc{ 0 };
			executeBinary<Posit, exact_reference>(opcode, double(a), double(b), dc, a, b, c, exact);
			executeBinary<Posit, double>(opcode, double(a), double(b), dc, a, b, c, viaDouble);
			if (exact != viaDouble) {
				++fails;
				if (reportTestCases) {
					std::cerr << "FAIL opcode " << opcode << ' ' << to_binary(a) << ' ' << to_binary(b) << " exact " << to_binary(exact)
					          << " double " << to_binary(viaDouble) << '\n';
				}
			}
		}
	}
	return fails;
}

// the operands go into the exact reference without loss: decoding an encoding and rounding the
// exact value back gives the same encoding, for every finite posit (exhaustively for posit<8,x>
// through a + 0 above, here at random for the wide ones)
template<typename Posit>
int VerifyOperandRoundTrip(int trials, bool reportTestCases) {
	using namespace rounding_oracle;
	Rng rng{ 0x2122ull + Posit::nbits * 7u + Posit::es };
	int fails = 0;
	for (int t = 0; t < trials; ++t) {
		Posit a;
		blockbinary<Posit::nbits, typename Posit::BlockType, BinaryNumberType::Signed> raw;
		raw.clear();
		std::uint64_t word = 0;
		for (unsigned i = 0; i < Posit::nbits; ++i) {
			if ((i % 64u) == 0) word = rng.next();
			raw.setbit(i, ((word >> (i % 64u)) & 1ull) != 0);
		}
		a.setbits(raw);
		if (a.isnar()) continue;
		const Dyadic x = valueOf(a);
		Posit back;
		roundExact(x.m.zero(), x.neg, [&](const Dyadic& m) { return cmpMagnitude(x, m); }, back);
		if (back != a) {
			++fails;
			if (reportTestCases) std::cerr << "FAIL round trip " << to_binary(a) << " came back as " << to_binary(back) << '\n';
		}
	}
	return fails;
}

template<typename Posit>
int Expect(const std::string& what, char op, const Posit& a, const Posit& b, const Posit& expected, bool reportTestCases) {
	Posit ref;
	exact_rounding<Posit>::binary(op, a, b, ref);
	if (ref == expected) return 0;
	if (reportTestCases) std::cerr << "FAIL " << what << ": " << to_binary(ref) << " expected " << to_binary(expected) << '\n';
	return 1;
}

template<typename Posit>
int VerifySpecialCases(bool reportTestCases) {
	const Posit maxpos(SpecificValue::maxpos), minpos(SpecificValue::minpos), maxneg(SpecificValue::maxneg), minneg(SpecificValue::minneg);
	const Posit zero(0), one(1), two(2), nar(SpecificValue::nar);
	int fails = 0;
	fails += Expect("maxpos * maxpos", '*', maxpos, maxpos, maxpos, reportTestCases);
	fails += Expect("maxpos * -maxpos", '*', maxpos, maxneg, maxneg, reportTestCases);
	fails += Expect("minpos * minpos", '*', minpos, minpos, minpos, reportTestCases);
	fails += Expect("minpos * -minpos", '*', minpos, minneg, minneg, reportTestCases);
	fails += Expect("minpos / maxpos", '/', minpos, maxpos, minpos, reportTestCases);
	fails += Expect("maxpos / minpos", '/', maxpos, minpos, maxpos, reportTestCases);
	fails += Expect("maxpos + maxpos", '+', maxpos, maxpos, maxpos, reportTestCases);
	fails += Expect("1 - 1", '-', one, one, zero, reportTestCases);
	fails += Expect("0 / 2", '/', zero, two, zero, reportTestCases);
	fails += Expect("0 * maxpos", '*', zero, maxpos, zero, reportTestCases);
	fails += Expect("1 / 0", '/', one, zero, nar, reportTestCases);
	fails += Expect("NaR + 1", '+', nar, one, nar, reportTestCases);
	fails += Expect("1 * NaR", '*', one, nar, nar, reportTestCases);
	fails += Expect("1 + 1", '+', one, one, two, reportTestCases);
	fails += Expect("2 / 2", '/', two, two, one, reportTestCases);
	// 1 + minpos lies far below the midpoint above 1
	fails += Expect("1 + minpos", '+', one, minpos, one, reportTestCases);
	Posit ref;
	exact_rounding<Posit>::sqrt(-one, ref);
	if (!ref.isnar()) { ++fails; if (reportTestCases) std::cerr << "FAIL sqrt(-1) is not NaR\n"; }
	exact_rounding<Posit>::sqrt(Posit(4), ref);
	if (ref != two) { ++fails; if (reportTestCases) std::cerr << "FAIL sqrt(4) is not 2\n"; }
	if (exact_rounding<Posit>::binary('^', one, one, ref)) { ++fails; if (reportTestCases) std::cerr << "FAIL '^' has an exact reference\n"; }
	return fails;
}

// The double reference of posit<64,2> division rounds the operands to 53 bits first. Over random
// operands it disagrees with the exact reference, and wherever they disagree the posit quotient
// is the exact reference's: the double reference reports a correct quotient as a failure, which
// is what regression level 4 saw. Fails when the two never disagree (then this test no longer
// shows anything) or when the quotient differs from the exact reference.
template<typename Posit>
int VerifyDoubleReferenceTooNarrow(int trials, bool reportTestCases) {
	Rng rng{ 0x64ull * Posit::es + 0x2121ull };
	int disagreements = 0, fails = 0;
	for (int t = 0; t < trials; ++t) {
		Posit a, b;
		a.setbits(rng.next());
		b.setbits(rng.next());
		if (a.isnar() || b.isnar() || b.iszero()) continue;
		Posit c, exact, viaDouble;
		double dc{ 0 };
		executeBinary<Posit, exact_reference>(OPCODE_DIV, double(a), double(b), dc, a, b, c, exact);
		viaDouble = correctlyRoundedReference<Posit>('/', double(a), double(b), double(a) / double(b));
		if (c != exact) {
			++fails;
			if (reportTestCases) ReportBinaryArithmeticError("FAIL", "/", a, b, c, exact);
		}
		if (exact != viaDouble) ++disagreements;
	}
	if (reportTestCases) std::cout << "  double reference disagrees with the exact one in " << disagreements << " of " << trials << " quotients\n";
	return fails + (disagreements == 0 ? 1 : 0);
}

}  // namespace reference_type
}  // namespace universal
}  // namespace sw

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
	using namespace sw::universal::reference_type;

	std::string test_suite  = "posit verifier reference type";
	std::string test_tag    = "reference type";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyDoubleReferenceTooNarrow<posit<64, 2>>(10000, reportTestCases), "posit<64,2>", "double too narrow");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<posit<8, 0>>(reportTestCases), "posit<8,0>", "exact reference");
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<posit<8, 2>>(reportTestCases), "posit<8,2>", "exact reference");

	nrOfFailedTestCases += ReportTestResult(VerifyAgainstDoubleReference<posit<32, 2>>(2000, reportTestCases), "posit<32,2>", "exact vs double");
	nrOfFailedTestCases += ReportTestResult(VerifyAgainstDoubleReference<posit<48, 2>>(2000, reportTestCases), "posit<48,2>", "exact vs double");

	nrOfFailedTestCases += ReportTestResult(VerifyOperandRoundTrip<posit<64, 2>>(2000, reportTestCases), "posit<64,2>", "operand round trip");
	nrOfFailedTestCases += ReportTestResult(VerifyOperandRoundTrip<posit<64, 4>>(2000, reportTestCases), "posit<64,4>", "operand round trip");
	nrOfFailedTestCases += ReportTestResult(VerifyOperandRoundTrip<posit<256, 4, std::uint64_t>>(500, reportTestCases), "posit<256,4>", "operand round trip");

	nrOfFailedTestCases += ReportTestResult(VerifySpecialCases<posit<16, 1>>(reportTestCases), "posit<16,1>", "special cases");
	nrOfFailedTestCases += ReportTestResult(VerifySpecialCases<posit<64, 2>>(reportTestCases), "posit<64,2>", "special cases");
	nrOfFailedTestCases += ReportTestResult(VerifySpecialCases<posit<128, 4, std::uint64_t>>(reportTestCases), "posit<128,4>", "special cases");

	nrOfFailedTestCases += ReportTestResult(VerifyDoubleReferenceTooNarrow<posit<64, 2>>(2000, reportTestCases), "posit<64,2>", "double too narrow");

	// the verifiers with their default reference, exact for posit<64,x>
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posit<64, 2>>(false, OPCODE_DIV, 500), "posit<64,2>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posit<64, 3>>(false, OPCODE_MUL, 500), "posit<64,3>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posit<64, 4>>(false, OPCODE_ADD, 500), "posit<64,4>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyUnaryOperatorThroughRandoms<posit<64, 2>>(false, OPCODE_SQRT, 500, 0.0), "posit<64,2>", "sqrt");
	// only sqrt and + - * / have an exact reference: anything else is cancelled, which counts as a failure
	if (VerifyUnaryOperatorThroughRandoms<posit<64, 2>>(false, OPCODE_EXP, 10, 0.0) != 1) ++nrOfFailedTestCases;
	if (VerifyBinaryOperatorThroughRandoms<posit<64, 2>>(false, OPCODE_POW, 10) != 1) ++nrOfFailedTestCases;
#endif

#if REGRESSION_LEVEL_2
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyDoubleReferenceTooNarrow<posit<64, 3>>(20000, reportTestCases), "posit<64,3>", "double too narrow");
	nrOfFailedTestCases += ReportTestResult(VerifyDoubleReferenceTooNarrow<posit<64, 4>>(20000, reportTestCases), "posit<64,4>", "double too narrow");
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
