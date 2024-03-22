/** ************************************************************************
* roundAndReplace: A = LU Iterative Refinement approach
* 
*    Addresses the fundamental problem of solving Ax = b efficiently.
*      
* @author:     James Quinlan
* @copyright:  Copyright (c) 2022 James Quinlan
* @license:    MIT Open Source license 
* 
* This file is part of the Mixed Precision Iterative Refinement project
* *************************************************************************
*/

// Environmental Configurations
#include<universal/utility/directives.hpp>
#include<universal/utility/long_double.hpp>
#include<universal/utility/bit_cast.hpp>

// Universal Number System Types
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit/posit.hpp>

// Higher Order Libraries
#include <universal/blas/blas.hpp>// contains <universal/blas/vector.hpp> + <universal/blas/matrix.hpp>
#include <universal/blas/utes/matnorm.hpp>
#include <universal/blas/utes/condest.hpp>
#include <universal/blas/utes/nbe.hpp>      // Normwise Backward Error

// Support Packages
#include <universal/blas/solvers/plu.hpp>
#include <universal/blas/solvers/backsub.hpp>
#include <universal/blas/solvers/forwsub.hpp>
#include <universal/blas/squeeze.hpp>
//#include "utils/isdiagdom.hpp"

// Matrix Test Suite
#include <universal/blas/matrices/testsuite.hpp>
//#include <luir/matrices/testsuite.hpp>  // local version

// File I/O
#include <iostream>
#include <fstream>

// Local Configuration
#include "configs.hpp"
//#include "sjm.hpp"
//#include<luir/squeeze.hpp>
// #include "../mtl4/utils/print_utils.hpp"

    // View Numerical Properties of Configuration
