// test_matrices_df.cpp: universal datafile creation and serialization of test matrices
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

// Serialization
#include <blas/serialization/datafile.hpp>

#include <universal/verification/test_suite.hpp>
#include <blas/matrices/testsuite.hpp>

namespace sw { namespace blas {
	using namespace sw::universal;

	template<bool SerializationFormat>
	void CreateTestMatrixCollection(const std::string& datafileFilename, const std::vector<std::string>& testMatrixNames)
	{
		// generate the file name
		std::string fileExtension = std::string(".txt");  // default is ASCII text format so the files are easy to inspect
		if constexpr (SerializationFormat == BinaryFormat) {
			fileExtension = std::string(".dat");
		}
		std::string filename = datafileFilename + fileExtension;
		std::cout << "Writing data set to file: " << filename << '\n';

		// create the datafile
		datafile<TextFormat> df;
		for (auto& testMatrixName : testMatrixNames) {
			matrix<double> m = getTestMatrix(testMatrixName);
			df.add(m, testMatrixName);
		}

		// write the datafile
		std::ofstream fo;
		fo.open(filename);
		df.save(fo, false);  // decimal format
		fo.close();
	}

	template<bool SerializationFormat = TextFormat>
	void LoadTestMatrixCollection(const std::string& datafileFilename, datafile<SerializationFormat>& df)
	{
		// generate the filename
		std::string fileExtension = std::string(".txt");  // default is ASCII text format so the files are easy to inspect
		if constexpr (SerializationFormat == BinaryFormat) {
			fileExtension = std::string(".dat");
		}
		std::string filename = datafileFilename + fileExtension;
		std::cout << "Reading data set from file: " << filename << '\n';

		// restore the datafile
		std::ifstream fi;
		fi.open(filename);
		df.restore(fi);
		fi.close();
	}
	}
}

// This is a program that we ran once to get the test matrices converted to data files
// 
// We have no code in the regression side of the test so CI is a NOP


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
	using namespace sw::blas;

	std::string test_suite  = "small matrices data file";
	std::string test_tag    = "small_matrices.dat";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// set up the set of test matrices
	std::vector<std::string> allTestMatrices = {
		"lambers_well",  //   2 x   2 well-conditioned matrix, K = 10.0
		"lambers_ill",   //   2 x   2 ill-conditioned matrix, K = 1.869050824603144e+08
		"h3",            //   3 x   3 test matrix, K = 1.8478e+11
		"int3",          //   3 x   3 integer test matrix (low condition number), K = 43.6115
		"faires74x3",    //   3 x   3 Burden Faires Ill-conditioned, K = 15999
		"q3",            //   3 x   3 Variable test matrix (edit entries), K = 1.2857e+06
		"q4",            //   4 x   4 test matrix, K = 2.35
		"q5",            //   5 x   5 test matrix, K = 1.1e+04
		"lu4",           //   4 x   4 test matrix, K = 11.6810
		"s4",            //   4 x   4 test matrix, K = 4.19
		"rand4",         //   4 x   4 random (low condition), K = 27.81
		"cage3",         //   5 x   5 Directed Weighted Graph, K = 1.884547e+01
		"b1_ss",         //   7 x   7 Chemical Process Simulation Problem, K = 1.973732e+02

		"west0132",      // 132 x 132 Chem. Simulation Process, K = 4.2e+11 
		"west0167",      // 167 x 167 Chemical Simulation Process, K = 2.827e+07
		"steam1",        // 240 x 240 Computational Fluid Dynamics, K = 2.827501e+07
		"steam3",        //  83 x  83 Computational Fluid Dynamics, K = 5.51e+10
		"fs_183_1",      // 183 x 183 2D/3D Problem Sequence, K = 1.5129e+13
		"fs_183_3",      // 183 x 183 2D/3D Problem Sequence, K = 1.5129e+13
		"bwm200",        // 200 x 200 Chemical simulation, K = 2.412527e+03
		"gre_343",       // 343 x 343 Directed Weighted Graph, K = 1.119763e+02
		"pores_1",       //  30 x  30 Computational Fluid Dynamics, K = 1.812616e+06
		"Stranke94",     //  10 x  10 Undirected Weighted Graph, K = 5.173300e+01
		"Trefethen_20",  //  20 x  20 Combinatorial Problem, K = 6.308860e+01
		"bcsstk01",      //  48 x  48 Structural Engineering, K = 8.8234e+05
		"bcsstk03",      // 112 x 112 Structural Engineering, K = 6.791333e+06
		"bcsstk04",      // 132 x 132 Structural Engineering, K = 2.292466e+06
		"bcsstk05",      // 153 x 153 Structural Engineering, K = 1.428114e+04
		"bcsstk22",      // 138 x 138 Structural Engineering, K = 1.107165e+05
		"lund_a",        // 147 x 147 Structural Engineering, K = 2.796948e+06
		"nos1",          // 237 x 237 Structural Engineering K = 1.991546e+07
		"arc130",        // 130 x 130    K = 6.0542e+10
		"saylr1",        // 238 x 238 Computational Fluid Dynamics, K = 7.780581e+08
		"tumorAntiAngiogenesis_2" // , K 1.9893e+10
	};

	/*  there is a bug in serialization, so we have disabled the df collection idea for the moment
	CreateTestMatrixCollection<TextFormat>("test_matrices_df", allTestMatrices);

	datafile<TextFormat> TestMatrixDF;
	LoadTestMatrixCollection<TextFormat>("test_matrices_df", TestMatrixDF);

	matrix<double> h3;
	TestMatrixDF.get("h3", h3);
	std::cout << "h3 matrix:\n" << h3 << '\n';

	*/

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else
	// CI is a NOP
	// we have no code in the regression side of the test

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
