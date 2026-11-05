// ordering.cpp: cfloat relational operators compare encodings, not differences
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cstdint>
#include <iostream>
#include <string>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_suite.hpp>

/*
   With subnormals, operator< used to compute lhs - rhs and test the sign of the difference, so
   a comparison cost a full subtraction (#1920). It now compares the encodings: with NaN set
   aside, the magnitude bits of a sign-magnitude encoding order the values, so after the NaN and
   zero checks a comparison is a limb-by-limb compare from the MSU down, reversed for negatives.

   This test holds the six relational operators to the value order:
   - exhaustively, for every pair of encodings of small configurations, in all eight combinations
     of hasSubnormals, hasMaxExpValues and isSaturating, against the double values. Without
     subnormals every all-zeros-exponent encoding is a zero, and the reference treats it so.
   - on random pairs of a 3-limb configuration, against the double values.
   - on random, neighbouring (one bit flipped, low or anywhere) and negated pairs of wide
     configurations, against the definition the operator used to have: the sign of the
     difference, computed in the subnormal twin of the type.
*/

namespace sw {
namespace universal {
namespace ordering {

struct Rng {  // splitmix64: reproducible on every platform
	std::uint64_t state;
	std::uint64_t next() {
		std::uint64_t z = (state += 0x9E3779B97F4A7C15ull);
		z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
		z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
		return z ^ (z >> 31);
	}
};

template<typename Cfloat>
void randomize(Cfloat& c, Rng& rng) {
	std::uint64_t word = 0;
	for (unsigned i = 0; i < Cfloat::nbits; ++i) {
		if ((i % 64u) == 0) word = rng.next();
		c.setbit(i, ((word >> (i % 64u)) & 1ull) != 0);
	}
}

// the six operators against a reference less-than and the NaN state of the operands
template<typename Cfloat>
int CheckPair(const Cfloat& a, const Cfloat& b, bool refLess, bool refGreater, bool reportTestCases) {
	const bool unordered = a.isnan() || b.isnan();
	const bool refEqual  = !unordered && !refLess && !refGreater;
	int fails = 0;
	auto check = [&](bool result, bool reference, const char* op) {
		if (result != reference) {
			++fails;
			if (reportTestCases && fails < 4)
				std::cerr << "FAIL " << to_binary(a) << ' ' << op << ' ' << to_binary(b) << " gave " << result << '\n';
		}
	};
	check(a < b, !unordered && refLess, "<");
	check(a > b, !unordered && refGreater, ">");
	check(a <= b, !unordered && (refLess || refEqual), "<=");
	check(a >= b, !unordered && (refGreater || refEqual), ">=");
	check(a == b, refEqual, "==");
	check(a != b, !refEqual, "!=");
	return fails;
}

template<typename Cfloat>
double referenceValue(const Cfloat& c) {
	return c.iszero() ? 0.0 : double(c);  // without subnormals every all-zeros-exponent encoding is 0
}

template<unsigned nbits, unsigned es, typename bt, bool sub, bool sup, bool sat>
int VerifyExhaustive(bool reportTestCases) {
	using Cfloat         = cfloat<nbits, es, bt, sub, sup, sat>;
	constexpr unsigned N = 1u << nbits;
	int fails = 0;
	Cfloat a{}, b{};
	for (unsigned i = 0; i < N; ++i) {
		a.setbits(i);
		const double da = referenceValue(a);
		for (unsigned j = 0; j < N; ++j) {
			b.setbits(j);
			const double db = referenceValue(b);
			fails += CheckPair(a, b, da < db, da > db, reportTestCases);
		}
	}
	return fails;
}

template<unsigned nbits, unsigned es, typename bt>
int VerifyAllFlags(const std::string& tag, int& total, bool reportTestCases) {
	int fails = 0;
	fails += ReportTestResult(VerifyExhaustive<nbits, es, bt, false, false, false>(reportTestCases), tag + " fff", "ordering");
	fails += ReportTestResult(VerifyExhaustive<nbits, es, bt, false, false, true>(reportTestCases), tag + " fft", "ordering");
	fails += ReportTestResult(VerifyExhaustive<nbits, es, bt, false, true, false>(reportTestCases), tag + " ftf", "ordering");
	fails += ReportTestResult(VerifyExhaustive<nbits, es, bt, false, true, true>(reportTestCases), tag + " ftt", "ordering");
	fails += ReportTestResult(VerifyExhaustive<nbits, es, bt, true, false, false>(reportTestCases), tag + " tff", "ordering");
	fails += ReportTestResult(VerifyExhaustive<nbits, es, bt, true, false, true>(reportTestCases), tag + " tft", "ordering");
	fails += ReportTestResult(VerifyExhaustive<nbits, es, bt, true, true, false>(reportTestCases), tag + " ttf", "ordering");
	fails += ReportTestResult(VerifyExhaustive<nbits, es, bt, true, true, true>(reportTestCases), tag + " ttt", "ordering");
	total += fails;
	return fails;
}

// random pairs of a configuration that a double holds exactly
template<unsigned nbits, unsigned es, typename bt, bool sub, bool sup, bool sat>
int VerifyRandomAgainstDouble(bool reportTestCases) {
	using Cfloat = cfloat<nbits, es, bt, sub, sup, sat>;
	static_assert(nbits - es <= 53, "the double reference must hold every value exactly");
	Rng rng{ 0x1920ull + nbits };
	int fails = 0;
	for (int trial = 0; trial < 200000; ++trial) {
		Cfloat a{}, b{};
		randomize(a, rng);
		randomize(b, rng);
		const double da = referenceValue(a), db = referenceValue(b);
		fails += CheckPair(a, b, da < db, da > db, reportTestCases);
	}
	return fails;
}

// the definition operator< had before #1920: the sign of lhs - rhs, in the subnormal twin of the type
template<unsigned nbits, unsigned es, typename bt, bool sub, bool sup, bool sat>
bool subtractionLess(const cfloat<nbits, es, bt, sub, sup, sat>& lhs, const cfloat<nbits, es, bt, sub, sup, sat>& rhs) {
	using Twin = cfloat<nbits, es, bt, true, sup, sat>;
	Twin l{}, r{};
	for (unsigned i = 0; i < nbits; ++i) {
		l.setbit(i, lhs.at(i));
		r.setbit(i, rhs.at(i));
	}
	if (lhs.iszero()) l.setzero();  // without subnormals every all-zeros-exponent encoding is 0
	if (rhs.iszero()) r.setzero();
	if (l.isinf(INF_TYPE_NEGATIVE) && r.isinf(INF_TYPE_NEGATIVE)) return false;
	if (l.isinf(INF_TYPE_POSITIVE) && r.isinf(INF_TYPE_POSITIVE)) return false;
	Twin diff = l - r;
	return !diff.iszero() && diff.sign();
}

template<unsigned nbits, unsigned es, typename bt, bool sub, bool sup, bool sat>
int VerifyWide(bool reportTestCases) {
	using Cfloat = cfloat<nbits, es, bt, sub, sup, sat>;
	Rng rng{ 0x1920ull * nbits + es + sizeof(bt) };
	int fails = 0;
	for (int trial = 0; trial < 3000; ++trial) {
		Cfloat a{}, b{};
		randomize(a, rng);
		switch (trial % 5) {
		case 0:  // an unrelated encoding
			randomize(b, rng);
			break;
		case 1:  // a neighbour: one low fraction bit flipped, so the compare reaches the low limbs
			{
				b = a;
				const unsigned k = static_cast<unsigned>(rng.next() % 8u);
				b.setbit(k, !a.at(k));
			}
			break;
		case 2:  // the negation
			b = a;
			b.setsign(!a.sign());
			break;
		case 3:  // a neighbour: one bit flipped anywhere below the sign, so the compare can stop in any limb
			{
				b = a;
				const unsigned k = static_cast<unsigned>(rng.next() % (nbits - 1u));
				b.setbit(k, !a.at(k));
			}
			break;
		default:  // the same value
			b = a;
			break;
		}
		if (a.isnan() || b.isnan()) continue;
		fails += CheckPair(a, b, subtractionLess(a, b), subtractionLess(b, a), reportTestCases);
	}
	return fails;
}

}  // namespace ordering
}  // namespace universal
}  // namespace sw

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
// #undef REGRESSION_LEVEL_OVERRIDE
#ifndef REGRESSION_LEVEL_OVERRIDE
#	undef REGRESSION_LEVEL_1
#	undef REGRESSION_LEVEL_2
#	undef REGRESSION_LEVEL_3
#	undef REGRESSION_LEVEL_4
#	define REGRESSION_LEVEL_1 1
#	define REGRESSION_LEVEL_2 0
#	define REGRESSION_LEVEL_3 0
#	define REGRESSION_LEVEL_4 0
#endif

