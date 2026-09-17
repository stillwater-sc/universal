// test_tracked.cpp: verification of the unified Tracked<T> interface
//
// Tracked<T> (include/sw/universal/utility/tracked.hpp) is the front door to error
// tracking: it picks a strategy from error_tracking_traits<T> and inherits the matching
// implementation, so a caller writes Tracked<float> or Tracked<posit<32,2>> without
// knowing which machinery runs underneath. The header also supplies the two wrappers
// for types that track uncertainty natively: TrackedAreal (areal's ubit) and
// TrackedInterval (interval's bounds).
//
// The contracts pinned here:
//   - the strategy each type selects, and the implementation it inherits, checked at
//     COMPILE time so a mis-routed type cannot reach a test at all
//   - an explicit strategy argument overrides the default and reaches the right class
//   - Tracked<T> really delegates: the numbers it reports are the ones the underlying
//     tracker reports, not a reimplementation
//   - TrackedAreal reads areal's ubit: an exact encoding reports no error, an uncertain
//     one reports the width to the next encoding
//   - TrackedInterval reports the interval width as its error, encloses the true result,
//     and never narrows through arithmetic
//   - both wrappers count one operation per operation and sum the operand counts
//
// One check below pins behaviour that is wrong (#1547); it says so, so that a fix is
// noticed here rather than silently changing what callers see.
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

#include <universal/number/areal/areal.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/interval/interval.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/utility/tracked.hpp>
#include <universal/verification/test_suite.hpp>

// set to 1 to run the exploratory walk-through instead of the regression suite
#define MANUAL_TESTING 0

namespace {

	using namespace sw::universal;

	using Posit32   = posit<32, 2, std::uint32_t>;
	using CFloat32  = cfloat<32, 8, std::uint32_t, true, false, false>;
	using Areal32   = areal<32, 8>;
	using Interval  = interval<double>;

	// ---- compile-time contract: which strategy, which implementation ---------------
	//
	// Tracked<T> is a thin facade over one of five classes. If the selection ever
	// changed -- a trait edited, a specialisation shadowed -- every runtime check below
	// would still pass while callers silently got a different tracker. These pin it.

	// the default strategy each type selects
	static_assert(Tracked<float>::strategy()    == ErrorStrategy::Exact,    "float defaults to Exact");
	static_assert(Tracked<double>::strategy()   == ErrorStrategy::Exact,    "double defaults to Exact");
	static_assert(Tracked<Posit32>::strategy()  == ErrorStrategy::Shadow,   "posit defaults to Shadow");
	static_assert(Tracked<CFloat32>::strategy() == ErrorStrategy::Shadow,   "cfloat defaults to Shadow");
	static_assert(Tracked<Areal32>::strategy()  == ErrorStrategy::Inherent, "areal tracks natively");
	static_assert(Tracked<Interval>::strategy() == ErrorStrategy::Inherent, "interval tracks natively");

	// the implementation each one inherits
	static_assert(std::is_base_of_v<TrackedExact<float>,     Tracked<float>>,     "float uses TrackedExact");
	static_assert(std::is_base_of_v<TrackedExact<double>,    Tracked<double>>,    "double uses TrackedExact");
	static_assert(std::is_base_of_v<TrackedShadow<Posit32>,  Tracked<Posit32>>,   "posit uses TrackedShadow");
	static_assert(std::is_base_of_v<TrackedShadow<CFloat32>, Tracked<CFloat32>>,  "cfloat uses TrackedShadow");
	static_assert(std::is_base_of_v<TrackedAreal<Areal32>,   Tracked<Areal32>>,   "areal uses TrackedAreal");
	static_assert(std::is_base_of_v<TrackedInterval<double>, Tracked<Interval>>,  "interval uses TrackedInterval");

	// an explicit strategy overrides the default
	static_assert(std::is_base_of_v<TrackedShadow<double>,      Tracked<double, ErrorStrategy::Shadow>>,
		"an explicit Shadow strategy reaches TrackedShadow");
	static_assert(std::is_base_of_v<TrackedBounded<double>,     Tracked<double, ErrorStrategy::Bounded>>,
		"an explicit Bounded strategy reaches TrackedBounded");
	static_assert(std::is_base_of_v<TrackedStatistical<double>, Tracked<double, ErrorStrategy::Statistical>>,
		"an explicit Statistical strategy reaches TrackedStatistical");
	static_assert(std::is_base_of_v<TrackedExact<double>,       Tracked<double, ErrorStrategy::Exact>>,
		"an explicit Exact strategy reaches TrackedExact");
	static_assert(Tracked<double, ErrorStrategy::Bounded>::strategy() == ErrorStrategy::Bounded,
		"the override is reported back");

