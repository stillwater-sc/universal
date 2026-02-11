// test_matrices.cpp: convert test matrix include files to data files
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

// Serialization - test_matrix.hpp reads matrices from .dat files
#include <blas/serialization/datafile.hpp>
#include <blas/serialization/test_matrix.hpp>

#include <universal/verification/test_suite.hpp>
#include <blas/matrices/testsuite.hpp>

namespace sw { namespace blas {
	using namespace sw::numeric::containers;

	static void WriteMatrixDataFile(const std::string& filename, const matrix<double>& A) {
		std::ofstream fo;
		fo.open(filename);
		fo << A;
		fo.close();
	}

	static void GenerateMatrixDataFiles(const std::vector<std::string>& testMatrixNames) {
		for (auto matrixName : testMatrixNames) {
			WriteMatrixDataFile(matrixName + std::string(".dat"), getTestMatrix(matrixName));
		}
	}

	////////////////////////////////////////////////////////////////////////
	// Test global getTestMatrix() from testsuite.hpp
	int VerifyGlobalGetTestMatrix(bool reportTestCases) {
		int nrOfFailedTests = 0;

		// Test all available matrices from testsuite.hpp
		std::vector<std::pair<std::string, std::pair<size_t, size_t>>> expectedDimensions = {
			{"lambers_well", {2, 2}},
			{"lambers_ill", {2, 2}},
			{"h3", {3, 3}},
			{"q3", {3, 3}},
			{"int3", {3, 3}},
			{"faires74x3", {3, 3}},
			{"q4", {4, 4}},
			{"lu4", {4, 4}},
			{"s4", {4, 4}},
			{"rand4", {4, 4}},
			{"q5", {5, 5}},
			{"b1_ss", {7, 7}},
			{"cage3", {5, 5}},
			{"pores_1", {30, 30}},
			{"Stranke94", {10, 10}},
			{"Trefethen_20", {20, 20}}
		};

		for (const auto& entry : expectedDimensions) {
			const std::string& name = entry.first;
			size_t expectedRows = entry.second.first;
			size_t expectedCols = entry.second.second;

			// Use global getTestMatrix (from testsuite.hpp)
			matrix<double> M = ::getTestMatrix(name);

			if (num_rows(M) != expectedRows || num_cols(M) != expectedCols) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: global getTestMatrix(" << name << ") = "
				                               << num_rows(M) << "x" << num_cols(M)
				                               << " (expected " << expectedRows << "x" << expectedCols << ")\n";
			}
		}

		// Test unknown matrix returns default (lu4)
		matrix<double> unknown = ::getTestMatrix("unknown_matrix_xyz");
		if (num_rows(unknown) != 4 || num_cols(unknown) != 4) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: global getTestMatrix(unknown) should return lu4 (4x4)\n";
		}

