// test_tracked_shadow.cpp: verification of TrackedShadow<T> error tracking
//
// TrackedShadow<T> (include/sw/universal/utility/tracked_shadow.hpp) tracks error for
// types that have no exact error decomposition -- posit above all -- by carrying a
// higher-precision shadow value alongside the tracked one. Every operation is computed
// twice, once in T and once in the shadow type, and the reported error is the gap
// between them.
//
// The contracts pinned here:
//   - error() is exactly |shadow - double(value)|, including the rounding a value
//     picks up merely by being CONSTRUCTED in T
//   - the value follows T's arithmetic and the shadow follows the shadow type's,
//     operation for operation, so neither chain may be computed from the other
//   - an operation that T represents exactly reports exactly zero error
//   - the shadow is a faithful reference: it stays within double rounding of an
//     independent double-double evaluation of the same expression
//   - operations() counts one per operation and sums the operand counts; the math
//     functions each count one
//   - absorption is counted in SHADOW space, at the shadow type's digits/2 threshold
//   - assignment from a raw value resets the tracking state
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>

#include <universal/number/posit/posit.hpp>
#include <universal/number/dd/dd.hpp>
#include <universal/utility/tracked_shadow.hpp>
#include <universal/verification/test_suite.hpp>

// set to 1 to run the exploratory walk-through instead of the regression suite
#define MANUAL_TESTING 0

namespace {

	using namespace sw::universal;

	using Posit32 = posit<32, 2, std::uint32_t>;
	using Posit16 = posit<16, 1, std::uint16_t>;
	using Posit8  = posit<8, 0, std::uint8_t>;

	// ---- reporting helpers -----------------------------------------------------

	int expect_exact(double actual, double wanted, const char* what, bool reportTestCases) {
		if (actual == wanted) return 0;
		if (reportTestCases) {
			std::cout << "    FAIL " << what << ": got " << std::setprecision(17) << std::scientific
			          << actual << ", expected " << wanted << std::defaultfloat << '\n';
		}
		return 1;
	}

	int expect_count(std::uint64_t actual, std::uint64_t wanted, const char* what, bool reportTestCases) {
		if (actual == wanted) return 0;
		if (reportTestCases) {
			std::cout << "    FAIL " << what << ": got " << actual << ", expected " << wanted << '\n';
		}
		return 1;
	}

	int expect_true(bool actual, const char* what, bool reportTestCases) {
		if (actual) return 0;
		if (reportTestCases) std::cout << "    FAIL " << what << '\n';
		return 1;
	}

	// ---- construction ----------------------------------------------------------
	//
	// A shadow tracker starts reporting error before any arithmetic happens: a value
	// that T cannot represent is already off by its conversion rounding.

	int VerifyConstruction(bool reportTestCases) {
		int fails = 0;

		TrackedShadow<Posit32> zero;
		fails += expect_exact(double(zero.value()), 0.0, "default ctor value", reportTestCases);
		fails += expect_exact(double(zero.shadow()), 0.0, "default ctor shadow", reportTestCases);
		fails += expect_exact(zero.error(), 0.0, "default ctor error", reportTestCases);
		fails += expect_count(zero.operations(), 0, "default ctor operations", reportTestCases);
		fails += expect_count(zero.absorptions(), 0, "default ctor absorptions", reportTestCases);
		fails += expect_true(zero.is_exact(), "default ctor is_exact", reportTestCases);

		// a value posit represents exactly carries no error
		for (double v : { 1.0, 1.5, 0.25, -2.0, 16.0 }) {
			TrackedShadow<Posit32> a = v;
			fails += expect_exact(double(a.value()), v, "exactly representable value", reportTestCases);
			fails += expect_exact(a.shadow(), v, "exactly representable shadow", reportTestCases);
			fails += expect_exact(a.error(), 0.0, "exactly representable error", reportTestCases);
			fails += expect_true(a.is_exact(), "exactly representable is_exact", reportTestCases);
		}

		// a value it cannot represent is already wrong by its conversion rounding,
		// and the tracker says so before a single operation is performed
		{
			const double v = 0.1;
			TrackedShadow<Posit8> a = v;
			const double rounded = double(Posit8(v));
			fails += expect_exact(a.shadow(), v, "shadow keeps the requested value", reportTestCases);
			fails += expect_exact(double(a.value()), rounded, "value is the rounded encoding", reportTestCases);
			fails += expect_exact(a.error(), std::abs(v - rounded), "construction error is the conversion gap",
				reportTestCases);
			fails += expect_true(!a.is_exact(), "rounded construction is not exact", reportTestCases);
			fails += expect_true(a.error() > 0.0, "posit<8,0> cannot represent 0.1", reportTestCases);
			fails += expect_count(a.operations(), 0, "construction is not an operation", reportTestCases);
		}

		return fails;
	}

