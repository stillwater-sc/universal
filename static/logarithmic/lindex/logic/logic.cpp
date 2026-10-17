// logic.cpp: lindex comparison operators (#1694)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// lindex compares two encodings as signed integers.  The claim the design doc makes is
// that this orders them by value, because phi is increasing and a negative is the two's
// complement of its magnitude.  The reference here does not read the encoding as a signed
// integer: it decodes the sign and the psi of the magnitude and orders by signed psi.  Where
// both values are finite doubles, the order of the doubles must agree as well; conversion
// to double is checked against the oracle in conversion/conversion.cpp.
//
// NaR follows the posit convention: equal to itself, and below every other value.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <cstdint>
#include <random>
#include <string>
#include <universal/number/lindex/lindex.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

using namespace sw::universal;

// -1, 0, 1 as a compares below, equal to or above b, by the decoded value
template<typename L>
int reference_order(const L& a, const L& b) {
	if (a.isnar() || b.isnar()) {
		if (a.isnar() && b.isnar()) return 0;
		return a.isnar() ? -1 : 1;
	}
	// signed psi as an exact rational over 2^fbits: (sign) (level * 2^fbits + index_bits)
	auto key = [](const L& v) {
		const long double m = static_cast<long double>(static_cast<std::uint64_t>(v.level()) << L::fbits) + static_cast<long double>(v.index_bits());
		return v.isneg() ? -m : m;
	};
	// a long double may round a 63-bit magnitude, so fall back to the fields when it ties
	const long double ka = key(a), kb = key(b);
	if (ka != kb) return (ka < kb) ? -1 : 1;
	if (a.isneg() != b.isneg()) return a.isneg() ? -1 : 1;
	if (a.level() != b.level()) return ((a.level() < b.level()) != a.isneg()) ? -1 : 1;
	if (a.index_bits() != b.index_bits()) return ((a.index_bits() < b.index_bits()) != a.isneg()) ? -1 : 1;
	return 0;
}

template<typename L>
int VerifyPair(const L& a, const L& b, bool reportTestCases) {
	const int ref = reference_order(a, b);
	const bool ok = ((a == b) == (ref == 0)) && ((a != b) == (ref != 0)) &&
	                ((a <  b) == (ref <  0)) && ((a >  b) == (ref >  0)) &&
	                ((a <= b) == (ref <= 0)) && ((a >= b) == (ref >= 0));
	// and the doubles agree, where both values are finite doubles
	bool doubles_ok = true;
	if (!a.isnar() && !b.isnar()) {
		const double da = double(a), db = double(b);
		if (std::isfinite(da) && std::isfinite(db) && da != db) doubles_ok = ((da < db) == (ref < 0));
	}
	if ((!ok || !doubles_ok) && reportTestCases) {
		std::cerr << "FAIL: " << type_tag(a) << ' ' << to_hex(a) << " vs " << to_hex(b) << ", reference " << ref
		          << (doubles_ok ? "" : ", disagrees with the doubles") << '\n';
	}
	return (ok && doubles_ok) ? 0 : 1;
}

// every pair of encodings
template<unsigned nbits, unsigned lbits>
int VerifyExhaustive(bool reportTestCases) {
	using L = lindex<nbits, lbits>;
	constexpr std::uint64_t nr = 1ull << nbits;
	int fails = 0;
	L a, b;
	for (std::uint64_t i = 0; i < nr; ++i) {
		a.setbits(i);
		for (std::uint64_t j = 0; j < nr; ++j) {
			b.setbits(j);
			fails += VerifyPair(a, b, reportTestCases);
		}
	}
	return fails;
}

// random pairs, plus every encoding against the specials and its own neighbours
template<unsigned nbits, unsigned lbits>
int VerifySampled(unsigned nrPairs, bool reportTestCases) {
	using L = lindex<nbits, lbits>;
	constexpr std::uint64_t mask = (nbits == 64) ? ~0ull : ((1ull << nbits) - 1ull);
	std::mt19937_64 rng(1694);
	const L specials[] = { L(SpecificValue::nar), L(SpecificValue::maxneg), L(SpecificValue::minneg), L(0),
	                       L(SpecificValue::minpos), L(SpecificValue::maxpos), L(1.0), L(-1.0) };
	int fails = 0;
	L a, b;
	for (unsigned k = 0; k < nrPairs; ++k) {
		a.setbits(rng() & mask);
		b.setbits(rng() & mask);
		fails += VerifyPair(a, b, reportTestCases);
		for (const L& s : specials) { fails += VerifyPair(a, s, reportTestCases); fails += VerifyPair(s, a, reportTestCases); }
		b.setbits((a.raw_bits() + 1ull) & mask);
		fails += VerifyPair(a, b, reportTestCases);
	}
	return fails;
}

}  // anonymous namespace

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

	std::string test_suite  = "lindex comparison (#1694)";
	std::string test_tag    = "logic";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<6, 3>(reportTestCases), "lindex<6,3>", "comparison");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive< 8, 3>(reportTestCases), "lindex< 8,3>", "comparison, every pair");
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive< 8, 2>(reportTestCases), "lindex< 8,2>", "comparison, every pair");
	nrOfFailedTestCases += ReportTestResult(VerifySampled<16, 3>(2000, reportTestCases), "lindex<16,3>", "comparison, sampled");
	nrOfFailedTestCases += ReportTestResult(VerifySampled<32, 3>(2000, reportTestCases), "lindex<32,3>", "comparison, sampled");
	nrOfFailedTestCases += ReportTestResult(VerifySampled<64, 3>(2000, reportTestCases), "lindex<64,3>", "comparison, sampled");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<10, 3>(reportTestCases), "lindex<10,3>", "comparison, every pair");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifySampled<64, 3>(50000, reportTestCases), "lindex<64,3>", "comparison, sampled");
#endif

#if REGRESSION_LEVEL_4
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
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
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
