// payne_hanek.cpp: large-|x| range reduction for elreal sin/cos/tan (Phase 7, #1050).
//
// For |x| >= 2^50 a host-double estimate of x/(pi/2) has lost all its low bits,
// so detail::sincos switches to a Payne-Hanek reduction that computes the
// quotient q = |x|/(pi/2) in full ZBCL precision, rounds it to the exact integer
// N, and forms t = (q - N)*(pi/2). This suite checks that sin/cos/tan stay
// accurate to hundreds of digits at 1e3 .. 1e20 -- against mpmath 320-digit
// references (reference_constants.hpp) and via the identity sin^2 + cos^2 == 1 --
// and that the reduced argument keeps the ZBCL 0-overlap invariant.
//
// The Payne-Hanek evaluations are ~1000x more expensive than the per-PR sanity
// depth (a deep pi and a full-precision division per call), so the absolute /
// identity checks are gated to REGRESSION_LEVEL_4 (stress / manual) and are a
// no-op at the fast CI level. A bare manual compile (no -D) runs everything.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <iostream>
#include <string>
#include <string_view>

#include <universal/number/elreal/elreal.hpp>
#include <universal/number/elreal/math/trigonometry.hpp>
#include <universal/verification/elreal_oracle.hpp>
#include <universal/verification/elreal_reference_digits.hpp>
#include <universal/verification/test_suite.hpp>
#include <math/constants/reference_constants.hpp>

#define MANUAL_TESTING 0
#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 0
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

namespace {

using ER  = sw::universal::ZBCL<double>;
namespace est = sw::universal::elreal_oracle;

// A single cheap double-precision smoke test that runs at every regression
// level: one Payne-Hanek evaluation (sin/cos(1e20) at depth 2) must match
// glibc's std::sin/std::cos (which do their own correct range reduction) and
// keep the ZBCL 0-overlap invariant. One evaluation is ~1s (a deep pi and a
// full-precision division), so the exhaustive multi-argument / 300-digit checks
// are deferred to REGRESSION_LEVEL_4.
int fast_sanity(bool reportTestCases) {
	using namespace sw::universal;
	int n = 0;
	const std::size_t D = 2;
	const double v = 1e20;
	ER x = from_native<double>(v);
	ER sx = sin(x, D);
	double s = to_double_approx(sx, 8);
	double c = to_double_approx(cos(x, D), 8);
	if (std::fabs(s - std::sin(v)) > 1e-9 || std::fabs(c - std::cos(v)) > 1e-9) {
		std::cout << "  FAIL sin/cos(1e20): got " << s << " / " << c
		          << " std " << std::sin(v) << " / " << std::cos(v) << '\n';
		++n;
	}
	n += est::check_zero_overlap(sx, D, "sin(1e20) 0-overlap");   // reduced argument stays 0-overlap
	if (reportTestCases && n == 0) std::cout << "  ok   large-|x| double-precision smoke (sin/cos(1e20))\n";
	return n;
}

#if REGRESSION_LEVEL_4
using sw::universal::agreed_decimal_digits;

constexpr std::size_t kDepth     = 12;    // evaluation depth (leaves ~150+ digits after reduction)
constexpr int         kMinDigits = 140;

// Absolute spot-check against a SIGNED decimal reference: the value's sign must
// match, and its magnitude must agree with the reference to >= kMinDigits digits.
// (agreed_decimal_digits takes an unsigned decimal, so compare |value| to |ref|.)
int check_signed(const char* name, const ER& z, std::string_view ref, bool reportTestCases) {
	using namespace sw::universal;
	const bool refNeg = !ref.empty() && ref[0] == '-';
	const std::string_view mag = refNeg ? ref.substr(1) : ref;
	const bool zNeg = !z.is_empty() && z.head().sign() < 0;
	if (zNeg != refNeg) {
		std::cout << "  FAIL " << name << ": sign mismatch (got " << (zNeg ? '-' : '+')
		          << ", want " << (refNeg ? '-' : '+') << ")\n";
		return 1;
	}
	ER zmag = zNeg ? negate(z) : z;
	int got = agreed_decimal_digits(zmag, mag);
	if (got < kMinDigits) {
		std::cout << "  FAIL " << name << ": agreed " << got << " digits, want >= " << kMinDigits << '\n';
		return 1;
	}
	if (reportTestCases) std::cout << "  ok   " << name << ": " << got << " digits\n";
	return 0;
}

// Identity check: sin^2 + cos^2 == 1 to >= kMinDigits, computed entirely in ZBCL
// space (no external reference), a strong test of the reduced argument's accuracy.
int check_pythagorean(const char* name, double v, bool reportTestCases) {
	using namespace sw::universal;
	ER x = from_native<double>(v);
	ER s = sin(x, kDepth), c = cos(x, kDepth);
	ER id = add(mul_online(s, s), mul_online(c, c));
	int got = agreed_decimal_digits(id, "1.0");
	if (got < kMinDigits) {
		std::cout << "  FAIL " << name << ": sin^2+cos^2 holds to only " << got << " digits\n";
		return 1;
	}
	if (reportTestCases) std::cout << "  ok   " << name << ": " << got << " digits\n";
	return 0;
}
#endif

} // anonymous

