// static_matrices.cpp: enumerate ALL test matrices and report condition numbers
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// This test exercises ALL matrix header files in blas/matrices/ by including them
// directly and exercising each matrix's data. This ensures full code coverage of
// the matrices directory.

#include <universal/utility/directives.hpp>
#include <blas/blas.hpp>

// Include ALL matrix header files for complete coverage
// Small matrices (fast compilation)
#include <blas/matrices/lambers_well.hpp>   //   2 x   2 well-conditioned
#include <blas/matrices/lambers_ill.hpp>    //   2 x   2 ill-conditioned
#include <blas/matrices/h3.hpp>             //   3 x   3 test matrix
#include <blas/matrices/q3.hpp>             //   3 x   3 test matrix
#include <blas/matrices/int3.hpp>           //   3 x   3 integer test matrix
#include <blas/matrices/faires74x3.hpp>     //   3 x   3 Burden Faires ill-conditioned
#include <blas/matrices/q4.hpp>             //   4 x   4 test matrix
#include <blas/matrices/q5.hpp>             //   5 x   5 test matrix
#include <blas/matrices/lu4.hpp>            //   4 x   4 test matrix
#include <blas/matrices/s4.hpp>             //   4 x   4 test matrix
#include <blas/matrices/rand4.hpp>          //   4 x   4 random matrix
#include <blas/matrices/rand8.hpp>          //   8 x   8 random matrix
#include <blas/matrices/rump6x6ill.hpp>     //   6 x   6 Rump ill-conditioned
#include <blas/matrices/b1_ss.hpp>          //   7 x   7 Chemical Process Simulation
#include <blas/matrices/cage3.hpp>          //   5 x   5 Directed Weighted Graph
#include <blas/matrices/Stranke94.hpp>      //  10 x  10 Undirected Weighted Graph
#include <blas/matrices/Trefethen_20.hpp>   //  20 x  20 Combinatorial Problem
#include <blas/matrices/wilk21.hpp>         //  21 x  21 Wilkinson test matrix
#include <blas/matrices/pores_1.hpp>        //  30 x  30 Computational Fluid Dynamics

// Medium matrices
#include <blas/matrices/bcsstk01.hpp>       //  48 x  48 Structural Engineering
#include <blas/matrices/steam3.hpp>         //  83 x  83 Computational Fluid Dynamics
#include <blas/matrices/bcsstk03.hpp>       // 112 x 112 Structural Engineering
#include <blas/matrices/arc130.hpp>         // 130 x 130
#include <blas/matrices/west0132.hpp>       // 132 x 132 Chemical Simulation
#include <blas/matrices/bcsstk04.hpp>       // 132 x 132 Structural Engineering
#include <blas/matrices/bcsstk22.hpp>       // 138 x 138 Structural Engineering
#include <blas/matrices/lund_a.hpp>         // 147 x 147 Structural Engineering
#include <blas/matrices/bcsstk05.hpp>       // 153 x 153 Structural Engineering
#include <blas/matrices/west0167.hpp>       // 167 x 167 Chemical Simulation
#include <blas/matrices/fs_183_1.hpp>       // 183 x 183 2D/3D Problem
#include <blas/matrices/fs_183_3.hpp>       // 183 x 183 2D/3D Problem
#include <blas/matrices/bwm200.hpp>         // 200 x 200 Chemical Simulation
#include <blas/matrices/nos1.hpp>           // 237 x 237 Structural Engineering
#include <blas/matrices/saylr1.hpp>         // 238 x 238 Computational Fluid Dynamics
#include <blas/matrices/steam1.hpp>         // 240 x 240 Computational Fluid Dynamics
#include <blas/matrices/gre_343.hpp>        // 343 x 343 Directed Weighted Graph
#include <blas/matrices/tumorAntiAngiogenesis_2.hpp>  // tumor model

