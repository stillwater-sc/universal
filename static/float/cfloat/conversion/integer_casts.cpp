// integer_casts.cpp: narrow signed integer sources, and exact integer casts that follow posit's rule
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Sources (#2087):
//   - a negative short or signed char was negated in promoted int and sign-extended into a uint64:
//     cfloat<32,8,uint32_t,true,false,false>(short(-3)) was -9.22e18, with an undefined shift;
//   - plain char took the unsigned path: with a signed char, cfloat<16,5,...>(char(-1)) was inf.
//   The reference is the int source, whose path is correct: every narrow value must give the same
//   encoding through the constructor and through assignment.
//
// Casts (#2087, posit's rule from #2073, amended by #2104): truncate toward zero, exactly at every
// width, saturate past the range (inf included) to +/-max ([0, max - 1] unsigned), and give the
// integer NaR for a NaN: min, or max for an unsigned type.  They used to
// cast a double, so cfloat<64,11,uint32_t>(16777217).to_int() was 16777216 and
// cfloat<80,15,uint32_t>(2^53 + 1) cast to long long was 2^53.  Narrow layouts are checked over every
// encoding against their exact double value; wide ones against exactly represented 64-bit integers.
#include <universal/utility/directives.hpp>
#define CFLOAT_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_suite.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <random>
#include <string>

namespace {

template<typename Cfloat>
bool same_encoding(const Cfloat& a, const Cfloat& b) {
	for (unsigned i = 0; i < Cfloat::nbits; ++i) if (a.at(i) != b.at(i)) return false;
	return true;
}

// every short, signed char and char gives the encoding of the same int
template<typename Cfloat>
int VerifyNarrowIntegerSources(bool reportTestCases) {
	int fails = 0;
	for (int v = std::numeric_limits<short>::min(); v <= std::numeric_limits<short>::max(); ++v) {
		const Cfloat ref(v);
		const Cfloat ctor(static_cast<short>(v));
		Cfloat asgn; asgn = static_cast<short>(v);
		if (!same_encoding(ctor, ref) || !same_encoding(asgn, ref)) {
			if (reportTestCases && fails < 10) std::cerr << "FAIL: short(" << v << ") -> " << to_binary(ctor) << ", expected " << to_binary(ref) << '\n';
			++fails;
		}
	}
	for (int v = -128; v <= 127; ++v) {
		const Cfloat ref(v);
		const Cfloat ctor(static_cast<signed char>(v));
		Cfloat asgn; asgn = static_cast<signed char>(v);
		if (!same_encoding(ctor, ref) || !same_encoding(asgn, ref)) {
			if (reportTestCases && fails < 10) std::cerr << "FAIL: signed char(" << v << ") -> " << to_binary(ctor) << ", expected " << to_binary(ref) << '\n';
			++fails;
		}
	}
	for (int k = 0; k < 256; ++k) {
		const char c = static_cast<char>(static_cast<unsigned char>(k));
		const Cfloat ref(static_cast<int>(c));   // the value plain char holds on this platform
		const Cfloat ctor(c);
		Cfloat asgn; asgn = c;
		if (!same_encoding(ctor, ref) || !same_encoding(asgn, ref)) {
			if (reportTestCases && fails < 10) std::cerr << "FAIL: char(" << static_cast<int>(c) << ") -> " << to_binary(ctor) << ", expected " << to_binary(ref) << '\n';
			++fails;
		}
	}
	return fails;
}

// the cast rule applied to an exact double value
template<typename Integer>
Integer reference_cast(double d, bool isNaN) {
	using L = std::numeric_limits<Integer>;
	if (isNaN) return std::is_signed_v<Integer> ? L::min() : L::max();
	const double t = std::trunc(d);
	if (!std::is_signed_v<Integer> && t < 0.0) return Integer(0);
	// a finite value casts to [-max, max], or [0, max - 1] for an unsigned type
	const double past = std::ldexp(1.0, L::digits);   // 2^digits, past max
	const uint64_t cap = static_cast<uint64_t>(L::max()) - (std::is_signed_v<Integer> ? 0u : 1u);
	const double a = std::fabs(t);
	const uint64_t mag = (a >= past) ? cap : std::min(cap, static_cast<uint64_t>(a));
	return (t < 0.0) ? static_cast<Integer>(-static_cast<Integer>(mag)) : static_cast<Integer>(mag);
}

// every encoding of a layout of at most 16 bits: its double value is exact
template<typename Cfloat>
int VerifyCastsOverEncodings(bool reportTestCases) {
	static_assert(Cfloat::nbits <= 16, "the double reference is exact up to 16 bits");
	int fails = 0;
	Cfloat c;
	for (uint64_t bits = 0; bits < (uint64_t(1) << Cfloat::nbits); ++bits) {
		c.setbits(bits);
		const bool nan = c.isnan();
		const double d = nan ? 0.0 : double(c);
		const int i = int(c);
		const long long ll = (long long)c;
		const long l = long(c);
		const unsigned u = c.template to_integer<unsigned>();
		const unsigned short us = c.template to_integer<unsigned short>();
		if (i != reference_cast<int>(d, nan) || ll != reference_cast<long long>(d, nan) || l != reference_cast<long>(d, nan)
		    || u != reference_cast<unsigned>(d, nan) || us != reference_cast<unsigned short>(d, nan)) {
			if (reportTestCases && fails < 10) std::cerr << "FAIL: " << to_binary(c) << " (" << d << ") -> int " << i << ", long long " << ll
			                                             << "; expected " << reference_cast<int>(d, nan) << ", " << reference_cast<long long>(d, nan) << '\n';
			++fails;
		}
	}
	return fails;
}

// wide layouts: 64-bit integers that the layout holds exactly, and halves added to them
template<typename Cfloat, unsigned magnitudeBits>
int VerifyWideCasts(unsigned samples, bool reportTestCases) {
	int fails = 0;
	std::mt19937_64 rng(2087);
	for (unsigned k = 0; k < samples; ++k) {
		const unsigned width = 1u + static_cast<unsigned>(rng() % magnitudeBits);   // spread over the magnitudes
		const uint64_t m = rng() >> (64u - width);
		const long long v = (rng() & 1u) ? -static_cast<long long>(m) : static_cast<long long>(m);
		const Cfloat c(v);
		if ((long long)c != v) {
			if (reportTestCases && fails < 10) std::cerr << "FAIL: " << v << " -> " << (long long)c << '\n';
			++fails;
		}
		const int expectInt = (v > std::numeric_limits<int>::max()) ? std::numeric_limits<int>::max()
		                    : (v < -std::numeric_limits<int>::max()) ? -std::numeric_limits<int>::max() : static_cast<int>(v);
		if (int(c) != expectInt) {
			if (reportTestCases && fails < 10) std::cerr << "FAIL: int(" << v << ") -> " << int(c) << ", expected " << expectInt << '\n';
			++fails;
		}
		// truncation toward zero: v + 1/2 (sign of v) truncates back to v, when the half fits
		if (width + 1u <= Cfloat::fbits + 1u && v != 0) {
			Cfloat h(0.5);
			Cfloat t = (v < 0) ? Cfloat(c - h) : Cfloat(c + h);
			if ((long long)t != v) {
				if (reportTestCases && fails < 10) std::cerr << "FAIL: " << v << " +/- 0.5 -> " << (long long)t << '\n';
				++fails;
			}
		}
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

	std::string test_suite  = "cfloat integer sources and casts";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	std::cout << "short(-3)        : " << double(cfloat<32, 8, uint32_t, true, false, false>(short(-3))) << '\n';
	std::cout << "(2^53 + 1) as ll : " << (long long)cfloat<80, 15, uint32_t>((1ll << 53) + 1) << '\n';
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors

#else

#if REGRESSION_LEVEL_1
	// the issue's values
	if (double(cfloat<32, 8, uint32_t, true, false, false>(short(-3))) != -3.0) {
		std::cerr << "FAIL: cfloat<32,8,uint32_t,true,false,false>(short(-3)) != -3\n";
		++nrOfFailedTestCases;
	}
	if (double(cfloat<16, 5, uint16_t, true, false, false>(static_cast<char>(-1))) != static_cast<double>(static_cast<char>(-1))) {
		std::cerr << "FAIL: cfloat<16,5,uint16_t,true,false,false>(char(-1)) != char(-1)\n";
		++nrOfFailedTestCases;
	}
	if (cfloat<64, 11, uint32_t>(16777217).to_int() != 16777217) {
		std::cerr << "FAIL: cfloat<64,11,uint32_t>(16777217).to_int() != 16777217\n";
		++nrOfFailedTestCases;
	}
	if ((long long)cfloat<80, 15, uint32_t>((1ll << 53) + 1) != (1ll << 53) + 1) {
		std::cerr << "FAIL: cfloat<80,15,uint32_t>(2^53 + 1) cast to long long != 2^53 + 1\n";
		++nrOfFailedTestCases;
	}
	{
		// the range ends and the special values
		using C = cfloat<80, 15, uint32_t>;
		const long long llmin = std::numeric_limits<long long>::min();
		const long long llmax = std::numeric_limits<long long>::max();
		const C two63(std::ldexp(1.0, 63));   // exact in double
		// llmin is the integer NaR (#2104): -2^63, exact in the cfloat, saturates to -llmax
		const bool ok = (long long)C(llmin) == -llmax
		             && (long long)two63 == llmax                    // 2^63 saturates
		             && (long long)(-two63) == -llmax                // -2^63 saturates
		             && (long long)(-two63 + C(1024)) == -llmax + 1023   // exact, just inside the range
		             && (long long)(-two63 - C(1)) == -llmax         // past it saturates
		             && (long long)C(SpecificValue::qnan) == llmin
		             && (long long)C(SpecificValue::infpos) == llmax
		             && (long long)C(SpecificValue::infneg) == -llmax
		             && int(C(SpecificValue::qnan)) == std::numeric_limits<int>::min()
		             && int(C(SpecificValue::maxpos)) == std::numeric_limits<int>::max()
		             && int(C(SpecificValue::maxneg)) == -std::numeric_limits<int>::max()
		             && C(SpecificValue::qnan).template to_integer<unsigned>() == std::numeric_limits<unsigned>::max()
		             && C(SpecificValue::infpos).template to_integer<unsigned>() == std::numeric_limits<unsigned>::max() - 1u
		             && int(C(0.75)) == 0 && int(C(-0.75)) == 0 && int(C(-1.75)) == -1;
		if (!ok) {
			std::cerr << "FAIL: cfloat<80,15,uint32_t> range ends, NaN or inf casts\n";
			++nrOfFailedTestCases;
		}
	}

	nrOfFailedTestCases += ReportTestResult(VerifyNarrowIntegerSources<cfloat<32, 8, uint32_t, true, false, false>>(reportTestCases), "cfloat<32,8,uint32_t,t,f,f>", "narrow sources");
	nrOfFailedTestCases += ReportTestResult(VerifyNarrowIntegerSources<cfloat<16, 5, uint16_t, true, false, false>>(reportTestCases), "cfloat<16,5,uint16_t,t,f,f>", "narrow sources");
	nrOfFailedTestCases += ReportTestResult(VerifyNarrowIntegerSources<cfloat<12, 4, uint8_t, true, true, true>>(reportTestCases),    "cfloat<12,4,uint8_t,t,t,t>",  "narrow sources");
	nrOfFailedTestCases += ReportTestResult(VerifyNarrowIntegerSources<cfloat<40, 8, uint8_t, true, false, false>>(reportTestCases),  "cfloat<40,8,uint8_t,t,f,f>",  "narrow sources");
	nrOfFailedTestCases += ReportTestResult(VerifyNarrowIntegerSources<cfloat<80, 15, uint64_t, true, false, false>>(reportTestCases), "cfloat<80,15,uint64_t,t,f,f>", "narrow sources");

	nrOfFailedTestCases += ReportTestResult(VerifyCastsOverEncodings<cfloat<16, 5, uint16_t, true, false, false>>(reportTestCases), "cfloat<16,5,uint16_t,t,f,f>", "casts");
	nrOfFailedTestCases += ReportTestResult(VerifyCastsOverEncodings<cfloat<16, 8, uint8_t, true, true, false>>(reportTestCases),   "cfloat<16,8,uint8_t,t,t,f>",  "casts");
	nrOfFailedTestCases += ReportTestResult(VerifyCastsOverEncodings<cfloat<16, 8, uint16_t, false, true, true>>(reportTestCases),  "cfloat<16,8,uint16_t,f,t,t>", "casts");
	nrOfFailedTestCases += ReportTestResult(VerifyCastsOverEncodings<cfloat<16, 11, uint16_t, true, false, false>>(reportTestCases), "cfloat<16,11,uint16_t,t,f,f>", "casts");
	nrOfFailedTestCases += ReportTestResult(VerifyCastsOverEncodings<cfloat<12, 4, uint8_t, false, false, false>>(reportTestCases), "cfloat<12,4,uint8_t,f,f,f>",  "casts");
	nrOfFailedTestCases += ReportTestResult(VerifyCastsOverEncodings<cfloat<8, 2, uint8_t, true, true, false>>(reportTestCases),    "cfloat<8,2,uint8_t,t,t,f>",   "casts");

	nrOfFailedTestCases += ReportTestResult((VerifyWideCasts<cfloat<64, 11, uint32_t>, 53>(2000, reportTestCases)),                  "cfloat<64,11,uint32_t>",        "wide casts");
	nrOfFailedTestCases += ReportTestResult((VerifyWideCasts<cfloat<80, 15, uint32_t>, 63>(2000, reportTestCases)),                  "cfloat<80,15,uint32_t>",        "wide casts");
	nrOfFailedTestCases += ReportTestResult((VerifyWideCasts<cfloat<96, 15, uint16_t, true, false, false>, 63>(2000, reportTestCases)), "cfloat<96,15,uint16_t,t,f,f>", "wide casts");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult((VerifyWideCasts<cfloat<80, 15, uint8_t>, 63>(20000, reportTestCases)),  "cfloat<80,15,uint8_t>",  "wide casts");
	nrOfFailedTestCases += ReportTestResult((VerifyWideCasts<cfloat<128, 15, uint64_t>, 63>(20000, reportTestCases)), "cfloat<128,15,uint64_t>", "wide casts");
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
