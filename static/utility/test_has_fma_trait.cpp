// test_has_fma_trait.cpp: tests for the has_fma capability trait + FusedMultiplyAddable concept
//
// has_fma<T> (include/sw/universal/traits/fma_traits.hpp) detects, by SFINAE, whether a
// type provides a type-preserving fused multiply-add reachable by the ADL two-step
// `using std::fma; fma(a,b,c)`. It resolves true for the native floating-point types and
// for every Universal number type that defines a free fma, and false otherwise -- in
// particular it rejects a merely double-convertible type (whose only fma would be the
// lossy std::fma-via-conversion). This suite pins that contract with static_asserts and
// exercises the FusedMultiplyAddable concept on a sample kernel.
//
// Sub-issue of #1189 (universal fma). Relates to #1198.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <iostream>
#include <string>

#include <universal/number/posit/posit.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/integer/integer.hpp>
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/number/bfloat16/bfloat16.hpp>
#include <universal/number/dd/dd.hpp>
#include <universal/number/qd/qd.hpp>
#include <universal/number/areal/areal.hpp>
#include <universal/number/takum/takum.hpp>
#include <universal/traits/fma_traits.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

	using namespace sw::universal;

	// a merely double-convertible type with NO type-preserving fma: has_fma must be false
	struct DoubleConvertibleNoFma {
		double v;
		operator double() const { return v; }
	};

	// ---- compile-time contract -------------------------------------------------
	// true: native floating-point (std::fma)
	static_assert(has_fma_v<float>,       "float has std::fma");
	static_assert(has_fma_v<double>,      "double has std::fma");
	static_assert(has_fma_v<long double>, "long double has std::fma");

	// true: Universal number types with a type-preserving free fma
	static_assert(has_fma_v<posit<32, 2>>,                                   "posit fma");
	static_assert(has_fma_v<cfloat<32, 8, std::uint32_t, true, false, false>>, "cfloat fma");
	static_assert(has_fma_v<integer<32, std::uint32_t>>,                     "integer fma");
	static_assert(has_fma_v<fixpnt<16, 8>>,                                  "fixpnt fma");
	static_assert(has_fma_v<bfloat16>,                                       "bfloat16 fma");
	static_assert(has_fma_v<dd>,                                             "dd fma");
	static_assert(has_fma_v<qd>,                                             "qd fma");
	static_assert(has_fma_v<areal<16, 5>>,                                   "areal fma");
	static_assert(has_fma_v<takum<32, 5>>,                                   "takum fma");

	// false: no type-preserving fma
	static_assert(!has_fma_v<DoubleConvertibleNoFma>, "double-convertible type has no type-preserving fma");
	static_assert(!has_fma_v<int>,                    "int has no fma");

#if defined(__cpp_concepts) && __cpp_concepts >= 201907L
	// FusedMultiplyAddable concept constrains a sample single-rounding kernel: a*x + b
	template<FusedMultiplyAddable Real>
	Real affine_fma(const Real& a, const Real& x, const Real& b) {
		using std::fma;
		return fma(a, x, b);
	}
	static_assert(FusedMultiplyAddable<double>,       "double models FusedMultiplyAddable");
	static_assert(FusedMultiplyAddable<posit<32, 2>>, "posit models FusedMultiplyAddable");
	static_assert(!FusedMultiplyAddable<DoubleConvertibleNoFma>, "double-convertible type does not model it");
#endif

	// ---- runtime sanity (mirrors the static contract for test-suite reporting) --
	template<typename T>
	int expect(bool actual, bool wanted, const char* tag, bool reportTestCases) {
		if (actual != wanted) {
			if (reportTestCases) std::cout << "    FAIL has_fma_v<" << tag << "> = " << actual << " expected " << wanted << '\n';
			return 1;
		}
		return 0;
	}

	int VerifyHasFma(bool reportTestCases) {
		int fails = 0;
		fails += expect<double>(has_fma_v<double>, true, "double", reportTestCases);
		fails += expect<posit<32, 2>>(has_fma_v<posit<32, 2>>, true, "posit<32,2>", reportTestCases);
		fails += expect<takum<32, 5>>(has_fma_v<takum<32, 5>>, true, "takum<32,5>", reportTestCases);
		fails += expect<DoubleConvertibleNoFma>(has_fma_v<DoubleConvertibleNoFma>, false,
			"DoubleConvertibleNoFma", reportTestCases);
#if defined(__cpp_concepts) && __cpp_concepts >= 201907L
		// the concept-constrained kernel computes a*x + b with a single rounding
		if (double(affine_fma(posit<32, 2>(2.0), posit<32, 2>(3.0), posit<32, 2>(1.0))) != 7.0) {
			++fails; if (reportTestCases) std::cout << "    FAIL affine_fma(posit) != 7\n";
		}
		if (affine_fma(2.0, 3.0, 1.0) != 7.0) {
			++fails; if (reportTestCases) std::cout << "    FAIL affine_fma(double) != 7\n";
		}
#endif
		return fails;
	}

}  // anonymous namespace

#define MANUAL_TESTING 0
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
	std::string test_suite = "has_fma capability trait + FusedMultiplyAddable concept (#1198)";
	int nrOfFailedTestCases = 0;
	bool reportTestCases = true;
	ReportTestSuiteHeader(test_suite, reportTestCases);

	// the core contract is enforced at compile time by the static_asserts above;
	// this runtime pass mirrors it for the regression harness.
	nrOfFailedTestCases += ReportTestResult(VerifyHasFma(reportTestCases), "has_fma trait + concept", "has_fma");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (const std::exception& err) {
	std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