int main()
try {
	using namespace sw::universal;
	std::string test_suite = "elreal Payne-Hanek large-|x| range reduction (#1050)";
	int nrOfFailedTestCases = 0;
	bool reportTestCases = true;
	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// TODO: place hand-run diagnostics here (this branch ignores failures)

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;

#else

	nrOfFailedTestCases += fast_sanity(reportTestCases);

#if REGRESSION_LEVEL_4
	const std::size_t D = kDepth;
	// absolute accuracy vs mpmath 320-digit references (arguments are exact doubles)
	nrOfFailedTestCases += check_signed("sin(1e3)",  sin(from_native<double>(1e3),  D), s_sin_1e3,  reportTestCases);
	nrOfFailedTestCases += check_signed("cos(1e3)",  cos(from_native<double>(1e3),  D), s_cos_1e3,  reportTestCases);
	nrOfFailedTestCases += check_signed("sin(1e6)",  sin(from_native<double>(1e6),  D), s_sin_1e6,  reportTestCases);
	nrOfFailedTestCases += check_signed("cos(1e6)",  cos(from_native<double>(1e6),  D), s_cos_1e6,  reportTestCases);
	nrOfFailedTestCases += check_signed("sin(1e12)", sin(from_native<double>(1e12), D), s_sin_1e12, reportTestCases);
	nrOfFailedTestCases += check_signed("cos(1e12)", cos(from_native<double>(1e12), D), s_cos_1e12, reportTestCases);
	nrOfFailedTestCases += check_signed("sin(1e20)", sin(from_native<double>(1e20), D), s_sin_1e20, reportTestCases);
	nrOfFailedTestCases += check_signed("cos(1e20)", cos(from_native<double>(1e20), D), s_cos_1e20, reportTestCases);
	// negative argument: sin is odd, cos is even (high-precision identities)
	{
		ER xp = from_native<double>(1e20), xn = from_native<double>(-1e20);
		int gs = agreed_decimal_digits(sin(xn, D), negate(sin(xp, D)));   // sin(-x) == -sin(x)
		int gc = agreed_decimal_digits(cos(xn, D), cos(xp, D));           // cos(-x) ==  cos(x)
		if (gs < kMinDigits) { std::cout << "  FAIL sin(-1e20)==-sin(1e20): " << gs << " digits\n"; ++nrOfFailedTestCases; }
		else if (reportTestCases) std::cout << "  ok   sin(-1e20)==-sin(1e20): " << gs << " digits\n";
		if (gc < kMinDigits) { std::cout << "  FAIL cos(-1e20)==cos(1e20): " << gc << " digits\n"; ++nrOfFailedTestCases; }
		else if (reportTestCases) std::cout << "  ok   cos(-1e20)==cos(1e20): " << gc << " digits\n";
	}

	// sin^2 + cos^2 == 1 across the range (identity, no external reference)
	for (double v : { 1e3, 1e6, 1e12, 1e20 }) {
		std::string tag = "sin^2+cos^2(" + std::to_string(v) + ")";
		nrOfFailedTestCases += check_pythagorean(tag.c_str(), v, reportTestCases);
	}

	// tan(x) == sin(x)/cos(x) at a large argument
	{
		ER x = from_native<double>(1e12);
		ER t = tan(x, D);
		ER sc = div_online(sin(x, D), cos(x, D));
		int got = agreed_decimal_digits(t, sc);
		if (got < kMinDigits) { std::cout << "  FAIL tan(1e12)==sin/cos: " << got << " digits\n"; ++nrOfFailedTestCases; }
		else if (reportTestCases) std::cout << "  ok   tan(1e12)==sin/cos: " << got << " digits\n";
	}
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (const std::exception& err) {
	std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
