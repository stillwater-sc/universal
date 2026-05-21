// constants.cpp: tests for elreal math constants (Phase E.1)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <universal/utility/directives.hpp>

#define ELREAL_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/elreal/elreal.hpp>
#include <universal/verification/test_suite.hpp>

#include <cmath>

// Reference high-precision values (~110 digits), used to verify the elreal
// constants at depth-1 precision (~106 bits / ~32 decimal digits). Sourced
// from the standard mathematical literature -- the same digit strings used
// to populate qd_constants.hpp.
static constexpr long double REF_PI    = 3.141592653589793238462643383279502884197169399375105820974944592307816406286208998628L;
static constexpr long double REF_E     = 2.718281828459045235360287471352662497757247093699959574966967627724076630353547594572L;
static constexpr long double REF_LN2   = 0.693147180559945309417232121458176568075500134360255254120680009493393621969694715605L;
static constexpr long double REF_LN10  = 2.302585092994045684017991454684364207601101488628772976033327900967572609677352480236L;
static constexpr long double REF_SQRT2 = 1.414213562373095048801688724209698078569671875376948073176679737990732478462107038850L;
static constexpr long double REF_PHI   = 1.618033988749894848204586834365638117720309179805762862135448622705260462818902449707L;

static int check_constant(const char* name, sw::universal::elreal value, long double ref) {
	int failed = 0;

	// Leading double must match the IEEE-754 round of the reference.
	double leading = value.at(0);
	double leading_ref = static_cast<double>(ref);
	if (leading != leading_ref) {
		std::cerr << "FAIL: " << name << " leading double " << leading
			<< " != IEEE round of reference " << leading_ref << '\n';
		++failed;
	}

	// Sum of all four components must match the reference within long-double
	// precision tolerance (long double on most platforms is at least 64 bits;
	// the elreal expansion carries ~212 bits, so the long-double check is
	// looser than the elreal contract -- but it's the strongest portable
	// check we can do without pulling in an external high-precision library).
	long double sum = 0.0L;
	for (std::size_t i = 0; i < value.computed_depth(); ++i) {
		sum += static_cast<long double>(value.at(i));
	}
	long double diff = std::abs(sum - ref);
	long double tol  = std::abs(ref) * 1.0e-30L;
	if (diff > tol) {
		std::cerr << "FAIL: " << name << " 4-component sum differs from reference\n"
			<< "  sum:   " << static_cast<double>(sum) << '\n'
			<< "  ref:   " << static_cast<double>(ref) << '\n'
			<< "  diff:  " << static_cast<double>(diff) << '\n'
			<< "  tol:   " << static_cast<double>(tol)  << '\n';
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

	// --- Leading-double round-trip against std library values ----------
	{
		if (elreal_pi().at(0) != M_PI) {
			std::cerr << "FAIL: elreal_pi().at(0) != M_PI\n";
			++nrOfFailedTestCases;
		}
		if (elreal_e().at(0) != M_E) {
			std::cerr << "FAIL: elreal_e().at(0) != M_E\n";
			++nrOfFailedTestCases;
		}
		if (elreal_ln2().at(0) != M_LN2) {
			std::cerr << "FAIL: elreal_ln2().at(0) != M_LN2\n";
			++nrOfFailedTestCases;
		}
		if (elreal_ln10().at(0) != M_LN10) {
			std::cerr << "FAIL: elreal_ln10().at(0) != M_LN10\n";
			++nrOfFailedTestCases;
		}
		if (elreal_sqrt2().at(0) != M_SQRT2) {
			std::cerr << "FAIL: elreal_sqrt2().at(0) != M_SQRT2\n";
			++nrOfFailedTestCases;
		}
	}

	// --- Multi-component expansion matches long-double reference -------
	nrOfFailedTestCases += check_constant("pi",    elreal_pi(),    REF_PI);
	nrOfFailedTestCases += check_constant("e",     elreal_e(),     REF_E);
	nrOfFailedTestCases += check_constant("ln2",   elreal_ln2(),   REF_LN2);
	nrOfFailedTestCases += check_constant("ln10",  elreal_ln10(),  REF_LN10);
	nrOfFailedTestCases += check_constant("sqrt2", elreal_sqrt2(), REF_SQRT2);
	nrOfFailedTestCases += check_constant("phi",   elreal_phi(),   REF_PHI);

	// --- Each constant has exactly 4 materialised components -----------
	// from_expansion populates _components eagerly; no generator is installed,
	// so computed_depth() == 4 and at(k>=4) returns 0.0 (the implicit-zero
	// extension noted in the Phase A docblock).
	{
		auto check_depth = [&](const char* name, const elreal& v) {
			if (v.computed_depth() != 4) {
				std::cerr << "FAIL: " << name << " computed_depth() = "
					<< v.computed_depth() << " (expected 4)\n";
				++nrOfFailedTestCases;
			}
			if (v.at(4) != 0.0 || v.at(10) != 0.0) {
				std::cerr << "FAIL: " << name << " at(>=4) is not the implicit zero "
					<< "extension\n";
				++nrOfFailedTestCases;
			}
		};
		check_depth("pi",   elreal_pi());
		check_depth("e",    elreal_e());
		check_depth("ln2",  elreal_ln2());
		check_depth("ln10", elreal_ln10());
	}

	// --- Each constant is greater than its IEEE leading double when the
	// trailing correction is positive (and less when negative). This locks
	// the depth-1 sign of the correction.
	{
		// pi: true pi > double(pi)  (correction is +1.22e-16)
		if (!(elreal_pi() > elreal(M_PI))) {
			std::cerr << "FAIL: elreal_pi() not greater than elreal(M_PI) "
				<< "(rational vs rounded comparison from Phase D)\n";
			++nrOfFailedTestCases;
		}
		// e: true e > double(e)     (correction is +1.45e-16)
		if (!(elreal_e() > elreal(M_E))) {
			std::cerr << "FAIL: elreal_e() not greater than elreal(M_E)\n";
			++nrOfFailedTestCases;
		}
		// ln10: true ln10 < double(ln10)  (correction is -2.17e-16)
		if (!(elreal_ln10() < elreal(M_LN10))) {
			std::cerr << "FAIL: elreal_ln10() not less than elreal(M_LN10) "
				<< "(correction sign disagreement)\n";
			++nrOfFailedTestCases;
		}
	}

	// --- Each constant is distinct from itself constructed from double:
	// the depth-1+ correction makes them observably different (this is the
	// same lazy-real distinctness test from Phase D's compare.cpp).
	{
		if (elreal_pi() == elreal(M_PI)) {
			std::cerr << "FAIL: elreal_pi() == elreal(M_PI) "
				<< "(lazy-real distinctness lost; correction not being seen)\n";
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
