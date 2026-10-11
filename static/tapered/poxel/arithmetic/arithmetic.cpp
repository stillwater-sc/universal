// arithmetic.cpp: poxel arithmetic, sticky-flag semantics
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// poxel arithmetic computes on the operands' lower endpoints and places the exact result
// in its tile; the ubit is set if that result is inexact OR either operand was an open
// tile (areal's semantics, #1631).  Verified exhaustively for small configurations:
//   - exact operands: the result tile CONTAINS the exact result -- exactly when it is a
//     lattice value, strictly inside an open tile otherwise -- and the ubit says which
//   - an open operand: the ubit is always set (the flag is sticky)
//   - NaR propagates; division by exact zero is NaR
// The reference value is formed in a double: for these lattices a sum, difference or
// product is exact there, and a quotient that is not a lattice value lies far more than a
// double ulp from every lattice value, so the containment check is exact.
//
// Sampled at 16 and 32 bits (poxel17, poxel33), exact operands only, where a double is no
// longer exact: the reference is ereal, in which the sums and products of the endpoints
// are exact, and a quotient is checked through lo * b < a < hi * b.  The same operation in
// posit<nbits - 1, es>, which rounds to nearest, must land on one end of the poxel's tile.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <cstdint>
#include <random>
#include <string>
#include <universal/number/poxel/poxel.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

enum class Op { add, sub, mul, div };

template<unsigned nbits, unsigned es>
int VerifyExhaustive(bool report) {
	using X = sw::universal::poxel<nbits, es, std::uint16_t>;
	int fails = 0;
	auto fail = [&](const std::string& what) { ++fails; if (report && fails < 10) std::cerr << "FAIL " << sw::universal::type_tag(X{}) << ": " << what << '\n'; };
	const unsigned N = 1u << nbits;
	for (unsigned i = 0; i < N; ++i) {
		X a; a.setbits(i);
		for (unsigned j = 0; j < N; ++j) {
			X b; b.setbits(j);
			for (Op op : { Op::add, Op::sub, Op::mul, Op::div }) {
				X r;
				switch (op) { case Op::add: r = a + b; break; case Op::sub: r = a - b; break; case Op::mul: r = a * b; break; case Op::div: r = a / b; break; }
				const std::string where = std::to_string(i) + (op == Op::add ? " + " : op == Op::sub ? " - " : op == Op::mul ? " * " : " / ") + std::to_string(j);
				if (a.isnar() || b.isnar() || (op == Op::div && b.iszero())) { if (!r.isnar()) fail(where + " must be NaR"); continue; }
				if (op == Op::div && b.lattice() == 0) continue;                       // (0, minpos) divisor: its representative is 0
				if (!a.isexact() || !b.isexact()) { if (r.isexact() && !r.isnar()) fail(where + ": an open operand must leave the ubit set"); continue; }
				// both exact: the result tile contains the exact result
				const double x = double(a), y = double(b);
				double v = 0.0;
				switch (op) { case Op::add: v = x + y; break; case Op::sub: v = x - y; break; case Op::mul: v = x * y; break; case Op::div: v = x / y; break; }
				const bool exact_in_double = (op != Op::div) || (v * y == x);
				const double lo = r.template lower<double>(), hi = r.template upper<double>();
				if (r.isexact()) {                                  // must BE the exact result
					if (!(exact_in_double && lo == v)) fail(where + " is marked exact but is not the exact result");
				}
				else if (!(lo < v && v < hi)) {                      // must strictly contain it
					fail(where + " = " + std::to_string(v) + " is not strictly inside its open tile");
				}
			}
		}
	}
	return fails;
}

// the wider standard tiles: identities and the flag on representative operations
template<typename X>
int VerifyIdentities(bool report) {
	int fails = 0;
	auto expect = [&](bool ok, const char* what) { if (!ok) { ++fails; if (report) std::cerr << "FAIL " << sw::universal::type_tag(X{}) << ' ' << what << '\n'; } };
	const X one(1), two(2), three(3), seven(7);
	expect((two + three) == X(5) && (two + three).isexact(), "2 + 3 = 5, exactly");
	expect((seven * three) == X(21) && (seven * three).isexact(), "7 * 3 = 21, exactly");
	const X third = one / three;
	expect(!third.isexact() && third.template lower<double>() < 1.0 / 3.0 && 1.0 / 3.0 < third.template upper<double>(), "1/3 is an open tile containing 1/3");
	expect(!(third * three).isexact(), "(1/3) * 3 keeps the ubit: the flag is sticky");
	expect((X(0.5) - X(0.5)).iszero(), "0.5 - 0.5 is exactly zero");
	expect(-(-third) == third && abs(-third) == third, "negation and abs of an open tile");
	return fails;
}

