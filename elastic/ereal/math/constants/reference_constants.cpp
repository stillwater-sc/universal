// reference_constants.cpp: regression tests validating ~320-digit reference
// strings against the precomputed qd expansions.
//
// Cross-validates the reference strings in
// `include/sw/math/constants/reference_constants.hpp` against the precomputed
// 4-component qd expansions in
// `include/sw/universal/number/qd/math/constants/qd_constants.hpp` (issue #911).
//
// Parse-performance caveat (#913): `ereal<N>::parse()` parses the full
// 320-digit reference strings in non-tractable time today (Horner accumulation
// grows `_limb` past maxlimbs). This test therefore uses *truncated* 80-digit
// substrings, which parse in under 1 ms and still fill the 4-component qd
// expansion being cross-checked. The full 320-digit strings remain the source
// of truth in the header (usable once #913 lands).
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <iomanip>
#include <string>
#include <string_view>
#include <universal/number/ereal/ereal.hpp>
#include <universal/number/qd/qd.hpp>
#include <math/constants/reference_constants.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

	using sw::universal::ereal;
	using sw::universal::qd;

	// Parse the first TRUNCATION characters of `sv` into ereal<19>. Parsing all
	// 320 digits hits the ereal::parse perf cliff (#913); truncating to ~80
	// chars keeps the cost sub-millisecond while still producing enough limbs to
	// cover the 4-component qd cross-check.
	constexpr std::size_t TRUNCATION = 80;

	ereal<19> parse_truncated(std::string_view sv) {
		std::string s(sv.substr(0, TRUNCATION));
		ereal<19> v;
		v.parse(s);
		return v;
	}

	// Compare the first 4 components of the parsed ereal<19> against the
	// 4-component qd expansion. The leading component must agree exactly (both
	// are the IEEE-rounded leading double of the same decimal string); trailing
	// components may drift a few ULP (different generation paths) and are checked
	// against a relative tolerance of 1e-30 of the leading component.
	int compare_qd_expansion(const char* name, const ereal<19>& v, const qd& reference,
	                         bool reportTestCases) {
		const auto& comps = v.limbs();
		bool ok = true;

		for (int i = 0; i < 4; ++i) {
			double parsed = (static_cast<int>(comps.size()) > i) ? comps[static_cast<size_t>(i)] : 0.0;
			double ref    = reference[i];
			if (parsed != ref) {
				double tolerance = std::abs(reference[0]) * 1e-30;
				if (tolerance == 0.0) tolerance = 1e-310;
				double diff = std::abs(parsed - ref);
				if (i == 0) {
					if (diff != 0.0) {
						if (reportTestCases) std::cout << "    FAIL " << name << " leading component:"
							<< " parsed=" << std::setprecision(17) << parsed
							<< " qd_expan=" << ref << '\n';
						ok = false;
					}
				}
				else if (diff > tolerance) {
					if (reportTestCases) std::cout << "    FAIL " << name << " component[" << i << "] drift:"
						<< " parsed=" << std::setprecision(17) << parsed
						<< " qd_expan=" << ref
						<< " diff=" << std::setprecision(3) << diff
						<< " tol=" << tolerance << '\n';
					ok = false;
				}
			}
		}
		return ok ? 0 : 1;
	}

	int check_constant(const char* name, const ereal<19>& parsed, const qd& reference,
	                   std::size_t min_components_expected, bool reportTestCases) {
		int failures = 0;
		const auto& comps = parsed.limbs();
		if (comps.size() < min_components_expected) {
			if (reportTestCases) std::cout << "    FAIL " << name << ": parsed produced "
				<< comps.size() << " components, expected at least "
				<< min_components_expected << '\n';
			++failures;
		}
		failures += compare_qd_expansion(name, parsed, reference, reportTestCases);
		return failures;
	}

	// Sanity: parsed value (as double) matches the known double approximation
	// within double precision. Catches gross parse errors for constants without
	// a qd counterpart.
	int check_double(const char* name, std::string_view sv, double expected, double tol,
	                 bool reportTestCases) {
		double sum = double(parse_truncated(sv));
		if (std::abs(sum - expected) > tol) {
			if (reportTestCases) std::cout << "    FAIL double(parse(" << name << "[80])) = "
				<< std::setprecision(17) << sum << ", expected " << expected << '\n';
			return 1;
		}
		return 0;
	}

	// =========================================================================
	// LEVEL 1: each named reference constant cross-checked against qd_constants
	// =========================================================================
	int VerifyReferenceConstants(bool reportTestCases) {
		using namespace sw::universal;
		int nrOfFailedTestCases = 0;

		// The min-components argument is the minimum number of materialised
		// components the parse must produce -- fewer than 4 means the parse is
		// silently truncating.
		constexpr std::size_t MIN_COMPS = 4;

		if (reportTestCases) std::cout << "  pi family...\n";
		nrOfFailedTestCases += check_constant("pi",     parse_truncated(s_pi),     qd_pi,   MIN_COMPS, reportTestCases);
		nrOfFailedTestCases += check_constant("pi_2",   parse_truncated(s_pi_2),   qd_pi_2, MIN_COMPS, reportTestCases);
		// pi_3 cross-check skipped: qd_pi_3 holds the wrong leading value
		// (~ pi/2) and only 2 components -- pre-existing transcription bug #914.
		nrOfFailedTestCases += check_constant("pi_4",   parse_truncated(s_pi_4),   qd_pi_4, MIN_COMPS, reportTestCases);
		nrOfFailedTestCases += check_constant("two_pi", parse_truncated(s_two_pi), qd_2pi,  MIN_COMPS, reportTestCases);
		nrOfFailedTestCases += check_constant("inv_pi", parse_truncated(s_inv_pi), qd_1_pi, MIN_COMPS, reportTestCases);
		nrOfFailedTestCases += check_constant("two_inv_pi", parse_truncated(s_two_inv_pi), qd_2_pi, MIN_COMPS, reportTestCases);

		if (reportTestCases) std::cout << "  e family...\n";
		nrOfFailedTestCases += check_constant("e",     parse_truncated(s_e),     qd_e,   MIN_COMPS, reportTestCases);
		nrOfFailedTestCases += check_constant("inv_e", parse_truncated(s_inv_e), qd_1_e, MIN_COMPS, reportTestCases);

		if (reportTestCases) std::cout << "  phi family...\n";
		nrOfFailedTestCases += check_constant("phi",     parse_truncated(s_phi),     qd_phi,   MIN_COMPS, reportTestCases);
		nrOfFailedTestCases += check_constant("inv_phi", parse_truncated(s_inv_phi), qd_1_phi, MIN_COMPS, reportTestCases);

		if (reportTestCases) std::cout << "  roots...\n";
		nrOfFailedTestCases += check_constant("sqrt2",     parse_truncated(s_sqrt2),     qd_sqrt2,   MIN_COMPS, reportTestCases);
		nrOfFailedTestCases += check_constant("sqrt3",     parse_truncated(s_sqrt3),     qd_sqrt3,   MIN_COMPS, reportTestCases);
		nrOfFailedTestCases += check_constant("sqrt5",     parse_truncated(s_sqrt5),     qd_sqrt5,   MIN_COMPS, reportTestCases);
		nrOfFailedTestCases += check_constant("inv_sqrt2", parse_truncated(s_inv_sqrt2), qd_1_sqrt2, MIN_COMPS, reportTestCases);

		if (reportTestCases) std::cout << "  logarithms...\n";
		nrOfFailedTestCases += check_constant("ln2",  parse_truncated(s_ln2),  qd_ln2,  MIN_COMPS, reportTestCases);
		nrOfFailedTestCases += check_constant("ln10", parse_truncated(s_ln10), qd_ln10, MIN_COMPS, reportTestCases);
		// qd_constants.hpp uses short names: qd_lge=log2(e), qd_lg10=log2(10),
		// qd_loge=log10(e), qd_log2=log10(2). Pin each string to the right qd.
		nrOfFailedTestCases += check_constant("log2e",   parse_truncated(s_log2e),   qd_lge,  MIN_COMPS, reportTestCases);
		nrOfFailedTestCases += check_constant("log2_10", parse_truncated(s_log2_10), qd_lg10, MIN_COMPS, reportTestCases);
		nrOfFailedTestCases += check_constant("log10e",  parse_truncated(s_log10e),  qd_loge, MIN_COMPS, reportTestCases);
		nrOfFailedTestCases += check_constant("log10_2", parse_truncated(s_log10_2), qd_log2, MIN_COMPS, reportTestCases);

		if (reportTestCases) std::cout << "  erf scaling...\n";
		nrOfFailedTestCases += check_constant("two_over_sqrt_pi",
		                                      parse_truncated(s_two_over_sqrt_pi), qd_2_sqrtpi, MIN_COMPS, reportTestCases);

		// Constants with no qd counterpart (s_three_pi, s_euler_gamma) -- checked
		// via the as-double sanity below.
		if (reportTestCases) std::cout << "  as-double sanity...\n";
		nrOfFailedTestCases += check_double("s_pi",          s_pi,          3.141592653589793, 1e-14, reportTestCases);
		nrOfFailedTestCases += check_double("s_e",           s_e,           2.718281828459045, 1e-14, reportTestCases);
		nrOfFailedTestCases += check_double("s_three_pi",    s_three_pi,    3.0 * 3.141592653589793, 1e-13, reportTestCases);
		nrOfFailedTestCases += check_double("s_euler_gamma", s_euler_gamma, 0.5772156649015329, 1e-15, reportTestCases);

		return nrOfFailedTestCases;
	}

}  // namespace

// Regression testing guards
#define MANUAL_TESTING 0
#ifndef REGRESSION_LEVEL_OVERRIDE
#	undef REGRESSION_LEVEL_1
#	undef REGRESSION_LEVEL_2
#	undef REGRESSION_LEVEL_3
#	undef REGRESSION_LEVEL_4
#	define REGRESSION_LEVEL_1 1
#	define REGRESSION_LEVEL_2 1
#	define REGRESSION_LEVEL_3 1
#	define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "ereal reference constants";
	bool        reportTestCases = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyReferenceConstants(reportTestCases), "ereal", "reference constants manual");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;  // ignore errors in manual mode

#else

	// The cross-check is deterministic (parses fixed reference strings), so a
	// single pass suffices at every regression level.
	nrOfFailedTestCases += ReportTestResult(VerifyReferenceConstants(reportTestCases), "ereal", "reference constants");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (const std::exception& err) {
	std::cerr << "Caught exception: " << err.what() << '\n';
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception\n";
	return EXIT_FAILURE;
}
