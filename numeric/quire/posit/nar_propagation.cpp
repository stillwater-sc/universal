//  nar_propagation.cpp : verify the quire honors the no-throw configuration and
//  propagates NaR as a value instead of throwing (issue #1226)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// This translation unit deliberately exercises the DEFAULT (opt-out) exception
// configuration: each number-system umbrella defaults *_THROW_ARITHMETIC_EXCEPTION
// to 0 and now forwards that to QUIRE_THROW_ARITHMETIC_EXCEPTION, so the quire must
// NOT throw. It covers the shared quire NaR machinery across the representative
// number systems whose quire_resolve NaN handling changed (posit, cfloat, lns, dbns).
#include <universal/number/posit/posit.hpp>       // includes posit/fdp.hpp (quire_mul/quire_resolve)
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/cfloat/fdp.hpp>        // fdp is opt-in for the non-posit types
#include <universal/number/lns/lns.hpp>
#include <universal/number/lns/fdp.hpp>
#include <universal/number/dbns/dbns.hpp>
#include <universal/number/dbns/fdp.hpp>
#include <universal/number/quire/quire.hpp>
#include <universal/verification/test_reporters.hpp>

#include <iostream>
#include <string>

// The non-finite API differs by type: posit uses setnar()/isnar(), while
// cfloat/lns/dbns use setnan()/isnan(). Bridge them with C++20 requires.
template<typename Scalar> void set_nonfinite(Scalar& x) {
	if constexpr (requires { x.setnar(); }) x.setnar(); else x.setnan();
}
template<typename Scalar> bool is_nonfinite(const Scalar& x) {
	if constexpr (requires { x.isnar(); }) return x.isnar(); else return x.isnan();
}

// A NaR operand reaching the quire sets the sticky non-finite state, propagates
// through further accumulation, and resolves back to the number system's NaR --
// without throwing (the reporter's C-ABI / noexcept use case).
template<typename Scalar>
int VerifyQuireNaRPropagation(bool reportTestCases) {
	using namespace sw::universal;
	int nrOfFailedTestCases = 0;
	std::string tag = type_tag(Scalar());

	Scalar nan; set_nonfinite(nan);
	Scalar one(1.0f);

	quire<Scalar> q;
	// the reproducer: this used to throw sw::universal::operand_is_nar
	q += quire_mul(nan, one);
	if (!q.isnan()) { ++nrOfFailedTestCases; if (reportTestCases) std::cout << "FAIL(" << tag << "): quire did not enter NaR state after NaR product\n"; }

	// NaR resolves back to the number system's NaR
	Scalar r = quire_resolve(q);
	if (!is_nonfinite(r)) { ++nrOfFailedTestCases; if (reportTestCases) std::cout << "FAIL(" << tag << "): quire_resolve did not return NaR\n"; }

	// NaR is sticky: accumulating a finite product does not clear it
	q += quire_mul(one, one);
	if (!q.isnan()) { ++nrOfFailedTestCases; if (reportTestCases) std::cout << "FAIL(" << tag << "): NaR state was not sticky under further accumulation\n"; }

	// a fresh assignment of a finite value clears the NaR state
	q = quire_mul(one, one);
	if (q.isnan()) { ++nrOfFailedTestCases; if (reportTestCases) std::cout << "FAIL(" << tag << "): finite assignment did not clear NaR state\n"; }

	return nrOfFailedTestCases;
}

// A NaR quire compares unordered (IEEE-style): every comparison is false, and
// inequality is true -- including against itself.
template<typename Scalar>
int VerifyQuireNaRComparisons(bool reportTestCases) {
	using namespace sw::universal;
	int nrOfFailedTestCases = 0;
	std::string tag = type_tag(Scalar());

	Scalar nan; set_nonfinite(nan);
	Scalar one(1.0f);

	quire<Scalar> a;  a += quire_mul(nan, one);  // NaR
	quire<Scalar> b;  b += quire_mul(one, one);  // finite

	auto expect = [&](bool cond, const char* what) {
		if (!cond) { ++nrOfFailedTestCases; if (reportTestCases) std::cout << "FAIL(" << tag << "): " << what << '\n'; }
	};
	expect(!(a == b), "NaR == finite should be false");
	expect(  a != b , "NaR != finite should be true");
	expect(!(a <  b), "NaR <  finite should be false");
	expect(!(a <= b), "NaR <= finite should be false");
	expect(!(a >  b), "NaR >  finite should be false");
	expect(!(a >= b), "NaR >= finite should be false");
	expect(!(a == a), "NaR == NaR should be false (unordered)");
	expect(  a != a , "NaR != NaR should be true (unordered)");

	return nrOfFailedTestCases;
}

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
#ifndef REGRESSION_LEVEL_OVERRIDE
#	undef REGRESSION_LEVEL_1
#	undef REGRESSION_LEVEL_2
#	undef REGRESSION_LEVEL_3
#	undef REGRESSION_LEVEL_4
#	define REGRESSION_LEVEL_1 1
#	define REGRESSION_LEVEL_2 1
#	define REGRESSION_LEVEL_3 0
#	define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite          = "quire NaR propagation (no-throw config, #1226)";
	std::string test_tag            = "quire NaR";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += VerifyQuireNaRPropagation<posit<16, 1>>(reportTestCases);
	nrOfFailedTestCases += VerifyQuireNaRComparisons<posit<16, 1>>(reportTestCases);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;  // ignore errors
#else

#	if REGRESSION_LEVEL_1
	// posit is the canonical quire user; also cover the other changed resolvers
	nrOfFailedTestCases += VerifyQuireNaRPropagation<posit<16, 1>>(reportTestCases);
	nrOfFailedTestCases += VerifyQuireNaRComparisons<posit<16, 1>>(reportTestCases);
	nrOfFailedTestCases += VerifyQuireNaRPropagation<cfloat<8, 3, std::uint8_t, true, false, false>>(reportTestCases);
	nrOfFailedTestCases += VerifyQuireNaRPropagation<lns<8, 4>>(reportTestCases);
	nrOfFailedTestCases += VerifyQuireNaRPropagation<dbns<8, 3>>(reportTestCases);
#	endif

#	if REGRESSION_LEVEL_2
	nrOfFailedTestCases += VerifyQuireNaRPropagation<posit<32, 2>>(reportTestCases);
	nrOfFailedTestCases += VerifyQuireNaRPropagation<posit<8, 0>>(reportTestCases);
	nrOfFailedTestCases += VerifyQuireNaRComparisons<cfloat<8, 3, std::uint8_t, true, false, false>>(reportTestCases);
	nrOfFailedTestCases += VerifyQuireNaRComparisons<lns<8, 4>>(reportTestCases);
#	endif

#	if REGRESSION_LEVEL_3
#	endif

#	if REGRESSION_LEVEL_4
#	endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
} catch (char const* msg) {
	std::cerr << msg << '\n';
	return EXIT_FAILURE;
} catch (const sw::universal::quire_exception& err) {
	// A quire exception here is itself a failure: this suite runs in the
	// no-throw configuration, so NaR/overflow must never throw.
	std::cerr << "unexpected quire exception (no-throw config was requested): " << err.what() << '\n';
	return EXIT_FAILURE;
} catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
