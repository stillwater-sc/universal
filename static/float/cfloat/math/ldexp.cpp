// ldexp.cpp: test suite runner for ldexp and frexp specialized for classic floats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// ldexp(x, n) is x * 2^n rounded once. It used to rewrite the exponent field of x, which
// is right only when x and the result are both normal: every subnormal input came back
// wrong (ldexp(2^-24, 24) on half gave 1.00098, not 1), a result below the normal range
// was not denormalized, and an overflow was not rounded or saturated (#1396). The oracle
// is IEEE double, which holds every value these types can take, and every scaled one,
// exactly: the only rounding is the one into the cfloat.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

// the value ldexp(x, n) must produce: x * 2^n in double, then one rounding into the cfloat,
// saturating to maxpos/maxneg when the type saturates
template<typename Cfloat>
Cfloat ldexp_reference(const Cfloat& x, int n) {
	Cfloat ref{};
	if (x.isnan()) { ref.setnan(NAN_TYPE_QUIET); return ref; }
	if (x.isinf()) { ref.setinf(x.sign()); return ref; }
	const double scaled = std::ldexp(double(x), n);
	if constexpr (Cfloat::isSaturating) {
		if (!ref.inrange(scaled)) {
			if (scaled > 0) ref.maxpos(); else ref.maxneg();
			return ref;
		}
	}
	ref = scaled;
	return ref;
}

// every encoding, scaled by every power of two that can move it across the whole range
// in either direction, one encoding in every `stride`
template<typename Cfloat>
int VerifyLdexp(bool reportTestCases, unsigned stride = 1) {
	constexpr unsigned nbits = Cfloat::nbits;
	constexpr size_t NR_ENCODINGS = (size_t(1) << nbits);
	// from the smallest subnormal to past the overflow threshold: MAX_EXP - MIN_EXP_SUBNORMAL
	constexpr int span = Cfloat::MAX_EXP - Cfloat::MIN_EXP_SUBNORMAL + 2;
	int nrOfFailedTests = 0;
	Cfloat x{};
	for (size_t i = 0; i < NR_ENCODINGS; i += stride) {
		x.setbits(i);
		for (int n = -span; n <= span; ++n) {
			const Cfloat result = ldexp(x, n);
			const Cfloat ref    = ldexp_reference(x, n);
			if (result.isnan() && ref.isnan()) continue;
			if (result.iszero() && ref.iszero()) continue;   // double's -0 is not every cfloat's
			if (result == ref && result.sign() == ref.sign()) continue;
			++nrOfFailedTests;
			if (reportTestCases && nrOfFailedTests < 25) {
				std::cout << "FAIL ldexp(" << to_binary(x) << " = " << double(x) << ", " << n << ") = "
				          << to_binary(result) << " = " << double(result) << ", expected "
				          << to_binary(ref) << " = " << double(ref) << '\n';
			}
		}
	}
	return nrOfFailedTests;
}

// frexp(x, &e) against the VALUE of x, not against ldexp: fraction * 2^e must be x. The
// round trip through ldexp cannot catch a frexp defect that ldexp mirrors, and the two
// used to mirror each other's subnormal error exactly.
template<typename Cfloat>
int VerifyFrexp(bool reportTestCases, unsigned stride = 1) {
	constexpr unsigned nbits = Cfloat::nbits;
	constexpr size_t NR_ENCODINGS = (size_t(1) << nbits);
	constexpr bool inHalfOpenUnit = (std::numeric_limits<Cfloat>::min_exponent <= 0);   // see frexp
	int nrOfFailedTests = 0;
	Cfloat x{};
	for (size_t i = 0; i < NR_ENCODINGS; i += stride) {
		x.setbits(i);
		if (x.isnan() || x.isinf() || x.iszero()) continue;
		int e = 0;
		const Cfloat fraction = frexp(x, &e);
		const double f = double(fraction);
		const bool exact = (std::ldexp(f, e) == double(x));
		const bool range = inHalfOpenUnit ? (std::abs(f) >= 0.5 && std::abs(f) < 1.0)
		                                  : (std::abs(f) >= 1.0 && std::abs(f) < 2.0);
		if (exact && range) continue;
		++nrOfFailedTests;
		if (reportTestCases && nrOfFailedTests < 25) {
			std::cout << "FAIL frexp(" << to_binary(x) << " = " << double(x) << ") = " << f << " * 2^" << e
			          << " = " << std::ldexp(f, e) << '\n';
		}
	}
	return nrOfFailedTests;
}

// the encoding configurations of one (nbits, es)
template<unsigned nbits, unsigned es>
int VerifyLdexpAllConfigurations(bool reportTestCases) {
	int fails = 0;
	fails += VerifyLdexp< cfloat<nbits, es, uint8_t, false, false, false> >(reportTestCases);
	fails += VerifyLdexp< cfloat<nbits, es, uint8_t, true,  false, false> >(reportTestCases);
	fails += VerifyLdexp< cfloat<nbits, es, uint8_t, false, true,  false> >(reportTestCases);
	fails += VerifyLdexp< cfloat<nbits, es, uint8_t, true,  true,  false> >(reportTestCases);
	fails += VerifyLdexp< cfloat<nbits, es, uint8_t, false, false, true > >(reportTestCases);
	fails += VerifyLdexp< cfloat<nbits, es, uint8_t, true,  false, true > >(reportTestCases);
	fails += VerifyFrexp< cfloat<nbits, es, uint8_t, false, false, false> >(reportTestCases);
	fails += VerifyFrexp< cfloat<nbits, es, uint8_t, true,  false, false> >(reportTestCases);
	fails += VerifyFrexp< cfloat<nbits, es, uint8_t, true,  true,  false> >(reportTestCases);
	// saturating with max-exponent values is left out: its maxpos is the bit pattern of
	// inf, which no reference can compare against meaningfully
	return fails;
}

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
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "cfloat<> mathlib ldexp and frexp validation";
	std::string test_tag    = "ldexp";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	{
		half x; x.setbits(0x0001);   // 2^-24, the smallest subnormal
		std::cout << "ldexp(" << x << ", 24) = " << ldexp(x, 24) << " (expected 1)\n";
	}
	nrOfFailedTestCases += ReportTestResult(VerifyLdexp< cfloat<8, 3, uint8_t, true, false, false> >(true), "cfloat<8,3,t,f,f>", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyLdexpAllConfigurations<8, 2>(reportTestCases), "cfloat<8,2> all configurations", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLdexpAllConfigurations<8, 3>(reportTestCases), "cfloat<8,3> all configurations", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLdexpAllConfigurations<8, 4>(reportTestCases), "cfloat<8,4> all configurations", test_tag);
	// half: one encoding in 61, every exponent
	nrOfFailedTestCases += ReportTestResult(VerifyLdexp<half>(reportTestCases, 61), type_tag(half()), test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyFrexp<half>(reportTestCases), type_tag(half()), "frexp");
	nrOfFailedTestCases += ReportTestResult(VerifyFrexp<bfloat_t>(reportTestCases), type_tag(bfloat_t()), "frexp");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyLdexpAllConfigurations<8, 5>(reportTestCases), "cfloat<8,5> all configurations", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLdexp<half>(reportTestCases), type_tag(half()), test_tag);
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyLdexp<bfloat_t>(reportTestCases, 7), type_tag(bfloat_t()), test_tag);
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
