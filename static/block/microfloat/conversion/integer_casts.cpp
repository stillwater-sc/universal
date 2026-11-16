// integer_casts.cpp: microfloat casts to the native integers follow the library's integer-cast rule (#2104)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The casts were static_casts of the microfloat's float: undefined behaviour for NaN, inf and every
// value outside the integer type, which covers ordinary values of the 8-bit formats:
// short(e5m2 57344), (signed char)e4m3(448), unsigned(e4m3(-1)).  x86 gave the integer indefinite
// (INT_MIN), ARM saturated and gave 0 for NaN.  They now follow posit's rule (#2073, amended by #2104):
//   - truncate toward zero;
//   - saturate a value past the range, inf included, to [-max, max], or [0, max - 1] for an unsigned
//     type, and every negative value to 0 for an unsigned type;
//   - NaN gives the integer NaR: min, or max for an unsigned type.
// Every encoding of each format is checked for every cast, against its exact double value.
#include <universal/utility/directives.hpp>
#include <universal/number/microfloat/microfloat.hpp>
#include <universal/verification/test_suite.hpp>

#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <string>
#include <type_traits>

namespace {

	// the rule, applied to an exact double value
	template<typename Integer>
	Integer ReferenceCast(double v) {
		using L = std::numeric_limits<Integer>;
		if (std::isnan(v)) return L::is_signed ? L::min() : L::max();
		const bool negative = std::signbit(v) && v != 0.0;
		if (negative && !L::is_signed) return Integer(0);
		const std::uint64_t cap = static_cast<std::uint64_t>(L::max()) - (L::is_signed ? 0u : 1u);
		const double a = std::trunc(std::fabs(v));
		std::uint64_t mag = (a >= std::ldexp(1.0, 64)) ? cap : static_cast<std::uint64_t>(a);   // inf included
		if (mag > cap) mag = cap;
		return negative ? static_cast<Integer>(-static_cast<Integer>(mag)) : static_cast<Integer>(mag);
	}

	template<typename Integer, typename Mf>
	int CheckCast(const Mf& x, const std::string& tag, const char* type, bool reportTestCases) {
		const double v = double(x);
		const Integer got = static_cast<Integer>(x);
		const Integer expected = ReferenceCast<Integer>(v);
		if (got == expected) return 0;
		if (reportTestCases) {
			std::cerr << "FAIL " << tag << " 0x" << std::hex << unsigned(x.bits()) << std::dec << " (" << v << ") -> " << type << ' '
			          << static_cast<long long>(got) << ", expected " << static_cast<long long>(expected) << '\n';
		}
		return 1;
	}

	template<typename Mf>
	int VerifyCastsOverEncodings(const std::string& tag, bool reportTestCases) {
		int fails = 0;
		for (unsigned bits = 0; bits < (1u << Mf::nbits); ++bits) {
			Mf x;
			x.setbits(bits);
			fails += CheckCast<signed char>(x, tag, "signed char", reportTestCases);
			fails += CheckCast<char>(x, tag, "char", reportTestCases);
			fails += CheckCast<short>(x, tag, "short", reportTestCases);
			fails += CheckCast<int>(x, tag, "int", reportTestCases);
			fails += CheckCast<long>(x, tag, "long", reportTestCases);
			fails += CheckCast<long long>(x, tag, "long long", reportTestCases);
			fails += CheckCast<unsigned short>(x, tag, "unsigned short", reportTestCases);
			fails += CheckCast<unsigned int>(x, tag, "unsigned int", reportTestCases);
			fails += CheckCast<unsigned long>(x, tag, "unsigned long", reportTestCases);
			fails += CheckCast<unsigned long long>(x, tag, "unsigned long long", reportTestCases);
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

	std::string test_suite  = "microfloat integer casts (#2104)";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors

#else

#if REGRESSION_LEVEL_1
	{
		// the cases the issue names, each undefined behaviour before
		int fails = 0;
		const double nan = std::numeric_limits<double>::quiet_NaN();
		if (int(e4m3(nan)) != std::numeric_limits<int>::min()) ++fails;
		if (unsigned(e4m3(nan)) != std::numeric_limits<unsigned>::max()) ++fails;
		if (int(e5m2(SpecificValue::infpos)) != std::numeric_limits<int>::max()) ++fails;
		if (int(e5m2(SpecificValue::infneg)) != -std::numeric_limits<int>::max()) ++fails;
		if (short(e5m2(57344.0f)) != std::numeric_limits<short>::max()) ++fails;
		if (short(e5m2(-57344.0f)) != -std::numeric_limits<short>::max()) ++fails;
		if (static_cast<signed char>(e4m3(448.0f)) != 127) ++fails;
		if (static_cast<signed char>(e4m3(-448.0f)) != -127) ++fails;
		if (unsigned(e4m3(-1.0f)) != 0u) ++fails;
		if (static_cast<unsigned short>(e5m2(57344.0f)) != 57344u) ++fails;
		if (int(e2m1(-5.0f)) != -4 || int(e2m1(1.5f)) != 1) ++fails;   // -5 rounds to -4; 1.5 truncates
		nrOfFailedTestCases += ReportTestResult(fails, "microfloat", "the issue's cases");
	}
	static_assert(int(e4m3(-7.5f)) == -7, "the casts are constexpr");
	static_assert(short(e5m2(57344.0f)) == 32767, "saturation is constexpr");

	nrOfFailedTestCases += ReportTestResult(VerifyCastsOverEncodings<e2m1>("e2m1", reportTestCases), "e2m1", "casts, every encoding");
	nrOfFailedTestCases += ReportTestResult(VerifyCastsOverEncodings<e2m3>("e2m3", reportTestCases), "e2m3", "casts, every encoding");
	nrOfFailedTestCases += ReportTestResult(VerifyCastsOverEncodings<e3m2>("e3m2", reportTestCases), "e3m2", "casts, every encoding");
	nrOfFailedTestCases += ReportTestResult(VerifyCastsOverEncodings<e4m3>("e4m3", reportTestCases), "e4m3", "casts, every encoding");
	nrOfFailedTestCases += ReportTestResult(VerifyCastsOverEncodings<e5m2>("e5m2", reportTestCases), "e5m2", "casts, every encoding");
	nrOfFailedTestCases += ReportTestResult(VerifyCastsOverEncodings<e5m2_saturating>("e5m2_saturating", reportTestCases), "e5m2_saturating", "casts, every encoding");
	nrOfFailedTestCases += ReportTestResult(VerifyCastsOverEncodings<microfloat<8, 7, false, false, true>>("e7m0", reportTestCases), "e7m0", "casts, every encoding");
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
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
