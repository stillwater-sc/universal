// test_tracked_statistical.cpp: verification of TrackedStatistical<T, Model> error estimation
//
// TrackedStatistical<T, Model> (include/sw/universal/utility/tracked_statistical.hpp) is the
// cheap tracker: instead of computing the rounding error, it accumulates a COUNT of ULPs,
// charging a fixed cost per operation and combining costs either in quadrature
// (ErrorModel::RandomWalk, errors assumed independent) or by summing them
// (ErrorModel::Linear, worst case).
//
// The contracts pinned here:
//   - the ULP helpers (ulp, ulp_distance, mantissa_bits) return the IEEE quantities
//   - combine_errors and add_operation_error implement exactly the two documented
//     formulas, and Linear is never below RandomWalk
//   - a fresh value costs nothing; one operation on fresh operands costs exactly half
//     a ULP under both models, and the models first diverge on the second operation
//   - error() is ulp_error() scaled by the ULP at the result
//   - operations() counts one per operation and sums the operand counts, while unary
//     minus and abs charge nothing
//   - subtraction that cancels is charged a magnified cost, capped at 500 ULP
//   - assignment from a raw value resets the tracking state
//
// #1545 moved this header's own ulp() into a detail namespace, where it no longer
// collides with sw::universal::ulp from native/ieee754_numeric.hpp. That collision made
// error() uncompilable in any translation unit that also used a number system, so this
// file includes one deliberately, as the regression for it.
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

#include <universal/native/ieee754.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/utility/tracked_statistical.hpp>
#include <universal/verification/test_suite.hpp>

// set to 1 to run the exploratory walk-through instead of the regression suite
#define MANUAL_TESTING 0

namespace {

	using namespace sw::universal;

	using Walk   = TrackedStatistical<double, ErrorModel::RandomWalk>;
	using Linear = TrackedStatistical<double, ErrorModel::Linear>;

	// ---- reporting helpers -----------------------------------------------------

	int expect_exact(double actual, double wanted, const char* what, bool reportTestCases) {
		if (actual == wanted) return 0;
		if (reportTestCases) {
			std::cout << "    FAIL " << what << ": got " << std::setprecision(17) << std::scientific
			          << actual << ", expected " << wanted << std::defaultfloat << '\n';
		}
		return 1;
	}

