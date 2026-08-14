// numeric_limits.cpp: verification of numeric_limits for both takum variants
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// epsilon() used to be computed as (++one) - one.  That subtraction runs through
// a double, so any takum finer than a double collapsed to zero:
//
//     numeric_limits<takum<64,3>>::epsilon()      == 0
//     numeric_limits<takum_log<64,3>>::epsilon()  == 0
//
// Zero is the most misleading answer available -- it asserts the type is exact --
// and it went unnoticed because nothing asserted anything about epsilon.
//
// So this suite checks the DEFINING PROPERTY rather than a table of expected
// numbers: epsilon is the step from 1.0 to its successor, which both variants
// encode as a step of 2^-p in c + m at c == 0.  Deriving the expectation from the
// codec means it stays true for any configuration, including ones nobody has
// instantiated yet.
#include <universal/utility/directives.hpp>

#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdint>
#include <limits>
#include <universal/number/takum/takum_log.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

// epsilon must never be zero.  A zero epsilon says the type represents every real
// exactly, which is false for every configuration of both variants.
template<typename TakumType>
int VerifyEpsilonNonZero(const char* tag, bool reportTestCases) {
	int nrOfFailedTests = 0;
	const TakumType eps = std::numeric_limits<TakumType>::epsilon();
	if (eps.iszero() || eps.isnar()) {
		++nrOfFailedTests;
		if (reportTestCases) {
			std::cout << "FAIL " << tag << " epsilon is "
			          << (eps.isnar() ? "NaR" : "zero") << '\n';
		}
	}
	// Compared as encodings.  Routing these through a double is the mistake this
	// whole file is about: at rbits 4 and 5 a perfectly valid small encoding
	// converts to 0.0, so double(eps) > 0.0 would fail for the wrong reason.
	if (eps.sign()) {
		++nrOfFailedTests;
		if (reportTestCases) std::cout << "FAIL " << tag << " epsilon is negative\n";
	}
	// and it must be a difference from 1.0, so strictly below 1
	if (!(eps < TakumType(1.0))) {
		++nrOfFailedTests;
		if (reportTestCases) std::cout << "FAIL " << tag << " epsilon is not below 1\n";
	}
	return nrOfFailedTests;
}

// For a linear takum the successor of 1.0 is (1 + 2^-p) * 2^0, so epsilon is
// exactly 2^-p and encodes as characteristic -p with an empty trailing field.
// Checked against the encoding, not against a double, because the whole failure
// was a double being too narrow to hold the answer.
template<unsigned nbits, unsigned rbits>
int VerifyLinearEpsilonExact(bool reportTestCases) {
	using TL = sw::universal::takum<nbits, rbits, std::uint64_t>;
	using Codec = typename TL::Codec;
	int nrOfFailedTests = 0;

	const unsigned p = Codec::layout_of(Codec::find_dr(0)).p;
	const int64_t want_c = -static_cast<int64_t>(p);
	const TL eps = std::numeric_limits<TL>::epsilon();

	if (want_c < Codec::min_characteristic()) {
		// 2^-p is below the format's range; epsilon saturates to minpos rather
		// than to zero.  takum<32,2> and takum<24,1> land here.
		if (eps.raw_bits() != std::numeric_limits<TL>::min().raw_bits()) {
			++nrOfFailedTests;
			if (reportTestCases) {
				std::cout << "FAIL takum<" << nbits << ',' << rbits
				          << "> ulp is unrepresentable, epsilon should saturate to minpos\n";
			}
		}
		return nrOfFailedTests;
	}

	auto d = Codec::decode(eps.magnitude_bits());
	if (d.c != want_c || d.M_bits != 0) {
		++nrOfFailedTests;
		if (reportTestCases) {
			std::cout << "FAIL takum<" << nbits << ',' << rbits << "> epsilon should encode 2^"
			          << want_c << ": got c=" << d.c << " M=" << d.M_bits << '\n';
		}
	}
	return nrOfFailedTests;
}

// For a logarithmic takum the successor of 1.0 has l = 2^-p, so its value is
// e^(2^-(p+1)) and epsilon is that minus one.  It is not exactly representable,
// so check it lies within an ulp of the format's own resolution.
template<unsigned nbits, unsigned rbits>
int VerifyLogarithmicEpsilon(bool reportTestCases) {
	using TL = sw::universal::takum_log<nbits, rbits, std::uint64_t>;
	using Codec = typename TL::Codec;
	int nrOfFailedTests = 0;

	const unsigned p = Codec::layout_of(Codec::find_dr(0)).p;
	const double x = std::pow(2.0, -static_cast<double>(p + 1));
	const double want = std::expm1(x);
	const TL eps      = std::numeric_limits<TL>::epsilon();
	// Narrow regime fields put the ulp below minpos; epsilon saturates there.
	if (eps.raw_bits() == std::numeric_limits<TL>::min().raw_bits()) return nrOfFailedTests;
	const double got  = double(eps);

	// The format resolves values to roughly 2^-p in the logarithmic domain, which
	// is a relative resolution of about that in the value domain; allow a handful.
	// Floored at 1e-13: both sides of this comparison pass through a double, so at
	// p = 59 the format is finer than the yardstick and the residual noise is the
	// double's, not the type's.
	double tolerance = 64.0 * std::pow(2.0, -static_cast<double>(p));
	if (tolerance < 1.0e-13) tolerance = 1.0e-13;
	const double rel = std::fabs((got - want) / want);
	if (!(rel < tolerance)) {
		++nrOfFailedTests;
		if (reportTestCases) {
			std::cout << "FAIL takum_log<" << nbits << ',' << rbits << "> epsilon got=" << got
			          << " want=" << want << " relerr=" << rel << " tol=" << tolerance << '\n';
		}
	}
	return nrOfFailedTests;
}

