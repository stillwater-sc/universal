// to_native_rounding.cpp: posit to float, double and long double is rounded to nearest, once
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// posit::to_native<Real>() used to sum the fraction bits in long double and narrow the product
// at the end (#1678).  Beyond long double's significand every further bit rounded on its own,
// adding the hidden bit rounded again and the narrowing a third time, so posit<128,2> and
// posit<256,2> converted to long double were not the nearest value in about one case in eight,
// and to double near a midpoint they rounded twice.  It now rounds once, from the bits.
//
// The reference is the exact value of the posit as a dyadic (utility/tile_oracle.hpp), taken
// from the poxel over the same lattice, whose decoder shares no code with posit's.  A result is
// correct when no neighbouring Real is strictly closer; an infinite result when the exact value
// is at or beyond the overflow threshold, max + ulp(max) / 2.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <cstdint>
#include <limits>
#include <random>
#include <string>
#include <universal/number/posit/posit.hpp>
#include <universal/number/poxel/poxel.hpp>
#include <universal/utility/tile_oracle.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

using namespace sw::universal;
namespace orc = sw::universal::oracle;

// the exact value of a long double, as a sum of doubles
orc::dyadic exact(long double v) {
	orc::dyadic s{};
	for (int i = 0; i < 4 && v != 0.0l; ++i) {
		const double d = static_cast<double>(v);
		s = orc::add(s, orc::from_double(d));
		v -= static_cast<long double>(d);
	}
	return s;
}

orc::dyadic magnitude(const orc::dyadic& v) { return (orc::compare(v, orc::dyadic{}) < 0) ? orc::neg(v) : v; }

// is r the round-to-nearest value of v in Real?
template<typename Real>
bool nearest(Real r, const orc::dyadic& v) {
	using L = std::numeric_limits<Real>;
	// the overflow threshold: max + ulp(max) / 2 = 2^emax - 2^(emax - digits - 1)
	const orc::dyadic threshold = orc::sub(orc::make_dyadic(1, L::max_exponent), orc::make_dyadic(1, L::max_exponent - L::digits - 1));
	const bool beyond = orc::compare(magnitude(v), threshold) >= 0;
	if (std::isinf(r)) return beyond && ((r < 0) == (orc::compare(v, orc::dyadic{}) < 0));
	if (beyond) return false;
	const orc::dyadic d = orc::sub(exact(static_cast<long double>(r)), v);
	for (Real n : { std::nextafter(r, -L::infinity()), std::nextafter(r, L::infinity()) }) {
		if (std::isinf(n)) continue;
		const orc::dyadic dn = orc::sub(exact(static_cast<long double>(n)), v);
		if (orc::compare(orc::mul(dn, dn), orc::mul(d, d)) < 0) return false;
	}
	return true;
}

template<unsigned nbits, unsigned es, typename bt>
int VerifyOne(const posit<nbits, es, bt>& p, int& fails, bool report) {
	if (p.isnar() || p.iszero()) return fails;
	const poxel<nbits + 1, es, bt> x(p);                       // the exact tile at p's value
	const orc::dyadic v = orc::exact_value(x);
	auto check = [&](bool ok, const char* to) {
		if (ok) return;
		++fails;
		if (report && fails < 10) std::cerr << "FAIL " << type_tag(p) << ' ' << to_binary(p) << " to " << to << " is not the nearest value\n";
	};
	check(nearest(float(p), v), "float");
	check(nearest(double(p), v), "double");
	check(nearest(static_cast<long double>(p), v), "long double");
	return fails;
}

template<unsigned nbits, unsigned es>
int VerifyExhaustive(bool report) {
	int fails = 0;
	for (std::uint64_t i = 0; i < (1ull << nbits); ++i) {
		posit<nbits, es, std::uint16_t> p;
		p.setbits(i);
		VerifyOne(p, fails, report);
	}
	return fails;
}

template<unsigned nbits, unsigned es>
int VerifySampled(unsigned samples, bool report) {
	std::mt19937_64 rng(0x1678u + nbits * 8u + es);
	int fails = 0;
	for (unsigned i = 0; i < samples; ++i) {
		posit<nbits, es, std::uint32_t> p;
		p.setbits(rng());
		for (unsigned b = 64; b < nbits; ++b) p.setbit(b, (rng() & 1u) != 0u);
		VerifyOne(p, fails, report);
	}
	return fails;
}

// the two posit<128,2> cases reported in #1678: long double truncated, double rounded twice
int VerifyReported(bool report) {
	using P = posit<128, 2, std::uint32_t>;
	auto from_text = [](const char* s) {
		P p;
		p.setbits(0);
		std::string bits;
		for (const char* c = s + 2; *c; ++c) if (*c == '0' || *c == '1') bits += *c;
		for (unsigned i = 0; i < bits.size(); ++i) p.setbit(static_cast<unsigned>(bits.size() - 1 - i), bits[i] == '1');
		return p;
	};
	const P a = from_text("0b0.00001.00.110101111110001100111100100100011110100111101110001001011110010100100010110010001000100011000011001111100000010000010000");
	const P b = from_text("0b0.0001.10.1001101011001010000101110111011011011111110011011111011111111111100011100110010001010100101110000010111011101000101011100");
	int fails = 0;
	VerifyOne(a, fails, report);
	VerifyOne(b, fails, report);
	if (double(b) != 0x1.9aca1776dfcdfp-10) { ++fails; if (report) std::cerr << "FAIL the reported double case\n"; }
	return fails;
}

}  // anonymous namespace

#define MANUAL_TESTING 0
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
	std::string test_suite  = "posit to native: round to nearest";
	std::string test_tag    = "to_native rounding";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;
	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	nrOfFailedTestCases += ReportTestResult(VerifyReported(true), "posit<128,2>", "reported cases");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else
#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyReported(reportTestCases),              "posit<128,2>", "reported cases");
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<8, 0>(reportTestCases),      "posit<8,0>",   test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<8, 2>(reportTestCases),      "posit<8,2>",   test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<12, 3>(reportTestCases),     "posit<12,3>",  test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySampled<32, 2>(2000, reportTestCases),  "posit<32,2>",  test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySampled<32, 5>(2000, reportTestCases),  "posit<32,5>",  test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySampled<64, 2>(2000, reportTestCases),  "posit<64,2>",  test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySampled<64, 5>(2000, reportTestCases),  "posit<64,5>",  test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySampled<80, 3>(2000, reportTestCases),  "posit<80,3>",  test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySampled<128, 2>(2000, reportTestCases), "posit<128,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySampled<128, 4>(2000, reportTestCases), "posit<128,4>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySampled<256, 2>(500, reportTestCases),  "posit<256,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySampled<256, 5>(500, reportTestCases),  "posit<256,5>", test_tag);
#endif
#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<16, 1>(reportTestCases),     "posit<16,1>",  test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<16, 2>(reportTestCases),     "posit<16,2>",  test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySampled<128, 2>(20000, reportTestCases), "posit<128,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySampled<256, 2>(5000, reportTestCases), "posit<256,2>", test_tag);
#endif
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif
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
