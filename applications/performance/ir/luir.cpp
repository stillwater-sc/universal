// LUIR: A = LU Iterative Refinement
//
//    Addresses the fundamental problem of solving Ax = b efficiently.
//     
// @author:     James Quinlan
// Copyright(c) 2017 James Quinlan
// SPDX-License-Identifier: MIT 
//
// @date : 2024 - 03 - 17
//
// This file is part of the Mixed Precision Iterative Refinement project.

// Environmental Configurations
#include<universal/utility/directives.hpp>
#include<universal/utility/long_double.hpp>
#include<universal/utility/bit_cast.hpp>

// Universal Number System Types
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit/posit.hpp>

// Higher Order Libraries
#include <blas/blas.hpp>
//#include <universal/blas/solvers/plu.hpp>
#include <blas/ext/solvers/posit_fused_backsub.hpp>
#include <blas/ext/solvers/posit_fused_forwsub.hpp>
#include <blas/utes/matnorm.hpp>
#include <blas/utes/condest.hpp>
#include <blas/utes/nbe.hpp>      // Normwise Backward Error

// Support Packages

#include <blas/squeeze.hpp>
//#include "utils/isdiagdom.hpp"

// Matrix Test Suite
#include <blas/matrices/testsuite.hpp>
//#include <luir/matrices/testsuite.hpp>  // local version

// File I/O
#include <iostream>
#include <fstream>

// Local Configuration
#include "configs.hpp"
//#include "sjm.hpp"
//#include<luir/squeeze.hpp>
// #include "../mtl4/utils/print_utils.hpp"

// Build Directory
// ../mixed-precision/build/src/luir

namespace sw {
    namespace universal {
        namespace blas {

            /// <summary>
            ///  dense matrix LU with partial pivoting (PA = LU) decomposition via DooLittle Method (in place)
            /// </summary>
            /// <typeparam name="Scalar"></typeparam>
            /// <param name="A">dense matrix to factor</param>
            /// <param name="P">associated permutation matrix</param>
            template<typename Scalar>
            void plu(matrix<Scalar>& A, matrix<size_t>& P) {
                Scalar x;
                size_t n = num_rows(A);
                for (size_t i = 0; i < n - 1; ++i) { // i-th row
                    P(i, 0) = i;
                    P(i, 1) = i;

                    Scalar absmax = abs(A(i, i));
                    size_t argmax = i;

                    // Select k >= i to maximize |U(k,i)| 
                    for (size_t k = i + 1; k < n; ++k) {
                        if (abs(A(k, i)) > absmax) {
                            absmax = abs(A(k, i));
                            argmax = k;
                        }
                    }

                    // Check for necessary swaps
                    if (argmax != i) {
                        P(i, 1) = argmax;
                        for (size_t j = 0; j < n; ++j) {  // j = i originally
                            x = A(i, j);
                            A(i, j) = A(argmax, j);
                            A(argmax, j) = x;
                        }
                    }

                    // Continue with row reduction
                    for (size_t k = i + 1; k < n; ++k) {  // objective row
                        A(k, i) = A(k, i) / A(i, i);
                        for (size_t j = i + 1; j < n; ++j) {
                            A(k, j) = A(k, j) - A(k, i) * A(i, j);
                        }
                    } // update L
                }
            }
        } // blas sub-namespace

        template<typename Working, typename Low>
        void roundReplace(blas::matrix<Working>& A, blas::matrix<Low>& Al, unsigned n) {
            // round then replace infinities
            Al = A;
            Low maxpos(SpecificValue::maxpos);
            for (unsigned i = 0; i < n; ++i) {
                for (unsigned j = 0; j < n; ++j) {
                    Low sgn = (Al(i, j) > 0) ? 1 : ((Al(i, j) < 0) ? -1 : 0);
                    if (isinf(abs(Al(i, j)))) {
                        Al(i, j) = sgn * (maxpos);
                    }
                }
            }
        } // Round and Replace


