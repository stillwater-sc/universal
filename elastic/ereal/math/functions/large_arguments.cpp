// large_arguments.cpp: sin, cos and tan of ereal at huge arguments (#2016)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <algorithm>
#include <cmath>
#include <limits>
#include <string>
#include <string_view>
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/ereal_precision.hpp>
#include <math/constants/reference_constants.hpp>

// sin, cos and tan used to reduce x by subtracting n 2 pi with the configuration's own pi,
// which loses about log2|x| bits: ereal<4> agreed with the truth to 107 bits at 1e100 and to
// none at 1e200, where sin(1e200) came out -0.3737 for -0.6440. They reduce by Payne and
// Hanek's method now (math/functions/payne_hanek.hpp), with as many bits of 1/(2 pi) as the
// exponent needs.
//
// Every check is exact (ereal_precision.hpp): the result is compared with a stored decimal
// reference (reference_constants.hpp, mpmath at 40000 bits) as a dyadic rational, never
// through a double, and must agree to the configuration's bits less kMargin. Measured at
// ereal<4>, <8> and <4, float>: 275-511 and 99-119 bits, every one above the configuration.

namespace {
	using namespace sw::universal;

	constexpr int kMargin = 16;

	int expect_bits(int got, int wanted, const std::string& what, bool reportTestCases) {
#ifdef EREAL_PRECISION_TRACE
		std::cout << "    " << what << ": " << got << " bits (wanted " << wanted << ")\n";
#endif
		if (got >= wanted) return 0;
		if (reportTestCases) std::cout << "    FAIL " << what << ": " << got << " bits, wanted at least " << wanted << '\n';
		return 1;
	}

	// bits to which v agrees with a signed decimal reference; -1 for a value that cannot agree
	template<typename Real>
	int bits_vs(const Real& v, std::string_view ref) {
		if (v.isnan() || v.isinf() || v.iszero()) return -1;
		Real w = v;
		if (!ref.empty() && ref.front() == '-') { ref.remove_prefix(1); w = -v; }
		if (w.isneg()) return -1;                     // the wrong sign
		const int cap = std::min(320, static_cast<int>(ereal_bits_of<Real>::value * 0.30103) + 4);
		return agreed_bits(w, ref, cap);
	}

	struct Case {
		const char*      tag;
		double           x;
		std::string_view sin_ref, cos_ref, tan_ref;
	};

	template<typename Real>
	int VerifyCases(const Case* cases, std::size_t n, bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		const int wanted = ereal_bits_of<Real>::value - kMargin;
		for (std::size_t i = 0; i < n; ++i) {
			const Case& c = cases[i];
			const Real x(c.x);
			const std::string at = std::string("(") + c.tag + ")";
			nrOfFailedTestCases += expect_bits(bits_vs(sin(x), c.sin_ref), wanted, "sin" + at, reportTestCases);
			nrOfFailedTestCases += expect_bits(bits_vs(cos(x), c.cos_ref), wanted, "cos" + at, reportTestCases);
			nrOfFailedTestCases += expect_bits(bits_vs(tan(x), c.tan_ref), wanted, "tan" + at, reportTestCases);
			// odd and even: the reduction works on |x|
			if (!(sin(-x) == -sin(x)) || !(cos(-x) == cos(x))) {
				if (reportTestCases) std::cout << "    FAIL sin/cos symmetry at -" << c.tag << '\n';
				++nrOfFailedTestCases;
			}
		}
		return nrOfFailedTestCases;
	}

	// double limbs: up to 1e300
	template<typename Real>
	int VerifyDoubleLimbs(bool reportTestCases) {
		static const Case cases[] = {
			{ "1e20",  1e20,  s_sin_1e20,  s_cos_1e20,  s_tan_1e20 },
			{ "1e30",  1e30,  s_sin_1e30,  s_cos_1e30,  s_tan_1e30 },
			{ "1e100", 1e100, s_sin_1e100, s_cos_1e100, s_tan_1e100 },
			{ "1e200", 1e200, s_sin_1e200, s_cos_1e200, s_tan_1e200 },
			{ "1e300", 1e300, s_sin_1e300, s_cos_1e300, s_tan_1e300 },
		};
		return VerifyCases<Real>(cases, sizeof(cases) / sizeof(cases[0]), reportTestCases);
	}

	// float limbs reach 3.4e38
	template<typename Real>
	int VerifyFloatLimbs(bool reportTestCases) {
		static const Case cases[] = {
			{ "1e20", 1e20, s_sin_1e20, s_cos_1e20, s_tan_1e20 },
			{ "1e30", 1e30, s_sin_1e30, s_cos_1e30, s_tan_1e30 },
			{ "3e38", 3e38, s_sin_3e38, s_cos_3e38, s_tan_3e38 },
		};
		return VerifyCases<Real>(cases, sizeof(cases) / sizeof(cases[0]), reportTestCases);
	}

#if LONG_DOUBLE_SUPPORT
	// an 80-bit or 128-bit long double reaches 2^16383: the reduction needs 17000 bits of
	// 1/(2 pi) there. IBM double-double stops at double's exponent and is skipped.
	int VerifyLongDoubleLimbs(bool reportTestCases) {
		if constexpr (std::numeric_limits<long double>::max_exponent < 16000) {
			return 0;
		}
		else {
			using Real = ereal<4, long double>;
			int nrOfFailedTestCases = 0;
			const int wanted = ereal_bits_of<Real>::value - kMargin;
			const Real x = ldexp(Real(5726623061.0L), 15968);
			nrOfFailedTestCases += expect_bits(bits_vs(sin(x), s_sin_2p16000), wanted, "sin(5726623061 * 2^15968)", reportTestCases);
			nrOfFailedTestCases += expect_bits(bits_vs(cos(x), s_cos_2p16000), wanted, "cos(5726623061 * 2^15968)", reportTestCases);
			return nrOfFailedTestCases;
		}
	}
#endif

}  // namespace

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
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

	std::string test_suite  = "ereal sin/cos/tan at large arguments (#2016)";
	std::string test_tag    = "large arguments";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyDoubleLimbs<ereal<4>>(reportTestCases), "ereal<4>", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyDoubleLimbs<ereal<4>>(reportTestCases), "ereal<4>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyDoubleLimbs<ereal<8>>(reportTestCases), "ereal<8>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyFloatLimbs<ereal<4, float>>(reportTestCases), "ereal<4, float>", test_tag);
#if LONG_DOUBLE_SUPPORT
	nrOfFailedTestCases += ReportTestResult(VerifyLongDoubleLimbs(reportTestCases), "ereal<4, long double>", test_tag);
#endif
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyDoubleLimbs<ereal<16>>(reportTestCases), "ereal<16>", test_tag);
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyDoubleLimbs<ereal<19>>(reportTestCases), "ereal<19>", test_tag);
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