	// ---- the two chains stay independent ---------------------------------------
	//
	// The whole design rests on the value following T's arithmetic while the shadow
	// follows the shadow type's. An implementation that derived one from the other --
	// shadowing the rounded value, say -- would report zero error everywhere, so each
	// operator is checked against both chains computed by hand.

	int VerifyBothChains(bool reportTestCases) {
		int fails = 0;

		const double da = 1.0 / 3.0, db = 7.0 / 11.0;
		const Posit16 pa = da, pb = db;
		const double sa = double(pa), sb = double(pb);  // the shadow keeps the double values
		(void)sa; (void)sb;

		TrackedShadow<Posit16> a = da, b = db;

		struct Chain {
			TrackedShadow<Posit16> tracked;
			Posit16 value;
			double  shadow;
			const char* tag;
		};
		const Chain chains[] = {
			{ a + b, Posit16(pa + pb), da + db, "addition" },
			{ a - b, Posit16(pa - pb), da - db, "subtraction" },
			{ a * b, Posit16(pa * pb), da * db, "multiplication" },
			{ a / b, Posit16(pa / pb), da / db, "division" },
		};
		for (const Chain& c : chains) {
			fails += expect_true(c.tracked.value() == c.value, c.tag, reportTestCases);
			fails += expect_exact(c.tracked.shadow(), c.shadow, c.tag, reportTestCases);
			// and the reported error is exactly the gap between the two chains
			fails += expect_exact(c.tracked.error(), std::abs(c.shadow - double(c.value)),
				"error is the gap between the chains", reportTestCases);
			fails += expect_count(c.tracked.operations(), 1, "one operation", reportTestCases);
		}

		// the same holds through a longer expression
		{
			auto r = (a + b) * (a - b);
			const Posit16 pv = Posit16(pa + pb) * Posit16(pa - pb);
			const double  sv = (da + db) * (da - db);
			fails += expect_true(r.value() == pv, "compound expression value", reportTestCases);
			fails += expect_exact(r.shadow(), sv, "compound expression shadow", reportTestCases);
			fails += expect_exact(r.error(), std::abs(sv - double(pv)), "compound expression error", reportTestCases);
			fails += expect_count(r.operations(), 3, "compound expression operations", reportTestCases);
		}

		// unary minus mirrors both chains and is not an operation
		{
			auto lossy = a + b;
			auto neg = -lossy;
			fails += expect_true(neg.value() == -lossy.value(), "unary minus value", reportTestCases);
			fails += expect_exact(neg.shadow(), -lossy.shadow(), "unary minus shadow", reportTestCases);
			fails += expect_exact(neg.error(), lossy.error(), "unary minus preserves the error", reportTestCases);
			fails += expect_count(neg.operations(), lossy.operations(), "unary minus is not an operation",
				reportTestCases);
		}

		// abs mirrors both chains
		{
			auto lossy = -(a + b);
			auto m = abs(lossy);
			fails += expect_true(m.value() == -lossy.value(), "abs value", reportTestCases);
			fails += expect_exact(m.shadow(), -lossy.shadow(), "abs shadow", reportTestCases);
			fails += expect_exact(m.error(), lossy.error(), "abs preserves the error", reportTestCases);
		}

		return fails;
	}

