/** ************************************************************************
* LUIR: A = LU Iterative Refinement with POSITS
*    Addresses fundamental important problem of solving Ax = b.
*      
*  @author:     James Quinlan
 * @date:       2022-12-13
 * @copyright:  Copyright (c) 2022 Stillwater Supercomputing, Inc.
 * @license:    MIT Open Source license 
* 
* This file is part of the Universal Number Library project. 
* *************************************************************************
*/
// Build Directory = universal/build/applications/numeric

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

// Matrix Test Suite
#include <universal/blas/matrices/testsuite.hpp>

// File I/O
#include <iostream>
#include <fstream>

// Local Configuration
#include "configs.hpp"


int main(int argc, char* argv[]) {
try {
	using namespace sw::universal;
	using namespace sw::universal::blas;

    // CLI Input Parser
    // const char* msg = "\n\n";
    if (argc != 3) {
		std::cerr << "Not enough input arguments.\n";
		std::cerr << "Usage:   % ./numeric_luir algo testMatrix\n";
		std::cerr << "Example: % ./numeric_luir 21 steam3\n";
		return EXIT_SUCCESS;  // signal successful completion for ctest
    }
	int algo = atoi(argv[1]);
    std::string testMatrix = std::string(argv[2]);
    std::streamsize old_precision = std::cout.precision();
    std::streamsize new_precision = 7;
    std::cout << std::setprecision(new_precision);
    

    // Write Configurations
    /*
    std::ofstream MyFile("configs_highamir.txt", std::ios_base::app); // Create and open a text file
    MyFile << "-------------------------\n";
    MyFile << "Algo = \t" << algo << "  \n";  // Write to the file
    MyFile << "--------------------------\n\n";
    MyFile << "(hbits, hes) = (" << hbits << ", " << hes << ") \n";
    MyFile << "(wbits, wes) = (" << wbits << ", " << wes << ") \n";
    MyFile << "(lbits, les) = (" << lbits << ", " << les << ") \n";
    MyFile.close();                          // Close the file
    */

    // Precision Templates
    #define CFLOAT 0   // 0 = POSITS (see also squeeze.hpp)
    // /** 
    #if CFLOAT 
        using WorkingPrecision  = cfloat<wbits,wes,uint32_t, true, false, false>;
        using LowPrecision      = cfloat<lbits,les,uint32_t, true, false, false>;
        using HighPrecision     =  cfloat<hbits,hes,uint32_t, true, false, false>;
    // */ 
    #else
        using WorkingPrecision  = posit<wbits,wes>;
        using LowPrecision      = posit<lbits,les>;
        using HighPrecision     = posit<hbits,hes>;
    #endif

    // Matrix and Vector Type alias
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
        #if CFLOAT
            std::cout << "Dynamic range fp<" << lbits << "," << les << "> = (" << m << ", " << M << ")" << std::endl;
        #else
            std::cout << "Dynamic range posit<" << lbits << "," << les << "> = (" << m << ", " << M << ")" << std::endl;
            // std::cout << "Dynamic range " << dynamic_range<LowPrecision>() << '\n';
        #endif
        
        // Unit Round-off
        std::cout << "Eps Low Precision      = " << u_L << std::endl;
        std::cout << "Eps Working Precision  = " << u_W << std::endl;
        std::cout << "Eps High Precision     = " << u_H << std::endl;
        std::cout << "Eps Test: 1 + u_L      = " << 1+u_L << " vs. " << 1 + u_L/2 << std::endl;
    } 

    /** ********************************************************************
     *  Read Matrix, then store a low precision version Al
     *  - Display Metadata for A (per request, see configs.hpp)
     * *********************************************************************
    */
    Mw A = getTestMatrix(testMatrix);
    Ml Al;
    unsigned n = num_cols(A);

    if constexpr (showAmax){std::cout << "(min(A), max(A)) = (" << minelement(A) << ", " << maxelement(A) << ")" << std::endl;}
    if constexpr (print){std::cout << "A = \n" << A << std::endl;}
    if constexpr (showCond){std::cout << "Condition Number = " << kappa(testMatrix) << std::endl;}
    if constexpr (showCondest){std::cout << "Condition estimate: " << condest(A) << std::endl;}
    if constexpr (showSize){std::cout << "Size: (" << n << ", " << n  << ")\n" << std::endl;}

    
    /** ********************************************************************
     *   Squeeze Matrix:
     *   t = theta \in (0,1], is a scaling factor,  
     * *********************************************************************
    */
    if constexpr (showProcesses){std::cout << "Process: Start Squeezing..." << std::endl;}
    WorkingPrecision t = 0.4; // 0.4
    WorkingPrecision mu = 1.0;
    Vw R(num_rows(A),1);  // Row Squeezer
    Vw S(num_rows(A),1);  // Column Squeezer

    if (algo == 21){ // Round, then replace inf (overflow) 
        roundReplace(A, Al, n);
        if constexpr (showAlgo){std::cout << "Algorithm: Round, then replace infinities." << std::endl;}
    }else if(algo == 22){ // Scale and Round
        scaleRound<WorkingPrecision, LowPrecision>(A, Al, t, mu);
        if constexpr (showAlgo){std::cout << "Algorithm " << algo << ": Scale, then round." << std::endl;}
    }else if(algo == 23 || algo == 24 || algo == 25){
        twosideScaleRound<WorkingPrecision, LowPrecision>(A, Al, R, S, t, mu, n, algo);
        if constexpr (showAlgo){std::cout << "Algorithm " << algo << ": Two-sided squeezing, RAS." << std::endl;}
    }else{ // Do nothing
        Al = A;
        if constexpr (showAlgo){std::cout << "Algorithm " << algo << ": Round only, i.e., A --> A (low).\n" << std::endl;}
    }
    // Print Squeezed A and Low Precision Al.  
    if constexpr (print){
        std::cout << "A (modified) = \n" << A << std::endl;
        std::cout << "Al (low precision) = \n" << Al << std::endl;
    }
    if constexpr (showProcesses){std::cout << "Squeezing Complete!\n" << std::endl;}


    /** ********************************************************************
     *  LU Factorization of Low Precision Matrix (key step)
     *  : A is factored into LU using low precision.
     *  : LU is then stored in working precision (note permuations included)
     *  : A = P*A is computed & stored in high precision for residual calc.
     * *********************************************************************
    */
    sw::universal::blas::matrix<size_t> P(n-1,2);
    if constexpr (showProcesses){std::cout << "Process: Factoring (PLU)..." << std::endl;}
    plu(Al, P, n);
    Mw LU(Al);
    if constexpr (showProcesses){std::cout << "Complete!\n" << std::endl;}

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
    if constexpr (print){
        std::cout << "PA = \n" << A << std::endl;
    }
    Mh Ah(A);


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
    Vh b = Ah*X;
    Vw x(X);  
    Vw bw(b);   // Note: also try b = P*mu*R*(AX), where A is original matrix.
    Vh r;        


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
    auto xn = backsub(LU,forwsub(LU,bw,n),n);
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
        auto c = backsub(LU,forwsub(LU,rn,n),n);         
        xn += c;
        auto maxnorm = (x - xn).infnorm();
        if ((maxnorm < 1e-7) || (niters > 20) || diverge) { //  && !(diverge)){ 
            stop = true;  
            //std::cout << "max norm > 1e-7 true"  << std::endl;
        }
        
        // Print Results
        std::cout << std::setw(4) << niters << std::setw(COLWIDTH) << maxnorm << std::setw(COLWIDTH) << nbe(A,xn,bw) <<'\n';
        if ((maxnorm > 1e+5)){diverge = true;}
    } //wend


    /**
     * Print solution vector
     */
    if(diverge){
        std::cout << "------------------------------------------------------------------"  << '\n';
        std::cout << "Solution Diverged after "<< niters << " iterations." << '\n';
    }else{
        std::cout << "------------------------------------------------------------------\n"  << std::endl;
        std::cout << "Showing first few elements of solution vector...\n" << std::endl;
        std::cout << "x (approx)" << std::setw(COLWIDTH) << "x (exact)" << '\n';
        std::cout << "------------------------------------------------"  << '\n';
        for(size_t i=0;i < 5;++i){
            std::cout << xn(i) << std::setw(COLWIDTH) << X(i) << '\n';
        }
    }
    if constexpr(showSol){
        xn.disp();
    }
	int nrOfFailedTestCases = 0;
    std::cout << std::setprecision(old_precision);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
    
} // try

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