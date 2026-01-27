#pragma once
// test_matrix.hpp: gather a test matrix from the Universal test matrix database
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project
#include <filesystem>
#include <iostream>
#include <fstream>
#include <map>
#include "TestMatrixDataDirConfig.hpp"

namespace sw {
    namespace blas {

		// pick up the common data directory for the test suite
		std::string dataDirectory() {
			return std::string(TEST_MATRIX_DATA_DIRECTORY);
		}

        /// <summary>
        /// get the test matrix from the Universal test matrix database
        /// </summary>
        /// <param name="testMatrix">name of the matrix to retrieve</param>
        /// <returns>a copy of the matrix</returns>
        inline matrix<double> getTestMatrix(const std::string &testMatrix) {
			//std::cout << "Current working directory: " << std::filesystem::current_path() << '\n';
			std::string filename = dataDirectory() + std::string("/") + testMatrix + ".dat";
			matrix<double> A;
			std::ifstream fi;
			fi.open(filename);
			if (fi.good()) {
				fi >> A;
				fi.close();
			}
			else {  // LCOV_EXCL_START
				std::cerr << "Unable to open matrix file " << filename << std::endl;
				fi.close();
				return A;
			}  // LCOV_EXCL_STOP
			fi.close();

			//std::cout << "Matrix " << testMatrix << " loaded\n";
			return A;

        }

		/// <summary>
		/// TestMatrixList is a list of test matrices that are used in the LUIR experiments
		/// </summary>
		const std::vector<std::string> TestMatrixList{
			"lambers_well",
			"lambers_ill",
			"h3",
			"q3",
			"int3",
			"faires74x3",
			"q4",
			"lu4",
			"s4",
			"rand4",
			"q5",
			"west0132",
			"west0167",
			"steam1",
			"steam3",
			"fs_183_1",
			"fs_183_3",
			"bwm200",
			"gre_343",
			"b1_ss",
			"cage3",
			"pores_1",
			"Stranke94",
			"saylr1",
			"Trefethen_20",
			"bcsstk01",
			"bcsstk03",
			"bcsstk04",
			"bcsstk05",
			"bcsstk22",
			"lund_a",
			"nos1",
			"arc130",
			"tumorAntiAngiogenesis_2"
		};

        const std::map<std::string, double> ConditionNumber {
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
			{"west0132", 4.2e+11},
			{"west0167", 2.827e+07},
			{"steam1", 2.827501e+07},
			{"steam3", 5.51e+10},
			{"fs_183_1", 1.5129e+13},
			{"fs_183_3", 1.5129e+13},
			{"bwm200", 2.412527e+03},
			{"gre_343", 1.119763e+02},
			{"b1_ss", 1.973732e+02},
			{"cage3", 1.884547e+01},
			{"pores_1", 1.812616e+06},
			{"Stranke94", 5.173300e+01},
			{"saylr1", 7.780581e+08},
			{"Trefethen_20", 6.308860e+01},
			{"bcsstk01", 8.8234e+05},
			{"bcsstk03", 6.791333e+06},
			{"bcsstk04", 2.292466e+06},
			{"bcsstk05", 1.428114e+04},
			{"bcsstk22", 1.107165e+05},
			{"lund_a", 2.796948e+06},
			{"nos1", 1.991546e+07},
			{"arc130", 6.0542e+10},
			{"tumorAntiAngiogenesis_2", 1.9893e+10}
		};  

		// Condition Number
		double kappa(const std::string& testMatrix) {
			if (ConditionNumber.find(testMatrix) != ConditionNumber.end()) {
				return ConditionNumber.at(testMatrix);
			}
			else {  // LCOV_EXCL_START
				std::cerr << "Condition number for matrix " << testMatrix << " not found" << std::endl;
			}  // LCOV_EXCL_STOP
			return 0.0;
		}
	}
}