        template<typename Working, typename Low>
        void scaleRound(blas::matrix<Working>& A,
            blas::matrix<Low>& Al,
            Working T,
            Working& mu,
            unsigned algo) {
            /* Algo 22:  scale by scalar, then round */
            Working Amax = maxelement(A);
            Low xmax(SpecificValue::maxpos);
            Working Xmax(xmax);

#define CFLOAT 0   // 0 = POSITS
            // /** 
#if CFLOAT 
            mu = (T * Xmax) / Amax;  // use for cfloats
#else
            mu = T / Amax;  // use for posits
#endif

            A = mu * A;  // Scale A
            Al = A;    // Round A = fl(A)
            // std::cout << "Al (after scaling)  = \n" << Al << std::endl;
            // std::cout << "--------------------------------------------" << std::endl;
            // std::cout << Xmax << "\t" << Amax << "\t  \t" << T << "\t" << mu << "\n" << std::endl;
            /*
            std::cout << "A (after scaling)  = \n" << A << std::endl;
            std::cout << "Xmax \t  Amax \t      T \t     mu " << std::endl;
            std::cout << "--------------------------------------------" << std::endl;
            std::cout << Xmax << "\t" << Amax << "\t  \t" << T << "\t" << mu << "\n" << std::endl;
            */
        } // Scale and Round

        /**
 * ***********************************************************************
 * Helper functions
 *  - row/column scaling
 *  - generate matrices R and S (see Higham)
 * ***********************************************************************
 */

        template<typename Scalar>
        void getR(blas::matrix<Scalar>& A, blas::vector<Scalar>& R, unsigned& n) {
            Scalar M;
            for (unsigned i = 0; i < n; ++i) {
                M = 0;
                for (unsigned j = 0; j < n; ++j) {
                    M = (abs(A(i, j)) > M) ? abs(A(i, j)) : M;
                }
                R(i) = 1 / M;
            }
        } // Get Row scaler

        template<typename Scalar>
        void getS(blas::matrix<Scalar>& A, blas::vector<Scalar>& S, unsigned& n) {
            Scalar M;
            for (unsigned j = 0; j < n; ++j) {
                M = 0;
                for (unsigned i = 0; i < n; ++i) {
                    M = (abs(A(i, j)) > M) ? abs(A(i, j)) : M;
                }
                S(j) = 1 / M;
            }
        } // Get Column scaler


        template<typename Scalar>
        void rowScale(blas::vector<Scalar>& R, blas::matrix<Scalar>& A, unsigned& n) {
            for (unsigned i = 0; i < n; ++i) {
                for (unsigned j = 0; j < n; ++j) {
                    A(i, j) = R(i) * A(i, j);
                }
            }
        } // Scale Rows of A

        template<typename Scalar>
        void colScale(blas::matrix<Scalar>& A, blas::vector<Scalar>& S, unsigned& n) {
            for (unsigned j = 0; j < n; ++j) {
                for (unsigned i = 0; i < n; ++i) {
                    A(i, j) = S(j) * A(i, j);
                }
            }
        } // Scale Columns of A

        template<typename Working, typename Low>
        void twosideScaleRound (blas::matrix<Working>& A,
                                blas::matrix<Low>& Al,
                                blas::vector<Working>& R,
                                blas::vector<Working>& S,
                                Working T,
                                Working& mu,
                                unsigned& n,
                                size_t algo = 24) {

            if (algo == 24) { xyyEQU(R, A, S, n); }
            if (algo == 25) {
                // nothing here to see
            }
            scaleRound(A, Al, T, mu, algo);
            /* Algo 23: general two-sided scaling, then round*/
            /*
            Low xmax(SpecificValue::maxpos);
            Working Xmax(xmax);
            Working beta = maxelement(A);
            Working mu = (T*Xmax) / beta;
            A = mu*A;   // Scale A
            B = A;     // Round
            */
            // std::cout << Xmax << "\t" << beta << "\t" << T << "\t" << mu << "\n" << std::endl;
        } // Two-sided Scale and Round


        template<typename Scalar>
        void xyyEQU(blas::vector<Scalar>& R,
                    blas::matrix<Scalar>& A,
                    blas::vector<Scalar>& S,
                    unsigned& n) {
            /* Algo 24: construct R and S */
            /* Algo 24: row and column equilibration */
            bool print = false;

            getR(A, R, n);          // Lines:1-4
            if (print) { std::cout << "R = \n" << R << std::endl; }

            rowScale(R, A, n);      // Line: 5,  A is row equilibrated
            if (print) { std::cout << "RA = \n" << A << std::endl; }

            getS(A, S, n);          // Lines: 6 - 9
            if (print) { std::cout << "S = \n" << S << std::endl; }

            colScale(A, S, n);
            if (print) { std::cout << "RAS = \n" << A << std::endl; }
        } // Construct R and S

    }
}

