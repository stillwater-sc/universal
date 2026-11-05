// nearest.cpp: conversion from double to dbns gives the nearest encoding, for every configuration
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// dbns<16,5> converted 1.5 to 1.4e122 and dbns<16,8> to -0.25 (#1933). Three defects, none of which
// conversion.cpp could see, since its VerifyConversion prints a table and checks nothing:
//   - the encoding was written into the most significant block alone, so every dbns wider than its
//     block type, dbns<10,4> with the default uint8_t included, converted almost every value wrongly;
//   - the (a, b) search admitted first base exponents beyond the field, and projecting them back
//     failed for a third of all values, which landed on the zero pattern or a far neighbour;
//   - minpos() produced the zero pattern, so minpos was 0 and minneg NaN.
// The value of a dbns is 0.5^a * 3^b, and by unique factorization each (a, b) is a distinct value,
// so the nearest encoding is well defined. The reference is a brute-force search over all of them.
#include <universal/utility/directives.hpp>
#include <universal/number/dbns/dbns.hpp>
#include <universal/verification/test_reporters.hpp>
#include <cmath>
#include <cstdint>
#include <random>
#include <vector>

namespace {

// every encoding whose value fits a double converts back to itself
template<unsigned nbits, unsigned fbbits, typename bt>
int VerifyRoundTrip(bool reportTestCases) {
	using namespace sw::universal;
	using Dbns = dbns<nbits, fbbits, bt>;
	int nrOfFailedTests = 0;
	for (std::uint64_t raw = 0; raw < (std::uint64_t(1) << nbits); ++raw) {
		Dbns a;
		a.setbits(raw);
		if (a.isnan() || a.iszero()) continue;
		double v = double(a);
		if (!std::isnormal(v)) continue;   // beyond double's range: 3^MAX_B overflows for wide fields
		Dbns b(v);
		if (!(a == b)) {
			if (reportTestCases && nrOfFailedTests < 3) std::cerr << "FAIL " << type_tag(Dbns()) << ' ' << to_binary(a) << " = " << v << " -> " << to_binary(b) << " = " << double(b) << '\n';
			++nrOfFailedTests;
		}
	}
	return nrOfFailedTests;
}

// a random value within range converts to the encoding nearest in log2
template<unsigned nbits, unsigned fbbits, typename bt>
int VerifyNearest(bool reportTestCases) {
	using namespace sw::universal;
	using Dbns = dbns<nbits, fbbits, bt>;
	std::vector<double> logs;   // log2 of every positive encoding that fits a double
	for (std::uint64_t raw = 0; raw < (std::uint64_t(1) << (nbits - 1)); ++raw) {
		Dbns a;
		a.setbits(raw);
		if (a.isnan() || a.iszero()) continue;
		double v = double(a);
		if (std::isnormal(v)) logs.push_back(std::log2(v));
	}
	double lo = logs[0], hi = logs[0];
	for (double l : logs) { if (l < lo) lo = l; if (l > hi) hi = l; }
	std::mt19937_64 rng(1933 + nbits * 64 + fbbits);
	std::uniform_real_distribution<double> uniform(lo, hi);
	int nrOfFailedTests = 0;
	for (int i = 0; i < 2000; ++i) {
		double l = uniform(rng);
		double v = ((i & 1) ? -1.0 : 1.0) * std::exp2(l);
		Dbns d(v);
		double got = std::fabs(std::log2(std::fabs(double(d))) - l);
		double best = 1.0e300;
		for (double x : logs) best = std::fmin(best, std::fabs(x - l));
		bool signOk = (d.sign() == (v < 0));
		if (!(got <= best + 1.0e-12) || !signOk) {
			if (reportTestCases && nrOfFailedTests < 3) std::cerr << "FAIL " << type_tag(Dbns()) << ' ' << v << " -> " << double(d) << ", log2 error " << got << " where " << best << " is reachable\n";
			++nrOfFailedTests;
		}
	}
	return nrOfFailedTests;
}

// minpos is 0.5^(MAX_A - 1), not zero; below half of it a value rounds to zero
template<unsigned nbits, unsigned fbbits, typename bt>
int VerifyLimits(bool reportTestCases) {
	using namespace sw::universal;
	using Dbns = dbns<nbits, fbbits, bt>;
	int nrOfFailedTests = 0;
	const double minpos = std::ldexp(1.0, -static_cast<int>((1u << fbbits) - 2));   // MAX_A = 2^fbbits - 1
	Dbns p(SpecificValue::minpos), n(SpecificValue::minneg);
	if (p.iszero() || double(p) != minpos) ++nrOfFailedTests;
	if (n.isnan() || double(n) != -minpos) ++nrOfFailedTests;
	if (double(std::numeric_limits<Dbns>::min()) != minpos) ++nrOfFailedTests;
	if (!Dbns(minpos * 0.49).iszero()) ++nrOfFailedTests;
	if (double(Dbns(minpos * 0.51)) != minpos) ++nrOfFailedTests;
	if (double(Dbns(minpos)) != minpos) ++nrOfFailedTests;
	if (nrOfFailedTests && reportTestCases) std::cerr << "FAIL " << type_tag(Dbns()) << " minpos " << double(p) << " minneg " << double(n) << '\n';
	return nrOfFailedTests;
}

// the configurations in the issue
int VerifyIssueCases(bool reportTestCases) {
	using namespace sw::universal;
	int nrOfFailedTests = 0;
	dbns<16, 5> a(1.5), b(0.75);
	if (double(a) != 1.5 || double(b) != 0.75 || !(b < a)) ++nrOfFailedTests;
	dbns<16, 8> c(1.5);
	if (double(c) != 1.5) ++nrOfFailedTests;
	dbns<8, 3> d(1.5);
	if (double(d) != 1.5) ++nrOfFailedTests;
	if (nrOfFailedTests && reportTestCases) std::cerr << "FAIL dbns<16,5>(1.5) = " << double(a) << ", dbns<16,8>(1.5) = " << double(c) << '\n';
	return nrOfFailedTests;
}

} // namespace

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

	std::string test_suite  = "dbns<> nearest conversion";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	dbns<16, 5> a(1.5);
	std::cout << to_binary(a) << " : " << a << '\n';

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyIssueCases(reportTestCases), "dbns<16,5>, dbns<16,8>", "issue cases");

	nrOfFailedTestCases += ReportTestResult(VerifyRoundTrip< 8, 3, std::uint8_t >(reportTestCases), "dbns<8,3,uint8>", "round trip");
	nrOfFailedTestCases += ReportTestResult(VerifyRoundTrip<10, 4, std::uint8_t >(reportTestCases), "dbns<10,4,uint8>", "round trip");
	nrOfFailedTestCases += ReportTestResult(VerifyRoundTrip<12, 3, std::uint8_t >(reportTestCases), "dbns<12,3,uint8>", "round trip");
	nrOfFailedTestCases += ReportTestResult(VerifyRoundTrip<12, 5, std::uint16_t>(reportTestCases), "dbns<12,5,uint16>", "round trip");
	nrOfFailedTestCases += ReportTestResult(VerifyRoundTrip<16, 5, std::uint8_t >(reportTestCases), "dbns<16,5,uint8>", "round trip");
	nrOfFailedTestCases += ReportTestResult(VerifyRoundTrip<16, 8, std::uint8_t >(reportTestCases), "dbns<16,8,uint8>", "round trip");

	nrOfFailedTestCases += ReportTestResult(VerifyNearest< 8, 3, std::uint8_t >(reportTestCases), "dbns<8,3,uint8>", "nearest");
	nrOfFailedTestCases += ReportTestResult(VerifyNearest<10, 4, std::uint8_t >(reportTestCases), "dbns<10,4,uint8>", "nearest");
	nrOfFailedTestCases += ReportTestResult(VerifyNearest<10, 6, std::uint8_t >(reportTestCases), "dbns<10,6,uint8>", "nearest");
	nrOfFailedTestCases += ReportTestResult(VerifyNearest<12, 3, std::uint16_t>(reportTestCases), "dbns<12,3,uint16>", "nearest");
	nrOfFailedTestCases += ReportTestResult(VerifyNearest<12, 5, std::uint8_t >(reportTestCases), "dbns<12,5,uint8>", "nearest");

	nrOfFailedTestCases += ReportTestResult(VerifyLimits< 8, 3, std::uint8_t >(reportTestCases), "dbns<8,3,uint8>", "minpos");
	nrOfFailedTestCases += ReportTestResult(VerifyLimits<10, 4, std::uint8_t >(reportTestCases), "dbns<10,4,uint8>", "minpos");
	nrOfFailedTestCases += ReportTestResult(VerifyLimits<12, 5, std::uint16_t>(reportTestCases), "dbns<12,5,uint16>", "minpos");
#endif

#if REGRESSION_LEVEL_2
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
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
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
