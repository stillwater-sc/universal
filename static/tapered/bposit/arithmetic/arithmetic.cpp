// arithmetic.cpp: + - * / of the bounded posit, correctly rounded
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The reference is the exact result rounded once into the bposit.  It is formed as
// B(double(a) op double(b)): the conversion is verified separately against an independent
// decoder (conversion.cpp), and the double step does not disturb the rounding.  A sum or
// product of these significands is exact in a double, and where a quotient is not, the
// double rounding is innocuous because 53 >= 2 s + 2 for an s-bit significand (Figueroa)
// -- s = fbits + 1 <= 25 for every configuration checked against this reference.
// bposit<64,6,5> has 57-bit significands, so it is checked by exact identities instead.
#include <universal/utility/directives.hpp>
#include <universal/number/bposit/bposit.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

enum class Op { add, sub, mul, div };

template<typename B>
bool check(Op op, const B& a, const B& b, bool report, int& fails) {
	B got, want;
	switch (op) {
	case Op::add: got = a + b; want = (a.isnar() || b.isnar()) ? B(sw::universal::SpecificValue::nar) : B(double(a) + double(b)); break;
	case Op::sub: got = a - b; want = (a.isnar() || b.isnar()) ? B(sw::universal::SpecificValue::nar) : B(double(a) - double(b)); break;
	case Op::mul: got = a * b; want = (a.isnar() || b.isnar()) ? B(sw::universal::SpecificValue::nar) : B(double(a) * double(b)); break;
	case Op::div: got = a / b; want = (a.isnar() || b.isnar() || b.iszero()) ? B(sw::universal::SpecificValue::nar) : B(double(a) / double(b)); break;
	}
	if (got == want) return true;
	++fails;
	if (report && fails < 10) {
		static const char* name[] = { "+", "-", "*", "/" };
		std::cerr << "FAIL " << sw::universal::type_tag(a) << ' ' << double(a) << ' ' << name[int(op)] << ' ' << double(b)
		          << " = " << double(got) << " (" << sw::universal::to_binary(got) << "), want " << double(want) << '\n';
	}
	return false;
}

// every pair of encodings, every operator
template<unsigned nbits, unsigned rs, unsigned es>
int VerifyExhaustive(bool report) {
	using B = sw::universal::bposit<nbits, rs, es, std::uint16_t>;
	static_assert(B::fbits + 1 <= 25, "the double reference needs 53 >= 2 s + 2");
	int fails = 0;
	const unsigned N = 1u << nbits;
	for (unsigned i = 0; i < N; ++i) {
		B a; a.setbits(i);
		for (unsigned j = 0; j < N; ++j) {
			B b; b.setbits(j);
			for (Op op : { Op::add, Op::sub, Op::mul, Op::div }) check(op, a, b, report, fails);
		}
	}
	return fails;
}

// random pairs, including operands near the extremes
template<unsigned nbits, unsigned rs, unsigned es, typename bt>
int VerifySampled(unsigned count, bool report) {
	using B = sw::universal::bposit<nbits, rs, es, bt>;
	static_assert(B::fbits + 1 <= 25, "the double reference needs 53 >= 2 s + 2");
	int fails = 0;
	std::uint64_t state = 0x2545F4914F6CDD1Dull;
	auto next = [&]() { state = state * 6364136223846793005ull + 1442695040888963407ull; B v; v.setbits(state >> (64 - nbits)); return v; };
	for (unsigned k = 0; k < count; ++k) {
		const B a = next(), b = next();
		for (Op op : { Op::add, Op::sub, Op::mul, Op::div }) check(op, a, b, report, fails);
	}
	return fails;
}

// bposit<64,6,5>: identities that need no reference
int VerifyIdentities64(bool report) {
	using B = sw::universal::bposit64;
	int fails = 0;
	const B one(1), zero(0), mone(-1);
	std::uint64_t state = 0xD1B54A32D192ED03ull;
	for (int k = 0; k < 100000; ++k) {
		state = state * 6364136223846793005ull + 1442695040888963407ull;
		B a; a.setbits(state);
		state = state * 6364136223846793005ull + 1442695040888963407ull;
		B b; b.setbits(state);
		if (a.isnar() || b.isnar()) continue;
		const bool ok = (a * one == a) && (a + zero == a) && (a / one == a) && ((a - a).iszero()) && (a * mone == -a)
		             && (a + b == b + a) && (a * b == b * a) && (a - b == -(b - a));
		if (!ok) { ++fails; if (report && fails < 10) std::cerr << "FAIL bposit64 identity at " << a.raw() << ", " << b.raw() << '\n'; }
	}
	// exact results stay exact: 2^-100 * 2^50 and a few integer products
	if (B::pow2(-100) * B::pow2(50) != B::pow2(-50)) { ++fails; if (report) std::cerr << "FAIL bposit64 power-of-two product\n"; }
	if (double(B(123456789) * B(1000)) != 123456789000.0) { ++fails; if (report) std::cerr << "FAIL bposit64 integer product\n"; }
	return fails;
}

}  // anonymous namespace

#define MANUAL_TESTING 0
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
	std::string test_suite  = "bposit arithmetic";
	std::string test_tag    = "arithmetic";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;
	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<8, 4, 0>(true), "bposit<8,4,0>", test_tag);
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else
#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<8, 4, 0>(reportTestCases), "bposit<8,4,0>", "exhaustive + - * /");
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<8, 3, 1>(reportTestCases), "bposit<8,3,1>", "exhaustive + - * /");
	nrOfFailedTestCases += ReportTestResult(VerifyIdentities64(reportTestCases), "bposit<64,6,5>", "identities");
#endif
#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<8, 2, 2>(reportTestCases), "bposit<8,2,2>", "exhaustive + - * /");
	nrOfFailedTestCases += ReportTestResult(VerifySampled<16, 6, 5, std::uint16_t>(100000, reportTestCases), "bposit<16,6,5>", "sampled + - * /");
	nrOfFailedTestCases += ReportTestResult(VerifySampled<16, 4, 2, std::uint16_t>(100000, reportTestCases), "bposit<16,4,2>", "sampled + - * /");
#endif
#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<10, 3, 2>(reportTestCases), "bposit<10,3,2>", "exhaustive + - * /");
	nrOfFailedTestCases += ReportTestResult(VerifySampled<32, 6, 5, std::uint32_t>(100000, reportTestCases), "bposit<32,6,5>", "sampled + - * /");
#endif
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught unexpected runtime error: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
