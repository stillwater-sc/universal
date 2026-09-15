// block_type_conversion.cpp: native floats into wide cfloats of every block type
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <cstdint>
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
   and its uint64_t branch was empty. It now sets the bits one at a time there.

   Checked for cfloat<80,11>, <96,11>, <128,15> and <256,19>, whose ranges and precisions hold every
   double exactly, over normal, subnormal and extreme doubles and floats, both signs: the
   uint64_t-block cfloat has the same encoding as the uint32_t- and uint8_t-block ones, and converts
   back to the value it was given. For long double, see static/conversions/long_double_conversion.cpp.
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

template<unsigned nbits, unsigned es, typename Real>
int VerifyValue(Real v, bool reportTestCases) {
	cfloat<nbits, es, std::uint64_t, true, false, false> wide;
	cfloat<nbits, es, std::uint32_t, true, false, false> mid;
	cfloat<nbits, es, std::uint8_t, true, false, false>  narrow;
	wide   = v;
	mid    = v;
	narrow = v;
	const bool sameEncoding = (to_binary(wide) == to_binary(mid)) && (to_binary(mid) == to_binary(narrow));
	const bool sameValue    = (static_cast<double>(wide) == static_cast<double>(v));  // every double is exact here
	if (sameEncoding && sameValue) return 0;
	if (reportTestCases)
		std::cerr << "FAIL: cfloat<" << nbits << ',' << es << ",uint64_t> = " << std::hexfloat << static_cast<double>(v)
		          << std::defaultfloat << " gave " << to_binary(wide) << "\n      uint32_t blocks give " << to_binary(mid)
		          << '\n';
	return 1;
}

template<unsigned nbits, unsigned es>
int VerifyConfiguration(bool reportTestCases) {
	int fails = 0;
	for (double d : Samples()) {
		fails += VerifyValue<nbits, es>(d, reportTestCases);
		const float f = static_cast<float>(d);
		if (std::isfinite(f) && f != 0.0f) fails += VerifyValue<nbits, es>(f, reportTestCases);
	}
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
	nrOfFailedTestCases += ReportTestResult(VerifyConfiguration<80, 11>(reportTestCases), "cfloat<80,11>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConfiguration<96, 11>(reportTestCases), "cfloat<96,11>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConfiguration<128, 15>(reportTestCases), "cfloat<128,15>", test_tag);
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