	// ---- what the type represents exactly costs nothing --------------------------

	int VerifyExactPositArithmetic(bool reportTestCases) {
		int fails = 0;

		// posit<32,2> represents small integers and dyadic fractions exactly, so these
		// chains must report no error at all
		struct Case { double a, b; };
		const Case cases[] = { {1.0, 2.0}, {3.0, 5.0}, {1.5, 0.25}, {16.0, -4.0}, {0.5, 0.125} };
		for (const Case& c : cases) {
			TrackedShadow<Posit32> a = c.a, b = c.b;
			auto sum  = a + b;
			auto diff = a - b;
			auto prod = a * b;
			fails += expect_exact(sum.error(),  0.0, "exact posit addition", reportTestCases);
			fails += expect_exact(diff.error(), 0.0, "exact posit subtraction", reportTestCases);
			fails += expect_exact(prod.error(), 0.0, "exact posit multiplication", reportTestCases);
			fails += expect_exact(double(sum.value()),  c.a + c.b, "exact posit sum value", reportTestCases);
			fails += expect_exact(double(prod.value()), c.a * c.b, "exact posit product value", reportTestCases);
			fails += expect_true(sum.is_exact() && diff.is_exact() && prod.is_exact(),
				"exact posit ops report is_exact", reportTestCases);
		}

		// and a quotient that lands on a representable value is exact too
		{
			TrackedShadow<Posit32> a = 3.0, b = 4.0;
			auto q = a / b;
			fails += expect_exact(double(q.value()), 0.75, "3/4 in posit<32,2>", reportTestCases);
			fails += expect_exact(q.error(), 0.0, "3/4 is exact", reportTestCases);
		}

		// 1/3 is not representable: the tracker must report the gap, and it must be
		// smaller than the gap a 16-bit posit leaves
		{
			TrackedShadow<Posit32> wide = 1.0;
			auto q32 = wide / TrackedShadow<Posit32>(3.0);
			TrackedShadow<Posit16> narrow = 1.0;
			auto q16 = narrow / TrackedShadow<Posit16>(3.0);
			fails += expect_true(q32.error() > 0.0, "1/3 is not exact in posit<32,2>", reportTestCases);
			fails += expect_true(q16.error() > q32.error(), "posit<16,1> is coarser than posit<32,2>",
				reportTestCases);
			fails += expect_true(q32.valid_bits() > q16.valid_bits(),
				"more error leaves fewer valid bits", reportTestCases);
		}

		return fails;
	}

	// ---- the shadow is a faithful reference --------------------------------------
	//
	// The error is only meaningful if the shadow really is the better answer. Both a
	// dot product and an accumulation are evaluated independently in double-double;
	// the shadow must agree with that to within double rounding, while the posit value
	// is allowed to be far off.

	int VerifyShadowIsFaithful(bool reportTestCases) {
		int fails = 0;
		const int n = 100;

		TrackedShadow<Posit16> dot = 0.0;
		dd reference = 0.0;
		for (int i = 0; i < n; ++i) {
			const double ai = 1.0 / (i + 1);
			const double bi = 1.0 / (i + 2);
			dot += TrackedShadow<Posit16>(ai) * TrackedShadow<Posit16>(bi);
			reference += dd(ai) * dd(bi);
		}

		const double ref = double(reference);
		const double shadowGap = std::abs(dot.shadow() - ref);
		// the shadow runs in double: a few hundred roundings leave it far below 1e-12
		if (!(shadowGap < 1e-12)) {
			++fails;
			if (reportTestCases) {
				std::cout << "    FAIL shadow drifted from the double-double reference by "
				          << std::scientific << shadowGap << std::defaultfloat << '\n';
			}
		}
		// the posit value, by contrast, is visibly off, and error() measures exactly that
		fails += expect_exact(dot.error(), std::abs(dot.shadow() - double(dot.value())),
			"error is the shadow-to-value gap", reportTestCases);
		if (!(dot.error() > 1e-6)) {
			++fails;
			if (reportTestCases) {
				std::cout << "    FAIL posit<16,1> dot product looks implausibly accurate: error "
				          << std::scientific << dot.error() << std::defaultfloat << '\n';
			}
		}
		fails += expect_count(dot.operations(), std::uint64_t(2 * n), "dot product operations", reportTestCases);

		// an accumulation whose terms fall below the running sum's resolution
		{
			TrackedShadow<Posit16> sum = 0.0;
			dd ref2 = 0.0;
			for (int i = 0; i < 1000; ++i) {
				sum += 0.001;
				ref2 += dd(0.001);
			}
			fails += expect_true(std::abs(sum.shadow() - double(ref2)) < 1e-12,
				"accumulated shadow tracks the reference", reportTestCases);
			fails += expect_exact(sum.error(), std::abs(sum.shadow() - double(sum.value())),
				"accumulated error is the shadow-to-value gap", reportTestCases);
			fails += expect_count(sum.operations(), 1000, "accumulation operations", reportTestCases);
		}

		return fails;
	}

