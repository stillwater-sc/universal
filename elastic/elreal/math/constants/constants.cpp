// constants.cpp: tests for elreal math constants (Phase E.1)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Validation strategy
// -------------------
// E.1's elreal constants deliberately re-use the precomputed 4-component
// expansions in qd_constants.hpp (precomputed offline by Scibuilders /
// Stillwater, used in production for years). The test contract is therefore:
//
//   (a) depth-0 matches the IEEE-754 round of the standard value (verified
//       against std::numbers for portability -- M_PI / M_E etc. are POSIX
//       and would need _USE_MATH_DEFINES on MSVC, so we don't use them);
//   (b) all four components match qd_<constant> component-by-component
//       (catches copy-paste errors in elreal_constants.hpp without needing
//       a higher-precision reference of our own);
//   (c) structural invariants hold (computed_depth() == 4, at(>=4) returns
//       the implicit zero extension, lazy-real distinctness from
//       elreal(std::numbers::pi_v<double>) works);
//   (d) all the constants exposed by elreal_constants.hpp get coverage,
//       not just the canonical pi/e/ln2/ln10 set.
//
// long double is intentionally avoided as a reference type: on MSVC, ARM,
// RISC-V 32 and most ARM64 platforms it is aliased to double and carries no
// extra precision, so it cannot validate the 4-component (~212-bit) elreal
// claim. Component-by-component comparison to qd_constants is the correct
// substitute -- qd is itself validated against published high-precision
// references through its own test suite.

#include <universal/utility/directives.hpp>

#define ELREAL_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/elreal/elreal.hpp>
// Include the qd umbrella header so the qd type is declared before its
// constants header consumes it. The constants header is then sufficient
// to access qd_pi, qd_e, etc. for the component-by-component validation.
#include <universal/number/qd/qd.hpp>
#include <universal/number/qd/math/constants/qd_constants.hpp>
#include <universal/verification/test_suite.hpp>

#include <cmath>
#include <numbers>

// Compare an elreal constant to a qd reference, component-by-component, at
// the four shared depths (0..3). qd's components are precomputed offline at
// ~212 bits cumulative; an exact match confirms elreal_constants.hpp ported
// the values correctly.
template<typename QdRef>
static int check_constant_vs_qd(const char* name,
                                const sw::universal::elreal& v,
                                const QdRef& ref) {
	int failed = 0;
	for (int i = 0; i < 4; ++i) {
		double got = v.at(static_cast<std::size_t>(i));
		double exp = ref[i];
		if (got != exp) {
			std::cerr << "FAIL: " << name << " component[" << i << "]: "
				<< "got " << got << " expected " << exp << '\n';
			++failed;
		}
	}
	return failed;
}

