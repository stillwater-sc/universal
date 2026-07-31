// unbounded.cpp: division by an interval containing zero, for Scalars with and
// without an infinity
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Division by an interval containing zero has an UNBOUNDED exact result set, and
// the only enclosure of it is [-inf, +inf]. That requires the Scalar to have an
// infinity -- and not every Scalar does:
//
//   double, cfloat  : has_infinity, and infinity() > max()          -> can represent
//   posit           : NaR instead of infinity; numeric_limits<posit>::infinity()
//                     returns MAXPOS, a finite value indistinguishable from data
//   lns             : has_infinity is correctly false
//
// Before the fix this produced the finite [-maxpos, maxpos] for posit: an
// interval claiming to enclose an unbounded set while excluding everything beyond
// maxpos. Silently returning a too-narrow enclosure is the one failure mode
// interval arithmetic must never have -- there is no correct value to return, so
// the operation now throws.
//
// interval<Scalar>::can_represent_unbounded is the compile-time query, derived
// from what infinity() actually IS rather than from the has_infinity claim,
// because the two disagree for posit.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <universal/number/posit/posit.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/lns/lns.hpp>
#include <universal/number/interval/interval.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

// Scalar HAS a usable infinity: the unbounded result must be representable, and
// the returned interval must actually be unbounded.
template<typename Scalar>
int VerifyUnboundedRepresentable(bool reportTestCases) {
	using namespace sw::universal;
	using I = interval<Scalar>;
	int nrOfFailures = 0;

	if constexpr (!I::can_represent_unbounded) {
		++nrOfFailures;
		if (reportTestCases) std::cerr << "FAIL: expected can_represent_unbounded == true\n";
	}
	else {
		try {
			const I q = I(Scalar(1), Scalar(4)) / I(Scalar(-1), Scalar(1));
			// A finite result here would be an enclosure that excludes part of the
			// true (unbounded) result set.
			if (!(q.lower() < -std::numeric_limits<Scalar>::max()) ||
			    !(q.upper() >  std::numeric_limits<Scalar>::max())) {
				++nrOfFailures;
				if (reportTestCases)
					std::cerr << "FAIL: [1,4]/[-1,1] returned a FINITE interval ["
					          << double(q.lower()) << ", " << double(q.upper())
					          << "] where the exact result set is unbounded\n";
			}
		}
		catch (const interval_unrepresentable_unbounded&) {
			++nrOfFailures;
			if (reportTestCases)
				std::cerr << "FAIL: threw although the Scalar has an infinity\n";
		}
	}
	return nrOfFailures;
}

// Scalar has NO usable infinity: the operation must throw rather than return a
// finite interval that claims to enclose an unbounded set.
template<typename Scalar>
int VerifyUnboundedThrows(bool reportTestCases) {
	using namespace sw::universal;
	using I = interval<Scalar>;
	int nrOfFailures = 0;

	if constexpr (I::can_represent_unbounded) {
		++nrOfFailures;
		if (reportTestCases) std::cerr << "FAIL: expected can_represent_unbounded == false\n";
	}
	else {
		bool threw = false;
		try {
			const I q = I(Scalar(1), Scalar(4)) / I(Scalar(-1), Scalar(1));
			if (reportTestCases)
				std::cerr << "FAIL: returned [" << double(q.lower()) << ", " << double(q.upper())
				          << "] instead of throwing; that interval excludes everything beyond max()\n";
		}
		catch (const interval_unrepresentable_unbounded&) {
			threw = true;
		}
		if (!threw) ++nrOfFailures;
	}
	return nrOfFailures;
}

// The guard must fire ONLY for a zero-containing divisor: ordinary division is
// untouched for every Scalar.
template<typename Scalar>
int VerifyOrdinaryDivisionUnaffected(bool reportTestCases) {
	using namespace sw::universal;
	using I = interval<Scalar>;
	int nrOfFailures = 0;
	try {
		const I q = I(Scalar(1), Scalar(4)) / I(Scalar(2), Scalar(4));
		// [1,4]/[2,4] = [0.25, 2]; require a FINITE enclosure strictly inside +-max() so the
		// test catches ordinary division being wrongly routed through the zero-divisor path
		// (which would return an unbounded / +-max() interval that still encloses [0.25, 2]).
		if (!q.isfinite() ||
		    !(q.lower() <= Scalar(0.25)) || !(q.upper() >= Scalar(2)) ||
		    !(q.lower() > -std::numeric_limits<Scalar>::max()) ||
		    !(q.upper() < std::numeric_limits<Scalar>::max())) {
			++nrOfFailures;
			if (reportTestCases)
				std::cerr << "FAIL: [1,4]/[2,4] = [" << double(q.lower()) << ", "
				          << double(q.upper()) << "], expected to enclose [0.25, 2]\n";
		}
	}
	catch (...) {
		++nrOfFailures;
		if (reportTestCases) std::cerr << "FAIL: ordinary division threw\n";
	}
	return nrOfFailures;
}

} // namespace

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "interval unbounded-result validation";
	std::string test_tag    = "unbounded division";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// Scalars with a real infinity.
	nrOfFailedTestCases += ReportTestResult(
		VerifyUnboundedRepresentable<float>(reportTestCases), "float", test_tag);
	nrOfFailedTestCases += ReportTestResult(
		VerifyUnboundedRepresentable<double>(reportTestCases), "double", test_tag);
	nrOfFailedTestCases += ReportTestResult(
		(VerifyUnboundedRepresentable<cfloat<32, 8>>(reportTestCases)), "cfloat<32,8>", test_tag);

	// Scalars without one: posit has NaR, lns reports has_infinity = false.
	nrOfFailedTestCases += ReportTestResult(
		(VerifyUnboundedThrows<posit<16, 2>>(reportTestCases)), "posit<16,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(
		(VerifyUnboundedThrows<posit<32, 2>>(reportTestCases)), "posit<32,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(
		(VerifyUnboundedThrows<lns<32, 16>>(reportTestCases)), "lns<32,16>", test_tag);

	// And nothing else changed.
	nrOfFailedTestCases += ReportTestResult(
		VerifyOrdinaryDivisionUnaffected<double>(reportTestCases), "double ordinary", test_tag);
	nrOfFailedTestCases += ReportTestResult(
		(VerifyOrdinaryDivisionUnaffected<posit<32, 2>>(reportTestCases)), "posit<32,2> ordinary", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Uncaught arithmetic exception: " << err.what() << std::endl;
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