	// ---- operation counting ------------------------------------------------------

	int VerifyOperationCounts(bool reportTestCases) {
		int fails = 0;

		TrackedShadow<Posit32> a = 2.0, b = 3.0, c = 5.0;
		fails += expect_count((a + b).operations(), 1, "one addition", reportTestCases);
		fails += expect_count((a * b).operations(), 1, "one multiplication", reportTestCases);
		fails += expect_count((a / b).operations(), 1, "one division", reportTestCases);
		fails += expect_count(((a + b) + c).operations(), 2, "two chained additions", reportTestCases);
		fails += expect_count(((a + b) * (a + c)).operations(), 3, "two additions and a product", reportTestCases);

		// each math function counts as one operation on top of its argument's count
		fails += expect_count(sqrt(a + b).operations(), 2, "addition then sqrt", reportTestCases);
		fails += expect_count(exp(a + b).operations(), 2, "addition then exp", reportTestCases);
		fails += expect_count(log(a + b).operations(), 2, "addition then log", reportTestCases);
		fails += expect_count(sin(a + b).operations(), 2, "addition then sin", reportTestCases);
		fails += expect_count(cos(a + b).operations(), 2, "addition then cos", reportTestCases);
		fails += expect_count(pow(a + b, 2).operations(), 2, "addition then pow", reportTestCases);
		// sqr is a multiplication of the value with itself, so it doubles the count and adds one
		fails += expect_count(sqr(a + b).operations(), 3, "addition then sqr", reportTestCases);

		return fails;
	}

	// ---- math functions run in both chains ---------------------------------------

	int VerifyMathFunctions(bool reportTestCases) {
		int fails = 0;

		TrackedShadow<Posit32> x = 2.0;

		{
			auto r = sqrt(x);
			fails += expect_true(r.value() == sw::universal::sqrt(Posit32(2.0)), "sqrt value", reportTestCases);
			fails += expect_exact(r.shadow(), std::sqrt(2.0), "sqrt shadow", reportTestCases);
			fails += expect_exact(r.error(), std::abs(std::sqrt(2.0) - double(r.value())), "sqrt error",
				reportTestCases);
			// sqrt(2) is irrational, so no finite posit represents it
			fails += expect_true(r.error() > 0.0, "sqrt(2) is not exact", reportTestCases);
		}
		{
			// sqrt of a perfect square stays exact in both chains
			TrackedShadow<Posit32> four = 4.0;
			auto r = sqrt(four);
			fails += expect_exact(double(r.value()), 2.0, "sqrt(4) value", reportTestCases);
			fails += expect_exact(r.shadow(), 2.0, "sqrt(4) shadow", reportTestCases);
			fails += expect_exact(r.error(), 0.0, "sqrt(4) is exact", reportTestCases);
		}
		{
			auto r = exp(TrackedShadow<Posit32>(1.0));
			fails += expect_exact(r.shadow(), std::exp(1.0), "exp shadow", reportTestCases);
			fails += expect_exact(r.error(), std::abs(std::exp(1.0) - double(r.value())), "exp error",
				reportTestCases);
		}
		{
			auto r = pow(TrackedShadow<Posit32>(1.5), 3);
			fails += expect_exact(r.shadow(), std::pow(1.5, 3), "pow shadow", reportTestCases);
			// 1.5^3 = 3.375 is dyadic, so posit<32,2> holds it exactly
			fails += expect_exact(double(r.value()), 3.375, "pow value", reportTestCases);
			fails += expect_exact(r.error(), 0.0, "pow of a dyadic base is exact", reportTestCases);
		}

		return fails;
	}