	// the traits the selection reads
	static_assert(error_tracking_traits<float>::has_exact_errors,      "float has exact errors");
	static_assert(!error_tracking_traits<Posit32>::has_exact_errors,   "posit has no exact errors");
	static_assert(error_tracking_traits<Areal32>::tracks_uncertainty,  "areal tracks uncertainty");
	static_assert(error_tracking_traits<Interval>::is_interval_type,   "interval is an interval type");

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

	// ---- the strategy names reported at run time -----------------------------------

	int VerifyStrategyNames(bool reportTestCases) {
		int fails = 0;

		fails += expect_true(std::string(Tracked<float>::strategy_name())    == "Exact",    "float", reportTestCases);
		fails += expect_true(std::string(Tracked<double>::strategy_name())   == "Exact",    "double", reportTestCases);
		fails += expect_true(std::string(Tracked<Posit32>::strategy_name())  == "Shadow",   "posit", reportTestCases);
		fails += expect_true(std::string(Tracked<CFloat32>::strategy_name()) == "Shadow",   "cfloat", reportTestCases);
		fails += expect_true(std::string(Tracked<Areal32>::strategy_name())  == "Inherent", "areal", reportTestCases);
		fails += expect_true(std::string(Tracked<Interval>::strategy_name()) == "Inherent", "interval", reportTestCases);
		fails += expect_true(std::string(Tracked<double, ErrorStrategy::Bounded>::strategy_name()) == "Bounded",
			"an override reports its own name", reportTestCases);

		// and the free function agrees with the class
		fails += expect_true(std::string(strategy_name(ErrorStrategy::Statistical)) == "Statistical",
			"the free strategy_name", reportTestCases);

		return fails;
	}

	// ---- Tracked<T> delegates rather than reimplements --------------------------------
	//
	// The facade must report exactly what the underlying tracker reports. Each case runs
	// the same computation through Tracked<T> and through the implementation class and
	// requires the answers to agree bit for bit.

	int VerifyDelegation(bool reportTestCases) {
		int fails = 0;

		// double: the two_sum residual of 1.0 + 1e-16 is the whole addend
		{
			Tracked<double> a = 1.0, b = 1e-16;
			auto viaFacade = a + b;
			TrackedExact<double> ea = 1.0, eb = 1e-16;
			auto viaClass = ea + eb;
			fails += expect_exact(viaFacade.value(), viaClass.value(), "double value", reportTestCases);
			fails += expect_exact(viaFacade.error(), viaClass.error(), "double error", reportTestCases);
			fails += expect_exact(viaFacade.error(), 1e-16, "double error is the residual", reportTestCases);
			fails += expect_count(viaFacade.operations(), viaClass.operations(), "double operations",
				reportTestCases);
		}

		// posit: the shadow gap
		{
			Tracked<Posit32> a = 1.0, b = 3.0;
			auto viaFacade = a / b;
			TrackedShadow<Posit32> sa = 1.0, sb = 3.0;
			auto viaClass = sa / sb;
			fails += expect_true(viaFacade.value() == viaClass.value(), "posit value", reportTestCases);
			fails += expect_exact(viaFacade.error(), viaClass.error(), "posit error", reportTestCases);
			fails += expect_true(viaFacade.error() > 0.0, "1/3 is not exact in a posit", reportTestCases);
		}

		// cfloat, which the traits also route to the shadow tracker
		{
			Tracked<CFloat32> a = 1.0, b = 3.0;
			auto q = a / b;
			fails += expect_true(q.error() > 0.0, "1/3 is not exact in a cfloat", reportTestCases);
			fails += expect_count(q.operations(), 1, "cfloat operations", reportTestCases);
			// a cfloat<32,8> mirrors IEEE single precision, so the gap is a float's
			fails += expect_exact(q.error(), std::abs(1.0 / 3.0 - double(float(1.0f / 3.0f))),
				"the cfloat gap is the float gap", reportTestCases);
		}

		// an explicit override really changes the machinery: the bounded tracker reports
		// bounds where the exact tracker reports a residual. What those bounds are for a
		// result that ROUNDS is up to the optimizer -- see the note at the top of
		// test_tracked_bounded.cpp, and #1544 -- so this checks the wiring, on operands whose own
		// width settles the answer.
		{
			Tracked<double, ErrorStrategy::Bounded> a(1.0, 2.0), b(10.0, 20.0);
			auto sum = a + b;
			fails += expect_true(sum.lo() <= 11.0 && 22.0 <= sum.hi(),
				"the bounded override encloses the sum", reportTestCases);
			fails += expect_exact(sum.error(), sum.radius(), "the bounded override reports a radius",
				reportTestCases);
			fails += expect_count(sum.operations(), 1, "the bounded override counts the operation",
				reportTestCases);
		}

		return fails;
	}