// Structural check: each constant must have exactly 4 materialised
// components and at(k>=4) must return the implicit-zero extension.
static int check_depth4(const char* name, const sw::universal::elreal& v) {
	int failed = 0;
	if (v.computed_depth() != 4) {
		std::cerr << "FAIL: " << name << " computed_depth() = "
			<< v.computed_depth() << " (expected 4)\n";
		++failed;
	}
	if (v.at(4) != 0.0 || v.at(10) != 0.0) {
		std::cerr << "FAIL: " << name
			<< " at(>=4) is not the implicit-zero extension\n";
		++failed;
	}
	return failed;
}

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "elreal Phase E.1 math constants";
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, false);

	// --- (a) depth-0 matches std::numbers round of the standard value ----
	// std::numbers (C++20) is portable across MSVC, libstdc++, libc++,
	// ARM and RISC-V toolchains; no _USE_MATH_DEFINES dance.
	{
		if (elreal_pi().at(0)    != std::numbers::pi_v<double>)
			{ std::cerr << "FAIL: elreal_pi().at(0) != std::numbers::pi_v<double>\n";       ++nrOfFailedTestCases; }
		if (elreal_e().at(0)     != std::numbers::e_v<double>)
			{ std::cerr << "FAIL: elreal_e().at(0) != std::numbers::e_v<double>\n";         ++nrOfFailedTestCases; }
		if (elreal_ln2().at(0)   != std::numbers::ln2_v<double>)
			{ std::cerr << "FAIL: elreal_ln2().at(0) != std::numbers::ln2_v<double>\n";     ++nrOfFailedTestCases; }
		if (elreal_ln10().at(0)  != std::numbers::ln10_v<double>)
			{ std::cerr << "FAIL: elreal_ln10().at(0) != std::numbers::ln10_v<double>\n";   ++nrOfFailedTestCases; }
		if (elreal_sqrt2().at(0) != std::numbers::sqrt2_v<double>)
			{ std::cerr << "FAIL: elreal_sqrt2().at(0) != std::numbers::sqrt2_v<double>\n"; ++nrOfFailedTestCases; }
		if (elreal_sqrt3().at(0) != std::numbers::sqrt3_v<double>)
			{ std::cerr << "FAIL: elreal_sqrt3().at(0) != std::numbers::sqrt3_v<double>\n"; ++nrOfFailedTestCases; }
		if (elreal_phi().at(0)   != std::numbers::phi_v<double>)
			{ std::cerr << "FAIL: elreal_phi().at(0) != std::numbers::phi_v<double>\n";     ++nrOfFailedTestCases; }
		if (elreal_lge().at(0)   != std::numbers::log2e_v<double>)
			{ std::cerr << "FAIL: elreal_lge().at(0) != std::numbers::log2e_v<double>\n";   ++nrOfFailedTestCases; }
	}

	// --- (b) full 4-component match against the qd_constants reference ---
	nrOfFailedTestCases += check_constant_vs_qd("pi",    elreal_pi(),    qd_pi);
	nrOfFailedTestCases += check_constant_vs_qd("pi_2",  elreal_pi_2(),  qd_pi_2);
	nrOfFailedTestCases += check_constant_vs_qd("pi_4",  elreal_pi_4(),  qd_pi_4);
	nrOfFailedTestCases += check_constant_vs_qd("2pi",   elreal_2pi(),   qd_2pi);
	nrOfFailedTestCases += check_constant_vs_qd("e",     elreal_e(),     qd_e);
	nrOfFailedTestCases += check_constant_vs_qd("ln2",   elreal_ln2(),   qd_ln2);
	nrOfFailedTestCases += check_constant_vs_qd("ln10",  elreal_ln10(),  qd_ln10);
	nrOfFailedTestCases += check_constant_vs_qd("lge",   elreal_lge(),   qd_lge);
	nrOfFailedTestCases += check_constant_vs_qd("lg10",  elreal_lg10(),  qd_lg10);
	nrOfFailedTestCases += check_constant_vs_qd("sqrt2", elreal_sqrt2(), qd_sqrt2);
	nrOfFailedTestCases += check_constant_vs_qd("sqrt3", elreal_sqrt3(), qd_sqrt3);
	nrOfFailedTestCases += check_constant_vs_qd("phi",   elreal_phi(),   qd_phi);

	// --- (c) structural invariants on every shipped constant --------------
	nrOfFailedTestCases += check_depth4("pi",    elreal_pi());
	nrOfFailedTestCases += check_depth4("pi_2",  elreal_pi_2());
	nrOfFailedTestCases += check_depth4("pi_4",  elreal_pi_4());
	nrOfFailedTestCases += check_depth4("2pi",   elreal_2pi());
	nrOfFailedTestCases += check_depth4("e",     elreal_e());
	nrOfFailedTestCases += check_depth4("ln2",   elreal_ln2());
	nrOfFailedTestCases += check_depth4("ln10",  elreal_ln10());
	nrOfFailedTestCases += check_depth4("lge",   elreal_lge());
	nrOfFailedTestCases += check_depth4("lg10",  elreal_lg10());
	nrOfFailedTestCases += check_depth4("sqrt2", elreal_sqrt2());
	nrOfFailedTestCases += check_depth4("sqrt3", elreal_sqrt3());
	nrOfFailedTestCases += check_depth4("phi",   elreal_phi());

	// --- (c) lazy-real distinctness: each multi-component constant is
	// observably different from the corresponding single-double value
	// (Phase D's compare operator walks the depth-1 correction).
	{
		// pi: correction (component 1) is +1.22e-16, so elreal_pi > rounded.
		if (!(elreal_pi() > elreal(std::numbers::pi_v<double>))) {
			std::cerr << "FAIL: elreal_pi() not greater than elreal(std::numbers::pi_v<double>) "
				<< "(depth-1 correction not being seen by compare)\n";
			++nrOfFailedTestCases;
		}
		// e: positive correction
		if (!(elreal_e() > elreal(std::numbers::e_v<double>))) {
			std::cerr << "FAIL: elreal_e() not greater than elreal(std::numbers::e_v<double>)\n";
			++nrOfFailedTestCases;
		}
		// ln10: negative correction
		if (!(elreal_ln10() < elreal(std::numbers::ln10_v<double>))) {
			std::cerr << "FAIL: elreal_ln10() not less than elreal(std::numbers::ln10_v<double>) "
				<< "(correction sign disagreement)\n";
			++nrOfFailedTestCases;
		}
		// Distinctness via !=
		if (elreal_pi() == elreal(std::numbers::pi_v<double>)) {
			std::cerr << "FAIL: elreal_pi() == elreal(std::numbers::pi_v<double>) "
				<< "(lazy-real distinctness lost; correction not visible)\n";
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