		return nrOfFailedTests;
	}

	////////////////////////////////////////////////////////////////////////
	// Test global kappa() from testsuite.hpp
	int VerifyGlobalKappa(bool reportTestCases) {
		int nrOfFailedTests = 0;

		// Test known condition numbers
		std::vector<std::pair<std::string, double>> expectedKappa = {
			{"lambers_well", 10.0},
			{"lambers_ill", 1.869050824603144e+08},
			{"h3", 1.8478e+11},
			{"q3", 1.2857e+06},
			{"int3", 43.6115},
			{"faires74x3", 15999},
			{"q4", 2.35},
			{"lu4", 11.6810},
			{"s4", 4.19},
			{"rand4", 27.81},
			{"q5", 1.1e+04},
			{"b1_ss", 1.973732e+02},
			{"cage3", 1.884547e+01},
			{"pores_1", 1.812616e+06},
			{"Stranke94", 5.173300e+01},
			{"Trefethen_20", 6.308860e+01}
		};

		for (const auto& entry : expectedKappa) {
			const std::string& name = entry.first;
			double expected = entry.second;

			// Use global kappa (from testsuite.hpp)
			double k = ::kappa(name);

			// Use relative tolerance for comparison
			double relError = std::abs(k - expected) / expected;
			if (relError > 0.001) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: global kappa(" << name << ") = " << k
				                               << " (expected " << expected << ")\n";
			}
		}

		// Test unknown matrix returns default (lu4's kappa)
		double unknownK = ::kappa("unknown_matrix_xyz");
		if (std::abs(unknownK - 11.6810) > 0.01) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: global kappa(unknown) should return 11.6810\n";
		}

		return nrOfFailedTests;
	}

	////////////////////////////////////////////////////////////////////////
	// Test matrix data values (not just dimensions)
	int VerifyMatrixValues(bool reportTestCases) {
		int nrOfFailedTests = 0;

		// Test lambers_well matrix values
		// lambers_well = [[1, 2], [3, 4]] or similar known values
		{
			matrix<double> A = ::getTestMatrix("lambers_well");
			if (num_rows(A) >= 2 && num_cols(A) >= 2) {
				// Just verify the matrix has non-zero values
				bool hasNonZero = false;
				for (size_t i = 0; i < num_rows(A) && !hasNonZero; ++i) {
					for (size_t j = 0; j < num_cols(A) && !hasNonZero; ++j) {
						if (A[i][j] != 0.0) hasNonZero = true;
					}
				}
				if (!hasNonZero) {
					++nrOfFailedTests;
					if (reportTestCases) std::cerr << "FAIL: lambers_well has all zero values\n";
				}
			}
		}

		// Test lu4 matrix values
		{
			matrix<double> A = ::getTestMatrix("lu4");
			if (num_rows(A) == 4 && num_cols(A) == 4) {
				// Verify diagonal has non-zero values (for LU decomposition test matrices)
				bool diagonalOK = true;
				for (size_t i = 0; i < 4; ++i) {
					if (A[i][i] == 0.0) diagonalOK = false;
				}
				if (!diagonalOK) {
					++nrOfFailedTests;
					if (reportTestCases) std::cerr << "FAIL: lu4 diagonal should be non-zero\n";
				}
			}
		}

		return nrOfFailedTests;
	}

	////////////////////////////////////////////////////////////////////////
	// Test WriteMatrixDataFile function
	int VerifyWriteMatrixDataFile(bool reportTestCases) {
		int nrOfFailedTests = 0;

		// Create a small test matrix
		matrix<double> A(3, 3);
		A[0][0] = 1.0; A[0][1] = 2.0; A[0][2] = 3.0;
		A[1][0] = 4.0; A[1][1] = 5.0; A[1][2] = 6.0;
		A[2][0] = 7.0; A[2][1] = 8.0; A[2][2] = 9.0;

		// Write to temporary file
		std::string tempFile = "/tmp/test_matrix_write.dat";
		WriteMatrixDataFile(tempFile, A);

		// Read back and verify
		matrix<double> B;
		std::ifstream fi(tempFile);
		if (fi.good()) {
			fi >> B;
			fi.close();

			if (num_rows(B) != 3 || num_cols(B) != 3) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: read back matrix has wrong dimensions\n";
			} else {
				// Verify values
				for (size_t i = 0; i < 3; ++i) {
					for (size_t j = 0; j < 3; ++j) {
						if (std::abs(A[i][j] - B[i][j]) > 0.0001) {
							++nrOfFailedTests;
							if (reportTestCases) std::cerr << "FAIL: matrix value mismatch at [" << i << "][" << j << "]\n";
						}
					}
				}
			}
		} else {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: could not read back written matrix file\n";
		}

		// Clean up
		std::remove(tempFile.c_str());

		return nrOfFailedTests;
	}

	////////////////////////////////////////////////////////////////////////
	// Compare file-loaded matrix with header-defined matrix
	int VerifyFileVsHeaderMatrices(bool reportTestCases) {
		int nrOfFailedTests = 0;

		// Compare matrices that are available in both formats
		std::vector<std::string> matricesToCompare = {
			"lambers_well",
			"lambers_ill",
			"lu4",
			"q4"
		};

		for (const auto& name : matricesToCompare) {
			// Get from header (global function)
			matrix<double> fromHeader = ::getTestMatrix(name);

			// Get from file (sw::blas function)
			matrix<double> fromFile = sw::blas::getTestMatrix(name);

			if (num_rows(fromFile) == 0 || num_cols(fromFile) == 0) {
				// File doesn't exist, skip comparison
				continue;
			}

			if (num_rows(fromHeader) != num_rows(fromFile) || num_cols(fromHeader) != num_cols(fromFile)) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: " << name << " dimension mismatch between header and file\n";
				continue;
			}

			// Compare values
			double maxError = 0.0;
			for (size_t i = 0; i < num_rows(fromHeader); ++i) {
				for (size_t j = 0; j < num_cols(fromHeader); ++j) {
					double err = std::abs(fromHeader[i][j] - fromFile[i][j]);
					if (err > maxError) maxError = err;
				}
			}

			// Allow for floating-point serialization differences (file stores limited precision)
			if (maxError > 1e-5) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: " << name << " value mismatch (max error: " << maxError << ")\n";
			}
		}

		return nrOfFailedTests;
	}

} }

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

	std::string test_suite  = "test matrices serialization";
	std::string test_tag    = "test_matrices";
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
	GenerateMatrixDataFiles(allTestMatrices);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else
	// Test the serialization/test_matrix.hpp functionality
	// Note: sw::blas::getTestMatrix() reads from .dat files (different from global getTestMatrix())
