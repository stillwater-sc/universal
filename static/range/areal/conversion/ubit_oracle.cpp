// ubit_oracle.cpp: the uncertainty bit of a conversion into areal, against an exact oracle
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// A conversion into areal truncates to the target and sets the ubit exactly when bits were dropped.
// The QA sweep in #1962 found three places where it did not (#1970):
//   - float and double into a subnormal target of an areal with more fraction bits than the source
//     (fbits >= 23 for float, >= 52 for double) shifted the source right and dropped bits with the
//     ubit clear: areal<30,5>(-5.4181703e-07f) came out truncated and marked exact;
//   - integers above 2^53 went through double when fbits < 53, which rounded them first:
//     areal<32,8>(2^60 - 1) came out as an exact 2^60, and INT64_MAX as an exact 2^63;
//   - the converting constructor areal<a,b>(areal<c,d>) had an empty body.
//
// The oracle needs no wider type. Clear the ubit of the result to get t: t is either the source
// itself or a truncation of its bits, so double(t) is exact. The ubit must then be set exactly when
// double(t) differs from the source, and |t| can never exceed |source|. That holds for saturation
// at maxpos and for underflow to (0, minpos) as well.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <random>
#include <string>

#include <universal/number/areal/areal.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

using sw::universal::areal;

// does the areal a, converted from the native value v, carry the right ubit?
template<typename Areal, typename Real>
bool UbitMatches(const Areal& a, Real v, double& got) {
	Areal t(a);
	t.reset(0);
	got = double(t);
	const double dv = static_cast<double>(v);
	const bool exact = (got == dv);
	const bool below = std::fabs(got) <= std::fabs(dv) && (got == 0.0 || std::signbit(got) == std::signbit(dv));
	return (a.at(0) == !exact) && below;
}

// native values across the target's subnormal range, its lowest normal binades, and below and
// above them, each with a random full-precision significand and either sign
template<typename Areal, typename Real>
int VerifyNativeUbit(int samples, bool reportTestCases) {
	using RL = std::numeric_limits<Real>;
	constexpr int fbits = static_cast<int>(Areal::fbits);
	constexpr int bias = (1 << (Areal::es - 1)) - 1;
	constexpr int minNormal = 1 - bias, minSubnormal = 1 - bias - fbits, maxExp = bias + 1;
	int fails = 0, reported = 0;
	std::mt19937_64 rng(1970 + Areal::nbits * 16 + Areal::es);
	std::uniform_int_distribution<int> lowExp(minSubnormal - 3, minNormal + 3), anyExp(minSubnormal - 3, maxExp + 2);
	std::uniform_int_distribution<std::uint64_t> bits;
	for (int i = 0; i < samples; ++i) {
		const int e = (i % 4 == 3) ? anyExp(rng) : lowExp(rng);   // mostly near the subnormal range
		if (e < RL::min_exponent - RL::digits || e >= RL::max_exponent) continue;
		Real m = Real(1) + std::ldexp(static_cast<Real>(bits(rng) >> (64 - (RL::digits - 1))), -(RL::digits - 1));   // [1, 2)
		Real v = std::ldexp(m, e);
		if (rng() & 1u) v = -v;
		if (v == Real(0) || !std::isfinite(v)) continue;
		const Areal a(v);
		if (a.isnan() || a.isinf()) continue;
		double got = 0.0;
		if (!UbitMatches(a, v, got)) {
			++fails;
			if (reportTestCases && reported++ < 5) std::cerr << "FAIL " << sw::universal::type_tag(a) << '(' << v << ") = " << got << " ubit " << a.at(0) << '\n';
		}
	}
	return fails;
}

// integers through convert_signed_integer and convert_unsigned_integer, for areals whose
// significand is narrower than double's: values near the powers of two above 2^53, and random
// 64-bit values, all of which double rounded before the fix
template<typename Areal>
int VerifyIntegerUbit(bool reportTestCases) {
	static_assert(Areal::fbits < 53, "the oracle needs double(t) to be exact");
	int fails = 0, reported = 0;
	auto checkSigned = [&](long long v) {
		const Areal a(v);
		Areal t(a);
		t.reset(0);
		const double got = double(t);
		// t <= |v| < 2^63 and t has at most 53 bits, so t is an exact long long
		const long long tv = (got >= 9.2233720368547758e18) ? std::numeric_limits<long long>::max() : static_cast<long long>(got);
		const bool exact = (got < 9.2233720368547758e18) && (tv == v);
		const bool ok = a.at(0) == !exact;
		if (!ok) { ++fails; if (reportTestCases && reported++ < 5) std::cerr << "FAIL " << sw::universal::type_tag(a) << '(' << v << ") = " << got << " ubit " << a.at(0) << '\n'; }
	};
	auto checkUnsigned = [&](unsigned long long v) {
		const Areal a(v);
		Areal t(a);
		t.reset(0);
		const double got = double(t);
		const bool exact = (got < 1.8446744073709552e19) && (static_cast<unsigned long long>(got) == v);
		const bool ok = a.at(0) == !exact;
		if (!ok) { ++fails; if (reportTestCases && reported++ < 5) std::cerr << "FAIL " << sw::universal::type_tag(a) << '(' << v << "u) = " << got << " ubit " << a.at(0) << '\n'; }
	};
	for (int k = 54; k < 63; ++k) {
		const long long p = 1ll << k;
		for (long long d : { -1ll, 0ll, 1ll, 3ll }) { checkSigned(p + d); checkSigned(-(p + d)); }
	}
	checkSigned(std::numeric_limits<long long>::max());
	checkSigned(std::numeric_limits<long long>::min());
	checkUnsigned(std::numeric_limits<unsigned long long>::max());
	checkUnsigned((1ull << 63) + 1ull);
	for (long long v : { 0ll, 1ll, -1ll, 1000ll, (1ll << 53) - 1, 1ll << 53 }) checkSigned(v);   // the double path, unchanged
	std::mt19937_64 rng(19700);
	for (int i = 0; i < 20000; ++i) {
		const std::uint64_t r = rng() >> (rng() % 40);   // magnitudes from 2^24 up to 2^64
		checkUnsigned(r);
		checkSigned(static_cast<long long>(r >> 1) * ((rng() & 1u) ? -1 : 1));
	}
	return fails;
}

