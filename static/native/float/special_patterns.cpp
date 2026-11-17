// special_patterns.cpp: NaN payloads, ulp at the edges, setbit bounds and signed-zero casts (#2059)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The QA sweep in #2039 found:
//   - checkNaN recognised only the canonical NaN patterns: float 0x7f800001, 0x7fc00001, 0x7fffffff
//     and double 0x7ff0000000000001 were "not NaN", and every long double was;
//   - ulp() was negative for negative arguments, inf at max() and NaN at inf;
//   - setbit(float&/double&, index) shifted past the width for an index >= 32/64;
//   - universal_cast of a -0 returned +0.
// The NaN patterns are built from bits through bit_cast, not std::nan, so every payload is exercised.
#include <universal/utility/directives.hpp>
#include <bit>
#include <cfloat>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>
#include <string>
#include <universal/native/ieee754.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/number/lns/lns.hpp>
#include <universal/number/dd/dd.hpp>
#include <universal/number/integer/integer.hpp>
#include <universal/number/convert/convert.hpp>
#include <universal/verification/test_suite.hpp>

namespace {
	using namespace sw::universal;

	int report(bool ok, const std::string& what, bool reportTestCases) {
		if (ok) return 0;
		if (reportTestCases) std::cerr << "FAIL: " << what << '\n';
		return 1;
	}

	template<typename Real, typename Bits>
	Real from_bits(Bits b) { Real r; std::memcpy(&r, &b, sizeof(Real)); return r; }

	// a long double from its two 64-bit words, the low word first in memory on a little-endian target
	inline long double long_double_from_words(std::uint64_t hi, std::uint64_t lo) {
		long double r{};
		std::uint64_t words[2] = { lo, hi };
		if constexpr (std::endian::native == std::endian::big) { words[0] = hi; words[1] = lo; }
		std::memcpy(&r, words, (sizeof(r) < sizeof(words)) ? sizeof(r) : sizeof(words));
		return r;
	}

	// checkNaN(long double) on bit patterns of the platform's own format: quiet, signalling and
	// payload NaNs. binary128 (aarch64 Linux, in the full CI tier) and IBM double-double had no
	// coverage on x86 (#2107).
	int VerifyLongDoubleNaNPatterns(bool reportTestCases) {
		int fails = 0;
		int type = NAN_TYPE_NEITHER;
		struct L { std::uint64_t hi, lo; int type; const char* label; };
#if LDBL_MANT_DIG == 113
		// binary128: sign, 15 exponent bits, 112 fraction bits; the quiet bit is fraction bit 111,
		// bit 47 of the high word
		const L patterns[] = {
			{ 0x7fff800000000000ull, 0x0ull,  NAN_TYPE_QUIET,      "binary128 canonical quiet NaN" },
			{ 0xffff800000000123ull, 0x45ull, NAN_TYPE_QUIET,      "binary128 negative quiet NaN with payload" },
			{ 0x7fff400000000000ull, 0x0ull,  NAN_TYPE_SIGNALLING, "binary128 signalling NaN" },
			{ 0x7fff000000000000ull, 0x1ull,  NAN_TYPE_SIGNALLING, "binary128 signalling NaN, payload in the low word" },
			{ 0xffff7fffffffffffull, ~0ull,   NAN_TYPE_SIGNALLING, "binary128 negative signalling NaN, full payload" },
		};
		const L notNaN[] = {
			{ 0x7fff000000000000ull, 0x0ull, NAN_TYPE_NEITHER, "binary128 +inf" },
			{ 0x3fff800000000000ull, 0x0ull, NAN_TYPE_NEITHER, "binary128 1.5" },
		};
#elif LDBL_MANT_DIG == 64
		// x87 extended: 16 bits of sign and exponent in the high word, an explicit integer bit 63
		// and the quiet bit 62 in the low word
		const L patterns[] = {
			{ 0x7fffull, 0xc000000000000000ull, NAN_TYPE_QUIET,      "x87 canonical quiet NaN" },
			{ 0xffffull, 0xc000000000000abcull, NAN_TYPE_QUIET,      "x87 negative quiet NaN with payload" },
			{ 0x7fffull, 0xa000000000000000ull, NAN_TYPE_SIGNALLING, "x87 signalling NaN" },
			{ 0x7fffull, 0x8000000000000001ull, NAN_TYPE_SIGNALLING, "x87 signalling NaN, low payload" },
		};
		const L notNaN[] = {
			{ 0x7fffull, 0x8000000000000000ull, NAN_TYPE_NEITHER, "x87 +inf" },
			{ 0x3fffull, 0xc000000000000000ull, NAN_TYPE_NEITHER, "x87 1.5" },
		};
#elif LDBL_MANT_DIG == 106
		// IBM double-double: the leading double carries the NaN; its quiet bit is 2^51. The leading
		// double is the first word in memory, which long_double_from_words puts in hi on big endian
		// and in lo on little endian
		constexpr bool little = (std::endian::native == std::endian::little);
		const L patterns[] = {
			{ little ? 0ull : 0x7ff8000000000000ull, little ? 0x7ff8000000000000ull : 0ull, NAN_TYPE_QUIET,      "double-double quiet NaN" },
			{ little ? 0ull : 0x7ff4000000000000ull, little ? 0x7ff4000000000000ull : 0ull, NAN_TYPE_SIGNALLING, "double-double signalling NaN" },
		};
		const L notNaN[] = {
			{ little ? 0ull : 0x7ff0000000000000ull, little ? 0x7ff0000000000000ull : 0ull, NAN_TYPE_NEITHER, "double-double +inf" },
		};
#else
		// long double is double: the double patterns above cover it
		const L patterns[] = { { 0, 0, NAN_TYPE_NEITHER, nullptr } };
		const L notNaN[]   = { { 0, 0, NAN_TYPE_NEITHER, nullptr } };
#endif
		for (const L& l : patterns) {
			if (l.label == nullptr) continue;
			const bool nan = checkNaN(long_double_from_words(l.hi, l.lo), type);
			fails += report(nan && type == l.type, std::string(l.label) + " is not classified as its NaN type", reportTestCases);
		}
		for (const L& l : notNaN) {
			if (l.label == nullptr) continue;
			fails += report(!checkNaN(long_double_from_words(l.hi, l.lo), type) && type == NAN_TYPE_NEITHER, std::string(l.label) + " is classified as a NaN", reportTestCases);
		}
		return fails;
	}

