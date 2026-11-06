// native.cpp: test suite runner for the native dbns multiply and divide (NATIVE_DBNS_ARITHMETIC)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// By default dbns multiplies and divides through double. NATIVE_DBNS_ARITHMETIC selects a native
// path that works on the exponents directly, which no build compiled and no test ran until #1940:
// the multiply projected an out-of-range exponent pair back with approximations of unity, off by up
// to 11%, and the divide subtracted the exponents in uint32_t, so 3 / 1.5 came out 0.
//
// The value of a dbns is (-1)^s * 0.5^a * 3^b, so a product is 0.5^(a1+a2) * 3^(b1+b2) and a
// quotient 0.5^(a1-a2) * 3^(b1-b2), exactly. The native result must be that pair when it is an
// encoding, and otherwise the encoding nearest it in log2: the rule conversion from double follows.
// The reference here is computed independently, by a binary search over every encoding sorted by
// log2.
#define NATIVE_DBNS_ARITHMETIC 1
#include <universal/utility/directives.hpp>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include <universal/number/dbns/dbns.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

template<typename Dbns>
struct Encoding {
	Dbns        value;
	long double log2v;   // log2 of the magnitude, from the exponents
};

// every positive encoding, sorted by log2
template<typename Dbns>
std::vector<Encoding<Dbns>> SortedEncodings() {
	std::vector<Encoding<Dbns>> enc;
	const long double l3 = 1.5849625007211561814537389439478L;
	Dbns d;
	for (std::uint64_t raw = 0; raw < (std::uint64_t(1) << (Dbns::nbits - 1)); ++raw) {
		d.setbits(raw);
		if (d.isnan() || d.iszero()) continue;
		enc.push_back({ d, static_cast<long double>(d.extractExponent(1)) * l3 - static_cast<long double>(d.extractExponent(0)) });
	}
	std::sort(enc.begin(), enc.end(), [](const Encoding<Dbns>& x, const Encoding<Dbns>& y) { return x.log2v < y.log2v; });
	return enc;
}

// is r the encoding nearest the exact value 0.5^a * 3^b with the given sign? Ties, possible only when
// the exact value is a lattice point between two encodings, accept either.
template<typename Dbns>
bool IsNearest(const std::vector<Encoding<Dbns>>& enc, long long a, long long b, bool negative, const Dbns& r) {
	const long double l3 = 1.5849625007211561814537389439478L;
	const long double L = static_cast<long double>(b) * l3 - static_cast<long double>(a);
	const long double minposLog = enc.front().log2v;
	if (L < minposLog - 1.0L) return r.iszero();                      // below half of minpos: zero
	if (r.iszero() || r.isnan() || r.sign() != negative) return false;
	auto it = std::lower_bound(enc.begin(), enc.end(), L, [](const Encoding<Dbns>& e, long double v) { return e.log2v < v; });
	long double best = 1.0e300L;
	for (auto c = (it == enc.begin() ? it : it - 1); c != enc.end() && c <= it; ++c) best = std::fmin(best, std::fabs(c->log2v - L));
	const long double got = std::fabs(static_cast<long double>(r.extractExponent(1)) * l3 - static_cast<long double>(r.extractExponent(0)) - L);
	return got <= best + 1.0e-12L;
}

