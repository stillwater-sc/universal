// small_matrices_df.cpp: data file creation and serialization for small test matrices
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <universal/blas/blas.hpp>
#include <universal/blas/generators.hpp>
#include <universal/blas/ext/solvers/fused_backsub.hpp>
#include <universal/blas/ext/solvers/fused_forwsub.hpp>
// Serialization
#include <universal/blas/serialization/datafile.hpp>

#include <universal/verification/test_suite.hpp>
#include <universal/blas/matrices/testsuite.hpp>

void CreateCollection(const std::string& dataFileName, const std::vector<std::string>& matrices)
{
	using namespace sw::universal;
	using namespace sw::universal::blas;

	datafile<TextFormat> df;
	for (auto testMatrixName : matrices) {
		matrix<double> m = getTestMatrix(testMatrixName);
		df.add(m, testMatrixName);
	}
	std::ofstream f;
	std::string filename = dataFileName + std::string(".dat");
	std::cout << "Writing data set to file: " << filename << '\n';
	f.open(filename);
	df.save(f, false);  // decimal format
	f.close();
}

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 1
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

	std::string test_suite  = "summary statistics";
	std::string test_tag    = "sumstat";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

#ifdef LATER



	{
		std::ofstream f;
		std::string filename = testMatrix + std::string(".dat");
		std::cout << "Writing matrix to file: " << filename << '\n';
		f.open(filename);
		datafile<TextFormat> df;
		df.add(lu4, "lu4");
		df.add(q3, "q3");
		df.save(f, false);  // decimal format
		f.close();
	}

	{
		std::ifstream f;
		std::string filename = testMatrix + std::string(".dat");
		std::cout << "Reading matrix from file: " << filename << '\n';
		f.open(filename);
		datafile<TextFormat> df;
		df.restore(f);
		f.close();
		matrix<double> ref2;

		std::cout << ref2 << '\n';
	}
#endif

	std::vector<std::string> smallMatrices = {
		"lambers_well",  // 2 x 2 well-conditioned matrix, K = 
		"lambers_ill",   // 2 x 2 ill-conditioned matrix, K = 
		"h3",            // 3 x 3 test matrix, K = 
		"int3",          // 3 x 3 integer test matrix (low condition number), K =
		"faires74x3",    // 3 x 3 Burden Faires Ill-conditioned, K =
		"q3",            // 3 x 3 Variable test matrix (edit entries) 
		"q4",            // 4 x 4 test matrix, K = 
		"q5",            // 4 x 4 test matrix, K = 
		"lu4",           // 4 x 4 test matrix, K = 
		"s4",            // 4 x 4 test matrix, K = 
		"rand4"          // 4 x 4 random (low condition), K = 
		"b1_ss",         // 7 x 7 Chemical Process Simulation Problem, K = 
		"cage3",         // 5 x 5 Directed Weighted Graph, K =   
	};

	CreateCollection("small_matrices", smallMatrices);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1

#endif

#if REGRESSION_LEVEL_2

#endif

#if REGRESSION_LEVEL_3

#endif

#if REGRESSION_LEVEL_4

#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Uncaught universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Uncaught universal internal exception: " << err.what() << std::endl;
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
