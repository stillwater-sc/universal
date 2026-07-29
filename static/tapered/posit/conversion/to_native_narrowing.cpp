// to_native_narrowing.cpp: regression test for posit::to_native<Real> narrowing
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// posit::to_native<Real>() used to assign each decoded factor to Real before
// forming the product:
//
//     Real s = (_sign ? -1.0l : 1.0l);
//     Real r = _regime.value();          // long double -> Real, early
//     Real e = _exponent.value();        // long double -> Real, early
//     Real f = (1.0l + _fraction.value());
//     return s * r * e * f;
//
// The component value()s all return long double, so those are four implicit
// narrowing conversions (MSVC C4244 at every instantiation site). More
// importantly they are LOSSY IN RANGE, not just in precision: for configurations
// with a large es the regime factor alone can under/overflow Real even when the
// full product is perfectly representable. posit<32,5> and posit<64,5> converted
// to float hit exactly that -- the regime underflowed float to 0, so the whole
// product came out 0 and representable subnormals were silently flushed away.
//
// Ground truth here is the same posit converted to double. double has ample
// exponent range for these values, so its result is exact, and the correct float
// result is therefore static_cast<float>(that double). Any Real whose conversion
// disagrees with that is wrong.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <iostream>
#include <universal/number/posit/posit.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

// Verify to_native<Real> against the double conversion of the same encoding.
// Returns the number of mismatches.
template<typename PositType, typename Real>
int VerifyNativeNarrowing(bool reportTestCases, unsigned long long maxCases = 65536ull) {
	constexpr unsigned nbits = PositType::nbits;
	int nrOfFailures = 0;

	const unsigned long long total = (nbits < 32u) ? (1ull << nbits) : maxCases;
	for (unsigned long long i = 0; i < total; ++i) {
		unsigned long long encoding = i;
		if (nbits >= 32u) {   // sample the space instead of enumerating it
			unsigned long long z = (i + 1ull) * 0x9E3779B97F4A7C15ull;
			z ^= z >> 30; z *= 0xBF58476D1CE4E5B9ull;
			z ^= z >> 27; z *= 0x94D049BB133111EBull; z ^= z >> 31;
			encoding = z;
		}

		PositType p;
		p.setbits(encoding);
		if (p.isnar()) continue;

		const double reference = double(p);
		if (!std::isfinite(reference)) continue;      // outside double's range: no ground truth

		const Real expected = static_cast<Real>(reference);
		const Real observed = Real(p);

		// Bit comparison, so a flushed-to-zero subnormal cannot hide behind a
		// tolerance and -0 vs +0 is caught.
		if (!(observed == expected) || (std::signbit(observed) != std::signbit(expected))) {
			++nrOfFailures;
			if (reportTestCases) {
				std::cerr << "FAIL " << typeid(PositType).name()
				          << " encoding 0x" << std::hex << encoding << std::dec
				          << ": double=" << reference
				          << " expected=" << expected
				          << " observed=" << observed << '\n';
			}
		}
	}
	return nrOfFailures;
}

} // namespace

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "posit to_native<Real> narrowing validation";
	std::string test_tag    = "to_native narrowing";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// The regression: large-es configurations whose regime alone underflows float.
	// These fail on the pre-fix implementation (446 of 448 sampled encodings came
	// back as +/-0 instead of the correct float subnormal).
	nrOfFailedTestCases += ReportTestResult(
		(VerifyNativeNarrowing<posit<32, 5>, float>(reportTestCases)), "posit<32,5> -> float", test_tag);
	nrOfFailedTestCases += ReportTestResult(
		(VerifyNativeNarrowing<posit<64, 5>, float>(reportTestCases)), "posit<64,5> -> float", test_tag);

	// Configurations that were already correct, so the fix cannot regress them.
	nrOfFailedTestCases += ReportTestResult(
		(VerifyNativeNarrowing<posit<8, 2>, float>(reportTestCases)), "posit<8,2> -> float", test_tag);
	nrOfFailedTestCases += ReportTestResult(
		(VerifyNativeNarrowing<posit<16, 2>, float>(reportTestCases)), "posit<16,2> -> float", test_tag);
	nrOfFailedTestCases += ReportTestResult(
		(VerifyNativeNarrowing<posit<16, 3>, float>(reportTestCases)), "posit<16,3> -> float", test_tag);
	nrOfFailedTestCases += ReportTestResult(
		(VerifyNativeNarrowing<posit<32, 2>, float>(reportTestCases)), "posit<32,2> -> float", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_internal_exception& err) {
	std::cerr << "Uncaught posit internal exception: " << err.what() << std::endl;
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
