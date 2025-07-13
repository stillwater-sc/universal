// scaling.cpp: error measurement of data scaling to fit small and narrow representations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

#include <universal/number/integer/integer.hpp>
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/number/lns/lns.hpp>

#include <universal/blas/blas.hpp>
#include <universal/blas/scaling.hpp>
#include <universal/verification/test_suite.hpp>

/*
 * When we want to take arbitrary vectors and want to faithfully calculate a 
 * dot product using lower precision types, we need to 'squeeze' the values
 * of the original vector such that the computational dynamics of the dot product
 * can be emulated. 
 * 
 * When you think about very constrained types like 8-bit floating-point formats
 * the risk of overflow and underflow of the products is the first problem
 * to solve. Secondly, for long vectors overflow and catastrophic cancellation
 * are also risks.
 *                 
 */

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
	using namespace sw::universal::blas;

	std::string test_suite  = "benchmark error in scaling operations";
	std::string test_tag    = "data distribution scaling";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);
	unsigned N{ 10000 };
	double mean{ 0.0 }, stddev{ 1.0 };

	using SrcType = double;
	auto dv = sw::universal::blas::gaussian_random_vector<SrcType>(N, mean, stddev);
		auto dminmax = blas::range(dv);
		std::cout << dminmax.first << ", " << dminmax.second << '\n';
	auto sv = compress<SrcType, float>(dv);
		auto sminmax = blas::range(sv);
		std::cout << sminmax.first << ", " << sminmax.second << '\n';
	auto hv = compress<SrcType, half>(dv);
		auto hminmax = blas::range(hv);
		std::cout << hminmax.first << ", " << hminmax.second << '\n';
	auto qv = compress<SrcType, quarter>(dv);
		auto qminmax = blas::range(qv);
		std::cout << qminmax.first << ", " << qminmax.second << " : " << symmetry_range<quarter>() << '\n';

	if (N < 15) {
		std::cout << dv << '\n';
		std::cout << sv << '\n';
		std::cout << hv << '\n';
		std::cout << qv << '\n';
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Uncaught arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Uncaught internal exception: " << err.what() << std::endl;
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
