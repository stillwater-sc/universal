// sqrt_precision.cpp: sqrt must deliver the format's precision, not the host's
//
// cfloat's sqrt was `cfloat(std::sqrt((double)a))` -- in BOTH branches of the
// CFLOAT_NATIVE_SQRT switch, the first marked "// TBD" and identical to the second, with
// the macro defaulting to 0 in three separate headers. The result therefore carried 53
// bits of significand whatever the format asked for: 53 of 112 for quad, 53 of 236 for
// octo, silently. Arguments outside a double's exponent range were worse than imprecise,
// because (double)a overflowed to infinity before std::sqrt was ever called, so
// sqrt(maxpos) returned inf (#1589).
//
// The oracle here does not use sqrt. A root is correct exactly when squaring it returns
// the argument, so the residual a - x*x is computed in a WIDER cfloat than the one under
// test and scored against an ulp of that format. Nothing the implementation produces is
// compared against another call to the same implementation.
//
// "Wider" has to mean wider in the exponent too, not only the fraction. Squaring the
// root of maxpos lands back at maxpos, so an oracle carrying the same es overflows to
// infinity at precisely the argument that matters here.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <string>

#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_suite.hpp>

// set to 1 to print the residual for each case instead of running the regression suite
#define MANUAL_TESTING 0

namespace {

	using namespace sw::universal;

	int expect_true(bool actual, const std::string& what, bool reportTestCases) {
		if (actual) return 0;
		if (reportTestCases) std::cout << "    FAIL " << what << '\n';
		return 1;
	}

	// How many bits of the argument the root actually reproduces, measured by squaring it
	// in a wider format. Returns a large number when the residual is exactly zero.
	template<typename C, typename Wider>
	double reproducedBits(const C& a) {
		C r = sqrt(a);
		if (r.isnan() || r.isinf()) return -1.0;
		Wider wa(a), wr(r);
		Wider residual = wa - wr * wr;
		if (residual.iszero()) return 1.0e9;              // exact
		// The relative residual is formed in the wider cfloat rather than by converting
		// to a double. An argument of 2^1500 is inf as a double, so double(residual) /
		// double(wa) is inf/inf -- the oracle would report nan for exactly the magnitudes
		// this test exists to cover. scale() is floor(log2|v|), which is the bit count.
		Wider rel = residual / wa;
		if (rel.iszero()) return 1.0e9;
		return -static_cast<double>(rel.scale());
	}

	// sqrt must reproduce the argument to within an ulp of the FORMAT, not of a double.
	template<typename C, typename Wider>
	int VerifyPrecision(const std::string& tag, bool reportTestCases) {
		int fails = 0;
		constexpr unsigned fbits = C::fbits;
		// one bit of slack for the rounding of the final iterate
		const double required = static_cast<double>(fbits) - 1.0;

		for (double d : { 2.0, 3.0, 5.0, 7.0, 10.0, 0.5, 0.25, 1.0e10, 1.0e-10, 1.0e100, 1.0e-100 }) {
			C a(d);
			if (a.iszero() || a.isinf()) continue;
			const double bits = reproducedBits<C, Wider>(a);
			fails += expect_true(bits >= required,
				tag + ": sqrt(" + std::to_string(d) + ") reproduces "
				+ std::to_string(bits) + " bits, needs " + std::to_string(required), reportTestCases);
		}

		// across the exponent range, including magnitudes a double cannot hold
		for (int e = -2000; e <= 2000; e += 250) {
			C p; p = 1.0;
			if (!p.setexponent(e)) continue;
			for (double m : { 1.0, 1.5, 3.0 }) {
				C a = C(m) * p;
				if (a.iszero() || a.isinf() || a.isdenormal()) continue;
				const double bits = reproducedBits<C, Wider>(a);
				fails += expect_true(bits >= required,
					tag + ": sqrt(" + std::to_string(m) + " * 2^" + std::to_string(e) + ") reproduces "
					+ std::to_string(bits) + " bits", reportTestCases);
			}
		}

		return fails;
	}