	int VerifyNaNPayloads(bool reportTestCases) {
		int fails = 0;
		int type = NAN_TYPE_NEITHER;
		// float: quiet bit 0x00400000
		struct F { std::uint32_t bits; int type; };
		const F floats[] = {
			{ 0x7f800001u, NAN_TYPE_SIGNALLING }, { 0x7fa00000u, NAN_TYPE_SIGNALLING }, { 0x7fbfffffu, NAN_TYPE_SIGNALLING },
			{ 0x7fc00000u, NAN_TYPE_QUIET },      { 0x7fffffffu, NAN_TYPE_QUIET },      { 0xffc00123u, NAN_TYPE_QUIET },
			{ 0xff800002u, NAN_TYPE_SIGNALLING },
#if !defined(_MSC_VER)
			{ 0x7fc00001u, NAN_TYPE_QUIET },      // MSVC classifies its signaling_NaN() pattern as signalling
#endif
		};
		for (const F& f : floats) {
			const bool nan = checkNaN(from_bits<float>(f.bits), type);
			fails += report(nan && type == f.type, "float " + to_hex(f.bits) + " is not classified as its NaN type", reportTestCases);
		}
		for (std::uint32_t inf : { 0x7f800000u, 0xff800000u, 0x3f800000u, 0x00000001u }) {
			fails += report(!checkNaN(from_bits<float>(inf), type) && type == NAN_TYPE_NEITHER, "float " + to_hex(inf) + " is classified as a NaN", reportTestCases);
		}
		// double: quiet bit 2^51
		struct D { std::uint64_t bits; int type; };
		const D doubles[] = {
			{ 0x7ff0000000000001ull, NAN_TYPE_SIGNALLING }, { 0x7ff4000000000000ull, NAN_TYPE_SIGNALLING },
			{ 0x7ff8000000000000ull, NAN_TYPE_QUIET },      { 0x7fffffffffffffffull, NAN_TYPE_QUIET },
			{ 0xfff8000000000abcull, NAN_TYPE_QUIET },
		};
		for (const D& d : doubles) {
			const bool nan = checkNaN(from_bits<double>(d.bits), type);
			fails += report(nan && type == d.type, "double " + to_hex(d.bits) + " is not classified as its NaN type", reportTestCases);
		}
		fails += report(!checkNaN(from_bits<double>(0x7ff0000000000000ull), type), "double +inf is classified as a NaN", reportTestCases);
		// infinities are unaffected by a payload test
		fails += report(checkInf(from_bits<float>(0xff800000u), type) && type == INF_TYPE_NEGATIVE, "float -inf is not -inf", reportTestCases);
		fails += report(!checkInf(from_bits<float>(0x7f800001u), type), "a float NaN is classified as inf", reportTestCases);
		// long double: at least the quiet NaN and the infinities
		const long double lq = std::numeric_limits<long double>::quiet_NaN();
		fails += report(checkNaN(lq, type) && type == NAN_TYPE_QUIET, "long double quiet_NaN() is not a quiet NaN", reportTestCases);
		fails += report(!checkNaN(1.5L, type), "long double 1.5 is classified as a NaN", reportTestCases);
		fails += report(checkInf(-std::numeric_limits<long double>::infinity(), type) && type == INF_TYPE_NEGATIVE, "long double -inf is not -inf", reportTestCases);
		fails += report(!checkInf(lq, type), "a long double NaN is classified as inf", reportTestCases);
		fails += VerifyLongDoubleNaNPatterns(reportTestCases);
		return fails;
	}