template<typename HighPrecision, typename WorkingPrecision, typename LowPrecision>
void ReportExperimentConfiguration() {
    LowPrecision     u_L = std::numeric_limits<LowPrecision>::epsilon();
    WorkingPrecision u_W = std::numeric_limits<WorkingPrecision>::epsilon();
    HighPrecision    u_H = std::numeric_limits<HighPrecision>::epsilon();

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

template<typename HighPrecision, typename WorkingPrecision, typename LowPrecision>
int RunRoundAndReplaceExperiment(const sw::universal::blas::matrix<double>& testMatrix) {

    using namespace sw::universal::blas;

    ReportExperimentConfiguration<HighPrecision, WorkingPrecision, LowPrecision>();

    /**
     * Matrix and Vector Type alias
     * - Mh: High Precision Matrix
     * - Vh: High Precision Vector
     * - Mw: Working Precision Matrix
     * - Vw: Working Precision Vector
     * - Ml: Low Precision Matrix
    */
    using Mh = sw::universal::blas::matrix<HighPrecision>;
    using Vh = sw::universal::blas::vector<HighPrecision>;
    using Mw = sw::universal::blas::matrix<WorkingPrecision>;
    using Vw = sw::universal::blas::vector<WorkingPrecision>;
    using Ml = sw::universal::blas::matrix<LowPrecision>;

    Mw A = testMatrix;
    Mw Al;

    RoundAndReplace(A, Al);

    return EXIT_SUCCESS;
}

template<typename HighPrecision, typename WorkingPrecision, typename LowPrecision>
int OriginalRoundAndReplace(unsigned algo, const std::string& testMatrix) {
    using namespace sw::universal::blas;

    /**
     * Matrix and Vector Type alias
     * - Mh: High Precision Matrix
     * - Vh: High Precision Vector
     * - Mw: Working Precision Matrix
     * - Vw: Working Precision Vector
     * - Ml: Low Precision Matrix
    */
    using Mh = sw::universal::blas::matrix<HighPrecision>;
    using Vh = sw::universal::blas::vector<HighPrecision>;
    using Mw = sw::universal::blas::matrix<WorkingPrecision>;
    using Vw = sw::universal::blas::vector<WorkingPrecision>;
    using Ml = sw::universal::blas::matrix<LowPrecision>;

    // Unit round-off OR Machine esp 
    LowPrecision     u_L = std::numeric_limits<LowPrecision>::epsilon();
    WorkingPrecision u_W = std::numeric_limits<WorkingPrecision>::epsilon();
    HighPrecision    u_H = std::numeric_limits<HighPrecision>::epsilon();



    /** ********************************************************************
     *  Read Matrix, then store a low precision version Al
     *  - Display Metadata for A (per request, see configs.hpp)
     *  - Sets the working precision matrix A
     *  - Sets the low precision matrix Al
     * *********************************************************************
    */
    Mw A = getTestMatrix(testMatrix);
    Ml Al;

    // Display Metadata
    unsigned n = num_cols(A);
    if constexpr (showNumProps) { std::cout << "Largest Consec. Int = " << std::pow(2, 4 * (int(lbits) - 3) / 5) << std::endl; }
    if constexpr (showAmax) { std::cout << "(min(A), max(A)) = (" << minelement(A) << ", " << maxelement(A) << ")" << std::endl; }
    if constexpr (printMat) { disp(A); } // std::cout << "A = \n" << A << std::endl;
    if constexpr (showCond) { std::cout << "Condition Number = " << kappa(testMatrix) << std::endl; }
    if constexpr (showCondest) { std::cout << "Condition estimate: " << condest(A) << std::endl; }
    if constexpr (showSize) { std::cout << "Size: (" << n << ", " << n << ")" << std::endl; }
    /*
    if (isdd(A)){
        std::cout << "isdd(A) = TRUE \n" << std::endl;
    }else{
        std::cout << "isdd(A) = FALSE \n" << std::endl;
    }
    */

    /** ********************************************************************
     *   Squeeze Matrix:
     *   t = theta \in (0,1], is a scaling factor,
     * *********************************************************************
    */
    if constexpr (showProcesses) { std::cout << "Process: Start Squeezing..." << std::endl; }
    WorkingPrecision t = 0.1; //2949990 Is there an optimal value?  Parameter sweep 0.75 west
    WorkingPrecision mu = 1.0;  // 16 best for posit<x,2>
    Vw R(num_rows(A), 1);  // Row Squeezer
    Vw S(num_rows(A), 1);  // Column Squeezer

    std::cout << "Working precision: " << type_tag(WorkingPrecision()) << "\n";
    if (algo == 21) { // Round, then replace inf (overflow) 
        roundReplace(A, Al, n);
        if constexpr (showAlgo) { std::cout << "Algorithm: Round, then replace infinities." << std::endl; }
    }
    else if (algo == 22) { // Scale and Round
        scaleRound<WorkingPrecision, LowPrecision>(A, Al, t, mu, algo);
        if constexpr (showAlgo) { std::cout << "Algorithm " << algo << ": Scale, then round." << std::endl; }
    }
    else if (algo == 23 || algo == 24 || algo == 25) {
        twosideScaleRound<WorkingPrecision, LowPrecision>(A, Al, R, S, t, mu, n, algo);
        if constexpr (showAlgo) { std::cout << "Algorithm " << algo << ": Two-sided squeezing, RAS." << std::endl; }
    }
    else { // Do nothing
        Al = A;
        if constexpr (showAlgo) { std::cout << "Algorithm " << algo << ": Round only, i.e., A --> A (low).\n" << std::endl; }
    }
    // Print Squeezed A and Low Precision Al.  
    if constexpr (printMat) {
        std::cout << "A (modified) = \n"; disp(A);// << A << std::endl;
        std::cout << "Al (low precision) = \n"; disp(Al); // << Al << std::endl;
    }
    if constexpr (showProcesses) { std::cout << "Squeezing Complete!\n" << std::endl; }
    /* ****************************************************************** */
    std::cout << "mu = " << mu << "\n";
    std::cout << "A = " << A << "\n";


    /** ********************************************************************
     *  LU Factorization of Low Precision Matrix (key step)
     *  : A is factored into LU using low precision.
     *  : LU is then stored in working precision (note permuations included)
     *  : A = P*A is computed & stored in high precision for residual calc.
     * *********************************************************************
    */
    sw::universal::blas::matrix<size_t> P(n, 2); // check the size.
    // plu only uses 0,1,2,...,n-2 (e.g., n=10, then 0,1,2,...,8)
    // since there is no need to pivot last row.  See plu. 
    if constexpr (showProcesses) { std::cout << "Process: Factoring (PLU)..." << std::endl; }
    plu(Al, P);
    Mw LU(Al);
    if constexpr (showProcesses) { std::cout << "Complete!\n" << std::endl; }
    if constexpr (printLU) {
        std::cout << "LU = \n"; disp(LU);
    }
    // Compute A = P*A;
    if constexpr (showProcesses) { std::cout << "Process: computing PA..." << std::endl; }
    for (size_t ii = 0; ii < n; ++ii) {
        if (P(ii, 0) != P(ii, 1)) {
            for (size_t jj = 0; jj < n; ++jj) {
                auto aij = A(P(ii, 0), jj);
                A(P(ii, 0), jj) = A(P(ii, 1), jj);
                A(P(ii, 1), jj) = aij;
            }
        }
    }
    if constexpr (showProcesses) { std::cout << "Complete!\n" << std::endl; }
    if constexpr (printPA) {
        std::cout << "P  = \n" << P << std::endl;
        std::cout << "PA = \n" << A << std::endl;
    }
    Mh Ah(A);
    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~ END ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */


    // insert SMJ test if wanted, however, A must be diag. dom to use.


    /**
     * ********************************************************************
     * Initializations:
     *  - Exact Solution = X (default = [1,1,...,1]' )
     *  - RHS n-vector   =  b  (in high precision)
     *
     * Store each in working precision, x and bw.
     * Residuals, r, stored in high precision
     * ********************************************************************
    */
    Vh X(n, 1);
    if (randsol) {
        X = uniform_random_vector<HighPrecision>(n);
    }
    Vh b = Ah * X;  // mu*R*b
    Vw x(X);      // y = Sx 
    Vw bw(b);     // Note: also try b = P*mu*R*(AX), where A is original matrix.
    Vh r;
    // does nothing: std::cout << "typename = " << typeid(n).name() << std::endl; 
     /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~ END ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */


    /**
     * ********************************************************************
     * Iterative Refinement Steps
     *  1. Factor A = LU in low precision (see above)
     *  2. Solve x = (LU)^{-1} b
     *  3. While not coverged
     *      a). r = b - Ax (high precision calculation)
     *      b). Solve Ac = r (c = corrector)
     *      c). Update solution: x = x + c
     *  4. Goto 3
     * ********************************************************************
    */
    if constexpr (showProcesses) { std::cout << "Process: computing initial solution..." << std::endl; }
    auto xn = backsub(LU, forwsub(LU, bw));
    if constexpr (showProcesses) { std::cout << "Complete!\n" << std::endl; }

    std::cout << "#   " << std::setw(COLWIDTH) << "||x - xn|| " << "\t" << std::setw(COLWIDTH) << " Normwise Backward Error " << '\n';
    std::cout << "------------------------------------------------------------------" << '\n';

    size_t niters = 0;
    bool stop = false;
    bool diverge = false;
    // auto maxnorm = (x - xn).infnorm();
    while (!stop) { //  && !(diverge)){
        niters += 1;
        Vh xh(xn);
        r = b - Ah * xh;
        Vw rn(r);
        auto c = backsub(LU, forwsub(LU, rn));
        xn += c;
        auto maxnorm = (x - xn).infnorm(); // nbe(A,xn,bw); 
        if ((nbe(A, xn, bw) < u_W) || (maxnorm < u_W) || (niters > MAXIT) || diverge) {  // 
            // Stop Crit
            // (nbe(A,xn,bw) < n*u_W)
            // (maxnorm < 1e-7)
            // (maxnorm/x.infnorm() < n*u_W)
            // forward error maxnorm/x.infnorm()
            stop = true;
        }

        // Print Results
        std::cout << std::setw(4) << niters << std::setw(COLWIDTH) << maxnorm << std::setw(COLWIDTH) << nbe(A, xn, bw) << '\n';
        if ((maxnorm > 1e+2)) { diverge = true; }
    } //wend
     /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~ WEND ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */


    /**
     * ********************************************************************
     * Print solution vector
     * ********************************************************************
     */
    if (diverge) {
        std::cout << "------------------------------------------------------------------" << '\n';
        std::cout << "Solution Diverged after " << niters << " iterations." << '\n';
    }
    else {
        //std::cout << std::setw(4) << niters << std::setw(COLWIDTH)  << std::setw(COLWIDTH) << nbe(A,xn,bw) <<'\n';
        std::cout << "------------------------------------------------------------------" << '\n';
        std::cout << "Solution Converged after " << niters << " iterations." << '\n';
        std::cout << " " << '\n';
        std::cout << "------------------------------------------------------------------\n" << std::endl;
        std::cout << "Showing first few elements of solution vector...\n" << std::endl;
        std::cout << "x (approx)" << std::setw(COLWIDTH) << "x (exact)" << '\n';
        std::cout << "------------------------------------------------" << '\n';
        size_t z = (n < 10) ? n : 10;
        for (size_t i = 0; i < z; ++i) {
            std::cout << xn(i) << std::setw(COLWIDTH) << X(i) << '\n';
        }
    }
    if constexpr (showSol) {
        xn.disp();
    }

    /**
     * Print Summary of Experiment to File for Analysis
     */
    if constexpr (write2file) {
        std::ofstream resultsFile("results.txt", std::ios_base::app); // Create & open
        auto const empty_pos = 0; //resultsFile.tellp();
        if (resultsFile.tellp() == empty_pos) {
            resultsFile << "Matrix \t Algo \t NumIts \t Error  \n";
            resultsFile << "----------------------------------------------------\n";
        }
        resultsFile << testMatrix << "\t" <<
            algo << "\t" <<
            niters << "\t\t" <<
            (x - xn).infnorm() << " \n";
        resultsFile.close();                          // Close the file
    }

    int nrOfFailedTestCases = 0;
    return nrOfFailedTestCases;
}

int main(int argc, char* argv[])
try {
    using namespace sw::universal;
    using namespace sw::universal::blas;

    std::string testMatrix = std::string("q4");
    std::streamsize old_precision = std::cout.precision();
    std::streamsize new_precision = 7;
    std::cout << std::setprecision(new_precision);
    
    //RoundAndReplace<fp64, fp32, fp16>(algo, testMatrix);

    // we want to create a table of results for the different low precision types

    matrix<double> Mt = getTestMatrix(testMatrix);

    RunRoundAndReplaceExperiment<fp64, fp32, fp16>(Mt);


    std::cout << std::setprecision(old_precision);
    return EXIT_SUCCESS;
}
catch (char const* msg) {
    std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
    return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
    std::cerr << "Caught unexpected universal arithmetic exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
    std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
catch (std::runtime_error& err) {
    std::cerr << "Caught unexpected runtime error: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
catch (...) {
    std::cerr << "Caught unknown exception" << std::endl;
    return EXIT_FAILURE;
}

