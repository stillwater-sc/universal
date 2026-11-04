// block_type_conversion.cpp: native floats into wide cfloats of every block type
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <limits>
#include <string>
#include <vector>
#define CFLOAT_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_suite.hpp>

/*
   A float or double assigned to a cfloat wider than 64 bits with uint64_t blocks left the cfloat
   at 0 (#1518): cfloat<128,15,uint64_t> = 1.5 was 0. convert_ieee754() places the source's
   fraction bits with a block copy written for blocks narrower than the uint64_t it copies from,
   and its uint64_t branch was empty. It set the bits one at a time there, which made the
   conversion 8-70x slower than with uint32_t blocks (#1918); it now shifts the fraction into the
   at most two limbs it lands in.

   Checked for cfloat<80,11>, <96,11>, <128,15> and <256,19>, whose ranges and precisions hold every
   double exactly, over normal, subnormal and extreme doubles and floats, both signs: the
   uint64_t-block cfloat has the same encoding as the uint32_t- and uint8_t-block ones, and converts
   back to the value it was given. Long double, which reaches these cfloats through
   convert(blocktriple) with a blocktriple of the cfloat's block type (#1520), is checked the same
   way: every block type gives the same encoding, and the value comes back where the cfloat holds all
   of its bits; with es >= 15 that includes min, denorm_min and max.

   Where the double's fraction lands relative to the 64-bit limbs depends on fbits = nbits - 1 - es:
   its lsb sits at bit fbits - 52. cfloat<65,11> puts it at bit 1, with the MSU holding only the
   sign; <128,11> puts it exactly on the limb boundary at bit 64 (no straddle); <129,11> one bit
   past it; the others straddle two limbs. A seeded sweep of random doubles over the whole
   exponent range, subnormals included, checks the encodings agree across block types.

   cfloat<80,8> has a narrower exponent range than double, so doubles far below its min normal
   2^-126 land in its subnormal range with fewer bits than they carry and are rounded before the
   fraction is placed. (2^53 - 1) * 2^-200 rounds up across a binade to 2^-147: the carry runs
   through every fraction bit. A carry out of the fraction into the exponent field, a subnormal
   rounding up to min normal, cannot come from a native float or double when nbits > 64: rounding
   at the top subnormal binade needs fbits < 53, which forces es >= 11, and then the cfloat's min
   normal lies at or below the double's smallest subnormal.
*/