// The rest of the surface, which had no coverage either.
template<typename TakumType>
int VerifyLimitsSurface(const char* tag, bool reportTestCases) {
	using L = std::numeric_limits<TakumType>;
	int nrOfFailedTests = 0;
	auto fail = [&](const char* what) {
		++nrOfFailedTests;
		if (reportTestCases) std::cout << "FAIL " << tag << ": " << what << '\n';
	};

	if (!L::is_specialized)                          fail("is_specialized should be true");
	if (!L::is_signed)                               fail("is_signed should be true");
	if (L::is_integer)                               fail("is_integer should be false");
	if (L::is_exact)                                 fail("is_exact should be false");
	if (L::has_infinity)                             fail("takum has no infinity encoding");
	if (!L::has_quiet_NaN)                           fail("NaR serves as quiet NaN");
	if (L::radix != 2)                               fail("radix should be 2");
	if (L::round_style != std::round_to_nearest)     fail("the codec rounds to nearest-even");

	// Checked on the ENCODINGS, not through a double.  At rbits 4 and 5 the
	// characteristic range exceeds a double's, so double(minpos) underflows to 0.0
	// and double(maxpos) overflows to infinity while both encodings are perfectly
	// valid.  Asserting double(min()) > 0 fails there for a reason that has nothing
	// to do with numeric_limits.
	if (L::min().iszero() || L::min().isnar())       fail("min should be minpos, not zero or NaR");
	if (L::min().sign())                             fail("min should be positive");
	if (L::max().iszero() || L::max().isnar())       fail("max should be maxpos");
	if (L::max().sign())                             fail("max should be positive");
	if (!(L::min() < L::max()))                      fail("max should exceed min");
	if (!L::lowest().sign())                         fail("lowest should be negative");
	if (L::lowest().raw_bits() != (-L::max()).raw_bits()) fail("lowest should be -max");
	if (!(L::min_exponent < L::max_exponent))        fail("exponent range should be ordered");
	if (L::denorm_min().raw_bits() != L::min().raw_bits()) fail("no denormals: denorm_min == min");
	return nrOfFailedTests;
}

} // anonymous namespace

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
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

#define CHECK_BOTH(N, R)                                                                        \
	nrOfFailedTestCases += ReportTestResult(                                                    \
		VerifyEpsilonNonZero<takum<N, R, std::uint64_t>>("takum<" #N "," #R ">", reportTestCases)  \
		+ VerifyLinearEpsilonExact<N, R>(reportTestCases)                                       \
		+ VerifyLimitsSurface<takum<N, R, std::uint64_t>>("takum<" #N "," #R ">", reportTestCases), \
		"takum<" #N "," #R ">", "numeric_limits");                                              \
	nrOfFailedTestCases += ReportTestResult(                                                    \
		VerifyEpsilonNonZero<takum_log<N, R, std::uint64_t>>("takum_log<" #N "," #R ">", reportTestCases) \
		+ VerifyLogarithmicEpsilon<N, R>(reportTestCases)                                       \
		+ VerifyLimitsSurface<takum_log<N, R, std::uint64_t>>("takum_log<" #N "," #R ">", reportTestCases), \
		"takum_log<" #N "," #R ">", "numeric_limits")

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "takum numeric_limits verification";
	std::string test_tag    = "numeric_limits";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	CHECK_BOTH(64, 3);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	CHECK_BOTH(32, 3);
	CHECK_BOTH(16, 3);
	// Narrow regime fields, where the ulp at 1.0 falls below minpos and epsilon has
	// to saturate.  Kept at level 1 deliberately: that fallback is the new branch,
	// and the coverage build runs level 1 only, so parking these deeper would leave
	// the interesting path compiled in but never executed.
	CHECK_BOTH(32, 2);
	CHECK_BOTH(24, 1);
#endif

	// The widths where the old subtraction silently produced zero: at nbits = 64
	// the trailing field runs to 59 bits and 1 + 2^-59 is not a double.
#if REGRESSION_LEVEL_2
	CHECK_BOTH(64, 3);
	CHECK_BOTH(48, 3);
#endif

#if REGRESSION_LEVEL_3
	CHECK_BOTH(12, 3);
#endif

#if REGRESSION_LEVEL_4
	CHECK_BOTH(20, 4);
	CHECK_BOTH(56, 3);
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
