#pragma once
// experiment_utils.hpp: helper functions for experiments with LUIR
//     
// Copyright(c) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT 
//
// This file is part of the Mixed Precision Iterative Refinement project.
// get the test matrix database API
#include <blas/serialization/test_matrix.hpp>

namespace sw {
    namespace universal {
		using namespace sw::numeric::containers;

        /// <summary>
        /// report the condition number of the test matrices
        /// </summary>
        void ReportKappaValuesForTestMatrices() {
            using namespace sw::blas;
            for (auto& matrixName : TestMatrixList) {
                std::cout << matrixName << '\n';
                matrix<double> ref = getTestMatrix(matrixName);
                std::cout << "Size: (" << ref.rows() << ", " << ref.cols() << ")\n";
                std::cout << "Condition Number = " << kappa(matrixName) << '\n';
                //    std::cout << "Condition estimate: " << condest(ref) << '\n';
            }
        }

        /// <summary>
        /// View the numerical properties of an LUIR experiment configuration
        /// </summary>
        /// <typeparam name="HighPrecision"></typeparam>
        /// <typeparam name="WorkingPrecision"></typeparam>
        /// <typeparam name="LowPrecision"></typeparam>
        template<typename HighPrecision, typename WorkingPrecision, typename LowPrecision>
        void ReportExperimentConfiguration() {

            LowPrecision     u_L = std::numeric_limits<LowPrecision>::epsilon();
            WorkingPrecision u_W = std::numeric_limits<WorkingPrecision>::epsilon();
            HighPrecision    u_H = std::numeric_limits<HighPrecision>::epsilon();

            constexpr bool Verbose = false;
            if constexpr (Verbose) {
                std::cout << "High    Precision : " << sw::universal::symmetry_range<HighPrecision>() << "\n";
                std::cout << "Working Precision : " << sw::universal::symmetry_range<WorkingPrecision>() << "\n";
                std::cout << "Low     Precision : " << sw::universal::symmetry_range<LowPrecision>() << "\n";

                // Unit Round-off
                LowPrecision oneThird = 1.0 / 3.0;
                std::cout << "Nearest Value to 1/3   = " << oneThird << std::endl;
                std::cout << "Eps Low Precision      = " << u_L << std::endl;
                std::cout << "Eps Working Precision  = " << u_W << std::endl;
                std::cout << "Eps High Precision     = " << u_H << std::endl;
                std::cout << "Eps Test: 1 + u_L      = " << 1 + u_L << " vs. " << 1 + u_L / 2 << std::endl;
                std::cout << "------------------------------------------------------------------------" << "\n\n";
            }
            else {
                std::cout << "[ "
                    << type_tag(u_H) << ", "
                    << type_tag(u_W) << ", "
                    << type_tag(u_L) << " ] ";
            }
        }

        /// <summary>
        /// print the results of a LUIR experiments
        /// </summary>
        /// <param name="ostr"></param>
        /// <param name="testMatrices"></param>
        /// <param name="typeLabels"></param>
        /// <param name="results"></param>
        void PrintIterativeRefinementExperimentResults(std::ostream& ostr, const std::vector<std::string>& testMatrices, const vector<std::string>& typeLabels, std::map<std::string, vector<std::pair<int, double>> >& results) {
            // create CSV output
           // create the header
            ostr << "Iterations";
            for (auto& e : typeLabels) {
                ostr << ',' << e;
            }
            ostr << '\n';
            for (auto& m : testMatrices) {
                ostr << m;
                if (auto it = results.find(m); it != results.end()) {
                    auto& r = results[m];
                    for (auto& e : r) {
                        ostr << ',' << e.first;
                    }
                }
                ostr << '\n';
            }
            ostr << '\n';
            ostr << "Error";
            for (auto& e : typeLabels) {
                ostr << ',' << e;
            }
            ostr << '\n';
            for (auto& m : testMatrices) {
                ostr << m;
                if (auto it = results.find(m); it != results.end()) {
                    auto& r = results[m];
                    for (auto& e : r) {
                        ostr << ',' << e.second;
                    }
                }
                ostr << '\n';
            }
        }

    }
}