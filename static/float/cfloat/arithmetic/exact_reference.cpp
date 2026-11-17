// exact_reference.cpp: the exact reference of a cfloat in the random-operand verifiers (#2123)
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
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/test_suite_randoms.hpp>

/*
   cfloat wider than double (c80, c96, c128) checked against a double reference that cannot hold
   its results, and its random operands filled only the low 64 bits of the encoding, so the random
   tests of those types ran with zero cases. The exact reference (cfloat_rounding_oracle.hpp)
   decodes the operands exactly and rounds by the cfloat rule. This test holds it to

   - cfloat arithmetic over every pair of 8-bit encodings, in every combination of subnormals,
     max-exponent values and saturation: ties, subnormals, flush to zero, overflow to inf or
     maxpos, inf and NaN operands, signed zeros;
   - double's reference for c32, where double holds the results;
   - an operand round trip for c80 and c128: decoding an encoding exactly and rounding it back
     gives the same encoding;
   - random operands that reach every bit of a 128-bit encoding.
*/

namespace sw { namespace universal {

using c32  = cfloat< 32,  8, std::uint32_t, true, false, false>;
using c64  = cfloat< 64, 11, std::uint64_t, true, false, false>;
using c80  = cfloat< 80, 11, std::uint16_t, true, false, false>;
using c128 = cfloat<128, 15, std::uint32_t, true, false, false>;

static_assert(has_exact_rounding<c32>);
static_assert(std::is_same_v<default_reference_t<c32>, double>);                                         // double holds c32
static_assert(std::is_same_v<default_reference_t<c64>, exact_reference>);                                // 53 digits, and double's range doubled
static_assert(std::is_same_v<default_reference_t<c80>, exact_reference>);
static_assert(std::is_same_v<default_reference_t<cfloat<96, 15, std::uint8_t, false, false, true>>, exact_reference>);
static_assert(std::is_same_v<default_reference_t<c128>, exact_reference>);
static_assert(!valid_reference_v<c80, double>);
static_assert(!valid_reference_v<c128, double>);

namespace cfloat_exact_reference {

template<typename Cfloat>
Cfloat apply(char op, const Cfloat& a, const Cfloat& b) {
	switch (op) {
	case '+': return a + b;
	case '-': return a - b;
	case '*': return a * b;
	default:  return a / b;
	}
}

// every pair of encodings, every operator, and sqrt of every encoding
template<typename Cfloat>
int VerifyExhaustive(bool reportTestCases) {
	constexpr unsigned N = 1u << Cfloat::nbits;
	int fails = 0;
	Cfloat a, b;
	for (unsigned i = 0; i < N; ++i) {
		a.setbits(i);
		for (unsigned j = 0; j < N; ++j) {
			b.setbits(j);
			for (char op : { '+', '-', '*', '/' }) {
				Cfloat ref;
				exact_rounding<Cfloat>::binary(op, a, b, ref);
				const Cfloat result = apply(op, a, b);
				if (!reference_equal(result, ref)) {
					++fails;
					if (reportTestCases && fails < 8) ReportBinaryArithmeticError("FAIL", std::string(1, op), a, b, result, ref);
				}
			}
		}
		Cfloat ref;
		exact_rounding<Cfloat>::sqrt(a, ref);
		const Cfloat root = sqrt(a);
		if (!reference_equal(root, ref)) {
			++fails;
			if (reportTestCases && fails < 8) ReportUnaryArithmeticError("FAIL", "sqrt", a, root, ref);
		}
	}
	return fails;
}

// where double holds the results, the exact reference and double's reference are the same value
template<typename Cfloat>
int VerifyAgainstDoubleReference(int trials, bool reportTestCases) {
	static_assert(double_reference_holds<Cfloat>);
	std::mt19937_64 eng(0x2123u);
	int fails = 0;
	for (int t = 0; t < trials; ++t) {
		Cfloat a, b;
		randomEncoding(a, eng);
		randomEncoding(b, eng);
		if (a.isnan() || b.isnan() || a.isinf() || b.isinf()) continue;
		for (RandomsOp op : { RandomsOp::OPCODE_ADD, RandomsOp::OPCODE_SUB, RandomsOp::OPCODE_MUL, RandomsOp::OPCODE_DIV }) {
			Cfloat c, exact, viaDouble;
			executeBinary<Cfloat, exact_reference>(op, double(a), double(b), a, b, c, exact);
			if (!executeBinary<Cfloat, double>(op, double(a), double(b), a, b, c, viaDouble)) continue;
			if (!reference_equal(exact, viaDouble)) {
				++fails;
				if (reportTestCases) std::cerr << "FAIL op " << int(op) << ' ' << to_binary(a) << ' ' << to_binary(b) << " exact " << to_binary(exact) << " double " << to_binary(viaDouble) << '\n';
			}
		}
	}
	return fails;
}

// decoding a finite encoding exactly and rounding it back gives the same encoding
template<typename Cfloat>
int VerifyOperandRoundTrip(int trials, bool reportTestCases) {
	using namespace cfloat_oracle;
	std::mt19937_64 eng(0x2123u + Cfloat::nbits);
	int fails = 0;
	for (int t = 0; t < trials; ++t) {
		Cfloat a;
		randomEncoding(a, eng);
		if (a.isnan() || a.isinf() || a.iszero()) continue;
		const Dyadic x = valueOf(a);
		Cfloat back;
		roundExact(x.neg, [&](const Dyadic& m) { return cmpMagnitude(x, m); }, back);
		if (back != a) {
			++fails;
			if (reportTestCases) std::cerr << "FAIL round trip " << to_binary(a) << " came back as " << to_binary(back) << '\n';
		}
	}
	return fails;
}

// random encodings reach every bit, also above bit 63: each bit is seen set and clear
template<typename Cfloat>
int VerifyRandomEncodingsReachEveryBit(bool reportTestCases) {
	std::mt19937_64 eng(0x2123u);
	std::vector<int> ones(Cfloat::nbits, 0);
	constexpr int trials = 256;
	for (int t = 0; t < trials; ++t) {
		Cfloat a;
		randomEncoding(a, eng);
		for (unsigned i = 0; i < Cfloat::nbits; ++i) ones[i] += a.at(i) ? 1 : 0;
	}
	int fails = 0;
	for (unsigned i = 0; i < Cfloat::nbits; ++i) {
		if (ones[i] == 0 || ones[i] == trials) {
			++fails;
			if (reportTestCases) std::cerr << "FAIL bit " << i << " is never " << (ones[i] == 0 ? "set" : "clear") << '\n';
		}
	}
	return fails;
}

// a type with setbit but no exact reference, as dd and qd are: randomEncoding keeps setbits
struct limbs_without_exact_reference {
	static constexpr unsigned nbits = 128;
	int setbitCalls{ 0 }, setbitsCalls{ 0 };
	void setbit(unsigned, bool) { ++setbitCalls; }
	void setbits(unsigned long long) { ++setbitsCalls; }
};
static_assert(!has_exact_rounding<limbs_without_exact_reference>);

int VerifyRandomEncodingKeepsSetbitsWithoutExactReference(bool reportTestCases) {
	std::mt19937_64 eng(0x2123u);
	limbs_without_exact_reference v;
	randomEncoding(v, eng);
	if (v.setbitCalls == 0 && v.setbitsCalls == 1) return 0;
	if (reportTestCases) std::cerr << "FAIL randomEncoding used setbit " << v.setbitCalls << " times, setbits " << v.setbitsCalls << '\n';
	return 1;
}

// the special cases at a wide type
template<typename Cfloat>
int VerifySpecialCases(bool reportTestCases) {
	const Cfloat maxpos(SpecificValue::maxpos), minpos(SpecificValue::minpos), one(1), zero(0);
	Cfloat inf, nan;
	inf.setinf(false);
	nan.setnan(NAN_TYPE_QUIET);
	int fails = 0;
	auto expect = [&](const char* what, char op, const Cfloat& a, const Cfloat& b, auto predicate) {
		Cfloat ref;
		exact_rounding<Cfloat>::binary(op, a, b, ref);
		if (!predicate(ref)) {
			++fails;
			if (reportTestCases) std::cerr << "FAIL " << what << ": " << to_binary(ref) << '\n';
		}
	};
	expect("maxpos * maxpos is inf", '*', maxpos, maxpos, [](const Cfloat& r) { return r.isinf() && !r.sign(); });
	expect("minpos * minpos is +0", '*', minpos, minpos, [](const Cfloat& r) { return r.iszero() && !r.sign(); });
	expect("-minpos * minpos is -0", '*', -minpos, minpos, [](const Cfloat& r) { return r.iszero() && r.sign(); });
	expect("1 - 1 is +0", '-', one, one, [](const Cfloat& r) { return r.iszero() && !r.sign(); });
	expect("1 / 0 is inf", '/', one, zero, [](const Cfloat& r) { return r.isinf() && !r.sign(); });
	expect("0 / 0 is NaN", '/', zero, zero, [](const Cfloat& r) { return r.isnan(); });
	expect("inf - inf is NaN", '-', inf, inf, [](const Cfloat& r) { return r.isnan(); });
	expect("inf * 0 is NaN", '*', inf, zero, [](const Cfloat& r) { return r.isnan(); });
	expect("1 / inf is 0", '/', one, inf, [](const Cfloat& r) { return r.iszero(); });
	expect("NaN + 1 is NaN", '+', nan, one, [](const Cfloat& r) { return r.isnan(); });
	expect("1 + minpos is 1", '+', one, minpos, [&](const Cfloat& r) { return r == one; });
	Cfloat ref;
	if (exact_rounding<Cfloat>::binary('^', one, one, ref)) { ++fails; if (reportTestCases) std::cerr << "FAIL '^' has an exact reference\n"; }
	exact_rounding<Cfloat>::sqrt(-one, ref);
	if (!ref.isnan()) { ++fails; if (reportTestCases) std::cerr << "FAIL sqrt(-1) is not NaN\n"; }
	exact_rounding<Cfloat>::sqrt(Cfloat(4), ref);
	if (ref != Cfloat(2)) { ++fails; if (reportTestCases) std::cerr << "FAIL sqrt(4) is not 2\n"; }
	return fails;
}

}  // namespace cfloat_exact_reference
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
	using namespace sw::universal::cfloat_exact_reference;