// the converting constructor: every encoding of a source configuration, into a target. The value
// converts with the target's rounding, and an inexact source stays inexact.
template<typename Target, typename Source>
int VerifyConvertingConstructor(bool reportTestCases) {
	int fails = 0, reported = 0;
	Source src;
	for (std::uint64_t raw = 0; raw < (std::uint64_t(1) << Source::nbits); ++raw) {
		src.setbits(raw);
		if (src.isnan()) {
			// a NaN keeps its kind: quiet stays quiet, signalling stays signalling
			const Target t(src);
			if (!t.isnan() || t.isnan(sw::universal::NAN_TYPE_QUIET) != src.isnan(sw::universal::NAN_TYPE_QUIET)
			               || t.isnan(sw::universal::NAN_TYPE_SIGNALLING) != src.isnan(sw::universal::NAN_TYPE_SIGNALLING)) {
				++fails;
				if (reportTestCases && reported++ < 5) std::cerr << "FAIL NaN kind " << sw::universal::to_binary(src) << " -> " << sw::universal::to_binary(t) << '\n';
			}
			continue;
		}
		const Target t(src);
		Target want{ double(src) };   // the source's lower end, converted
		if (src.at(0) && !want.isnan() && !want.isinf()) want.set(0);
		if (!(t == want) || t.at(0) != want.at(0)) {
			++fails;
			if (reportTestCases && reported++ < 5) std::cerr << "FAIL " << sw::universal::type_tag(t) << '(' << sw::universal::to_binary(src) << ") = " << sw::universal::to_binary(t) << ", expected " << sw::universal::to_binary(want) << '\n';
		}
		// soundness: an exact result needs an exact source whose value it holds
		if (!t.at(0) && !t.isinf() && (src.at(0) || double(t) != double(src))) ++fails;
	}
	return fails;
}

