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
#include <cfloat>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>
#include <string>
#include <universal/native/ieee754.hpp>
#include <universal/number/cfloat/cfloat.hpp>
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
