// sqrt.cpp: sqrt of a valid against an exact oracle: containment and one-tile tightness of each end
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// sqrt is monotone, so each end of sqrt(v) is the root of the matching end of v, rounded
// outward to a tile: a lattice point when the root is one exactly, otherwise the open gap that
// holds it. An open end whose root is a lattice point moves to the gap beyond that point. A
// negative lower end clamps to a closed 0, and an entirely negative valid gives NaR (#1690).
//
// The oracle decides every end exactly: the lattice values of these configurations have at
// most 14 significand bits, so their squares are exact in double, and a root is located by
// comparing the input end against the squares of the lattice points, never by a rounded sqrt.
#include <universal/utility/directives.hpp>
#include <cstdint>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <universal/number/valid/valid.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

template<typename Valid>
class SqrtOracle {
public:
	using poxel_type = typename Valid::poxel_type;
	using key_type   = typename Valid::key_type;
	static constexpr key_type kmax = Valid::kmax;

	SqrtOracle() {
		// the nonnegative lattice points, key 0, 2, ..., kmax - 1
		for (key_type k = 0; k < kmax; k += 2) squares.push_back(value(k) * value(k));
	}

	static double value(key_type k) {   // the value of the lattice point at an even key
		poxel_type p{};
		p.setkey(k);
		return p.template lower<double>();
	}
	static Valid from_keys(key_type lo, key_type hi) {
		poxel_type a{}, b{};
		a.setkey(lo);
		b.setkey(hi);
		return Valid(a, b);
	}

	// the tile of sqrt(x), for x the value of the lattice point at the even key k > 0: the
	// point whose square is x, or the gap between the points whose squares bracket it
	key_type root_tile(key_type k) const {
		const double x = value(k);
		std::size_t lo = 0, hi = squares.size() - 1;   // the largest j with squares[j] <= x
		while (lo < hi) {
			const std::size_t mid = lo + (hi - lo + 1) / 2;
			if (squares[mid] <= x) lo = mid; else hi = mid - 1;
		}
		const key_type point = static_cast<key_type>(2 * lo);
		return (squares[lo] == x) ? point : point + 1;
	}

	// the expected lower key of sqrt for an input lower key a (with a nonnegative upper end)
	key_type expected_lo(key_type a) const {
		if (a <= 0) return 0;                              // the nonnegative part starts at a closed 0
		if ((a & 1) == 0) return root_tile(a);             // a closed point
		const key_type t = root_tile(a - 1);               // an open gap above the point a - 1
		return ((t & 1) == 0) ? t + 1 : t;                 // an exact root moves to the gap above it
	}
	// the expected upper key of sqrt for an input upper key b >= 0
	key_type expected_hi(key_type b) const {
		if (b == kmax) return kmax;                        // (maxpos, inf) has no upper bound
		if (b == 0) return 0;
		if ((b & 1) == 0) return root_tile(b);             // a closed point
		const key_type t = root_tile(b + 1);               // an open gap below the point b + 1
		return ((t & 1) == 0) ? t - 1 : t;                 // an exact root moves to the gap below it
	}

	// one input, given by its tile keys: 0 on success
	int check(key_type a, key_type b, bool reportTestCases, int& reported) const {
		const Valid v = from_keys(a, b);
		const Valid r = sqrt(v);
		bool ok;
		key_type wantLo = 0, wantHi = 0;
		if (b < 0) {
			ok = r.isnar() && r.isnan();
		}
		else {
			wantLo = expected_lo(a);
			wantHi = expected_hi(b);
			if (wantHi < wantLo) wantHi = wantLo;
			ok = !r.isnar() && r.lo_key() == wantLo && r.hi_key() == wantHi;
		}
		if (!ok && reportTestCases && reported++ < 10) {
			std::cerr << "FAIL sqrt(keys [" << a << ", " << b << "]) = keys [" << r.lo_key() << ", " << r.hi_key() << "]"
			          << (r.isnar() ? " NaR" : "") << ", expected [" << wantLo << ", " << wantHi << "]\n";
		}
		return ok ? 0 : 1;
	}

private:
	std::vector<double> squares;
};

// every interval of a small configuration: every pair of tile keys a <= b
template<typename Valid>
int VerifyAllIntervals(bool reportTestCases) {
	using key_type = typename Valid::key_type;
	const SqrtOracle<Valid> oracle;
	int fails = 0, reported = 0;
	for (key_type a = -Valid::kmax; a <= Valid::kmax; ++a)
		for (key_type b = a; b <= Valid::kmax; ++b) fails += oracle.check(a, b, reportTestCases, reported);
	return fails;
}

// every single tile of a larger configuration, and random intervals
template<typename Valid>
int VerifyTilesAndSamples(bool reportTestCases, int samples) {
	using key_type = typename Valid::key_type;
	const SqrtOracle<Valid> oracle;
	int fails = 0, reported = 0;
	for (key_type k = -Valid::kmax; k <= Valid::kmax; ++k) fails += oracle.check(k, k, reportTestCases, reported);
	std::mt19937_64 rng(1690);
	std::uniform_int_distribution<long long> key(-static_cast<long long>(Valid::kmax), static_cast<long long>(Valid::kmax));
	for (int i = 0; i < samples; ++i) {
		key_type a = static_cast<key_type>(key(rng)), b = static_cast<key_type>(key(rng));
		if (b < a) std::swap(a, b);
		fails += oracle.check(a, b, reportTestCases, reported);
	}
	return fails;
}

// NaR propagates, and the accessors the enclosure utilities read agree with valid's own
template<typename Valid>
int VerifyAccessors(bool reportTestCases) {
	int fails = 0;
	const Valid nar = Valid::nar();
	if (!nar.isnan() || !sqrt(nar).isnan()) ++fails;
	const Valid v(1.0, 4.0);
	if (v.isnan() || v.lo_key() != v.lb().key() || v.hi_key() != v.ub().key()) ++fails;
	const Valid r = sqrt(v);
	if (r.lower() != 1.0 || r.upper() != 2.0 || r.lower_open() || r.upper_open()) ++fails;
	if (fails && reportTestCases) std::cerr << "FAIL accessors: sqrt([1, 4]) = [" << r.lower() << ", " << r.upper() << "]\n";
	return fails;
}

}  // namespace

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

	std::string test_suite  = "valid sqrt";
	std::string test_tag    = "sqrt";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	using V = valid<8, 1>;
	std::cout << sqrt(V(2.0)).lower() << ' ' << sqrt(V(2.0)).upper() << '\n';
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyAccessors<valid<8, 1>>(reportTestCases), "valid<8,1>", "isnan, lo_key, hi_key");
	nrOfFailedTestCases += ReportTestResult(VerifyAllIntervals<valid<6, 0>>(reportTestCases), "valid<6,0>", "sqrt of every interval");
	nrOfFailedTestCases += ReportTestResult(VerifyAllIntervals<valid<8, 1>>(reportTestCases), "valid<8,1>", "sqrt of every interval");
	nrOfFailedTestCases += ReportTestResult(VerifyAllIntervals<valid<10, 1>>(reportTestCases), "valid<10,1>", "sqrt of every interval");
	nrOfFailedTestCases += ReportTestResult(VerifyTilesAndSamples<valid<16, 2>>(reportTestCases, 20000), "valid<16,2>", "sqrt of every tile");
#endif

#if REGRESSION_LEVEL_2
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyAllIntervals<valid<12, 2>>(reportTestCases), "valid<12,2>", "sqrt of every interval");
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught unexpected runtime error: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
