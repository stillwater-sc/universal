// integer.cpp: exact conversion between takum and the native integer types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Integers convert into a takum exactly and round once (#2079).  They used to go through a double,
// which rounded every integer above 2^53 first: takum<64,3>((1LL<<53)+1) came out as 2^53.
// Integer casts truncate toward zero and saturate to the integer type, and NaR gives the type's
// minimum -- posit's rule (#2073), adopted for takum in #2079.  They cast a double, which made NaR
// and every out-of-range value undefined behaviour.
//
// The reference is exact integer arithmetic on integer<384>: a takum value S * 2^e is held as
// S * 2^(e + 64), which is an integer for every encoding these layouts have (e >= -61).
#include <universal/utility/directives.hpp>
#define TAKUM_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/takum/takum.hpp>
#include <universal/number/integer/integer.hpp>
#include <universal/verification/test_suite.hpp>

#include <climits>
#include <cstdint>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <vector>

namespace {

// 384 bits: the largest rbits = 3 value, near 2^255, held as v * 2^64
using Exact = sw::universal::integer<384, std::uint32_t, sw::universal::IntegerNumberType::IntegerNumber>;
constexpr int kScale = 64;   // a value v is held as v * 2^64

Exact from_uint64(std::uint64_t v) {
	Exact r(static_cast<std::uint32_t>(v >> 32));
	r <<= 32;
	r += Exact(static_cast<std::uint32_t>(v & 0xFFFFFFFFu));
	return r;
}

// |x| * 2^64 for a finite non-zero takum, from its exact decode
template<typename Takum>
Exact scaled_magnitude(const Takum& x) {
	using namespace sw::universal;
	const auto op = takum_wide::decode_operand<typename Takum::Codec>(false, x.magnitude_bits());
	Exact r = from_uint64(op.S);
	const long long shift = op.e + kScale;
	if (shift >= 0) r <<= static_cast<int>(shift);
	else r >>= static_cast<int>(-shift);   // not reached: e >= -61 in every layout tested
	return r;
}

// Integer -> takum is the round-to-nearest of |v| onto the takum's grid, ties to the even
// encoding.  Checked against both neighbours of the result, positive side; the sign follows by
// two's complement negation, which is checked separately.
template<typename Takum>
int CheckIntegerRounding(std::uint64_t v, bool reportTestCases) {
	using namespace sw::universal;
	if (v == 0) return Takum(0ull).iszero() ? 0 : 1;
	const Takum t(static_cast<unsigned long long>(v));
	const Exact target = from_uint64(v) << kScale;
	const Exact here = scaled_magnitude(t);
	const Exact dHere = (here > target) ? (here - target) : (target - here);
	int fails = 0;
	auto checkNeighbour = [&](std::uint64_t bits) {
		Takum n;
		n.setbits(bits);
		if (n.iszero() || n.isnar() || n.sign()) return;   // past the positive range
		const Exact there = scaled_magnitude(n);
		const Exact dThere = (there > target) ? (there - target) : (target - there);
		if (dThere < dHere || (dThere == dHere && (t.raw_bits() & 1u))) ++fails;
	};
	checkNeighbour(t.raw_bits() - 1u);
	checkNeighbour(t.raw_bits() + 1u);
	if (fails && reportTestCases) {
		std::cerr << "FAIL: " << type_tag(t) << "(" << v << ") = 0x" << std::hex << t.raw_bits() << std::dec
		          << " is not the nearest encoding\n";
	}
	// the signed path, through the negation the format defines
	if (v <= static_cast<std::uint64_t>(LLONG_MAX)) {
		const Takum s(-static_cast<long long>(v));
		if (s != -t) {
			if (reportTestCases) std::cerr << "FAIL: " << type_tag(t) << "(-" << v << ") is not the negation\n";
			++fails;
		}
	}
	return fails;
}

std::vector<std::uint64_t> IntegerSamples(std::size_t nrRandom) {
	std::vector<std::uint64_t> v;
	for (std::uint64_t i = 0; i <= 1024; ++i) v.push_back(i);
	for (unsigned k = 1; k < 64; ++k) {
		const std::uint64_t p = std::uint64_t(1) << k;
		for (std::uint64_t d = 0; d <= 3; ++d) { v.push_back(p + d); v.push_back(p - d); }
	}
	// the sweep's cases and the ends of the range
	v.push_back((std::uint64_t(1) << 53) + 1u);
	v.push_back((std::uint64_t(1) << 54) + 2u);
	v.push_back(static_cast<std::uint64_t>(LLONG_MAX));
	v.push_back(static_cast<std::uint64_t>(LLONG_MAX) + 1u);
	v.push_back(std::numeric_limits<std::uint64_t>::max());
	std::mt19937_64 rng(2079);
	for (std::size_t i = 0; i < nrRandom; ++i) {
		const unsigned width = 1u + static_cast<unsigned>(rng() % 64u);
		v.push_back(width == 64 ? rng() : (rng() & ((std::uint64_t(1) << width) - 1u)));
	}
	return v;
}

template<typename Takum>
int VerifyIntegerConversion(const std::vector<std::uint64_t>& samples, bool reportTestCases) {
	int nrOfFailedTests = 0;
	for (std::uint64_t v : samples) nrOfFailedTests += CheckIntegerRounding<Takum>(v, reportTestCases);
	return nrOfFailedTests;
}

// The integer a cast must produce, from the exact value: truncate toward zero, saturate, NaR -> min
template<typename Integer, typename Takum>
Integer ReferenceCast(const Takum& x) {
	using Limits = std::numeric_limits<Integer>;
	if (x.iszero()) return Integer(0);
	if (x.isnar()) return Limits::min();
	const Exact truncated = scaled_magnitude(x) >> kScale;
	const bool negative = x.sign();
	if (truncated == Exact(0)) return Integer(0);
	if (negative && !Limits::is_signed) return Limits::min();
	// 2^digits is the first magnitude past the range, on either side (min = -2^digits)
	const Exact limit = Exact(1) << Limits::digits;
	if (truncated >= limit) return negative ? Limits::min() : Limits::max();
	std::uint64_t m = 0;
	Exact t = truncated;
	for (int b = 0; b < 64; ++b) {
		if (t.test(0)) m |= std::uint64_t(1) << b;
		t >>= 1;
	}
	using U = std::make_unsigned_t<Integer>;
	return negative ? static_cast<Integer>(U(0) - static_cast<U>(m)) : static_cast<Integer>(m);
}

template<typename Takum>
int CheckCasts(const Takum& x, bool reportTestCases) {
	using namespace sw::universal;
	int fails = 0;
	auto report = [&](const char* type, long double got, long double expected) {
		if (reportTestCases) std::cerr << "FAIL: " << type << '(' << type_tag(x) << " 0x" << std::hex << x.raw_bits() << std::dec
		                               << ") = " << got << ", expected " << expected << '\n';
		++fails;
	};
	if (int(x) != ReferenceCast<int>(x))                       report("int", (long double)int(x), (long double)ReferenceCast<int>(x));
	if (long(x) != ReferenceCast<long>(x))                     report("long", (long double)long(x), (long double)ReferenceCast<long>(x));
	if ((long long)(x) != ReferenceCast<long long>(x))         report("long long", (long double)(long long)(x), (long double)ReferenceCast<long long>(x));
	return fails;
}

template<typename Takum>
int VerifyIntegerCasts(std::size_t nrRandom, bool reportTestCases) {
	using namespace sw::universal;
	int nrOfFailedTests = 0;
	// the special values and the range ends
	for (SpecificValue code : { SpecificValue::nar, SpecificValue::maxpos, SpecificValue::maxneg,
	                            SpecificValue::minpos, SpecificValue::minneg, SpecificValue::zero }) {
		nrOfFailedTests += CheckCasts(Takum(code), reportTestCases);
	}
	// the values around the integers and the 2^31 / 2^32 / 2^63 / 2^64 boundaries
	for (double d : { 0.5, 0.999, 1.0, 1.5, 2.0, 2147483647.0, 2147483648.0, 4294967295.0, 4294967296.0,
	                  9223372036854775807.0, 18446744073709551616.0, 1e30 }) {
		nrOfFailedTests += CheckCasts(Takum(d), reportTestCases);
		nrOfFailedTests += CheckCasts(Takum(-d), reportTestCases);
	}
	std::mt19937_64 rng(2079);
	constexpr unsigned nbits = Takum::nbits;
	for (std::size_t i = 0; i < nrRandom; ++i) {
		Takum x;
		x.setbits(nbits == 64 ? rng() : (rng() & ((std::uint64_t(1) << nbits) - 1u)));
		nrOfFailedTests += CheckCasts(x, reportTestCases);
		// a value near the integers, where truncation actually has bits to drop
		Takum y(static_cast<double>(static_cast<long long>(rng() >> (rng() % 64u))) * ((rng() & 1u) ? 1.0 : -1.0) + 0.75);
		nrOfFailedTests += CheckCasts(y, reportTestCases);
	}
	return nrOfFailedTests;
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
#define REGRESSION_LEVEL_2 0
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "takum integer conversion";
	std::string test_tag    = "integer";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	takum<64, 3> a((1LL << 53) + 1);
	std::cout << std::hex << a.raw_bits() << std::dec << '\n';
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors

#else

#if REGRESSION_LEVEL_1
	{
		// the sweep's repros
		int fails = 0;
		if (takum<64, 3>((1LL << 53) + 1).raw_bits() != 0x6d80000000000002ull) ++fails;
		if ((long long)takum<64, 3>((1LL << 54) + 2) != (1LL << 54) + 2) ++fails;
		if (int(takum<64, 3>(SpecificValue::nar)) != INT_MIN) ++fails;
		if ((long long)takum<64, 3>(SpecificValue::maxpos) != LLONG_MAX) ++fails;
		if ((long long)takum<64, 3>(SpecificValue::maxneg) != LLONG_MIN) ++fails;
		if ((long long)takum<64, 3>(LLONG_MIN) != LLONG_MIN) ++fails;
		if ((long long)takum<64, 3>(LLONG_MAX) != LLONG_MAX) ++fails;   // 2^63 - 1 rounds to 2^63, which saturates
		{
			char c = -5;     // operator=(char) must treat a signed char as signed
			takum<16, 3> t;
			t = c;
			if (double(t) != -5.0) ++fails;
		}
#if BIT_CAST_IS_CONSTEXPR
		static_assert(takum<64, 3>((1LL << 53) + 1).raw_bits() == 0x6d80000000000002ull, "constexpr exact integer conversion");
		static_assert(int(takum<32, 3>(-7)) == -7, "constexpr integer cast");
#endif
		nrOfFailedTestCases += ReportTestResult(fails, "takum<64,3>", "sweep repros");
	}
	{
		const auto samples = IntegerSamples(2000);
		nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<takum<64, 3>>(samples, reportTestCases), "takum<64,3>", "integer -> takum");
		nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<takum<61, 3>>(samples, reportTestCases), "takum<61,3>", "integer -> takum");
		nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<takum<57, 3>>(samples, reportTestCases), "takum<57,3>", "integer -> takum");
		nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<takum<48, 3>>(samples, reportTestCases), "takum<48,3>", "integer -> takum");
		nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<takum<32, 3>>(samples, reportTestCases), "takum<32,3>", "integer -> takum");
		nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<takum<16, 3>>(samples, reportTestCases), "takum<16,3>", "integer -> takum");
		nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<takum<12, 3>>(samples, reportTestCases), "takum<12,3>", "integer -> takum");
	}
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerCasts<takum<64, 3>>(2000, reportTestCases), "takum<64,3>", "integer casts");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerCasts<takum<32, 3>>(2000, reportTestCases), "takum<32,3>", "integer casts");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerCasts<takum<16, 3>>(2000, reportTestCases), "takum<16,3>", "integer casts");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerCasts<takum<8, 3>>(256, reportTestCases), "takum<8,3>", "integer casts");
#endif

#if REGRESSION_LEVEL_2
	{
		const auto samples = IntegerSamples(50000);
		nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<takum<64, 3>>(samples, reportTestCases), "takum<64,3>", "integer -> takum");
		nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<takum<63, 3, std::uint16_t>>(samples, reportTestCases), "takum<63,3,uint16_t>", "integer -> takum");
	}
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerCasts<takum<64, 3>>(50000, reportTestCases), "takum<64,3>", "integer casts");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerCasts<takum<48, 2>>(50000, reportTestCases), "takum<48,2>", "integer casts");
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
