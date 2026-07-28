// test_generic_fma_kernel.cpp: generic ADL fma-accumulation kernel over natives + Universal types
//
// Part of the universal fma epic #1189 (#1199). A single generic kernel
//   template<typename Real> Real dot_fma(x, y) { using std::fma; acc = fma(x[i], y[i], acc); ... }
// must instantiate and run for the native floating-point types (via std::fma) and for
// every Universal number type that provides a free fma -- the ADL two-step
// `using std::fma; fma(a,b,c)` resolving uniformly is what lets a mixed-precision kernel
// be written once and run over any arithmetic type. This suite exercises exactly that.
//
// Scope note (#1204): Universal is now number-systems-only; the general BLAS was
// extracted to MTL5 / mp-blas. So the *library* generic-dot fma-accumulation path lives
// there (selected via the has_fma<T> trait from #1198), not in Universal. This file is the
// generic instantiation TEST that keeps the per-type fma overloads ADL-reachable + consistent.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include <universal/number/posit/posit.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/integer/integer.hpp>
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/number/bfloat16/bfloat16.hpp>
#include <universal/number/dd/dd.hpp>
#include <universal/number/qd/qd.hpp>
#include <universal/number/areal/areal.hpp>
#include <universal/number/takum/takum.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

	using namespace sw::universal;

	// the generic mixed-precision kernel: dot product via the ADL fma two-step
	template<typename Real>
	Real dot_fma(const std::vector<Real>& x, const std::vector<Real>& y) {
		using std::fma;   // std::fma for natives; sw::universal::fma reached by ADL for Universal types
		Real acc(0);
		for (std::size_t i = 0; i < x.size(); ++i) acc = fma(x[i], y[i], acc);
		return acc;
	}

	// instantiate the kernel for Real and check the exact integer dot product.
	// x = {1,2,3,4}, y = {5,6,7,8} -> 5 + 12 + 21 + 32 = 70, exactly representable in every type here.
	template<typename Real>
	int VerifyDotFma(const char* tag, bool reportTestCases) {
		std::vector<Real> x = { Real(1), Real(2), Real(3), Real(4) };
		std::vector<Real> y = { Real(5), Real(6), Real(7), Real(8) };
		Real r = dot_fma(x, y);
		if (double(r) != 70.0) {
			if (reportTestCases) std::cout << "    FAIL dot_fma<" << tag << "> = " << double(r) << " expected 70\n";
			return 1;
		}
		return 0;
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
	std::string test_suite = "generic ADL fma-accumulation kernel over natives + Universal types (#1199)";
	int nrOfFailedTestCases = 0;
	bool reportTestCases = true;
	ReportTestSuiteHeader(test_suite, reportTestCases);

	int fails = 0;
	// native floating-point (std::fma)
	fails += VerifyDotFma<float>("float", reportTestCases);
	fails += VerifyDotFma<double>("double", reportTestCases);
	fails += VerifyDotFma<long double>("long double", reportTestCases);
	// Universal number types with a type-preserving free fma (ADL)
	fails += VerifyDotFma<posit<32, 2>>("posit<32,2>", reportTestCases);
	fails += VerifyDotFma<cfloat<32, 8, std::uint32_t, true, false, false>>("cfloat<32,8>", reportTestCases);
	fails += VerifyDotFma<integer<32, std::uint32_t>>("integer<32>", reportTestCases);
	fails += VerifyDotFma<fixpnt<32, 16>>("fixpnt<32,16>", reportTestCases);
	fails += VerifyDotFma<bfloat16>("bfloat16", reportTestCases);
	fails += VerifyDotFma<dd>("dd", reportTestCases);
	fails += VerifyDotFma<qd>("qd", reportTestCases);
	fails += VerifyDotFma<areal<20, 6>>("areal<20,6>", reportTestCases);
	fails += VerifyDotFma<takum<32, 5>>("takum<32,5>", reportTestCases);

	nrOfFailedTestCases += ReportTestResult(fails, "generic ADL dot_fma across all types", "fma");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (const std::exception& err) {
	std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