	int VerifyUlp(bool reportTestCases) {
		int fails = 0;
		const double eps = std::numeric_limits<double>::epsilon();
		fails += report(ulp(1.0) == eps && ulp(-1.0) == eps, "ulp(+-1.0) is not epsilon", reportTestCases);
		fails += report(ulp(std::numeric_limits<double>::max()) == std::ldexp(1.0, 971), "ulp(DBL_MAX) is not 2^971", reportTestCases);
		fails += report(ulp(-std::numeric_limits<double>::max()) == std::ldexp(1.0, 971), "ulp(-DBL_MAX) is not 2^971", reportTestCases);
		fails += report(std::isinf(ulp(std::numeric_limits<double>::infinity())), "ulp(inf) is not inf", reportTestCases);
		fails += report(std::isnan(ulp(std::numeric_limits<double>::quiet_NaN())), "ulp(NaN) is not NaN", reportTestCases);
		fails += report(ulp(0.0) == std::numeric_limits<double>::denorm_min(), "ulp(0) is not denorm_min", reportTestCases);
		fails += report(ulp(-0.5f) == std::ldexp(1.0f, -24), "ulp(-0.5f) is not 2^-24", reportTestCases);
		fails += report(ulp(std::numeric_limits<float>::max()) == std::ldexp(1.0f, 104), "ulp(FLT_MAX) is not 2^104", reportTestCases);
		return fails;
	}

	int VerifySetbitBounds(bool reportTestCases) {
		int fails = 0;
		float f = 1.0f;
		setbit(f, 32);
		setbit(f, 200, false);
		fails += report(f == 1.0f, "setbit(float, >= 32) changed the value", reportTestCases);
		setbit(f, 31);
		fails += report(f == -1.0f, "setbit(float, 31) is not the sign", reportTestCases);
		double d = 1.0;
		setbit(d, 64);
		setbit(d, 1000, false);
		fails += report(d == 1.0, "setbit(double, >= 64) changed the value", reportTestCases);
		setbit(d, 0);
		fails += report(d == 1.0 + std::numeric_limits<double>::epsilon(), "setbit(double, 0) is not the last fraction bit", reportTestCases);
		return fails;
	}

	int VerifySignedZeroCast(bool reportTestCases) {
		int fails = 0;
		using C16 = cfloat<16, 5, std::uint16_t, true, false, false>;
		using C32 = cfloat<32, 8, std::uint32_t, true, false, false>;
		const C16 nz(-0.0);
		const C32 r = universal_cast<C32>(nz);
		fails += report(r.iszero() && r.sign(), "universal_cast of -0 is not -0", reportTestCases);
		C32 t(1.0);
		universal_assign(t, nz);
		fails += report(t.iszero() && t.sign(), "universal_assign of -0 is not -0", reportTestCases);
		const C32 p = universal_cast<C32>(C16(0.0));
		fails += report(p.iszero() && !p.sign(), "universal_cast of +0 is not +0", reportTestCases);
		return fails;
	}

