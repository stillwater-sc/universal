// addition_oracle.cpp: exact dyadic validation of quad-double cascade (qd_cascade) addition
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// universal#1317: qd_cascade sqrt/exp/log were 13-15 decimal digits worse than qd. The cause was
// not in any of those functions: expansion_ops::add_cascades() produced a sum whose value was
// exact but whose components overlapped, and the renormalization that compresses 8 components
// back to 4 silently dropped the fourth. Roughly one addition in eight lost its entire fourth
// component - a relative error of 2^-160 where the format carries 2^-212 - and everything built
// on addition inherited it.
//
// The existing suites did not catch this because they check self-consistency: a result that is
// normalized, that round-trips, and that matches the other implementation of the same flawed
// algorithm passes them all. This suite is different: the reference is exact dyadic-rational
// arithmetic (dyadic_exact.hpp, backed by einteger), which shares no code with the expansion
// arithmetic under test.
//
// Every quad-double is a sum of four doubles, and every double is a dyadic rational, so the exact
// sum of two quad-doubles is a dyadic rational that the oracle computes with NO rounding at all.
// The only question is how far the computed 4-component result sits from it.
#include <universal/utility/directives.hpp>
#include <universal/number/qd_cascade/qd_cascade.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/dyadic_exact.hpp>

namespace {

	// exact dyadic value of a quad-double: the sum of its four components, no rounding
	template<typename Cascade, unsigned NR_LIMBS>
	sw::universal::dyadic exact_value(const Cascade& v) {
		using namespace sw::universal;
		dyadic d;
		for (unsigned i = 0; i < NR_LIMBS; ++i) d = d + dyadic::from_double(v[static_cast<int>(i)]);
		return d;
	}

	// is |a| <= |b|? both are exact dyadics, so this is an exact integer comparison once the
	// two numerators are brought to a common scale
	bool magnitude_leq(const sw::universal::dyadic& a, const sw::universal::dyadic& b) {
		using namespace sw::universal;
		dyadic::bigint na, nb;
		int common{ 0 };
		dyadic_align(a, b, na, nb, common);
		return abs(na) <= abs(nb);
	}

	// |computed - exact| <= 2^-toleranceBits * |exact|
	bool within_tolerance(const sw::universal::dyadic& computed, const sw::universal::dyadic& exact, int toleranceBits) {
		using namespace sw::universal;
		dyadic residual = exact - computed;
		if (residual.iszero()) return true;
		// scale the residual up by 2^toleranceBits instead of scaling the value down: exact, and
		// it keeps the comparison in integers
		dyadic scaled(residual.numerator, residual.scale + toleranceBits);
		return magnitude_leq(scaled, exact);
	}

	// a deterministic full-precision quad-double: four components, each ~2^-53 below the previous,
	// which is the shape that arises inside every iterative algorithm (Newton, Taylor, Horner).
	// Operands built from a single double are NOT enough: their sum is exactly representable in
	// two components and the defect never shows.
	class Generator {
	public:
		Generator(std::uint64_t seed) : state{ seed } {}
		double next() {   // uniform in [0.5, 1)
			state = state * 6364136223846793005ull + 1442695040888963407ull;
			return 0.5 + 0.5 * (double((state >> 11) % (1ull << 53)) / double(1ull << 53));
		}
		sw::universal::qd_cascade nextQuadDouble() {
			double h = next();
			return sw::universal::qd_cascade(h, h * next() * 0x1p-53, h * next() * 0x1p-106, h * next() * 0x1p-159);
		}
	private:
		std::uint64_t state;
	};

	// The format carries 212 bits. A correctly renormalized sum lands within a few ulps of that;
	// the defect this suite exists to catch left an error of 2^-160, fifty orders of magnitude
	// away. 205 bits leaves headroom for a legitimately different rounding without letting a
	// dropped component through.
	constexpr int TOLERANCE_BITS = 205;