	// The arguments where the old implementation did not merely lose precision but
	// returned a wrong value outright.
	template<typename C, typename Wider>
	int VerifyOutsideDoubleRange(const std::string& tag, bool reportTestCases) {
		int fails = 0;

		C maxpos(SpecificValue::maxpos);
		C root = sqrt(maxpos);
		// (double)maxpos is inf for these formats, so the old path returned inf
		fails += expect_true(!root.isinf(), tag + ": sqrt(maxpos) is finite", reportTestCases);
		fails += expect_true(!root.isnan(), tag + ": sqrt(maxpos) is a number", reportTestCases);
		fails += expect_true(root.scale() == (maxpos.scale() / 2) || root.scale() == (maxpos.scale() / 2) + 1,
			tag + ": sqrt(maxpos) has half the scale", reportTestCases);
		fails += expect_true(reproducedBits<C, Wider>(maxpos) >= static_cast<double>(C::fbits) - 1.0,
			tag + ": sqrt(maxpos) carries the format's precision", reportTestCases);

		C minpos(SpecificValue::minpos);
		C mroot = sqrt(minpos);
		fails += expect_true(!mroot.iszero(), tag + ": sqrt(minpos) is not zero", reportTestCases);
		fails += expect_true(!mroot.isnan(), tag + ": sqrt(minpos) is a number", reportTestCases);

		return fails;
	}

	// The special encodings, which must not go near the iteration
	template<typename C>
	int VerifySpecialValues(const std::string& tag, bool reportTestCases) {
		int fails = 0;
		C zero(0.0), inf(SpecificValue::infpos), nan(SpecificValue::qnan);

		fails += expect_true(sqrt(zero).iszero(),  tag + ": sqrt(+0) is +0", reportTestCases);
		fails += expect_true(sqrt(inf).isinf(),    tag + ": sqrt(+inf) is +inf", reportTestCases);
		fails += expect_true(sqrt(nan).isnan(),    tag + ": sqrt(nan) is nan", reportTestCases);
		{
			// isneg() is true for -0, so an implementation that tests the sign before the
			// zero returns NaN here. IEEE 754 requires -0, with the sign preserved.
			C negZero(0.0); negZero.setsign(true);
			C r = sqrt(negZero);
			fails += expect_true(r.iszero(), tag + ": sqrt(-0) is zero", reportTestCases);
			fails += expect_true(r.sign(),   tag + ": sqrt(-0) keeps the sign", reportTestCases);
			// -inf must NOT follow -0 through that door: sqrt(-inf) is NaN
			C negInf(SpecificValue::infneg);
			fails += expect_true(sqrt(negInf).isnan(), tag + ": sqrt(-inf) is nan", reportTestCases);
			C negative(-4.0);
			fails += expect_true(sqrt(negative).isnan(), tag + ": sqrt(-4) is nan", reportTestCases);
		}
		fails += expect_true(sqrt(C(1.0)) == C(1.0), tag + ": sqrt(1) is exactly 1", reportTestCases);
		fails += expect_true(sqrt(C(4.0)) == C(2.0), tag + ": sqrt(4) is exactly 2", reportTestCases);
		fails += expect_true(sqrt(C(0.25)) == C(0.5), tag + ": sqrt(0.25) is exactly 0.5", reportTestCases);

		return fails;
	}

	// A format whose fraction a double holds entirely must still match the host exactly,
	// because for those the host answer is already the best the format can carry.
	int VerifyNarrowFormatsUnchanged(bool reportTestCases) {
		using F32 = cfloat<32, 8, std::uint32_t, true, false, false>;
		using F64 = cfloat<64, 11, std::uint64_t, true, false, false>;
		int fails = 0;

		for (double d : { 2.0, 3.0, 0.5, 1.0e10, 1.0e-10, 1.0e100, 1.234567890123456 }) {
			F64 b(d);
			fails += expect_true(double(sqrt(b)) == std::sqrt(double(b)),
				"cfloat<64,11>: sqrt matches the host for " + std::to_string(d), reportTestCases);
			F32 a(d);
			fails += expect_true(double(sqrt(a)) == double(F32(std::sqrt(double(a)))),
				"cfloat<32,8>: sqrt matches the host for " + std::to_string(d), reportTestCases);
		}
		return fails;
	}

}  // anonymous namespace