// exact operands drawn at random from the whole lattice
template<unsigned nbits, unsigned es, typename bt>
int VerifySampled(unsigned samples, bool report) {
	using X = sw::universal::poxel<nbits, es, bt>;
	using P = sw::universal::posit<nbits - 1, es, bt>;
	using E = sw::universal::ereal<8>;                       // exact for these sums and products
	std::mt19937_64 rng(0x1631u + nbits);
	int fails = 0;
	auto exact_tile = [&]() {
		X x;
		do { x.setbits(rng() << 1); } while (x.isnar());      // ubit clear: a lattice point
		return x;
	};
	for (unsigned i = 0; i < samples; ++i) {
		const X a = exact_tile(), b = exact_tile();
		const double x = double(a), y = double(b);
		for (Op op : { Op::add, Op::sub, Op::mul, Op::div }) {
			const std::string where = std::to_string(x) + (op == Op::add ? " + " : op == Op::sub ? " - " : op == Op::mul ? " * " : " / ") + std::to_string(y);
			auto fail = [&](const std::string& what) { ++fails; if (report && fails < 10) std::cerr << "FAIL " << sw::universal::type_tag(X{}) << ' ' << where << ": " << what << '\n'; };
			X r;
			P pr(0);
			switch (op) {
			case Op::add: r = a + b; pr = P(a) + P(b); break;
			case Op::sub: r = a - b; pr = P(a) - P(b); break;
			case Op::mul: r = a * b; pr = P(a) * P(b); break;
			case Op::div: r = a / b; pr = P(a) / P(b); break;
			}
			if (op == Op::div && b.iszero()) { if (!r.isnar()) fail("division by zero must be NaR"); continue; }
			if (r.isnar()) { fail("NaR from finite operands"); continue; }
			const double lo = r.template lower<double>(), hi = r.template upper<double>();
			// the exact result v against the tile: sign of v - e for an endpoint e
			auto cmp = [&](double e) -> int {
				E d;
				if (op == Op::div) {                                    // v - e has the sign of (x - e y) / y
					d = E(x) - E(e) * E(y);
					if (y < 0.0) d = -d;
				}
				else {
					const E v = (op == Op::add) ? E(x) + E(y) : (op == Op::sub) ? E(x) - E(y) : E(x) * E(y);
					d = v - E(e);
				}
				return d.iszero() ? 0 : (d < E(0.0) ? -1 : 1);
			};
			if (r.isexact()) {
				if (cmp(lo) != 0) fail("marked exact but is not the exact result");
			}
			else {
				if (!(std::isinf(lo) || cmp(lo) > 0) || !(std::isinf(hi) || cmp(hi) < 0)) fail("the exact result is not strictly inside the open tile");
			}
			// posit rounds to nearest: one of the tile's two ends
			X up(r);
			if (!r.isexact()) ++up;                                    // the point tile that ends it
			if (!(pr == P(r)) && !(pr == P(up))) fail("posit's rounding is not an end of the tile");
		}
	}
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
	std::string test_suite  = "poxel arithmetic";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;
	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<8, 0>(true), "poxel<8,0>", "exhaustive");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else
#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<8, 0>(reportTestCases), "poxel<8,0>", "exhaustive + - * /");
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<9, 2>(reportTestCases), "poxel<9,2>", "exhaustive + - * /");
	nrOfFailedTestCases += ReportTestResult(VerifyIdentities<poxel17>(reportTestCases), "poxel17", "identities");
	nrOfFailedTestCases += ReportTestResult(VerifyIdentities<poxel33>(reportTestCases), "poxel33", "identities");
	nrOfFailedTestCases += ReportTestResult(VerifySampled<17, 2, std::uint32_t>(20000, reportTestCases), "poxel17", "sampled + - * /");
	nrOfFailedTestCases += ReportTestResult(VerifySampled<33, 2, std::uint64_t>(20000, reportTestCases), "poxel33", "sampled + - * /");
#endif
#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<10, 1>(reportTestCases), "poxel<10,1>", "exhaustive + - * /");
	nrOfFailedTestCases += ReportTestResult(VerifySampled<17, 2, std::uint32_t>(100000, reportTestCases), "poxel17", "sampled + - * /");
	nrOfFailedTestCases += ReportTestResult(VerifySampled<33, 2, std::uint64_t>(100000, reportTestCases), "poxel33", "sampled + - * /");
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