	std::string test_suite  = "cfloat verifier exact reference";
	std::string test_tag    = "exact reference";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<cfloat<8, 3, std::uint8_t, false, false, true>>(reportTestCases), "cfloat<8,3,f,f,t>", test_tag);
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	// every combination of subnormals, max-exponent values, saturation (es = 1 needs both of the first two)
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<cfloat<8, 1, std::uint8_t, true,  true,  false>>(reportTestCases), "cfloat<8,1,t,t,f>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<cfloat<8, 3, std::uint8_t, false, false, false>>(reportTestCases), "cfloat<8,3,f,f,f>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<cfloat<8, 3, std::uint8_t, true,  false, false>>(reportTestCases), "cfloat<8,3,t,f,f>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<cfloat<8, 3, std::uint8_t, false, true,  false>>(reportTestCases), "cfloat<8,3,f,t,f>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<cfloat<8, 4, std::uint8_t, true,  true,  false>>(reportTestCases), "cfloat<8,4,t,t,f>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<cfloat<8, 3, std::uint8_t, false, false, true >>(reportTestCases), "cfloat<8,3,f,f,t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<cfloat<8, 3, std::uint8_t, true,  false, true >>(reportTestCases), "cfloat<8,3,t,f,t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<cfloat<8, 3, std::uint8_t, false, true,  true >>(reportTestCases), "cfloat<8,3,f,t,t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<cfloat<8, 4, std::uint8_t, true,  true,  true >>(reportTestCases), "cfloat<8,4,t,t,t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<cfloat<8, 5, std::uint8_t, false, false, false>>(reportTestCases), "cfloat<8,5,f,f,f>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyAgainstDoubleReference<c32>(2000, reportTestCases), "c32", "exact vs double");

	nrOfFailedTestCases += ReportTestResult(VerifyOperandRoundTrip<c80>(2000, reportTestCases), "c80", "operand round trip");
	nrOfFailedTestCases += ReportTestResult(VerifyOperandRoundTrip<c128>(2000, reportTestCases), "c128", "operand round trip");
	nrOfFailedTestCases += ReportTestResult(VerifyRandomEncodingsReachEveryBit<c128>(reportTestCases), "c128", "random encodings");
	nrOfFailedTestCases += ReportTestResult(VerifyRandomEncodingKeepsSetbitsWithoutExactReference(reportTestCases), "dd-like", "random encodings");

	nrOfFailedTestCases += ReportTestResult(VerifySpecialCases<c80>(reportTestCases), "c80", "special cases");
	nrOfFailedTestCases += ReportTestResult(VerifySpecialCases<c128>(reportTestCases), "c128", "special cases");

	// the verifiers with their default reference, exact for c80 and c128
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<c80>(false, RandomsOp::OPCODE_DIV, 1000), "c80", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<c128>(false, RandomsOp::OPCODE_ADD, 1000), "c128", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyUnaryOperatorThroughRandoms<c128>(false, RandomsOp::OPCODE_SQRT, 1000), "c128", "sqrt");
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