// A source past every native type's range: areal has one binade more than IEEE, so the top
// binade of areal<64,11>, maxpos included, overflows double, and long double is double on some
// platforms. The converting constructor goes through a blocktriple, so it needs no native type
// (CodeRabbit). Widening to areal<80,15> and narrowing back must give the same encoding, exactly,
// for every sampled encoding, the top binade included; narrowing maxpos to areal<32,8> saturates.
int VerifyWideSourceRoundTrip(bool reportTestCases) {
	using Src = areal<64, 11, std::uint8_t>;
	using Wide = areal<80, 15, std::uint8_t>;
	using Narrow = areal<32, 8, std::uint8_t>;
	int fails = 0, reported = 0;
	std::mt19937_64 rng(19701);
	auto roundTrip = [&](const Src& x) {
		if (x.isnan()) return;
		const Wide w(x);
		const Src back(w);
		bool ok = !w.isinf() || x.isinf();
		ok = ok && (w.at(0) == x.at(0)) && (back.at(0) == x.at(0));
		for (unsigned i = 0; i < Src::nbits && ok; ++i) ok = (back.at(i) == x.at(i));
		if (!ok) {
			++fails;
			if (reportTestCases && reported++ < 5) std::cerr << "FAIL round trip " << sw::universal::to_binary(x) << " -> " << sw::universal::to_binary(w) << " -> " << sw::universal::to_binary(back) << '\n';
		}
	};
	Src x;
	for (int i = 0; i < 20000; ++i) {
		std::uint64_t raw = rng();
		if (i % 4 == 0) raw |= 0x7FE0'0000'0000'0000ull;   // the top binades, near maxpos
		x.setbits(raw);
		roundTrip(x);
	}
	roundTrip(Src(sw::universal::SpecificValue::maxpos));
	roundTrip(Src(sw::universal::SpecificValue::maxneg));
	roundTrip(Src(sw::universal::SpecificValue::minpos));
	roundTrip(Src(sw::universal::SpecificValue::minneg));
	const Narrow n{ Src(sw::universal::SpecificValue::maxpos) };
	if (n.isinf() || !(n.at(0)) || double(n) != double(Narrow(sw::universal::SpecificValue::maxpos))) {
		++fails;
		if (reportTestCases) std::cerr << "FAIL areal<64,11> maxpos into areal<32,8> = " << sw::universal::to_binary(n) << ", expected (maxpos, inf)\n";
	}
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

	std::string test_suite  = "areal conversion ubit oracle";
	std::string test_tag    = "ubit";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	areal<30, 5> a(-5.4181703e-07f);
	std::cout << to_binary(a) << " ubit " << a.at(0) << '\n';

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	constexpr int n = 20000;
	// float into the configurations the sweep listed, wider than float's significand
	nrOfFailedTestCases += ReportTestResult(VerifyNativeUbit<areal<30, 5>, float>(n, reportTestCases), "areal<30,5>", "float ubit");
	nrOfFailedTestCases += ReportTestResult(VerifyNativeUbit<areal<31, 4>, float>(n, reportTestCases), "areal<31,4>", "float ubit");
	nrOfFailedTestCases += ReportTestResult(VerifyNativeUbit<areal<32, 3>, float>(n, reportTestCases), "areal<32,3>", "float ubit");
	nrOfFailedTestCases += ReportTestResult(VerifyNativeUbit<areal<32, 4>, float>(n, reportTestCases), "areal<32,4>", "float ubit");
	nrOfFailedTestCases += ReportTestResult(VerifyNativeUbit<areal<32, 5>, float>(n, reportTestCases), "areal<32,5>", "float ubit");
	nrOfFailedTestCases += ReportTestResult(VerifyNativeUbit<areal<29, 3>, float>(n, reportTestCases), "areal<29,3>", "float ubit");
	// and narrower ones, which were already right
	nrOfFailedTestCases += ReportTestResult(VerifyNativeUbit<areal<16, 5>, float>(n, reportTestCases), "areal<16,5>", "float ubit");
	nrOfFailedTestCases += ReportTestResult(VerifyNativeUbit<areal<32, 8>, float>(n, reportTestCases), "areal<32,8>", "float ubit");
	// double into the configurations the sweep listed, wider than double's significand
	nrOfFailedTestCases += ReportTestResult(VerifyNativeUbit<areal<60, 5>, double>(n, reportTestCases), "areal<60,5>", "double ubit");
	nrOfFailedTestCases += ReportTestResult(VerifyNativeUbit<areal<62, 4>, double>(n, reportTestCases), "areal<62,4>", "double ubit");
	nrOfFailedTestCases += ReportTestResult(VerifyNativeUbit<areal<64, 3>, double>(n, reportTestCases), "areal<64,3>", "double ubit");
	nrOfFailedTestCases += ReportTestResult(VerifyNativeUbit<areal<64, 6>, double>(n, reportTestCases), "areal<64,6>", "double ubit");
	nrOfFailedTestCases += ReportTestResult(VerifyNativeUbit<areal<64, 8>, double>(n, reportTestCases), "areal<64,8>", "double ubit");
	nrOfFailedTestCases += ReportTestResult(VerifyNativeUbit<areal<57, 3>, double>(n, reportTestCases), "areal<57,3>", "double ubit");
	nrOfFailedTestCases += ReportTestResult(VerifyNativeUbit<areal<64, 11>, double>(n, reportTestCases), "areal<64,11>", "double ubit");
	nrOfFailedTestCases += ReportTestResult(VerifyNativeUbit<areal<32, 8>, double>(n, reportTestCases), "areal<32,8>", "double ubit");
	// integers above 2^53
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerUbit<areal<32, 8>>(reportTestCases), "areal<32,8>", "integer ubit");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerUbit<areal<16, 5>>(reportTestCases), "areal<16,5>", "integer ubit");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerUbit<areal<64, 11>>(reportTestCases), "areal<64,11>", "integer ubit");
	// the converting constructor, narrowing and widening
	nrOfFailedTestCases += ReportTestResult(VerifyConvertingConstructor<areal<16, 5, std::uint8_t>, areal<8, 3, std::uint8_t>>(reportTestCases), "areal<16,5> from areal<8,3>", "converting ctor");
	nrOfFailedTestCases += ReportTestResult(VerifyConvertingConstructor<areal<8, 3, std::uint8_t>, areal<12, 4, std::uint8_t>>(reportTestCases), "areal<8,3> from areal<12,4>", "converting ctor");
	nrOfFailedTestCases += ReportTestResult(VerifyConvertingConstructor<areal<10, 2, std::uint8_t>, areal<12, 5, std::uint8_t>>(reportTestCases), "areal<10,2> from areal<12,5>", "converting ctor");
	nrOfFailedTestCases += ReportTestResult(VerifyConvertingConstructor<areal<32, 8, std::uint8_t>, areal<16, 5, std::uint8_t>>(reportTestCases), "areal<32,8> from areal<16,5>", "converting ctor");
	nrOfFailedTestCases += ReportTestResult(VerifyWideSourceRoundTrip(reportTestCases), "areal<64,11> via areal<80,15>", "converting ctor round trip");
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
	std::cerr << "Caught unexpected runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