	// ---- TrackedAreal reads the ubit ---------------------------------------------------

	int VerifyTrackedAreal(bool reportTestCases) {
		int fails = 0;

		// an exactly representable value has ubit 0 and nothing to report
		{
			Tracked<Areal32> a = 1.0;
			fails += expect_true(Areal32(1.0).ubit() == false, "1.0 is exact in areal<32,8>", reportTestCases);
			fails += expect_true(a.is_exact(), "an exact encoding is exact", reportTestCases);
			fails += expect_exact(a.error(), 0.0, "an exact encoding has no error", reportTestCases);
			fails += expect_exact(a.relative_error(), 0.0, "and no relative error", reportTestCases);
			fails += expect_count(a.operations(), 0, "construction is not an operation", reportTestCases);
			// areal's valid_bits uses nbits as its precision proxy
			fails += expect_exact(a.valid_bits(), 32.0, "an exact areal reports nbits valid bits",
				reportTestCases);
		}

		// a value between two encodings sets the ubit, and the reported error is the
		// width to the next encoding
		{
			const double v = 0.1;
			Tracked<Areal32> a = v;
			Areal32 raw = v;
			fails += expect_true(raw.ubit(), "0.1 is uncertain in areal<32,8>", reportTestCases);
			fails += expect_true(!a.is_exact(), "an uncertain encoding is not exact", reportTestCases);
			Areal32 next = raw;
			++next;
			fails += expect_exact(a.error(), std::abs(double(next) - double(raw)),
				"the error is the width to the next encoding", reportTestCases);
			fails += expect_true(a.error() > 0.0, "an uncertain value has a positive error", reportTestCases);
			fails += expect_true(a.valid_bits() < 32.0, "and fewer than nbits valid bits", reportTestCases);
		}

		// operation counting
		{
			Tracked<Areal32> a = 1.0, b = 0.5, c = 0.25;
			fails += expect_count((a + b).operations(), 1, "one addition", reportTestCases);
			fails += expect_count(((a + b) * c).operations(), 2, "an addition and a product", reportTestCases);
			fails += expect_count((-(a + b)).operations(), 1, "unary minus charges nothing", reportTestCases);
			// the value follows areal's own arithmetic
			fails += expect_true((a + b).value() == Areal32(1.0) + Areal32(0.5), "the value is areal's",
				reportTestCases);
		}

		// comparisons forward to the underlying value
		{
			Tracked<Areal32> a = 1.0, b = 2.0;
			fails += expect_true(a < b && b > a && a != b, "ordering forwards to areal", reportTestCases);
			fails += expect_true(a == Tracked<Areal32>(1.0), "equality forwards to areal", reportTestCases);
		}

		return fails;
	}

	// ---- TrackedInterval reports the width ----------------------------------------------

