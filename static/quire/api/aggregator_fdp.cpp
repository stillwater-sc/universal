// aggregator_fdp.cpp: the number-system aggregator header must expose quire_mul / fdp
//
// #1201: quire_mul (the entry point for exact fused-dot / super-accumulator use) lives in
// each type's fdp.hpp. posit.hpp includes posit/fdp.hpp, but cfloat.hpp and lns.hpp did
// not include theirs, so `#include <.../cfloat.hpp>` alone left quire_mul(cfloat, cfloat)
// undeclared -- a discoverability gap that forced callers (e.g. the mp-blas accumulator
// study) to add fdp.hpp by hand. This regression pins the fix: it includes ONLY the
// aggregator headers (deliberately NOT the fdp.hpp headers) and exercises quire_mul + fdp
// for posit, cfloat, and lns. If any aggregator stops pulling in its fdp support, this
// fails to COMPILE.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

// aggregator headers ONLY -- NO explicit fdp.hpp includes (that is exactly what #1201 fixes)
#include <universal/number/posit/posit.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/lns/lns.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

	using namespace sw::universal;

	// quire_mul and fdp must both be reachable from the aggregator header alone.
	// Power-of-2 inputs keep the exact product-sum exactly representable in posit, cfloat,
	// and lns (log-domain), so the resolved dot product is exactly 8 for every type.
	template<typename Scalar>
	int VerifyAggregatorFdp(const char* tag, bool reportTestCases) {
		// quire_mul: unrounded full-precision product accumulated into a quire
		Scalar a(2.0), b(4.0);
		quire<Scalar> q;
		q += quire_mul(a, b);   // exact product 8 -- compiles only if quire_mul is visible

		// fdp: resolved fused dot product; 2*2 + 2*2 = 8, exact across posit/cfloat/lns
		std::vector<Scalar> x = { Scalar(2.0), Scalar(2.0) };
		std::vector<Scalar> y = { Scalar(2.0), Scalar(2.0) };
		Scalar d = fdp(x, y);
		if (double(d) != 8.0) {
			if (reportTestCases) std::cout << "    FAIL fdp<" << tag << "> = " << double(d) << " expected 8\n";
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
	std::string test_suite = "aggregator header exposes quire_mul / fdp (#1201)";
	int nrOfFailedTestCases = 0;
	bool reportTestCases = true;
	ReportTestSuiteHeader(test_suite, reportTestCases);

	int fails = 0;
	fails += VerifyAggregatorFdp<posit<32, 2>>("posit<32,2>", reportTestCases);
	fails += VerifyAggregatorFdp<cfloat<32, 8, std::uint32_t, true, false, false>>("cfloat<32,8>", reportTestCases);
	fails += VerifyAggregatorFdp<lns<16, 8>>("lns<16,8>", reportTestCases);

	nrOfFailedTestCases += ReportTestResult(fails, "quire_mul / fdp reachable from aggregator header", "fdp");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (const std::exception& err) {
	std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