	// ---- absorption is detected in shadow space ----------------------------------
	//
	// detect_absorption compares the SHADOW operands, so the threshold is the shadow
	// type's digits/2. posit<32,2> shadows in double: 26.5 bits.

	int VerifyAbsorptionThreshold(bool reportTestCases) {
		int fails = 0;

		struct Case { double b; bool absorbed; const char* tag; };
		const Case cases[] = {
			{ 0.5,                  false, "1.0 + 0.5" },
			{ std::ldexp(1.0, -26), false, "1.0 + 2^-26 (below the threshold)" },
			{ std::ldexp(1.0, -27), true,  "1.0 + 2^-27 (above the threshold)" },
			{ std::ldexp(1.0, -40), true,  "1.0 + 2^-40" },
			{ 1.0,                  false, "1.0 + 1.0 (equal magnitude)" },
		};
		for (const Case& c : cases) {
			TrackedShadow<Posit32> a = 1.0, b = c.b;
			auto sum = a + b;
			fails += expect_count(sum.absorptions(), c.absorbed ? 1u : 0u, c.tag, reportTestCases);
			fails += expect_true(sum.had_absorption() == c.absorbed, "had_absorption agrees with the count",
				reportTestCases);
		}

		// counts accumulate, and multiplication carries them without adding
		{
			TrackedShadow<Posit32> sum = 1.0;
			for (int i = 0; i < 5; ++i) sum += std::ldexp(1.0, -40);
			fails += expect_count(sum.absorptions(), 5, "five absorbed additions", reportTestCases);
			auto scaled = sum * TrackedShadow<Posit32>(2.0);
			fails += expect_count(scaled.absorptions(), 5, "multiplication carries the count", reportTestCases);
			fails += expect_count(sqrt(sum).absorptions(), 5, "sqrt carries the count", reportTestCases);
		}

		return fails;
	}

	// ---- assignment resets the tracking state ------------------------------------

	int VerifyResetOnAssignment(bool reportTestCases) {
		int fails = 0;

		TrackedShadow<Posit16> a = 1.0;
		a /= TrackedShadow<Posit16>(3.0);
		for (int i = 0; i < 3; ++i) a += std::ldexp(1.0, -40);
		fails += expect_true(a.error() > 0.0, "error accumulated before the reset", reportTestCases);
		fails += expect_true(a.operations() > 0, "operations accumulated before the reset", reportTestCases);
		fails += expect_true(a.absorptions() > 0, "absorptions accumulated before the reset", reportTestCases);

		a = 2.0;
		fails += expect_exact(double(a.value()), 2.0, "value after assignment", reportTestCases);
		fails += expect_exact(a.shadow(), 2.0, "shadow after assignment", reportTestCases);
		fails += expect_exact(a.error(), 0.0, "error reset by assignment", reportTestCases);
		fails += expect_count(a.operations(), 0, "operations reset by assignment", reportTestCases);
		fails += expect_count(a.absorptions(), 0, "absorptions reset by assignment", reportTestCases);

		// copy assignment carries the state instead
		TrackedShadow<Posit16> lossy = TrackedShadow<Posit16>(1.0) / TrackedShadow<Posit16>(3.0);
		TrackedShadow<Posit16> copy = 0.0;
		copy = lossy;
		fails += expect_exact(copy.error(), lossy.error(), "copy carries the error", reportTestCases);
		fails += expect_count(copy.operations(), lossy.operations(), "copy carries the operations", reportTestCases);

		return fails;
	}