int main() try {
	using namespace sw::universal;
	using namespace sw::universal::ordering;

	std::string test_suite          = "cfloat ordering of encodings";
	std::string test_tag            = "ordering";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	VerifyAllFlags<8, 3, std::uint8_t>("cfloat<8,3,uint8_t>", nrOfFailedTestCases, reportTestCases);
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;  // ignore failures
#else

#	if REGRESSION_LEVEL_1
	// every pair of encodings, in one limb and across two
	VerifyAllFlags<6, 2, std::uint8_t>("cfloat<6,2,uint8_t>", nrOfFailedTestCases, reportTestCases);
	VerifyAllFlags<8, 3, std::uint8_t>("cfloat<8,3,uint8_t>", nrOfFailedTestCases, reportTestCases);
	VerifyAllFlags<9, 4, std::uint8_t>("cfloat<9,4,uint8_t>", nrOfFailedTestCases, reportTestCases);
	VerifyAllFlags<10, 3, std::uint8_t>("cfloat<10,3,uint8_t>", nrOfFailedTestCases, reportTestCases);
	VerifyAllFlags<10, 3, std::uint16_t>("cfloat<10,3,uint16_t>", nrOfFailedTestCases, reportTestCases);

	// random pairs across three limbs
	nrOfFailedTestCases += ReportTestResult(VerifyRandomAgainstDouble<20, 5, std::uint8_t, true, false, false>(reportTestCases), "cfloat<20,5,uint8_t> tff", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyRandomAgainstDouble<20, 5, std::uint8_t, false, false, false>(reportTestCases), "cfloat<20,5,uint8_t> fff", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyRandomAgainstDouble<20, 5, std::uint8_t, true, true, true>(reportTestCases), "cfloat<20,5,uint8_t> ttt", test_tag);

	// wide configurations against the sign of the difference
	nrOfFailedTestCases += ReportTestResult(VerifyWide<80, 11, std::uint16_t, true, false, false>(reportTestCases), "cfloat<80,11,uint16_t> tff", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyWide<128, 15, std::uint8_t, true, false, false>(reportTestCases), "cfloat<128,15,uint8_t> tff", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyWide<196, 19, std::uint64_t, true, false, false>(reportTestCases), "cfloat<196,19,uint64_t> tff", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyWide<256, 19, std::uint32_t, true, true, false>(reportTestCases), "cfloat<256,19,uint32_t> ttf", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyWide<196, 19, std::uint64_t, false, false, false>(reportTestCases), "cfloat<196,19,uint64_t> fff", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyWide<128, 15, std::uint8_t, false, true, true>(reportTestCases), "cfloat<128,15,uint8_t> ftt", test_tag);
#	endif

#	if REGRESSION_LEVEL_2
#	endif

#	if REGRESSION_LEVEL_3
#	endif

#	if REGRESSION_LEVEL_4
#	endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
} catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
} catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
} catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
} catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
} catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