// Also include testsuite.hpp for getTestMatrix() and kappa() functions
#include <blas/matrices/testsuite.hpp>

#include <iostream>
#include <iomanip>
#include <string>

// Helper to test a matrix and report results
template<typename Matrix>
bool testMatrix(const std::string& name, const Matrix& A, double expectedKappa, int& nrTests, int& nrPass) {
    ++nrTests;

    size_t rows = num_rows(A);
    size_t cols = num_cols(A);

    bool isSquare = (rows == cols);
    bool isValid = (rows > 0 && cols > 0);
    bool pass = isSquare && isValid;

    if (pass) ++nrPass;

    // Categorize by condition number
    std::string category;
    if (expectedKappa < 100) {
        category = "well-cond";
    } else if (expectedKappa < 1e6) {
        category = "moderate";
    } else if (expectedKappa < 1e10) {
        category = "ill-cond";
    } else {
        category = "severe";
    }

    std::cout << std::left << std::setw(24) << name
              << std::right << std::setw(6) << rows
              << std::setw(6) << cols
              << std::scientific << std::setprecision(2)
              << std::setw(14) << expectedKappa
              << std::fixed
              << std::setw(12) << category
              << (pass ? "" : " FAIL")
              << "\n";

    return pass;
}

int main(int argc, char* argv[])
try {
    using namespace sw::numeric::containers;

    int nrTests = 0;
    int nrPass = 0;

    std::cout << "Complete Static Test Matrices: Condition Number Report\n";
    std::cout << "=======================================================\n\n";

    std::cout << std::left << std::setw(24) << "Matrix"
              << std::right << std::setw(6) << "Rows"
              << std::setw(6) << "Cols"
              << std::setw(14) << "Kappa"
              << std::setw(12) << "Category"
              << "\n";
    std::cout << std::string(62, '-') << "\n";

    // Test ALL matrices - small matrices first
    testMatrix("lambers_well", lambers_well, 10.0, nrTests, nrPass);
    testMatrix("lambers_ill", lambers_ill, 1.869e+08, nrTests, nrPass);
    testMatrix("h3", h3, 1.8478e+11, nrTests, nrPass);
    testMatrix("q3", q3, 1.2857e+06, nrTests, nrPass);
    testMatrix("int3", int3, 43.6115, nrTests, nrPass);
    testMatrix("faires74x3", faires74x3, 15999, nrTests, nrPass);
    testMatrix("q4", q4, 2.35, nrTests, nrPass);
    testMatrix("q5", q5, 1.1e+04, nrTests, nrPass);
    testMatrix("lu4", lu4, 11.6810, nrTests, nrPass);
    testMatrix("s4", s4, 4.19, nrTests, nrPass);
    testMatrix("rand4", rand4, 27.81, nrTests, nrPass);
    testMatrix("rand8", rand8, 100.0, nrTests, nrPass);  // estimated
    testMatrix("rump6x6ill", rump6x6ill, 1e+16, nrTests, nrPass);  // severely ill-conditioned
    testMatrix("b1_ss", b1_ss, 1.973732e+02, nrTests, nrPass);
    testMatrix("cage3", cage3, 1.884547e+01, nrTests, nrPass);
    testMatrix("Stranke94", Stranke94, 5.173300e+01, nrTests, nrPass);
    testMatrix("Trefethen_20", Trefethen_20, 6.308860e+01, nrTests, nrPass);
    testMatrix("wilk21", wilk21, 42.0, nrTests, nrPass);
    testMatrix("pores_1", pores_1, 1.812616e+06, nrTests, nrPass);

    // Medium matrices
    testMatrix("bcsstk01", bcsstk01, 8.8234e+05, nrTests, nrPass);
    testMatrix("steam3", steam3, 5.51e+10, nrTests, nrPass);
    testMatrix("bcsstk03", bcsstk03, 6.791333e+06, nrTests, nrPass);
    testMatrix("arc130", arc130, 6.0542e+10, nrTests, nrPass);
    testMatrix("west0132", west0132, 4.2e+11, nrTests, nrPass);
    testMatrix("bcsstk04", bcsstk04, 2.292466e+06, nrTests, nrPass);
    testMatrix("bcsstk22", bcsstk22, 1.107165e+05, nrTests, nrPass);
    testMatrix("lund_a", lund_a, 2.796948e+06, nrTests, nrPass);
    testMatrix("bcsstk05", bcsstk05, 1.428114e+04, nrTests, nrPass);
    testMatrix("west0167", west0167, 2.827e+07, nrTests, nrPass);
    testMatrix("fs_183_1", fs_183_1, 1.5129e+13, nrTests, nrPass);
    testMatrix("fs_183_3", fs_183_3, 1.5129e+13, nrTests, nrPass);
    testMatrix("bwm200", bwm200, 2.412527e+03, nrTests, nrPass);
    testMatrix("nos1", nos1, 1.991546e+07, nrTests, nrPass);
    testMatrix("saylr1", saylr1, 7.780581e+08, nrTests, nrPass);
    testMatrix("steam1", steam1, 2.827501e+07, nrTests, nrPass);
    testMatrix("gre_343", gre_343, 1.119763e+02, nrTests, nrPass);
    testMatrix("tumorAntiAngiogenesis_2", tumorAntiAngiogenesis_2, 1.9893e+10, nrTests, nrPass);

    std::cout << std::string(62, '-') << "\n";
    std::cout << "\nMatrix validation: " << nrPass << " of " << nrTests << " passed\n\n";

    // Test testsuite.hpp API functions
    std::cout << "Testing testsuite.hpp API functions:\n";
    std::cout << std::string(40, '-') << "\n";

    // Test getTestMatrix() with valid name
    ++nrTests;
    matrix<double> test_lu4 = getTestMatrix("lu4");
    if (num_rows(test_lu4) == 4 && num_cols(test_lu4) == 4) {
        std::cout << "  getTestMatrix(\"lu4\"): PASS (4x4)\n";
        ++nrPass;
    } else {
        std::cout << "  getTestMatrix(\"lu4\"): FAIL\n";
    }

    // Test getTestMatrix() with invalid name (should return lu4)
    ++nrTests;
    matrix<double> fallback = getTestMatrix("nonexistent_matrix");
    if (num_rows(fallback) == num_rows(test_lu4) && num_cols(fallback) == num_cols(test_lu4)) {
        std::cout << "  getTestMatrix(invalid): PASS (returns lu4 fallback)\n";
        ++nrPass;
    } else {
        std::cout << "  getTestMatrix(invalid): FAIL\n";
    }

    // Test kappa() with valid name
    ++nrTests;
    double k = kappa("lambers_well");
    if (std::abs(k - 10.0) < 0.001) {
        std::cout << "  kappa(\"lambers_well\"): PASS (" << k << ")\n";
        ++nrPass;
    } else {
        std::cout << "  kappa(\"lambers_well\"): FAIL (got " << k << ")\n";
    }

    // Test kappa() with invalid name (should return lu4's kappa)
    ++nrTests;
    double fallbackKappa = kappa("nonexistent_matrix");
    if (std::abs(fallbackKappa - 11.6810) < 0.001) {
        std::cout << "  kappa(invalid): PASS (returns lu4 kappa = " << fallbackKappa << ")\n";
        ++nrPass;
    } else {
        std::cout << "  kappa(invalid): FAIL (got " << fallbackKappa << ")\n";
    }

    std::cout << std::string(40, '-') << "\n";
    std::cout << "\nTotal: " << nrPass << " of " << nrTests << " tests passed\n";

    return (nrPass == nrTests) ? EXIT_SUCCESS : EXIT_FAILURE;
}
catch (char const* msg) {
    std::cerr << "Caught exception: " << msg << std::endl;
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
