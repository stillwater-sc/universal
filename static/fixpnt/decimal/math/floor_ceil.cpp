// floor_ceil.cpp: floor and ceil of dfixpnt against integer floor and ceiling division
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cstdint>
#include <iostream>
#include <random>
#include <string>
#include <universal/number/dfixpnt/dfixpnt.hpp>
#include <universal/verification/test_suite.hpp>

/*
   floor() of a negative value with a fraction, and ceil() of a positive one, add or subtract
   one unit. They built that unit as a default-constructed dfixpnt with one digit set, but
   dfixpnt is trivially constructible, so every other digit and the sign were indeterminate
   (#1476): floor(-12.375) came out as -46513 with gcc, and clang crashed inside floor().

   A dfixpnt<ndigits, radix> holding the scaled integer n has the value n / 10^radix, so the
   reference is integer arithmetic: floor is the floor division of n by 10^radix, times
   10^radix, and ceil the ceiling division. Values whose floor or ceiling does not fit the
   type are skipped. A result of zero may carry the sign of its operand, as IEEE does
   (ceil(-0.375) is -0), so values are compared, not signs.
*/

namespace sw { namespace universal {

namespace floor_ceil {

inline std::int64_t pow10(unsigned k) {
	std::int64_t p = 1;
	for (unsigned i = 0; i < k; ++i) p *= 10;
	return p;
}

// the dfixpnt holding the scaled integer n, built digit by digit
template<typename F>
F FromScaled(std::int64_t n) {
	F v;
	v.setzero();
	std::uint64_t m = static_cast<std::uint64_t>(n < 0 ? -n : n);
	for (unsigned i = 0; i < F::ndigits; ++i) {
		v.setdigit(i, static_cast<unsigned>(m % 10));
		m /= 10;
	}
	v.setsign(n < 0);
	return v;
}

// the scaled integer a dfixpnt holds
template<typename F>
std::int64_t ToScaled(const F& v) {
	std::int64_t n = 0;
	for (unsigned i = F::ndigits; i > 0; --i) n = n * 10 + static_cast<std::int64_t>(v.digit(i - 1));
	return v.sign() ? -n : n;
}

// floor and ceil of the scaled integer n, both as scaled integers
inline std::int64_t FloorScaled(std::int64_t n, std::int64_t unit) {
	std::int64_t q = n / unit;
	if (n % unit != 0 && n < 0) --q;
	return q * unit;
}
inline std::int64_t CeilScaled(std::int64_t n, std::int64_t unit) {
	std::int64_t q = n / unit;
	if (n % unit != 0 && n > 0) ++q;
	return q * unit;
}

template<typename F>
int VerifyValue(std::int64_t n, bool reportTestCases, const std::string& config) {
	const std::int64_t unit = pow10(F::radix), limit = pow10(F::ndigits) - 1;
	const F v = FromScaled<F>(n);
	int fails = 0;
	auto check = [&](const char* op, const F& got, std::int64_t expected) {
		// With no integer digit the integer part is 0. Decide that before the range filter: the
		// reference floor of a negative fraction, and ceil of a positive one, is +-10^radix,
		// outside the type, and those are exactly the cases that exercise UnitOf() there.
		if constexpr (F::radix == F::ndigits) {
			expected = 0;
		}
		else if (expected < -limit || expected > limit) {
			return;  // the result does not fit the type
		}
		if (ToScaled(got) != expected) {
			++fails;
			if (reportTestCases || fails == 1)
				std::cerr << "FAIL: " << config << " " << op << " of scaled " << n << " = scaled " << ToScaled(got)
				          << ", expected " << expected << '\n';
		}
	};
	check("floor", floor(v), FloorScaled(n, unit));
	check("ceil", ceil(v), CeilScaled(n, unit));
	return fails;
}

// every value of the type when exhaustive, else random ones, plus the edges either side of 0
template<unsigned ndigits, unsigned radix, DecimalEncoding encoding, bool arithmetic, typename bt>
int VerifyFloorCeil(bool exhaustive, unsigned samples, bool reportTestCases) {
	using F = dfixpnt<ndigits, radix, encoding, arithmetic, bt>;
	const char* enc = (encoding == DecimalEncoding::BCD) ? "BCD" : (encoding == DecimalEncoding::BID) ? "BID" : "DPD";
	const std::string config = std::string("dfixpnt<") + std::to_string(ndigits) + ", " + std::to_string(radix) + ", " +
	                           enc + (arithmetic == Modulo ? ", Modulo, " : ", Saturate, ") + "uint" +
	                           std::to_string(sizeof(bt) * 8) + "_t>";
	const std::int64_t limit = pow10(ndigits) - 1;
	int nrOfFailedTestCases = 0;
	if (exhaustive) {
		for (std::int64_t n = -limit; n <= limit; ++n)
			nrOfFailedTestCases += VerifyValue<F>(n, reportTestCases, config);
		return nrOfFailedTestCases;
	}
	std::mt19937_64 rng(1476u + ndigits * 10u + radix);
	const std::int64_t unit = pow10(radix);
	for (std::int64_t n :
	     {std::int64_t(1), std::int64_t(-1), unit - 1, 1 - unit, unit, -unit, unit + 1, -unit - 1, limit, -limit})
		nrOfFailedTestCases += VerifyValue<F>(n, reportTestCases, config);
	for (unsigned k = 0; k < samples; ++k) {
		const std::int64_t n = static_cast<std::int64_t>(rng() % static_cast<std::uint64_t>(2 * limit + 1)) - limit;
		nrOfFailedTestCases += VerifyValue<F>(n, reportTestCases, config);
	}
	return nrOfFailedTestCases;
}

// all three encodings of one shape
template<unsigned ndigits, unsigned radix, bool arithmetic, typename bt>
int VerifyEncodings(bool exhaustive, unsigned samples, bool reportTestCases) {
	return VerifyFloorCeil<ndigits, radix, DecimalEncoding::BCD, arithmetic, bt>(exhaustive, samples, reportTestCases) +
	       VerifyFloorCeil<ndigits, radix, DecimalEncoding::BID, arithmetic, bt>(exhaustive, samples, reportTestCases) +
	       VerifyFloorCeil<ndigits, radix, DecimalEncoding::DPD, arithmetic, bt>(exhaustive, samples, reportTestCases);
}

} // namespace floor_ceil

}} // namespace sw::universal

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
	using namespace sw::universal::floor_ceil;

	std::string test_suite  = "dfixpnt floor and ceil";
	std::string test_tag    = "floor/ceil";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	using F = dfixpnt<8, 3, DecimalEncoding::BCD, Modulo, std::uint8_t>;
	std::cout << "floor(-12.375) = " << floor(F(-12.375)) << " (-13.000)\n";
	std::cout << "ceil ( 12.375) = " << ceil(F(12.375)) << " (13.000)\n";
	nrOfFailedTestCases +=
	    ReportTestResult(VerifyEncodings<4, 2, Modulo, std::uint8_t>(true, 0, true), "dfixpnt<4,2>", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	{
		// the cases reported in #1476
		using F = dfixpnt<8, 3, DecimalEncoding::BCD, Modulo, std::uint8_t>;
		int fails = 0;
		if (!(floor(F(-12.375)) == F(-13))) {
			++fails;
			std::cerr << "FAIL: floor(-12.375) = " << floor(F(-12.375)) << ", expected -13.000\n";
		}
		if (!(ceil(F(12.375)) == F(13))) {
			++fails;
			std::cerr << "FAIL: ceil(12.375) = " << ceil(F(12.375)) << ", expected 13.000\n";
		}
		nrOfFailedTestCases += ReportTestResult(fails, "dfixpnt<8,3,BCD>", "reported cases");
	}
	// exhaustive: every value of small shapes, in every encoding and both arithmetic modes
	nrOfFailedTestCases += ReportTestResult(VerifyEncodings<4, 2, Modulo, std::uint8_t>(true, 0, reportTestCases),
	                                        "dfixpnt<4,2,Modulo>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyEncodings<4, 2, Saturate, std::uint8_t>(true, 0, reportTestCases),
	                                        "dfixpnt<4,2,Saturate>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyEncodings<4, 1, Modulo, std::uint16_t>(true, 0, reportTestCases),
	                                        "dfixpnt<4,1,uint16_t>", test_tag);
	// the edges: no fraction digits, and no integer digits
	nrOfFailedTestCases += ReportTestResult(VerifyEncodings<3, 0, Modulo, std::uint8_t>(true, 0, reportTestCases),
	                                        "dfixpnt<3,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyEncodings<3, 3, Modulo, std::uint8_t>(true, 0, reportTestCases),
	                                        "dfixpnt<3,3>", test_tag);
	// sampled: wider shapes and limbs
	nrOfFailedTestCases += ReportTestResult(VerifyEncodings<8, 3, Modulo, std::uint8_t>(false, 20000, reportTestCases),
	                                        "dfixpnt<8,3>", test_tag);
	nrOfFailedTestCases +=
	    ReportTestResult(VerifyEncodings<12, 5, Modulo, std::uint32_t>(false, 20000, reportTestCases),
	                     "dfixpnt<12,5,uint32_t>", test_tag);
	nrOfFailedTestCases +=
	    ReportTestResult(VerifyEncodings<16, 8, Saturate, std::uint64_t>(false, 20000, reportTestCases),
	                     "dfixpnt<16,8,uint64_t>", test_tag);
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyEncodings<6, 3, Modulo, std::uint8_t>(true, 0, reportTestCases),
	                                        "dfixpnt<6,3>", test_tag);
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyEncodings<7, 4, Saturate, std::uint16_t>(true, 0, reportTestCases),
	                                        "dfixpnt<7,4,Saturate>", test_tag);
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases +=
	    ReportTestResult(VerifyEncodings<18, 9, Modulo, std::uint64_t>(false, 1000000, reportTestCases),
	                     "dfixpnt<18,9,uint64_t>", test_tag);
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
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