#if REGRESSION_LEVEL_1
	{
		// Test dataDirectory()
		std::string dataDir = sw::blas::dataDirectory();
		if (dataDir.empty()) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "FAIL: dataDirectory() returned empty\n";
		} else {
			if (reportTestCases) std::cout << "PASS: dataDirectory() = " << dataDir << "\n";
		}

		// Test TestMatrixList is populated
		if (sw::blas::TestMatrixList.empty()) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "FAIL: TestMatrixList is empty\n";
		} else {
			if (reportTestCases) std::cout << "PASS: TestMatrixList has " << sw::blas::TestMatrixList.size() << " matrices\n";
		}

		// Test ConditionNumber map is populated
		if (sw::blas::ConditionNumber.empty()) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "FAIL: ConditionNumber map is empty\n";
		} else {
			if (reportTestCases) std::cout << "PASS: ConditionNumber map has " << sw::blas::ConditionNumber.size() << " entries\n";
		}

		// Test kappa() for known matrices (from serialization module)
		double k = sw::blas::kappa("lambers_well");
		if (std::abs(k - 10.0) < 0.001) {
			if (reportTestCases) std::cout << "PASS: kappa(lambers_well) = " << k << "\n";
		} else {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "FAIL: kappa(lambers_well) = " << k << " (expected 10.0)\n";
		}

		// Test kappa() for unknown matrix (should return 0.0 and print error)
		double unknownK = sw::blas::kappa("nonexistent_matrix");
		if (unknownK == 0.0) {
			if (reportTestCases) std::cout << "PASS: kappa(unknown) = 0.0 (expected)\n";
		} else {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "FAIL: kappa(unknown) = " << unknownK << " (expected 0.0)\n";
		}
	}

	// Test global getTestMatrix() from testsuite.hpp
	nrOfFailedTestCases += ReportTestResult(sw::blas::VerifyGlobalGetTestMatrix(reportTestCases), "testsuite.hpp", "getTestMatrix");

	// Test global kappa() from testsuite.hpp
	nrOfFailedTestCases += ReportTestResult(sw::blas::VerifyGlobalKappa(reportTestCases), "testsuite.hpp", "kappa");

	// Test matrix data values
	nrOfFailedTestCases += ReportTestResult(sw::blas::VerifyMatrixValues(reportTestCases), "matrices", "value verification");
#endif

#if REGRESSION_LEVEL_2
	{
		using namespace sw::numeric::containers;

		// Test loading a small matrix from file using sw::blas::getTestMatrix
		matrix<double> A = sw::blas::getTestMatrix("lambers_well");
		if (num_rows(A) == 2 && num_cols(A) == 2) {
			if (reportTestCases) std::cout << "PASS: getTestMatrix(lambers_well) = 2x2\n";
		} else {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "FAIL: getTestMatrix(lambers_well) = " << num_rows(A) << "x" << num_cols(A) << "\n";
		}

		// Test loading another matrix
		matrix<double> B = sw::blas::getTestMatrix("lu4");
		if (num_rows(B) == 4 && num_cols(B) == 4) {
			if (reportTestCases) std::cout << "PASS: getTestMatrix(lu4) = 4x4\n";
		} else {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "FAIL: getTestMatrix(lu4) = " << num_rows(B) << "x" << num_cols(B) << "\n";
		}

		// Test loading non-existent matrix (should return empty matrix)
		matrix<double> C = sw::blas::getTestMatrix("nonexistent_matrix");
		if (num_rows(C) == 0 && num_cols(C) == 0) {
			if (reportTestCases) std::cout << "PASS: getTestMatrix(unknown) = 0x0 (expected)\n";
		} else {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "FAIL: getTestMatrix(unknown) = " << num_rows(C) << "x" << num_cols(C) << "\n";
		}
	}

	// Test WriteMatrixDataFile function
	nrOfFailedTestCases += ReportTestResult(sw::blas::VerifyWriteMatrixDataFile(reportTestCases), "serialization", "WriteMatrixDataFile");

	// Compare file-loaded vs header-defined matrices
	nrOfFailedTestCases += ReportTestResult(sw::blas::VerifyFileVsHeaderMatrices(reportTestCases), "matrices", "file vs header comparison");
#endif

#if REGRESSION_LEVEL_3
	{
		using namespace sw::numeric::containers;

		// Test loading all matrices in TestMatrixList
		int loadedCount = 0;
		for (const auto& matrixName : sw::blas::TestMatrixList) {
			matrix<double> M = sw::blas::getTestMatrix(matrixName);
			if (num_rows(M) > 0 && num_cols(M) > 0) {
				++loadedCount;
			} else {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cout << "FAIL: getTestMatrix(" << matrixName << ") load failed\n";
			}
		}
		if (reportTestCases) std::cout << "Loaded " << loadedCount << "/" << sw::blas::TestMatrixList.size() << " matrices\n";
	}
#endif

#if REGRESSION_LEVEL_4
	{
		// Verify condition numbers are available for all matrices in TestMatrixList
		int kappaCount = 0;
		for (const auto& matrixName : sw::blas::TestMatrixList) {
			double k = sw::blas::kappa(matrixName);
			if (k > 0.0) {
				++kappaCount;
			} else {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cout << "FAIL: kappa(" << matrixName << ") not found\n";
			}
		}
		if (reportTestCases) std::cout << "Found kappa for " << kappaCount << "/" << sw::blas::TestMatrixList.size() << " matrices\n";
	}
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
