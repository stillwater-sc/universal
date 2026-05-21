// reference_constants.cpp: validate ~320-digit reference strings against qd_constants.hpp
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Resolves issue #911. Cross-validates the reference strings in
// `include/sw/math/constants/reference_constants.hpp` against the
// precomputed 4-component qd expansions in
// `include/sw/universal/number/qd/math/constants/qd_constants.hpp`.
//
// IMPORTANT: parse-performance caveat (#913)
// ------------------------------------------
// `ereal<N>::parse()` parses the full 320-digit reference strings in
// non-tractable time today -- the Horner accumulation makes `_limb`
// grow past maxlimbs, and the per-step cost grows with it. This
// validation test therefore uses *truncated* 80-digit substrings of
// the 320-digit reference strings, which parse in under 1 ms and are
// still enough to fill the 4-component qd expansion we cross-check
// against.
//
// The full 320-digit strings remain the source of truth in the header
// (they will become usable once #913 lands the parse-complexity fix);
// this test exercises the strings against the 4-component qd
// expansion only, which is the strongest cross-check available today.

#include <universal/utility/directives.hpp>

#include <universal/number/ereal/ereal.hpp>
#include <universal/number/qd/qd.hpp>
#include <math/constants/reference_constants.hpp>
#include <universal/verification/test_suite.hpp>

#include <cmath>
#include <iomanip>
#include <iostream>

namespace {

	using sw::universal::ereal;
	using sw::universal::qd;

	// Parse the first N characters of `sv` into ereal<19>. The factory
	// helpers in reference_constants.hpp would parse all 320 digits,
	// which hits the ereal::parse perf cliff (#913); truncating to ~80
	// chars keeps the cost in the sub-millisecond range while still
	// producing enough limbs to cover the 4-component qd cross-check.
	constexpr std::size_t TRUNCATION = 80;

	ereal<19> parse_truncated(std::string_view sv) {
		std::string s(sv.substr(0, TRUNCATION));
		ereal<19> v;
		v.parse(s);
		return v;
	}

	// Compare the first 4 components of the parsed ereal<19> against the
	// 4-component qd expansion. Tolerance per component:
	// - Leading component must match exactly (it is the leading double
	//   produced by the parse, which itself comes from the same decimal
	//   string).
	// - Trailing components must match to within 2 ulp of the leading
	//   (so |diff| <= ulp(component[0]) * 2^-52 * 2 = ~ 1e-31 for c0 ~ 1).
	int compare_qd_expansion(const char* name,
	                         const ereal<19>& v,
	                         const qd& reference)
	{
		const auto& comps = v.limbs();
		bool ok = true;

		for (int i = 0; i < 4; ++i) {
			double parsed = (static_cast<int>(comps.size()) > i) ? comps[static_cast<size_t>(i)] : 0.0;
			double ref    = reference[i];
			if (parsed != ref) {
				// Trailing components can drift by a few ULP because the
				// parse and the precomputed expansion came from different
				// generation paths (qd_constants was hand-curated; the
				// parse computes via expansion_sum). Allow a relative
				// drift of 1e-30 of the leading component.
				double tolerance = std::abs(reference[0]) * 1e-30;
				if (tolerance == 0.0) tolerance = 1e-310;
				double diff = std::abs(parsed - ref);
				if (i == 0) {
					// The leading component must agree exactly: the parse
					// produces the IEEE-rounded leading double, and so
					// does the precomputed table -- both round to the
					// same value.
					if (diff != 0.0) {
						std::cerr << "FAIL " << name << " leading component:\n"
							<< "  parsed   = " << std::setprecision(17) << parsed << "\n"
							<< "  qd_expan = " << std::setprecision(17) << ref << "\n";
						ok = false;
					}
				}
				else if (diff > tolerance) {
					std::cerr << "FAIL " << name << " component[" << i << "] drift:\n"
						<< "  parsed   = " << std::setprecision(17) << parsed << "\n"
						<< "  qd_expan = " << std::setprecision(17) << ref << "\n"
						<< "  diff     = " << std::setprecision(3)  << diff << "\n"
						<< "  tol      = " << std::setprecision(3)  << tolerance << "\n";
					ok = false;
				}
			}
		}
		return ok ? 0 : 1;
	}