	int VerifyAdditionAgainstOracle(bool reportTestCases, unsigned nrOfTestCases, std::uint64_t seed) {
		using namespace sw::universal;
		int nrOfFailedTests = 0;
		Generator gen(seed);
		for (unsigned i = 0; i < nrOfTestCases; ++i) {
			qd_cascade a = gen.nextQuadDouble();
			qd_cascade b = gen.nextQuadDouble();
			qd_cascade sum = a + b;

			dyadic exact = exact_value<qd_cascade, 4>(a) + exact_value<qd_cascade, 4>(b);
			dyadic computed = exact_value<qd_cascade, 4>(sum);

			if (!within_tolerance(computed, exact, TOLERANCE_BITS)) {
				++nrOfFailedTests;
				if (reportTestCases) {
					std::cerr << "FAIL: qd_cascade addition is more than 2^-" << TOLERANCE_BITS << " relative from exact\n";
					std::cerr << "  a   = " << to_quad(a) << '\n';
					std::cerr << "  b   = " << to_quad(b) << '\n';
					std::cerr << "  a+b = " << to_quad(sum) << '\n';
				}
			}
		}
		return nrOfFailedTests;
	}

	// The symptom that made universal#1317 visible: sqrt is Newton iteration on top of addition
	// and division, so a dropped component in the sum surfaces as a wrong root. r*r - a is an
	// exact dyadic computation (multiplication of dyadics is exact), so this is a true residual,
	// not a floating-point estimate of one.
	int VerifySqrtResidual(bool reportTestCases, unsigned nrOfTestCases, std::uint64_t seed) {
		using namespace sw::universal;
		int nrOfFailedTests = 0;
		Generator gen(seed);
		for (unsigned i = 0; i < nrOfTestCases; ++i) {
			qd_cascade a = gen.nextQuadDouble();
			qd_cascade r = sqrt(a);

			dyadic root = exact_value<qd_cascade, 4>(r);
			dyadic square = root * root;
			dyadic exact = exact_value<qd_cascade, 4>(a);

			// sqrt is not exactly representable, so the residual is bounded rather than zero:
			// |r^2 - a| <= 2^-TOLERANCE_BITS * |a|
			if (!within_tolerance(square, exact, TOLERANCE_BITS)) {
				++nrOfFailedTests;
				if (reportTestCases) {
					std::cerr << "FAIL: sqrt residual exceeds 2^-" << TOLERANCE_BITS << " relative\n";
					std::cerr << "  a       = " << to_quad(a) << '\n';
					std::cerr << "  sqrt(a) = " << to_quad(r) << '\n';
				}
			}
		}
		return nrOfFailedTests;
	}

}  // anonymous namespace

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

	std::string test_suite         = "quad-double cascade addition against an exact dyadic oracle";
	std::string test_tag           = "qd_cascade addition oracle";
	bool reportTestCases           = false;
	int nrOfFailedTestCases        = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += VerifyAdditionAgainstOracle(true, 32, 0xC0FFEEull);
	nrOfFailedTestCases += VerifySqrtResidual(true, 32, 0xBEEFull);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else  // !MANUAL_TESTING

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyAdditionAgainstOracle(reportTestCases, 64, 0xC0FFEEull), test_tag, "addition vs exact dyadic");
	nrOfFailedTestCases += ReportTestResult(VerifySqrtResidual(reportTestCases, 32, 0xBEEFull), test_tag, "sqrt residual vs exact dyadic");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyAdditionAgainstOracle(reportTestCases, 256, 0x1234ull), test_tag, "addition vs exact dyadic (level 2)");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyAdditionAgainstOracle(reportTestCases, 1024, 0x5678ull), test_tag, "addition vs exact dyadic (level 3)");
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyAdditionAgainstOracle(reportTestCases, 4096, 0x9ABCull), test_tag, "addition vs exact dyadic (level 4)");
	nrOfFailedTestCases += ReportTestResult(VerifySqrtResidual(reportTestCases, 512, 0xDEF0ull), test_tag, "sqrt residual vs exact dyadic (level 4)");
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