	// ---- the TrackedPosit alias ---------------------------------------------------

	int VerifyTrackedPositAlias(bool reportTestCases) {
		int fails = 0;

		static_assert(std::is_same_v<TrackedPosit<32, 2, std::uint32_t>, TrackedShadow<Posit32>>,
			"TrackedPosit must alias TrackedShadow over the same posit");

		TrackedPosit<32, 2, std::uint32_t> a = 1.0, b = 2.0;
		auto c = a + b;
		fails += expect_exact(double(c.value()), 3.0, "alias addition value", reportTestCases);
		fails += expect_exact(c.error(), 0.0, "alias addition error", reportTestCases);
		fails += expect_count(c.operations(), 1, "alias addition operations", reportTestCases);

		return fails;
	}

	// ---- the exploratory narrative, kept for manual inspection --------------------

#if MANUAL_TESTING
	void ReportTrackedShadowBehaviour() {
		std::cout << "\n=== TrackedShadow<posit<32,2>> walk-through ===\n";

		Posit32 px = 3.14159265358979;
		TrackedShadow<Posit32> x = px;
		auto y = x * x;
		auto z = sqrt(y);
		std::cout << to_binary(px) << " : x         = " << px << '\n';
		std::cout << to_binary(y.value()) << " : x^2       = " << y.value() << '\n';
		std::cout << to_binary(z.value()) << " : sqrt(x^2) = " << z.value() << '\n';
		z.report(std::cout);

		std::cout << "\n1/3 and (1/3)*3 in posit<32,2>:\n";
		auto q = TrackedShadow<Posit32>(1.0) / TrackedShadow<Posit32>(3.0);
		q.report(std::cout);
		auto back = q * TrackedShadow<Posit32>(3.0);
		back.report(std::cout);

		std::cout << "\n1000 additions of 0.001 in posit<16,1>:\n";
		TrackedShadow<Posit16> sum = 0.0;
		for (int i = 0; i < 1000; ++i) sum += 0.001;
		sum.report(std::cout);
	}
#endif  // MANUAL_TESTING

}  // anonymous namespace

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

	std::string test_suite  = "TrackedShadow<T> shadow-reference error tracking";
	std::string test_tag    = "tracked_shadow";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	ReportTrackedShadowBehaviour();
	nrOfFailedTestCases += VerifyBothChains(true);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;  // ignore failures
#else  // !MANUAL_TESTING

	// The whole suite is a few thousand posit operations, so it all belongs at level 1:
	// CI configures REGRESSION_LEVEL_1 only, and a contract that is not checked there
	// does not gate.
#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyConstruction(reportTestCases), test_tag, "construction");
	nrOfFailedTestCases += ReportTestResult(VerifyBothChains(reportTestCases), test_tag, "value and shadow chains");
	nrOfFailedTestCases += ReportTestResult(VerifyExactPositArithmetic(reportTestCases), test_tag, "exact posit arithmetic");
	nrOfFailedTestCases += ReportTestResult(VerifyShadowIsFaithful(reportTestCases), test_tag, "shadow fidelity");
	nrOfFailedTestCases += ReportTestResult(VerifyOperationCounts(reportTestCases), test_tag, "operation counts");
	nrOfFailedTestCases += ReportTestResult(VerifyMathFunctions(reportTestCases), test_tag, "math functions");
	nrOfFailedTestCases += ReportTestResult(VerifyAbsorptionThreshold(reportTestCases), test_tag, "absorption threshold");
	nrOfFailedTestCases += ReportTestResult(VerifyResetOnAssignment(reportTestCases), test_tag, "reset on assignment");
	nrOfFailedTestCases += ReportTestResult(VerifyTrackedPositAlias(reportTestCases), test_tag, "TrackedPosit alias");
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
	std::cerr << "Caught unexpected universal arithmetic exception: " << err.what() << std::endl;
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