	int check_constant(const char* name,
	                   const ereal<19>& parsed,
	                   const qd& reference,
	                   std::size_t min_components_expected)
	{
		int failures = 0;
		const auto& comps = parsed.limbs();
		if (comps.size() < min_components_expected) {
			std::cerr << "FAIL " << name << ": parsed produced "
				<< comps.size() << " components, expected at least "
				<< min_components_expected << "\n";
			++failures;
		}
		failures += compare_qd_expansion(name, parsed, reference);
		return failures;
	}

}  // namespace

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "issue #911: reference string constants vs qd_constants.hpp";
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, false);

	// Parse each reference string into ereal<19>; compare to qd_<name>
	// for the constants that have a qd expansion. The min-components
	// argument is the minimum number of materialised components the
	// parse must produce -- anything less than 4 indicates the parse
	// is silently truncating.
	constexpr std::size_t MIN_COMPS = 4;

	nrOfFailedTestCases += check_constant("pi",     parse_truncated(s_pi),     qd_pi,     MIN_COMPS);
	nrOfFailedTestCases += check_constant("pi_2",   parse_truncated(s_pi_2),   qd_pi_2,   MIN_COMPS);
	// pi_3 cross-check skipped: qd_pi_3 in qd_constants.hpp holds the
	// wrong leading value (~ pi/2 instead of pi/3) and only 2 components
	// instead of 4 -- a pre-existing transcription bug independent of
	// this PR. Tracked in issue #914.
	nrOfFailedTestCases += check_constant("pi_4",   parse_truncated(s_pi_4),   qd_pi_4,   MIN_COMPS);
	nrOfFailedTestCases += check_constant("two_pi", parse_truncated(s_two_pi), qd_2pi,    MIN_COMPS);

	nrOfFailedTestCases += check_constant("inv_pi", parse_truncated(s_inv_pi), qd_1_pi,   MIN_COMPS);
	nrOfFailedTestCases += check_constant("two_inv_pi", parse_truncated(s_two_inv_pi), qd_2_pi, MIN_COMPS);

	nrOfFailedTestCases += check_constant("e",     parse_truncated(s_e),     qd_e,     MIN_COMPS);
	nrOfFailedTestCases += check_constant("inv_e", parse_truncated(s_inv_e), qd_1_e,   MIN_COMPS);

	nrOfFailedTestCases += check_constant("phi",     parse_truncated(s_phi),     qd_phi,     MIN_COMPS);
	nrOfFailedTestCases += check_constant("inv_phi", parse_truncated(s_inv_phi), qd_1_phi,   MIN_COMPS);

	nrOfFailedTestCases += check_constant("sqrt2",     parse_truncated(s_sqrt2),     qd_sqrt2,    MIN_COMPS);
	nrOfFailedTestCases += check_constant("sqrt3",     parse_truncated(s_sqrt3),     qd_sqrt3,    MIN_COMPS);
	nrOfFailedTestCases += check_constant("sqrt5",     parse_truncated(s_sqrt5),     qd_sqrt5,    MIN_COMPS);
	nrOfFailedTestCases += check_constant("inv_sqrt2", parse_truncated(s_inv_sqrt2), qd_1_sqrt2,  MIN_COMPS);

	nrOfFailedTestCases += check_constant("ln2",  parse_truncated(s_ln2),  qd_ln2,  MIN_COMPS);
	nrOfFailedTestCases += check_constant("ln10", parse_truncated(s_ln10), qd_ln10, MIN_COMPS);

	// Sanity: parsed value sums (as double) match the std::numbers
	// approximation to within double precision. Catches gross parse
	// errors.
	{
		ereal<19> pi19 = parse_truncated(s_pi);
		double sum = double(pi19);
		double expected = 3.141592653589793;  // double(pi)
		if (std::abs(sum - expected) > 1e-14) {
			std::cerr << "FAIL: double(parse(s_pi[80])) = "
				<< std::setprecision(17) << sum
				<< ", expected " << expected << "\n";
			++nrOfFailedTestCases;
		}
	}
	{
		ereal<19> e19 = parse_truncated(s_e);
		double sum = double(e19);
		double expected = 2.718281828459045;  // double(e)
		if (std::abs(sum - expected) > 1e-14) {
			std::cerr << "FAIL: double(parse(s_e[80])) = "
				<< std::setprecision(17) << sum
				<< ", expected " << expected << "\n";
			++nrOfFailedTestCases;
		}
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << '\n';
	return EXIT_FAILURE;
}
catch (const std::exception& err) {
	std::cerr << "Caught exception: " << err.what() << '\n';
	return EXIT_FAILURE;
}