	int expect_count(std::size_t actual, std::size_t wanted, const char* what, bool reportTestCases) {
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

	// ---- the ULP helpers ---------------------------------------------------------

	// The ULP spacing at v, computed independently of the header under test.
	double spacing(double v) {
		return std::nextafter(v, std::numeric_limits<double>::infinity()) - v;
	}

	// #1545: this header and native/ieee754.hpp both used to define sw::universal::ulp,
	// and an unqualified call was ambiguous once both were visible. Both headers are
	// included above, so these calls not compiling would be the regression.
	int VerifyNoUlpAmbiguity(bool reportTestCases) {
		int fails = 0;
		fails += expect_exact(ulp(1.0), std::numeric_limits<double>::epsilon(),
			"the native ulp resolves", reportTestCases);
		// and a tracker used alongside a number system still reports its error
		Walk a = 1.0, b = 2.0;
		auto c = a + b;
		fails += expect_exact(c.error(), 0.5 * spacing(c.value()),
			"error() compiles and answers next to a number system header", reportTestCases);
		posit<32, 2> p = 1.0;   // the header whose presence used to break the build
		fails += expect_exact(double(p), 1.0, "and the number system still works", reportTestCases);
		return fails;
	}

	int VerifyUlpHelpers(bool reportTestCases) {
		int fails = 0;
		const double eps = std::numeric_limits<double>::epsilon();

		// The header's own ulp() cannot be CALLED from here -- see the INCLUDES note at
		// the top -- so it is exercised through error(), which is ulp_error() scaled by
		// the ULP at the result, against nextafter as the independent oracle.
		for (double v : { 1.0, 2.0, 0.5, 1024.0, 1e-10, 1e10 }) {
			Walk a = v, b = v;
			auto c = a + b;                       // one operation: exactly 0.5 ULP
			fails += expect_exact(c.ulp_error(), 0.5, "one operation costs half a ULP", reportTestCases);
			fails += expect_exact(c.error(), 0.5 * spacing(c.value()),
				"error is half the spacing at the result", reportTestCases);
		}
		fails += expect_exact(spacing(1.0), eps, "the spacing at 1.0 is the epsilon", reportTestCases);

		fails += expect_exact(ulp_distance(1.0, 1.0), 0.0, "a value is zero ULPs from itself",
			reportTestCases);
		fails += expect_exact(ulp_distance(1.0, 1.0 + eps), 1.0, "one ULP apart", reportTestCases);
		fails += expect_exact(ulp_distance(1.0, 1.0 + 4.0 * eps), 4.0, "four ULPs apart", reportTestCases);
		fails += expect_true(std::isinf(ulp_distance(1.0, std::numeric_limits<double>::infinity())),
			"a non-finite argument is infinitely far", reportTestCases);

		fails += expect_count(std::size_t(mantissa_bits<float>()),  23, "float mantissa bits", reportTestCases);
		fails += expect_count(std::size_t(mantissa_bits<double>()), 52, "double mantissa bits", reportTestCases);

		return fails;
	}

	// ---- the two error models ----------------------------------------------------

	int VerifyErrorModels(bool reportTestCases) {
		int fails = 0;

		// RandomWalk combines in quadrature, Linear by summing -- assert the formulas
		// directly on the public statics, with a Pythagorean triple so the RandomWalk
		// result is exact in binary.
		fails += expect_exact(Walk::combine_errors(3.0, 4.0, 0.0), 5.0,
			"RandomWalk combines in quadrature", reportTestCases);
		fails += expect_exact(Linear::combine_errors(3.0, 4.0, 0.0), 7.0,
			"Linear sums", reportTestCases);
		fails += expect_exact(Walk::combine_errors(0.0, 3.0, 4.0), 5.0,
			"the operation cost enters the quadrature", reportTestCases);
		fails += expect_exact(Linear::combine_errors(1.0, 2.0, 0.5), 3.5,
			"the operation cost enters the sum", reportTestCases);
		fails += expect_exact(Walk::add_operation_error(3.0, 4.0), 5.0,
			"RandomWalk unary combination", reportTestCases);
		fails += expect_exact(Linear::add_operation_error(3.0, 4.0), 7.0,
			"Linear unary combination", reportTestCases);

		// quadrature never exceeds the sum, for any non-negative inputs
		const double samples[] = { 0.0, 0.25, 0.5, 1.0, 2.5, 17.0 };
		for (double e1 : samples) {
			for (double e2 : samples) {
				if (!(Walk::combine_errors(e1, e2, 0.5) <= Linear::combine_errors(e1, e2, 0.5))) {
					++fails;
					if (reportTestCases) {
						std::cout << "    FAIL RandomWalk exceeded Linear at (" << e1 << ", " << e2 << ")\n";
					}
				}
			}
		}

		// the documented per-operation costs
		fails += expect_exact(Walk::ADD_COST,   0.5, "ADD_COST", reportTestCases);
		fails += expect_exact(Walk::MUL_COST,   0.5, "MUL_COST", reportTestCases);
		fails += expect_exact(Walk::DIV_COST,   0.5, "DIV_COST", reportTestCases);
		fails += expect_exact(Walk::SQRT_COST,  0.5, "SQRT_COST", reportTestCases);
		fails += expect_exact(Walk::TRANS_COST, 1.0, "TRANS_COST", reportTestCases);

		fails += expect_true(std::string(Walk::model_name()) == "RandomWalk", "RandomWalk model name",
			reportTestCases);
		fails += expect_true(std::string(Linear::model_name()) == "Linear", "Linear model name",
			reportTestCases);
		fails += expect_true(std::string(Walk::strategy_name()) == "Statistical", "strategy name",
			reportTestCases);

		return fails;
	}

	// ---- accumulation through operations -----------------------------------------

	int VerifyAccumulation(bool reportTestCases) {
		int fails = 0;

		// a fresh value has cost nothing yet
		Walk a = 1.0;
		fails += expect_exact(a.value(), 1.0, "fresh value", reportTestCases);
		fails += expect_exact(a.ulp_error(), 0.0, "fresh ULP error", reportTestCases);
		fails += expect_count(a.operations(), 0, "fresh operations", reportTestCases);
		fails += expect_true(a.is_exact(), "fresh is_exact", reportTestCases);
		fails += expect_exact(a.error(), 0.0, "fresh error", reportTestCases);
		fails += expect_exact(a.relative_error(), 0.0, "fresh relative_error", reportTestCases);
		fails += expect_exact(a.valid_bits(), double(mantissa_bits<double>()), "fresh valid_bits",
			reportTestCases);

		// one operation on fresh operands costs exactly half a ULP under either model
		{
			Walk b = 2.0;
			Linear la = 1.0, lb = 2.0;
			fails += expect_exact((a + b).ulp_error(), 0.5, "one addition, RandomWalk", reportTestCases);
			fails += expect_exact((la + lb).ulp_error(), 0.5, "one addition, Linear", reportTestCases);
			fails += expect_exact((a * b).ulp_error(), 0.5, "one multiplication", reportTestCases);
			fails += expect_exact((a / b).ulp_error(), 0.5, "one division", reportTestCases);
			fails += expect_true(!(a + b).is_exact(), "an operation ends exactness", reportTestCases);
			fails += expect_count((a + b).operations(), 1, "one operation counted", reportTestCases);
		}

		// the models diverge from the second operation onward
		{
			Walk x = 1.0, y = 2.0, z = 3.0;
			Linear lx = 1.0, ly = 2.0, lz = 3.0;
			const double walkTwo   = ((x + y) + z).ulp_error();
			const double linearTwo = ((lx + ly) + lz).ulp_error();
			fails += expect_exact(walkTwo, std::sqrt(0.5), "two additions in quadrature", reportTestCases);
			fails += expect_exact(linearTwo, 1.0, "two additions summed", reportTestCases);
			fails += expect_true(walkTwo < linearTwo, "quadrature grows more slowly", reportTestCases);
		}

		// Linear accumulation is exactly additive, so a chain is predictable
		{
			Linear sum = 0.0;
			for (int i = 0; i < 4; ++i) sum += Linear(1.0);
			// each step adds a fresh operand (0 error) plus ADD_COST
			fails += expect_exact(sum.ulp_error(), 4.0 * 0.5, "four Linear additions", reportTestCases);
			fails += expect_count(sum.operations(), 4, "four operations", reportTestCases);
		}

		// the ULP error never decreases through an operation
		{
			Walk acc = 1.0;
			double previous = acc.ulp_error();
			for (int i = 0; i < 10; ++i) {
				acc = acc + Walk(1.0);
				if (!(acc.ulp_error() >= previous)) {
					++fails;
					if (reportTestCases) std::cout << "    FAIL the ULP error decreased at step " << i << '\n';
				}
				previous = acc.ulp_error();
			}
		}

		// error() is the ULP count scaled by the ULP at the result
		{
			Walk p = 1024.0, q = 3.0;
			auto r = p + q;
			fails += expect_exact(r.error(), r.ulp_error() * spacing(r.value()),
				"error is the scaled ULP count", reportTestCases);
			fails += expect_exact(r.relative_error(), r.error() / std::abs(r.value()),
				"relative_error", reportTestCases);
		}

		return fails;
	}

	// ---- operation counting ------------------------------------------------------

	int VerifyOperationCounts(bool reportTestCases) {
		int fails = 0;

		Walk a = 2.0, b = 3.0, c = 5.0;
		fails += expect_count((a + b).operations(), 1, "one addition", reportTestCases);
		fails += expect_count((a - b).operations(), 1, "one subtraction", reportTestCases);
		fails += expect_count((a * b).operations(), 1, "one multiplication", reportTestCases);
		fails += expect_count((a / b).operations(), 1, "one division", reportTestCases);
		fails += expect_count(((a + b) + c).operations(), 2, "two chained additions", reportTestCases);
		fails += expect_count(((a + b) * (a + c)).operations(), 3, "two additions and a product",
			reportTestCases);

		// unary minus and abs charge nothing and leave the error alone
		{
			auto lossy = a + b;
			fails += expect_count((-lossy).operations(), lossy.operations(), "unary minus charges nothing",
				reportTestCases);
			fails += expect_exact((-lossy).ulp_error(), lossy.ulp_error(), "unary minus keeps the error",
				reportTestCases);
			fails += expect_count(abs(lossy).operations(), lossy.operations(), "abs charges nothing",
				reportTestCases);
			fails += expect_exact(abs(lossy).ulp_error(), lossy.ulp_error(), "abs keeps the error",
				reportTestCases);
		}

		// sqrt charges the sqrt cost; the transcendentals charge a full ULP
		{
			fails += expect_count(sqrt(a).operations(), 1, "sqrt counts one", reportTestCases);
			fails += expect_exact(sqrt(a).ulp_error(), Walk::SQRT_COST, "sqrt costs SQRT_COST",
				reportTestCases);
			fails += expect_count(exp(a).operations(), 1, "exp counts one", reportTestCases);
			fails += expect_exact(exp(a).ulp_error(), Walk::TRANS_COST, "exp costs TRANS_COST",
				reportTestCases);
			for (const char* tag : { "log", "sin", "cos" }) (void)tag;
			fails += expect_exact(log(a).ulp_error(), Walk::TRANS_COST, "log costs TRANS_COST",
				reportTestCases);
			fails += expect_exact(sin(a).ulp_error(), Walk::TRANS_COST, "sin costs TRANS_COST",
				reportTestCases);
			fails += expect_exact(cos(a).ulp_error(), Walk::TRANS_COST, "cos costs TRANS_COST",
				reportTestCases);
		}

		// binary exponentiation charges n operations for x^n
		fails += expect_count(pow(a, 2).operations(), 2, "pow(x,2)", reportTestCases);
		fails += expect_count(pow(a, 10).operations(), 10, "pow(x,10)", reportTestCases);
		fails += expect_count(pow(a, 1).operations(), 0, "pow(x,1) returns the base", reportTestCases);
		fails += expect_count(pow(a, -2).operations(), 3, "pow(x,-2) adds the reciprocal", reportTestCases);

		return fails;
	}

	// ---- subtraction charges for cancellation ------------------------------------
	//
	// The one place the model looks at the data: when a difference is tiny against its
	// operands, the ULP cost is magnified by the ratio, capped at 1000x.

	int VerifyCancellationCost(bool reportTestCases) {
		int fails = 0;

		// a difference of the same order as its operands is charged the flat cost
		{
			Walk a = 3.0, b = 1.0;
			auto d = a - b;
			fails += expect_exact(d.value(), 2.0, "3 - 1", reportTestCases);
			fails += expect_exact(d.ulp_error(), 0.5, "a harmless subtraction costs ADD_COST",
				reportTestCases);
		}

		// a difference 1e13 times smaller than its operands saturates the cap:
		// ADD_COST * 1000 = 500 ULP
		{
			Walk a = 1.0, b = 1.0 - 1e-13;
			auto d = a - b;
			fails += expect_exact(d.ulp_error(), 500.0, "catastrophic cancellation saturates the cap",
				reportTestCases);
			fails += expect_true(d.ulp_error() > (a - Walk(0.5)).ulp_error(),
				"cancellation costs more than an ordinary subtraction", reportTestCases);
		}

		// a moderate cancellation is charged proportionally, between the two extremes
		{
			Walk a = 1.0, b = 1.0 - 1e-3;
			auto d = a - b;
			fails += expect_true(d.ulp_error() > 0.5 && d.ulp_error() < 500.0,
				"a moderate cancellation is charged in between", reportTestCases);
		}

		return fails;
	}

	// ---- comparison under uncertainty ---------------------------------------------

	int VerifyUncertainComparison(bool reportTestCases) {
		int fails = 0;

		Walk a = 1.0, b = 2.0;
		// two fresh, distinct values carry no error, so they are definitely different
		fails += expect_true(a.definitely_different(b), "1 and 2 are definitely different", reportTestCases);
		fails += expect_true(!a.possibly_equal(b), "1 and 2 are not possibly equal", reportTestCases);
		// a value is never definitely different from itself
		fails += expect_true(!a.definitely_different(a), "a value equals itself", reportTestCases);
		fails += expect_true(a.possibly_equal(a), "a value is possibly equal to itself", reportTestCases);
		// the two predicates are exact complements
		const Walk samples[] = { Walk(1.0), Walk(2.0), Walk(1.0) + Walk(1e-16), Walk(0.0) };
		for (const Walk& x : samples) {
			for (const Walk& y : samples) {
				if (x.possibly_equal(y) == x.definitely_different(y)) {
					++fails;
					if (reportTestCases) std::cout << "    FAIL possibly_equal is not the complement\n";
				}
			}
		}

		return fails;
	}

	// ---- assignment resets the tracking state --------------------------------------

	int VerifyResetOnAssignment(bool reportTestCases) {
		int fails = 0;

		Walk a = 1.0;
		a += Walk(2.0);
		a *= Walk(3.0);
		fails += expect_true(a.ulp_error() > 0.0, "error accumulated before the reset", reportTestCases);
		fails += expect_count(a.operations(), 2, "operations accumulated before the reset", reportTestCases);

		a = 5.0;
		fails += expect_exact(a.value(), 5.0, "value after assignment", reportTestCases);
		fails += expect_exact(a.ulp_error(), 0.0, "error reset by assignment", reportTestCases);
		fails += expect_count(a.operations(), 0, "operations reset by assignment", reportTestCases);
		fails += expect_true(a.is_exact(), "is_exact after assignment", reportTestCases);

		// copy assignment carries the state instead
		Walk lossy = Walk(1.0) + Walk(2.0);
		Walk copy = 0.0;
		copy = lossy;
		fails += expect_exact(copy.ulp_error(), lossy.ulp_error(), "copy carries the error", reportTestCases);
		fails += expect_count(copy.operations(), lossy.operations(), "copy carries the operations",
			reportTestCases);

		return fails;
	}

	// ---- the validation helper -------------------------------------------------------

	int VerifyStatisticalValidation(bool reportTestCases) {
		int fails = 0;

		Walk a = 1.0, b = 1.0 / 3.0;
		auto c = a + b;
		// measured against its own value, the estimate is trivially conservative
		auto self = StatisticalValidation<double, ErrorModel::RandomWalk>::compute(c, c.value());
		fails += expect_exact(self.actual_error, 0.0, "no error against itself", reportTestCases);
		fails += expect_exact(self.estimated_ulps, c.ulp_error(), "the estimate is carried over",
			reportTestCases);
		fails += expect_true(self.conservative, "an estimate above zero is conservative", reportTestCases);

		// measured against a reference that is far off, the estimate is NOT conservative.
		// Doubling is exact in binary, so the gap is exactly the value itself.
		auto off = StatisticalValidation<double, ErrorModel::RandomWalk>::compute(c, 2.0 * c.value());
		fails += expect_exact(off.actual_error, c.value(), "the actual error is the gap to the reference",
			reportTestCases);
		fails += expect_true(!off.conservative, "an estimate below the truth is not conservative",
			reportTestCases);

		return fails;
	}

	// ---- the metrics, including the cases that used to lie -----------------------------

	int VerifyErrorMetrics(bool reportTestCases) {
		int fails = 0;

		// 1. valid_bits() is capped at the type's mantissa. Before #1546 it subtracted
		// log2(ulp_error), which is NEGATIVE below one ULP, so a single operation
		// reported 53 valid bits of a double -- one more than the type has, and more
		// than before the operation.
		{
			Walk a = 1.0, b = 2.0;
			auto c = a + b;
			fails += expect_exact(c.ulp_error(), 0.5, "one operation costs half a ULP", reportTestCases);
			fails += expect_exact(c.valid_bits(), double(mantissa_bits<double>()),
				"and leaves the type's precision, not more", reportTestCases);
			fails += expect_true(c.valid_bits() <= a.valid_bits(),
				"an operation never adds precision", reportTestCases);
			fails += expect_true(c.valid_bits() <= double(mantissa_bits<double>()),
				"valid_bits never exceeds the mantissa", reportTestCases);
		}

		// an error of several ULPs does cost bits
		{
			Walk acc = 1.0;
			for (int i = 0; i < 64; ++i) acc = acc + Walk(1.0);
			fails += expect_true(acc.ulp_error() > 1.0, "the error grew past a ULP", reportTestCases);
			fails += expect_true(acc.valid_bits() < double(mantissa_bits<double>()),
				"so the valid bits fell", reportTestCases);
			fails += expect_true(acc.valid_bits() >= 0.0, "and never went negative", reportTestCases);
		}

		// 2. Total cancellation is the WORST case, and is now charged as such. The
		// magnification branch used to skip result == 0 and fall through to the flat
		// ADD_COST, so losing every bit cost less than losing most of them.
		{
			Walk x = 1.0;
			auto total = x - x;
			Walk a = 1.0, b = 1.0 - 1e-13;
			auto partial = a - b;
			fails += expect_exact(total.value(), 0.0, "x - x is zero", reportTestCases);
			fails += expect_exact(total.ulp_error(), 500.0,
				"total cancellation is charged the capped cost", reportTestCases);
			fails += expect_true(total.ulp_error() >= partial.ulp_error(),
				"and is never cheaper than partial cancellation", reportTestCases);
		}

		// 3. Division by zero says so, instead of carrying an ordinary half-ULP estimate
		// alongside an infinity and reporting a NaN error with 53 valid bits.
		{
			Walk a = 1.0, zero = 0.0;
			auto q = a / zero;
			fails += expect_true(std::isinf(q.value()), "division by zero gives an infinity",
				reportTestCases);
			fails += expect_true(std::isinf(q.ulp_error()), "the ULP error is unbounded",
				reportTestCases);
			fails += expect_true(std::isinf(q.error()), "so is the absolute error", reportTestCases);
			fails += expect_true(!std::isnan(q.error()), "and it is not a NaN", reportTestCases);
			fails += expect_exact(q.valid_bits(), 0.0, "an infinity has no valid bits", reportTestCases);
		}

		// a zero value with an accumulated error has no relative accuracy to report
		{
			Walk z(0.0, 4.0, 2);
			fails += expect_true(!std::isfinite(z.relative_error()),
				"a zero carrying error has no relative error", reportTestCases);
		}

		return fails;
	}

	// ---- the exploratory narrative, kept for manual inspection -------------------------

#if MANUAL_TESTING
	void ReportTrackedStatisticalBehaviour() {
		std::cout << "\n=== TrackedStatistical<double> walk-through ===\n";

		std::cout << "ULP of a few values:\n";
		for (double v : { 1.0, 2.0, 0.5, 1e10, 1e-10 }) {
			std::cout << "  spacing(" << v << ") = " << spacing(v) << "\n";
		}

		std::cout << "\nthe two models over a chain of 100 additions:\n";
		Walk w = 0.0;
		Linear l = 0.0;
		for (int i = 0; i < 100; ++i) { w += Walk(0.1); l += Linear(0.1); }
		std::cout << "  RandomWalk: "; w.report(std::cout);
		std::cout << "  Linear:     "; l.report(std::cout);

		std::cout << "\ncatastrophic cancellation, 1.0 - (1.0 - 1e-13):\n";
		auto d = Walk(1.0) - Walk(1.0 - 1e-13);
		d.report(std::cout);
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

	std::string test_suite  = "TrackedStatistical<T, Model> ULP-count error estimation";
	std::string test_tag    = "tracked_statistical";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	ReportTrackedStatisticalBehaviour();
	nrOfFailedTestCases += VerifyErrorModels(true);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;  // ignore failures
#else  // !MANUAL_TESTING

	// A few hundred operations, so it all belongs at level 1: CI configures
	// REGRESSION_LEVEL_1 only, and a contract that is not checked there does not gate.
#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyUlpHelpers(reportTestCases), test_tag, "ULP helpers");
	nrOfFailedTestCases += ReportTestResult(VerifyErrorModels(reportTestCases), test_tag, "error models");
	nrOfFailedTestCases += ReportTestResult(VerifyAccumulation(reportTestCases), test_tag, "accumulation");
	nrOfFailedTestCases += ReportTestResult(VerifyOperationCounts(reportTestCases), test_tag, "operation counts");
	nrOfFailedTestCases += ReportTestResult(VerifyCancellationCost(reportTestCases), test_tag, "cancellation cost");
	nrOfFailedTestCases += ReportTestResult(VerifyUncertainComparison(reportTestCases), test_tag, "uncertain comparison");
	nrOfFailedTestCases += ReportTestResult(VerifyResetOnAssignment(reportTestCases), test_tag, "reset on assignment");
	nrOfFailedTestCases += ReportTestResult(VerifyStatisticalValidation(reportTestCases), test_tag, "validation helper");
	nrOfFailedTestCases += ReportTestResult(VerifyErrorMetrics(reportTestCases), test_tag, "error metrics");
	nrOfFailedTestCases += ReportTestResult(VerifyNoUlpAmbiguity(reportTestCases), test_tag, "no ulp ambiguity");
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
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