// every pair of encodings, both signs of the result, through * and /
template<unsigned nbits, unsigned fbbits, typename bt>
int VerifyNativeMulDiv(bool reportTestCases) {
	using namespace sw::universal;
	using Dbns = dbns<nbits, fbbits, bt>;
	const auto enc = SortedEncodings<Dbns>();
	const long long maxA = static_cast<long long>(Dbns::MAX_A), maxB = static_cast<long long>(Dbns::MAX_B);
	int nrOfFailedTests = 0, reported = 0;
	auto check = [&](const Dbns& x, const Dbns& y, const Dbns& r, long long a, long long b, bool negative, char op) {
		const bool exactPair = (a >= 0 && a <= maxA && b >= 0 && b <= maxB && !(a == maxA && b == 0));
		bool ok;
		if (exactPair) {
			Dbns want;
			want.setbits(0);
			want.setexponent(0, static_cast<std::uint32_t>(a));
			want.setexponent(1, static_cast<std::uint32_t>(b));
			want.setsign(negative);
			ok = (r == want);
		}
		else {
			ok = IsNearest(enc, a, b, negative, r);
		}
		if (!ok) {
			++nrOfFailedTests;
			if (reportTestCases && reported++ < 10) {
				std::cerr << "FAIL " << type_tag(Dbns()) << ' ' << double(x) << ' ' << op << ' ' << double(y) << " = " << double(r)
				          << " " << to_binary(r) << ", exact 0.5^" << a << " * 3^" << b << (exactPair ? " (an encoding)" : "") << '\n';
			}
		}
	};
	for (const auto& ex : enc) {
		for (const auto& ey : enc) {
			for (int signs = 0; signs < 4; ++signs) {
				const Dbns x = (signs & 1) ? -ex.value : ex.value;
				const Dbns y = (signs & 2) ? -ey.value : ey.value;
				const bool negative = ((signs & 1) != 0) != ((signs & 2) != 0);
				const long long ax = x.extractExponent(0), bx = x.extractExponent(1);
				const long long ay = y.extractExponent(0), by = y.extractExponent(1);
				check(x, y, x * y, ax + ay, bx + by, negative, '*');
				check(x, y, x / y, ax - ay, bx - by, negative, '/');
				Dbns z = x;
				z *= y;
				check(x, y, z, ax + ay, bx + by, negative, '*');
			}
		}
	}
	// special operands
	const Dbns zero(0.0), one(1.0), nan(sw::universal::SpecificValue::qnan);
	if (!(zero * one).iszero() || !(one * zero).iszero() || !(zero / one).iszero()) ++nrOfFailedTests;
	if (!(one / zero).isnan() || !(nan * one).isnan() || !(one * nan).isnan() || !(nan / one).isnan()) ++nrOfFailedTests;
	return nrOfFailedTests;
}

