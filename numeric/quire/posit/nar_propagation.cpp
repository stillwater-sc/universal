//  nar_propagation.cpp : verify the quire honors the no-throw configuration and
//  propagates NaR as a value instead of throwing (issue #1226)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// This translation unit deliberately exercises the DEFAULT (opt-out) exception
// configuration: posit.hpp defaults POSIT_THROW_ARITHMETIC_EXCEPTION to 0 and now
// forwards that to QUIRE_THROW_ARITHMETIC_EXCEPTION, so the quire must NOT throw.
#include <universal/number/posit/posit.hpp>
#include <universal/number/quire/quire.hpp>
#include <universal/verification/test_reporters.hpp>

#include <iostream>
#include <string>

// A NaR operand reaching the quire sets the sticky non-finite state, propagates
// through further accumulation, and resolves back to the posit's NaR -- without
// throwing (the reporter's C-ABI / noexcept use case).
template<typename Posit>
int VerifyQuireNaRPropagation(bool reportTestCases) {
	using namespace sw::universal;
	int nrOfFailedTestCases = 0;

	Posit nar; nar.setnar();
	Posit one(1.0f);

	quire<Posit> q;
	// the reproducer: this used to throw sw::universal::operand_is_nar
	q += quire_mul(nar, one);
	if (!q.isnan()) { ++nrOfFailedTestCases; if (reportTestCases) std::cout << "FAIL: quire did not enter NaR state after NaR product\n"; }

	// NaR resolves back to the posit's NaR
	Posit r = quire_resolve(q);
	if (!r.isnar()) { ++nrOfFailedTestCases; if (reportTestCases) std::cout << "FAIL: quire_resolve did not return NaR\n"; }

	// NaR is sticky: accumulating a finite product does not clear it
	q += quire_mul(one, one);
	if (!q.isnan()) { ++nrOfFailedTestCases; if (reportTestCases) std::cout << "FAIL: NaR state was not sticky under further accumulation\n"; }

	// a fresh assignment of a finite value clears the NaR state
	q = quire_mul(one, one);
	if (q.isnan()) { ++nrOfFailedTestCases; if (reportTestCases) std::cout << "FAIL: finite assignment did not clear NaR state\n"; }

	return nrOfFailedTestCases;
}

// A NaR quire compares unordered (IEEE-style): every comparison is false, and
// inequality is true -- including against itself.
template<typename Posit>
int VerifyQuireNaRComparisons(bool reportTestCases) {
	using namespace sw::universal;
	int nrOfFailedTestCases = 0;

	Posit nar; nar.setnar();
	Posit one(1.0f);

	quire<Posit> a;  a += quire_mul(nar, one);  // NaR
	quire<Posit> b;  b += quire_mul(one, one);  // finite

	auto expect = [&](bool cond, const char* what) {
		if (!cond) { ++nrOfFailedTestCases; if (reportTestCases) std::cout << "FAIL: " << what << '\n'; }
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

	std::string test_suite          = "posit<> quire NaR propagation (no-throw config, #1226)";
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
	nrOfFailedTestCases += VerifyQuireNaRPropagation<posit<16, 1>>(reportTestCases);
	nrOfFailedTestCases += VerifyQuireNaRComparisons<posit<16, 1>>(reportTestCases);
#	endif

#	if REGRESSION_LEVEL_2
	nrOfFailedTestCases += VerifyQuireNaRPropagation<posit<32, 2>>(reportTestCases);
	nrOfFailedTestCases += VerifyQuireNaRComparisons<posit<32, 2>>(reportTestCases);
	nrOfFailedTestCases += VerifyQuireNaRPropagation<posit<8, 0>>(reportTestCases);
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