	// +0 and -0 cast across the type families (#2107): a target with a -0 (cfloat, dd) keeps the
	// sign of a signed zero, one without it (posit, fixpnt, lns, integer) gives +0; either way the
	// result is zero
	template<typename Target, typename Source>
	int CheckZeroCast(const Source& src, bool targetHasNegativeZero, const char* label, bool reportTestCases) {
		const bool negative = std::signbit(static_cast<double>(src));
		const bool expectSign = targetHasNegativeZero && negative;
		int fails = 0;
		const Target r = universal_cast<Target>(src);
		const double rv = static_cast<double>(r);
		if (!(r.iszero() && rv == 0.0 && std::signbit(rv) == expectSign)) {
			++fails;
			if (reportTestCases) std::cerr << "FAIL: universal_cast of " << (negative ? "-0" : "+0") << " to " << label << " is " << rv << '\n';
		}
		Target t(1);
		universal_assign(t, src);
		const double tv = static_cast<double>(t);
		if (!(t.iszero() && tv == 0.0 && std::signbit(tv) == expectSign)) {
			++fails;
			if (reportTestCases) std::cerr << "FAIL: universal_assign of " << (negative ? "-0" : "+0") << " to " << label << " is " << tv << '\n';
		}
		return fails;
	}

	template<typename Source>
	int CheckZeroCastsFrom(const Source& src, bool reportTestCases) {
		using C32 = cfloat<32, 8, std::uint32_t, true, false, false>;
		int fails = 0;
		fails += CheckZeroCast<C32>(src, true, "cfloat<32,8>", reportTestCases);
		fails += CheckZeroCast<dd>(src, true, "dd", reportTestCases);
		fails += CheckZeroCast<posit<32, 2>>(src, false, "posit<32,2>", reportTestCases);
		fails += CheckZeroCast<fixpnt<16, 8>>(src, false, "fixpnt<16,8>", reportTestCases);
		fails += CheckZeroCast<lns<16, 8>>(src, false, "lns<16,8>", reportTestCases);
		fails += CheckZeroCast<integer<32>>(src, false, "integer<32>", reportTestCases);
		return fails;
	}

	int VerifyZeroCastAcrossTypes(bool reportTestCases) {
		using C16 = cfloat<16, 5, std::uint16_t, true, false, false>;
		int fails = 0;
		fails += CheckZeroCastsFrom(C16(0.0), reportTestCases);
		fails += CheckZeroCastsFrom(C16(-0.0), reportTestCases);
		fails += CheckZeroCastsFrom(dd(0.0), reportTestCases);
		fails += CheckZeroCastsFrom(dd(-0.0), reportTestCases);
		fails += CheckZeroCastsFrom(posit<16, 1>(0), reportTestCases);
		fails += CheckZeroCastsFrom(fixpnt<16, 8>(0), reportTestCases);
		fails += CheckZeroCastsFrom(lns<16, 8>(0), reportTestCases);
		fails += CheckZeroCastsFrom(integer<32>(0), reportTestCases);
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
#define REGRESSION_LEVEL_2 0
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "native NaN payloads, ulp, setbit and signed-zero casts (#2059)";
	std::string test_tag    = "special patterns";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyNaNPayloads(reportTestCases), test_tag, "NaN payloads");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyNaNPayloads(reportTestCases), test_tag, "NaN payloads");
	nrOfFailedTestCases += ReportTestResult(VerifyUlp(reportTestCases), test_tag, "ulp");
	nrOfFailedTestCases += ReportTestResult(VerifySetbitBounds(reportTestCases), test_tag, "setbit bounds");
	nrOfFailedTestCases += ReportTestResult(VerifySignedZeroCast(reportTestCases), test_tag, "universal_cast -0");
	nrOfFailedTestCases += ReportTestResult(VerifyZeroCastAcrossTypes(reportTestCases), test_tag, "universal_cast +-0 across types");
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