int main(int argc, char* argv[]) {
try {
    using namespace sw::universal;
    using namespace sw::universal::blas;

    // CLI Input Parser
    // const char* msg = "\n\n";
    if (argc != 3) {
        std::cerr << "Not enough input arguments.\n";
        std::cerr << "Usage:   % ./luir algo testMatrix\n";
        std::cerr << "Example: % ./luir 21 steam3\n";
        std::cerr << "Target matrix options are:\n\
                        lambers_well  // 2 x 2 well-conditioned matrix, K = \n\
                        lambers_ill   // 2 x 2 ill-conditioned matrix, K = \n\
                        h3            // 3 x 3 test matrix, K = \n\
                        q3            // 3 x 3 Variable test matrix (edit entries) \n\
                        int3          // 3 x 3 integer test matrix (low condition number), K = \n\
                        faires74x3    // 3 x 3 Burden Faires Ill-conditioned, K = \n\
                        q4            // 4 x 4 test matrix, K = \n\
                        q5            // 4 x 4 test matrix, K = \n\
                        lu4           // 4 x 4 test matrix, K = \n\
                        s4            // 4 x 4 test matrix, K = \n\
                        rand4         // 4 x 4 random (low condition), K = \n\
                        west0132      // 132 x 132 Chem. Simulation Process, K =  \n\
                        west0167      // 167 x 167 Chemical Simulation Process, K =    \n\
                        west0479      // 479 x 479 Chemical Simulation Process, K =   \n\
                        steam1        // 240 x 240 Computational Fluid Dynamics, K =    \n\
                        steam3        //  83 x 83  Computational Fluid Dynamics, K =   \n\
                        fs_183_1      // 183 x 183 2D/3D Problem Sequence, K =   \n\
                        fs_183_3      // 183 x 183 2D/3D Problem Sequence, K =    \n\
                        bwm200        // 200 x 200 Chem. simulation K = 1e3.\n\
                        gre_343       // 343 x 343 Directed Weighted Graph, K = \n\
                        b1_ss         // 7x7 Chemical Process Simulation Problem, K = \n\
                        cage3         // 5 x 5 Directed Weighted Graph, K =   \n\
                        pores_1       // 30 x 30 Computational Fluid Dynamics, K = \n\
                        Stranke94     // 10 x 10 Undirected Weighted Graph, K = \n\
                        Trefethen_20  // 20 x 20 Combinatorial Problem, K = \n\
                        bcsstk01      // 48 x 48 Structural Engineering, K = \n\
                        bcsstk03      // 112 x 112 Structural Engineering, K = \n\
                        bcsstk04      // 132 x 132 Structural Engineering, K = \n\
                        bcsstk05      // 153 x 153 Structural Engineering, K = \n\
                        bcsstk22      // 138 x 138 Structural Engineering, K = \n\
                        lund_a        // 147 x 147 Structural Engineering, K =   \n\
                        nos1          // 237 x 237 Structural Engineering K = 1e7  \n\
                        arc130        //    \n\
                        saylr1        // 238 x 238 Computational Fluid Dynamics, K = \n\
                        tumorAntiAngiogenesis_2      // , K =\n";
        return EXIT_SUCCESS;  // signal successful completion for ctest
    }
    int algo = atoi(argv[1]);
    std::string testMatrix = std::string(argv[2]);
    std::streamsize old_precision = std::cout.precision();
    std::streamsize new_precision = 7;
    std::cout << std::setprecision(new_precision);
    

    /**
     * Precision Templates
     * Set the precision of the experiment
     * - HighPrecision: 64-bit posit or float
     * - WorkingPrecision: 32-bit posit or float
     * - LowPrecision: 16-bit posit or float
    */
    //#define CFLOAT 1   // 0 = POSITS (see also squeeze.hpp)
    // /** 
    #if CFLOAT 
        using WorkingPrecision  = cfloat<wbits,wes,uint32_t, true, false, false>;
        using LowPrecision      = cfloat<lbits,les,uint32_t, true, false, false>;
        using HighPrecision     = cfloat<hbits,hes,uint32_t, true, false, false>;
    // */ 
    #else
        using WorkingPrecision  = posit<wbits,wes>;
        using LowPrecision      = posit<lbits,les>;
        using HighPrecision     = posit<hbits,hes>;
    #endif


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

    // View Numerical Properties of Configuration
    if constexpr (showNumProps){
        LowPrecision m, M;
        m.minpos();
        M.maxpos();
        
        // std::cout << symmetry_range(LowPrecision()) << "\n";
        #if CFLOAT
            std::cout << "Dynamic range fp<" << lbits << "," << les << "> = (" << m << ", " << M << ")" << std::endl;
        #else
            std::cout << "Dynamic range posit<" << lbits << "," << les << "> = (" << m << ", " << M << ")" << std::endl;
        #endif
        
        // Unit Round-off
        LowPrecision oneThird = 1.0/3.0;
        std::cout << "Nearest Value to 1/3   = " << oneThird << std::endl;
        std::cout << "Eps Low Precision      = " << u_L << std::endl;
        std::cout << "Eps Working Precision  = " << u_W << std::endl;
        std::cout << "Eps High Precision     = " << u_H << std::endl;
        std::cout << "Eps Test: 1 + u_L      = " << 1+u_L << " vs. " << 1 + u_L/2 << std::endl;
        std::cout << "------------------------------------------------------------------------"  << "\n\n";
    } 

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
    if constexpr (showNumProps){std::cout << "Largest Consec. Int = " << std::pow(2,4*(int(lbits) - 3)/5) << std::endl;}
    if constexpr (showAmax){std::cout << "(min(A), max(A)) = (" << minelement(A) << ", " << maxelement(A) << ")" << std::endl;}
    if constexpr (printMat){disp(A);} // std::cout << "A = \n" << A << std::endl;
    if constexpr (showCond){std::cout << "Condition Number = " << kappa(testMatrix) << std::endl;}
    if constexpr (showCondest){std::cout << "Condition estimate: " << condest(A) << std::endl;}
    if constexpr (showSize){std::cout << "Size: (" << n << ", " << n  << ")" << std::endl;}
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
    if constexpr (showProcesses){std::cout << "Process: Start Squeezing..." << std::endl;}
    WorkingPrecision t =  0.1; //2949990 Is there an optimal value?  Parameter sweep 0.75 west
    WorkingPrecision mu = 1.0;  // 16 best for posit<x,2>
    Vw R(num_rows(A),1);  // Row Squeezer
    Vw S(num_rows(A),1);  // Column Squeezer

    std::cout << "Working precision: " << type_tag(WorkingPrecision()) << "\n";
    if (algo == 21){ // Round, then replace inf (overflow) 
        roundReplace(A, Al, n);
        if constexpr (showAlgo){std::cout << "Algorithm: Round, then replace infinities." << std::endl;}
    }else if(algo == 22){ // Scale and Round
        scaleRound<WorkingPrecision, LowPrecision>(A, Al, t, mu, algo);
        if constexpr (showAlgo){std::cout << "Algorithm " << algo << ": Scale, then round." << std::endl;}
    }else if(algo == 23 || algo == 24 || algo == 25){
        twosideScaleRound<WorkingPrecision, LowPrecision>(A, Al, R, S, t, mu, n, algo);
        if constexpr (showAlgo){std::cout << "Algorithm " << algo << ": Two-sided squeezing, RAS." << std::endl;}
    }else{ // Do nothing
        Al = A;
        if constexpr (showAlgo){std::cout << "Algorithm " << algo << ": Round only, i.e., A --> A (low).\n" << std::endl;}
    }
    // Print Squeezed A and Low Precision Al.  
    if constexpr (printMat){
        std::cout << "A (modified) = \n"; disp(A);// << A << std::endl;
        std::cout << "Al (low precision) = \n";disp(Al); // << Al << std::endl;
    }
    if constexpr (showProcesses){std::cout << "Squeezing Complete!\n" << std::endl;}
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
    sw::universal::blas::matrix<size_t> P(n,2); // check the size.
    // plu only uses 0,1,2,...,n-2 (e.g., n=10, then 0,1,2,...,8)
    // since there is no need to pivot last row.  See plu. 
    if constexpr (showProcesses){std::cout << "Process: Factoring (PLU)..." << std::endl;}
    plu(Al, P);
    Mw LU(Al);
    if constexpr (showProcesses){std::cout << "Complete!\n" << std::endl;}
    if constexpr (printLU){
        std::cout << "LU = \n"; disp(LU); 
    }
    // Compute A = P*A;
    if constexpr (showProcesses){std::cout << "Process: computing PA..." << std::endl;}
    for (size_t ii = 0; ii < n; ++ii){
        if(P(ii,0) != P(ii,1)){
            for (size_t jj = 0; jj < n; ++jj){
                auto aij = A(P(ii,0),jj);
                A(P(ii,0),jj) = A(P(ii,1),jj);
                A(P(ii,1),jj) = aij;
            }
        }
    }
    if constexpr (showProcesses){std::cout << "Complete!\n" << std::endl;}
    if constexpr (printPA){
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
    Vh X(n,1);
    if (randsol){
        X = uniform_random_vector<HighPrecision>(n);
    }
    Vh b = Ah*X;  // mu*R*b
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
    if constexpr (showProcesses){std::cout << "Process: computing initial solution..." << std::endl;}
    auto xn = backsub(LU,forwsub(LU,bw));
    if constexpr (showProcesses){std::cout << "Complete!\n" << std::endl;}

    std::cout << "#   " << std::setw(COLWIDTH) << "||x - xn|| " << "\t" << std::setw(COLWIDTH)  << " Normwise Backward Error " << '\n'; 
    std::cout << "------------------------------------------------------------------"  << '\n';
    
    size_t niters = 0;
    bool stop = false;
    bool diverge = false;
    // auto maxnorm = (x - xn).infnorm();
    while (!stop){ //  && !(diverge)){
        niters += 1;
        Vh xh(xn);
        r = b - Ah*xh;   
        Vw rn(r);        
        auto c = backsub(LU,forwsub(LU,rn));         
        xn += c;
        auto maxnorm = (x - xn).infnorm(); // nbe(A,xn,bw); 
        if ((nbe(A,xn,bw) < u_W) || (maxnorm < u_W) || (niters > MAXIT) || diverge) {  // 
            // Stop Crit
            // (nbe(A,xn,bw) < n*u_W)
            // (maxnorm < 1e-7)
            // (maxnorm/x.infnorm() < n*u_W)
            // forward error maxnorm/x.infnorm()
            stop = true;  
        }
        
        // Print Results
        std::cout << std::setw(4) << niters << std::setw(COLWIDTH) << maxnorm << std::setw(COLWIDTH) << nbe(A,xn,bw) <<'\n';
        if ((maxnorm > 1e+2)){diverge = true;}
    } //wend
     /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~ WEND ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */


    /**
     * ********************************************************************
     * Print solution vector
     * ********************************************************************
     */
    if(diverge){
        std::cout << "------------------------------------------------------------------"  << '\n';
        std::cout << "Solution Diverged after "<< niters << " iterations." << '\n';
    }else{
        //std::cout << std::setw(4) << niters << std::setw(COLWIDTH)  << std::setw(COLWIDTH) << nbe(A,xn,bw) <<'\n';
        std::cout << "------------------------------------------------------------------"  << '\n';
        std::cout << "Solution Converged after "<< niters << " iterations." << '\n';
        std::cout << " "  << '\n';
        std::cout << "------------------------------------------------------------------\n"  << std::endl;
        std::cout << "Showing first few elements of solution vector...\n" << std::endl;
        std::cout << "x (approx)" << std::setw(COLWIDTH) << "x (exact)" << '\n';
        std::cout << "------------------------------------------------"  << '\n';
        size_t z = (n < 10) ? n : 10;
        for(size_t i=0;i < z;++i){
            std::cout << xn(i) << std::setw(COLWIDTH) << X(i) << '\n';
        }
    }
    if constexpr(showSol){
        xn.disp();
    }
    
    /**
     * Print Summary of Experiment to File for Analysis
     */
    if constexpr(write2file){
        std::ofstream resultsFile("results.txt", std::ios_base::app); // Create & open
        auto const empty_pos = 0; //resultsFile.tellp();
        if (resultsFile.tellp() == empty_pos){
            resultsFile << "Matrix \t Algo \t NumIts \t Error  \n";
            resultsFile << "----------------------------------------------------\n";
        }
        resultsFile <<  testMatrix << "\t" << 
                        algo << "\t" << 
                        niters << "\t\t" <<  
                        (x - xn).infnorm() << " \n";
        resultsFile.close();                          // Close the file
    }

    int nrOfFailedTestCases = 0;
    std::cout << std::setprecision(old_precision);
    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
    
}   // try
    /**
     * Catch error messages
     */
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
} // main


/*
Map to highest precision bits (not 0,1).
p.setbits(0x5fff)/Amax


pattern is 0.10.111111111....1111
0101
p.setbits(0x5fff)
pattern is 0.01.00000..0000
theodore omtzigt5:49 PM
top nibble is 0010 = 2
p.setbits(0x2000)

If posit20, 0x5ffff, and then 0x20000 
posit28, 0x5ffffff and 0x2000000


std::numeric_limits<posit<16,2>>::epsilon()

or 

ulp(p) = |p* - p|
ulp(c) for c = cfloat
 

*/