namespace sw {
namespace universal {

namespace block_type_conversion {

inline std::vector<double> Samples() {
	using limit = std::numeric_limits<double>;
	std::vector<double> xs = {1.0, 1.5, 3.25, 0.1, 1.0 / 3.0, 1.0e300, 1.0e-300, limit::max(), limit::min(),
	                          limit::min() / 3.0,  // a subnormal double
	                          limit::denorm_min(), std::ldexp(1.0, 1000), std::ldexp(1.0, -1000)};
	const std::size_t n = xs.size();
	for (std::size_t i = 0; i < n; ++i) xs.push_back(-xs[i]);
	return xs;
}

// the uint64_t-block cfloat has the encoding of the uint32_t- and uint8_t-block ones, and, when the
// cfloat holds v exactly, converts back to v in v's own type
template<unsigned nbits, unsigned es, typename Real>
int VerifyValue(Real v, bool exact, bool reportTestCases) {
	cfloat<nbits, es, std::uint64_t, true, false, false> wide;
	cfloat<nbits, es, std::uint32_t, true, false, false> mid;
	cfloat<nbits, es, std::uint8_t, true, false, false>  narrow;
	wide   = v;
	mid    = v;
	narrow = v;
	const bool sameEncoding = (to_binary(wide) == to_binary(mid)) && (to_binary(mid) == to_binary(narrow));
	const bool sameValue    = !exact || (static_cast<Real>(wide) == v);
	if (sameEncoding && sameValue) return 0;
	if (reportTestCases)
		std::cerr << "FAIL: cfloat<" << nbits << ',' << es << ",uint64_t> = " << std::hexfloat << static_cast<double>(v)
		          << std::defaultfloat << " gave " << to_binary(wide) << "\n      uint32_t blocks give " << to_binary(mid)
		          << '\n';
	return 1;
}

// random significands at random exponents over the double's whole range, subnormals included
inline std::vector<double> RandomSamples(std::size_t n) {
	std::vector<double> xs;
	xs.reserve(n);
	std::uint64_t state = 1918;
	auto next = [&state]() {  // splitmix64: reproducible on every platform
		std::uint64_t z = (state += 0x9E3779B97F4A7C15ull);
		z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
		z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
		return z ^ (z >> 31);
	};
	while (xs.size() < n) {
		const std::uint64_t bits = next();
		double d{0.0};
		static_assert(sizeof(d) == sizeof(bits), "double must be 64 bits");
		std::memcpy(&d, &bits, sizeof(d));
		if (std::isfinite(d) && d != 0.0) xs.push_back(d);
	}
	return xs;
}

// a deep subnormal of cfloat<80,8> whose rounding carries across a binade
inline int VerifyRoundingCarry(bool reportTestCases) {
	const double x        = std::ldexp(9007199254740991.0, -200);  // (2^53 - 1) * 2^-200
	const double expected = std::ldexp(1.0, -147);                   // the cfloat's grid is 2^-197
	int fails = 0;
	for (double v : {x, -x}) {
		fails += VerifyValue<80, 8>(v, false, reportTestCases);
		cfloat<80, 8, std::uint64_t, true, false, false> c;
		c = v;
		if (static_cast<double>(c) != (v < 0 ? -expected : expected)) {
			if (reportTestCases)
				std::cerr << "FAIL: cfloat<80,8,uint64_t> = " << std::hexfloat << v << " gave " << static_cast<double>(c)
				          << std::defaultfloat << " : " << to_binary(c) << '\n';
			++fails;
		}
	}
	return fails;
}

template<unsigned nbits, unsigned es>
int VerifyConfiguration(bool reportTestCases) {
	int fails = 0;
	for (double d : RandomSamples(2000)) fails += VerifyValue<nbits, es>(d, (es >= 11), reportTestCases);
	for (double d : Samples()) {  // every double is exact when es >= 11, every float in all of them
		fails += VerifyValue<nbits, es>(d, (es >= 11), reportTestCases);
		const float f = static_cast<float>(d);
		if (std::isfinite(f) && f != 0.0f) fails += VerifyValue<nbits, es>(f, true, reportTestCases);
	}
#if LONG_DOUBLE_SUPPORT
	// long double reaches these cfloats through convert(blocktriple), whose blocktriple takes the
	// cfloat's block type. Exact when the cfloat holds every significand bit and the range.
	using ld                  = std::numeric_limits<long double>;
	constexpr unsigned fbits  = nbits - 1u - es;
	const bool         wideEnough = unsigned(ld::digits) <= fbits + 1u;
	std::vector<long double> lds = {1.0l / 3.0l, 0.1l, 1.0l + std::ldexp(1.0l, 2 - ld::digits)};
	if constexpr (es >= 15) {  // the long double's whole range, x87 and binary128 alike
		lds.push_back(ld::min());
		lds.push_back(ld::min() / 3.0l);
		lds.push_back(ld::denorm_min());
		lds.push_back(ld::max());
	}
	for (long double x : lds) {
		for (long double y : {x, -x}) fails += VerifyValue<nbits, es>(y, wideEnough, reportTestCases);
	}
#endif
	return fails;
}

}  // namespace block_type_conversion

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
	using namespace sw::universal::block_type_conversion;

	std::string test_suite          = "cfloat native conversion across block types";
	std::string test_tag            = "uint64_t blocks";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	cfloat<128, 15, std::uint64_t, true, false, false> a;
	a = 1.5;
	std::cout << "cfloat<128,15,uint64_t> = 1.5 : " << to_binary(a) << " : " << a << '\n';

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;  // ignore failures
#else

#	if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyConfiguration<65, 11>(reportTestCases), "cfloat<65,11>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConfiguration<80, 8>(reportTestCases), "cfloat<80,8>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyRoundingCarry(reportTestCases), "cfloat<80,8> rounding carry", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConfiguration<80, 11>(reportTestCases), "cfloat<80,11>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConfiguration<96, 11>(reportTestCases), "cfloat<96,11>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConfiguration<128, 11>(reportTestCases), "cfloat<128,11>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConfiguration<128, 15>(reportTestCases), "cfloat<128,15>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConfiguration<129, 11>(reportTestCases), "cfloat<129,11>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConfiguration<256, 19>(reportTestCases), "cfloat<256,19>", test_tag);
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
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
} catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