#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	// The oracle type needs a wider EXPONENT as well as a wider fraction. Squaring the
	// root of maxpos lands back at maxpos, so an oracle with the same es overflows to
	// infinity at exactly the argument this test exists to cover -- which is how the
	// first version of this suite reported a failure against a correct implementation.
	// es < 21 is a hard limit in cfloat_impl.hpp, so octo's oracle gets 20, which is
	// just enough to square 2^262143.
	using Quad  = cfloat<128, 15, std::uint64_t, true, false, false>;
	using QuadW = cfloat<192, 17, std::uint64_t, true, false, false>;
#if MANUAL_TESTING || REGRESSION_LEVEL_2 || REGRESSION_LEVEL_3
	// used in manual mode and at levels 2 and 3; clang warns -Wunused-local-typedef
	// when a build enables none of them, which the level-1-only CI config does
	using Octo  = cfloat<256, 19, std::uint64_t, true, false, false>;
	using OctoW = cfloat<320, 20, std::uint64_t, true, false, false>;
#endif
	// fbits = 48, so a guard that keys on the fraction width alone routes this to the
	// host -- but es = 15 puts its maxpos past DBL_MAX and its minpos below DBL_MIN, so
	// the host answers inf and zero. Whether the host can answer is a property of the
	// VALUE, not of the fraction width.
	using NarrowWide  = cfloat<64, 15, std::uint64_t, true, false, false>;
	using NarrowWideW = cfloat<128, 17, std::uint64_t, true, false, false>;
#if REGRESSION_LEVEL_2
	// only exercised at level 2; clang warns -Wunused-local-typedef when it is not
	using Xtndd  = cfloat<80, 11, std::uint64_t, true, false, false>;
	using XtnddW = cfloat<144, 13, std::uint64_t, true, false, false>;
#endif

	std::string test_suite  = "cfloat sqrt precision (#1589)";
	std::string test_tag    = "sqrt precision";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	std::cout << "bits of the argument reproduced by squaring the root:\n";
	std::cout << "  quad sqrt(2)      : " << reproducedBits<Quad, QuadW>(Quad(2.0)) << " of 112\n";
	std::cout << "  quad sqrt(3)      : " << reproducedBits<Quad, QuadW>(Quad(3.0)) << " of 112\n";
	std::cout << "  octo sqrt(2)      : " << reproducedBits<Octo, OctoW>(Octo(2.0)) << " of 236\n";
	{
		Quad mp(SpecificValue::maxpos);
		std::cout << "  quad sqrt(maxpos) : " << (sqrt(mp).isinf() ? "inf" : "finite")
		          << ", scale " << sqrt(mp).scale() << " (argument scale " << mp.scale() << ")\n";
	}
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifySpecialValues<Quad>("quad", reportTestCases),
		test_tag, "special values");
	nrOfFailedTestCases += ReportTestResult(VerifyNarrowFormatsUnchanged(reportTestCases),
		test_tag, "narrow formats unchanged");
	nrOfFailedTestCases += ReportTestResult(VerifyPrecision<Quad, QuadW>("quad", reportTestCases),
		test_tag, "quad carries 112 bits");
	nrOfFailedTestCases += ReportTestResult(VerifyOutsideDoubleRange<Quad, QuadW>("quad", reportTestCases),
		test_tag, "quad outside double's range");
	nrOfFailedTestCases += ReportTestResult(
		VerifyOutsideDoubleRange<NarrowWide, NarrowWideW>("cfloat<64,15>", reportTestCases),
		test_tag, "narrow fraction, wide exponent");
	nrOfFailedTestCases += ReportTestResult(VerifySpecialValues<NarrowWide>("cfloat<64,15>", reportTestCases),
		test_tag, "narrow fraction special values");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyPrecision<Xtndd, XtnddW>("xtndd", reportTestCases),
		test_tag, "xtndd carries 68 bits");
	nrOfFailedTestCases += ReportTestResult(VerifyOutsideDoubleRange<Xtndd, XtnddW>("xtndd", reportTestCases),
		test_tag, "xtndd outside double's range");
	nrOfFailedTestCases += ReportTestResult(VerifyPrecision<Octo, OctoW>("octo", reportTestCases),
		test_tag, "octo carries 236 bits");
	nrOfFailedTestCases += ReportTestResult(VerifyOutsideDoubleRange<Octo, OctoW>("octo", reportTestCases),
		test_tag, "octo outside double's range");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifySpecialValues<Octo>("octo", reportTestCases),
		test_tag, "octo special values");
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