// The configurations of conversion/nearest.cpp, with fields up to 32 bits wide, where an exhaustive
// pass is out of reach: random operand pairs, checked against an oracle that scans the smaller field
// in full for the pair nearest the exact result. A product's exponents can reach 2^33, so distances
// are taken from exponent differences, |(b - b')*log2(3) - (a - a')|, which stay small and exact
// enough for a tight bound that still accepts a tie (CodeRabbit on PR #1947).
template<unsigned nbits, unsigned fbbits, typename bt>
int VerifyNativeSampled(int samples, bool reportTestCases) {
	using namespace sw::universal;
	using Dbns = dbns<nbits, fbbits, bt>;
	const long double l3 = 1.5849625007211561814537389439478L;
	const long long maxA = static_cast<long long>(Dbns::MAX_A), maxB = static_cast<long long>(Dbns::MAX_B);
	auto pairLog = [&](long long a, long long b) { return static_cast<long double>(b) * l3 - static_cast<long double>(a); };
	// the log2 distance between the exact pair (a, b) and a candidate (ca, cb), from the differences
	auto distance = [&](long long a, long long b, long long ca, long long cb) {
		return std::fabs(static_cast<long double>(b - cb) * l3 - static_cast<long double>(a - ca));
	};
	// the least log2 distance from the exact pair (a, b) to an encodable pair, the zero pattern excluded
	auto oracle = [&](long long ea, long long eb) {
		const long double L = pairLog(ea, eb);
		long double best = 1.0e300L;
		auto consider = [&](long long a, long long b) {
			if (a < 0 || a > maxA || b < 0 || b > maxB || (a == maxA && b == 0)) return;
			best = std::fmin(best, distance(ea, eb, a, b));
		};
		if (maxA <= maxB) {
			for (long long a = 0; a <= maxA; ++a) {
				long double bf = (L + static_cast<long double>(a)) / l3;
				long long b0 = static_cast<long long>(std::floor(bf));
				for (long long b = b0 - 1; b <= b0 + 1; ++b) consider(a, std::clamp(b, 0ll, maxB));
			}
		}
		else {
			for (long long b = 0; b <= maxB; ++b) {
				long double af = static_cast<long double>(b) * l3 - L;
				long long a0 = static_cast<long long>(std::floor(af));
				for (long long a = a0 - 1; a <= a0 + 1; ++a) consider(std::clamp(a, 0ll, maxA), b);
			}
		}
		return best;
	};
	std::mt19937_64 rng(1940 + nbits * 64 + fbbits);
	std::uniform_int_distribution<long long> ua(0, maxA), ub(0, maxB);
	auto randomOperand = [&]() {
		Dbns d;
		do {
			d.setbits(0);
			d.setexponent(0, static_cast<std::uint32_t>(ua(rng)));
			d.setexponent(1, static_cast<std::uint32_t>(ub(rng)));
			d.setsign((rng() & 1u) != 0);
		} while (d.iszero() || d.isnan());
		return d;
	};
	int nrOfFailedTests = 0, reported = 0;
	const long double minposLog = -static_cast<long double>(maxA - 1);
	for (int i = 0; i < samples; ++i) {
		const Dbns x = randomOperand(), y = randomOperand();
		const long long ax = x.extractExponent(0), bx = x.extractExponent(1);
		const long long ay = y.extractExponent(0), by = y.extractExponent(1);
		const bool negative = x.sign() != y.sign();
		for (int op = 0; op < 2; ++op) {
			const long long a = op == 0 ? ax + ay : ax - ay;
			const long long b = op == 0 ? bx + by : bx - by;
			const Dbns r = op == 0 ? x * y : x / y;
			const long double L = pairLog(a, b);
			bool ok;
			if (a >= 0 && a <= maxA && b >= 0 && b <= maxB && !(a == maxA && b == 0)) {
				ok = !r.iszero() && !r.isnan() && r.sign() == negative
				  && static_cast<long long>(r.extractExponent(0)) == a && static_cast<long long>(r.extractExponent(1)) == b;
			}
			else if (L < minposLog - 1.0L) {
				ok = r.iszero();
			}
			else {
				ok = !r.iszero() && !r.isnan() && r.sign() == negative
				  && distance(a, b, r.extractExponent(0), r.extractExponent(1)) <= oracle(a, b) + 1.0e-12L;
			}
			if (!ok) {
				++nrOfFailedTests;
				if (reportTestCases && reported++ < 5) std::cerr << "FAIL " << type_tag(Dbns()) << (op == 0 ? " * " : " / ") << ": exact 0.5^" << a << " * 3^" << b << " -> " << to_binary(r) << '\n';
			}
		}
	}
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

	std::string test_suite  = "dbns native multiply and divide";
	std::string test_tag    = "native * /";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	dbns<8, 3> a(3.0), b(1.5);
	std::cout << double(a / b) << " " << to_binary(a / b) << '\n';

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyNativeMulDiv< 5, 2, std::uint8_t >(reportTestCases), "dbns<5,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNativeMulDiv< 6, 2, std::uint8_t >(reportTestCases), "dbns<6,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNativeMulDiv< 8, 3, std::uint8_t >(reportTestCases), "dbns<8,3>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNativeMulDiv< 8, 5, std::uint8_t >(reportTestCases), "dbns<8,5>", test_tag);

	// the configurations of conversion/nearest.cpp, sampled
	nrOfFailedTestCases += ReportTestResult(VerifyNativeSampled<16,  5, std::uint8_t >(2000, reportTestCases), "dbns<16,5>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNativeSampled<16,  8, std::uint8_t >(2000, reportTestCases), "dbns<16,8>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNativeSampled<34,  1, std::uint8_t >(2000, reportTestCases), "dbns<34,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNativeSampled<32, 26, std::uint32_t>(2000, reportTestCases), "dbns<32,26,uint32>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNativeSampled<24, 19, std::uint8_t >(2000, reportTestCases), "dbns<24,19>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNativeSampled<20, 10, std::uint16_t>( 500, reportTestCases), "dbns<20,10,uint16>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNativeSampled<34,  1, std::uint64_t>(2000, reportTestCases), "dbns<34,1,uint64>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNativeSampled<40, 30, std::uint64_t>(2000, reportTestCases), "dbns<40,30,uint64>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNativeSampled<48, 20, std::uint64_t>(  20, reportTestCases), "dbns<48,20,uint64>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNativeSampled<10,  6, std::uint8_t >(2000, reportTestCases), "dbns<10,6>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNativeSampled<12,  3, std::uint8_t >(2000, reportTestCases), "dbns<12,3>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNativeSampled<12,  3, std::uint16_t>(2000, reportTestCases), "dbns<12,3,uint16>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNativeSampled<12,  5, std::uint16_t>(2000, reportTestCases), "dbns<12,5,uint16>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNativeSampled<16,  8, std::uint16_t>(2000, reportTestCases), "dbns<16,8,uint16>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNativeSampled<31, 15, std::uint32_t>( 200, reportTestCases), "dbns<31,15,uint32>", test_tag);
#endif

#if REGRESSION_LEVEL_2
	// multi-block: dbns wider than its uint8_t block type
	nrOfFailedTestCases += ReportTestResult(VerifyNativeMulDiv<10, 4, std::uint8_t >(reportTestCases), "dbns<10,4>", test_tag);
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyNativeMulDiv<10, 6, std::uint16_t>(reportTestCases), "dbns<10,6,uint16>", test_tag);
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyNativeMulDiv<12, 5, std::uint8_t >(reportTestCases), "dbns<12,5>", test_tag);
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