	int VerifyTrackedInterval(bool reportTestCases) {
		int fails = 0;

		// a value with bounds reports its width as the error
		{
			Tracked<Interval> y(0.99, 1.01);
			fails += expect_true(!y.is_exact(), "a proper interval is not exact", reportTestCases);
			fails += expect_exact(y.error(), double(Interval(0.99, 1.01).width()),
				"the error is the interval width", reportTestCases);
			fails += expect_true(y.error() >= 1.01 - 0.99, "the width is not understated", reportTestCases);
			fails += expect_exact(y.midpoint(), 1.0, "the midpoint of [0.99,1.01]", reportTestCases);
		}

		// arithmetic encloses the true result and never narrows
		{
			Tracked<Interval> x(1.0, 1.0), y(0.99, 1.01);
			auto sum = x + y;
			fails += expect_true(double(sum.value().lower()) <= 1.99 && 2.01 <= double(sum.value().upper()),
				"the sum encloses [1.99, 2.01]", reportTestCases);
			fails += expect_true(sum.error() >= y.error(), "the sum is no narrower than its operand",
				reportTestCases);
			fails += expect_exact(sum.midpoint(), 2.0, "the midpoint of the sum", reportTestCases);
			fails += expect_count(sum.operations(), 1, "one addition", reportTestCases);

			auto prod = y * y;
			fails += expect_true(double(prod.value().lower()) <= 0.9801 && 1.0201 <= double(prod.value().upper()),
				"the square encloses [0.9801, 1.0201]", reportTestCases);
			fails += expect_true(prod.error() > y.error(), "squaring widens", reportTestCases);
			fails += expect_count(prod.operations(), 1, "one multiplication", reportTestCases);
			fails += expect_count(((x + y) * y).operations(), 2, "an addition and a product", reportTestCases);
		}

		// valid_bits falls as the interval widens
		{
			Tracked<Interval> tight(0.9999, 1.0001), loose(0.9, 1.1);
			fails += expect_true(tight.valid_bits() > loose.valid_bits(),
				"a tighter interval leaves more valid bits", reportTestCases);
			fails += expect_true(loose.valid_bits() < 53.0, "a loose interval is well below full precision",
				reportTestCases);
		}

		return fails;
	}

	// ---- behaviour that is pinned although it is arguably wrong ---------------------------

	int VerifyKnownDefects(bool reportTestCases) {
		int fails = 0;

		// A degenerate interval [v,v] is exact -- is_exact() says so -- yet error()
		// reports the smallest subnormal rather than zero, because interval::width()
		// rounds its result outward unconditionally and nextafter(0) is denorm_min.
		// Harmless in magnitude, but it means "exact" and "zero error" disagree, and a
		// caller testing error() == 0 to detect an exact value never sees one.
		Tracked<Interval> point(1.0, 1.0);
		fails += expect_true(point.is_exact(), "a degenerate interval is exact", reportTestCases);
		fails += expect_exact(point.error(), std::numeric_limits<double>::denorm_min(),
			"but its error is one subnormal, not zero (known defect #1547)", reportTestCases);
		fails += expect_exact(point.valid_bits(), 53.0,
			"valid_bits takes the is_exact path and reports full precision", reportTestCases);

		return fails;
	}

	// ---- the exploratory narrative, kept for manual inspection -----------------------------

#if MANUAL_TESTING
	void ReportTrackedBehaviour() {
		std::cout << "\n=== Tracked<T> across the number systems ===\n";

		std::cout << "\nTracked<double> (Exact), 1.0 + 1e-15:\n";
		auto d = Tracked<double>(1.0) + Tracked<double>(1e-15);
		d.report(std::cout);

		std::cout << "\nTracked<posit<32,2>> (Shadow), sqrt(pi^2):\n";
		Tracked<Posit32> x = 3.14159265358979;
		auto z = sqrt(x * x);
		z.report(std::cout);

		std::cout << "\nTracked<areal<32,8>> (Inherent), 1.0 + 0.1:\n";
		Tracked<Areal32> a = 1.0, b = 0.1;
		auto s = a + b;
		s.report(std::cout);

		std::cout << "\nTracked<interval<double>> (Inherent), [1,1] + [0.99,1.01]:\n";
		Tracked<Interval> p(1.0, 1.0), q(0.99, 1.01);
		auto r = p + q;
		r.report(std::cout);
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

	std::string test_suite  = "Tracked<T> unified error tracking interface";
	std::string test_tag    = "tracked";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	ReportTrackedBehaviour();
	nrOfFailedTestCases += VerifyDelegation(true);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;  // ignore failures
#else  // !MANUAL_TESTING

	// The strategy selection is enforced by the static_asserts above; these runtime
	// checks cover the behaviour. It all belongs at level 1: CI configures
	// REGRESSION_LEVEL_1 only, and a contract that is not checked there does not gate.
#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyStrategyNames(reportTestCases), test_tag, "strategy names");
	nrOfFailedTestCases += ReportTestResult(VerifyDelegation(reportTestCases), test_tag, "delegation");
	nrOfFailedTestCases += ReportTestResult(VerifyTrackedAreal(reportTestCases), test_tag, "TrackedAreal");
	nrOfFailedTestCases += ReportTestResult(VerifyTrackedInterval(reportTestCases), test_tag, "TrackedInterval");
	nrOfFailedTestCases += ReportTestResult(VerifyKnownDefects(reportTestCases), test_tag, "pinned known defects");
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
