/* ************************************************************************
* LUIR: Iterative Refinement with POSITS
*    Addresses fundamental important problem of solving Ax = b.
*      
* Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
* Author: James Quinlan
* Modified: 2022-12-13 (see history)
* 
* This file is part of the universal numbers project, 
* which is released under an MIT Open Source license.
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

#include "configs.hpp"

const char* msg = "\n\n";

// input ALGO
int main(int argc, char* argv[]) {
try {
	using namespace sw::universal;
	using namespace sw::universal::blas;

    
    if (argc != 3) {
		std::cerr << "Not enough input arguments.\n";
		std::cerr << "Usage:   % ./numeric_luir algo testMatrix\n";
		std::cerr << "Example: % ./numeric_luir 21 steam3\n";
		return EXIT_SUCCESS;  // signal successful completion for ctest
    }

    // -----------------------------------------------------//
	int algo = atoi(argv[1]);
    // if constexpr (showAlgo){std::cout << "Algo = " << algo << ".\n\n";}
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
        #endif
        // std::cout << "Dynamic range " << dynamic_range<LowPrecision>() << '\n';
       
        // Unit Round-off
        std::cout << "Eps Low Precision      = " << u_L << std::endl;
        std::cout << "Eps Working Precision  = " << u_W << std::endl;
        std::cout << "Eps High Precision     = " << u_H << std::endl;
        std::cout << "Eps Test: 1 + u_L      = " << 1+u_L << " vs. " << 1 + u_L/2 << std::endl;
    } 

    // Read Matrix
    Mw A = getTestMatrix(testMatrix);
    
    Ml Al; // Declare low precision matrix to store A
    unsigned n = num_cols(A);

    // std::cout << "random uniform = " << uniform_random_vector<WorkingPrecision>(5,0,1) << std::endl;


    // A's Meta data
    if constexpr (showAmax){std::cout << "(min(A), max(A)) = (" << minelement(A) << ", " << maxelement(A) << ")" << std::endl;}
    if constexpr (print){std::cout << "A = \n" << A << std::endl;}
    if constexpr (showCond){std::cout << "Condition Number = " << kappa(testMatrix) << std::endl;}
    if constexpr (showCondest){std::cout << "Condition estimate: " << condest(A) << std::endl;}
    if constexpr (showSize){std::cout << "Size: (" << n << ", " << n  << ")\n" << std::endl;}

    /* ********************************************************************
    Squeezing Matrix:
        t = theta \in (0,1], is a scaling factor,  
    * *********************************************************************
    */
    WorkingPrecision t = 0.4; // 0.4
    WorkingPrecision mu = 1.0;
    Vw R(num_rows(A),1);  // Row Squeezer
    Vw S(num_rows(A),1);  // Column Squeezer

    // Round, then replace inf (overflow)
    if (algo == 21){ //Round, then replace
        roundReplace(A, Al);
        if constexpr (showAlgo){std::cout << "Algorithm: Round, then replace infinities.\n\n" << std::endl;}
    
    // Scale and Round
    }else if(algo == 22){
        scaleRound<WorkingPrecision, LowPrecision>(A, Al, t, mu);
        if constexpr (showAlgo){std::cout << "Algorithm " << algo << ": Scale, then round.\n\n" << std::endl;}
    
    }else if(algo == 23 || algo == 24 || algo == 25){
        twosideScaleRound<WorkingPrecision, LowPrecision>(A, Al, R, S, t, mu, algo);
        if constexpr (showAlgo){std::cout << "Algorithm " << algo << ": Two-sided squeezing, RAS.\n\n" << std::endl;}
    }else{
        // Do nothing
        Al = A;
        if constexpr (showAlgo){std::cout << "Algorithm " << algo << ": Round only, i.e., A --> A (low).\n\n" << std::endl;}
    }
    // Print Squeezed A and Low Precision Al.  
    if constexpr (print){
        std::cout << "A (modified) = \n" << A << std::endl;
        std::cout << "Al (low precision) = \n" << Al << std::endl;
    }
    // ----------------------------------------------------------------------------



    // LU Factorization of Low Precision
    auto [P, L, U] = plu(Al);
    if constexpr (print){std::cout << "PLU = \n" << P << "\n" << L << "\n" << U << std::endl;}

    // store in working precision
    Mw Pw(P);
    Mw Lw(L);
    Mw Uw(U);
    if constexpr (print){std::cout << "Lw = \n" << Lw << "\n Uw = " <<  Uw << std::endl;}

    // Compute new (permuted) A
    A = Pw*A; 
    if constexpr (print){
        std::cout << "PA = \n" << A << std::endl;
        std::cout << "LU (low precision)= \n" << L*U << std::endl;
        std::cout << "LU (working precision) = \n" << Lw*Uw << std::endl;
    }
    // matvec(b,A,x);  // quire-enabled

    // Create high precision version 
    Mh Ah(A);       // High precision A
    Vh X(n,1);      // X is exact solution = [1, 1, 1, ..., 1]
    Vh b = Ah*X;    // b vector in high precision.
    

    // Store working
    Vw x(X);  
    Vw bw(b); // Note: also try b = P*mu*R*(AX), where A is original matrix.
    
    auto xn = backsub(Uw,forwsub(Lw,bw));

    // Results Header
    std::cout << "#   " << std::setw(COLWIDTH) << "||x - xn|| " << "\t" << std::setw(COLWIDTH)  << " Normwise Backward Error " << '\n'; 
    std::cout << "------------------------------------------------------------------"  << '\n';

    // Iterative Refinement 
    Vh r; // High precision residual vector
    size_t niters = 0;
    bool diverge = false;
    while(((x - xn).norm() > 1e-7) &&  (niters < 10)  && !(diverge)){
        niters += 1;
        // ----------------------------------------------------------- 
        // Residual Calculation (high precision)
        Vh xh(xn);
        r = b - Ah*xh;  // high precision calculation
        Vw rn(r);       // Store in working presicion

        // Solve Ac = r where A = LU (low precision)
        auto c = backsub(Uw,forwsub(Lw,rn));          
        xn += c;  // update solution vector with corrector
            
        // Print Results
        std::cout << std::setw(4) << niters   << std::setw(COLWIDTH) << (x - xn).norm() << std::setw(COLWIDTH)  << nbe(A,xn,bw) << '\n';

        if( ((x - xn).norm() > 1e+12) ){diverge = true;}
    } //wend

    // Print solution vector
    if(diverge){
        std::cout << "------------------------------------------------------------------"  << '\n';
        std::cout << "Solution Diverged after "<< niters << " iterations." << '\n';
    }else{
        std::cout << "------------------------------------------------------------------"  << '\n';
        std::cout << "Showing first few elements of solution vector..." << '\n';
        std::cout << "x = " << '\n';
        for(size_t i=0;i < 5;++i){
            std::cout << xn(i) << '\n';
        }
    }

    if constexpr(showSol){
        xn.disp();
    }

    // ----------------------------------------------------------------------------

	int nrOfFailedTestCases = 0;
    std::cout << std::setprecision(old_precision);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
    
} //try

    // Error messages
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


// Modification History
/* ************************************************************************
2022-11-26 
- Created luir.cpp as posit version of highamir.cpp
***************************************************************************
*/

// References
/* ************************************************************************
Higham, N. J., & Mary, T. (2019). A new preconditioner that exploits 
    low-rank approximations to factorization error. SIAM Journal on 
    Scientific Computing, 41(1), A59-A82.
***************************************************************************
*/ 


// Iterative Refinement 
// ``The general stratagem in numerical calculations is that it is best to compute a 
//   quantity by adding a small correction term to a (previous) approximation." Kincaid-Cheny, p.76.
//   e.g., In Bisection method, the midpoint c is computed as c = a + (b — a)/2 rather than 
//   as c = (a + b)/2 to adhere to the stratagem above.  



// NOTES and out takes while in development.
/* ************************************************************************
The purpose of this code is to solve Ax = b using iterative refinement method.
It uses multi-precision arithmetic, in particular
low precision to factor A = LU.  The method should save both time and space
as low-precision produces lower bit-footprint. Note, factoring 
with Gaussian elimination is O(n^3).  Bottom-line: Arithmetic on low-precision
will increase preformance.

Literature describes the algorithm working with three precisions:
1. Uw - working precision at which the data A, B and solution x are stored.
2. Uf - precision at which the factorization of A = LU is computed (and stored)
3. Ur - precision the residuals are calculated (higher precision)
That is, precision satisfy:
Ur < Uw < Uf 

U* = Unit roundoff, so Ur is small(est) = highest precision. 

The process:
1. Read A at working precision
2. Generate b at working precision, b = A*X where X = ones(n,1).
    For simplicity, we generate b so that the exact solution is x = 1.
3. Cast A and b in low-precision
4. Factor A(low) = LU
5. Solve, x = U \ (L \ b) (thus x is low precision)
6. Cast x to high precision 
7. Calculate r = b - Ax (using quire)
8. Solve, LU d = r
9. x = x + d
10. Goto 7
*/ 

// It factors A into a low-precision LU, then solves for x.
//      This estimate is then refined by solving (LU)d = r = b - Ax, and
//      overwriting x with x + d, that is, x = x + d.  
//      The process is repeated until convergence.  


/*
size_t nr = size(b);
size_t nc = size(x);

for (size_t i = 0; i < nr; ++i) {
	sw::universal::quire<nbits, es> q(0);
	for (size_t j = 0; j < nc; ++j) {
		q += sw::universal::quire_mul(A(i,j), x[j]);
	}
	sw::universal::convert(q.to_value(), b[i]);  
}
b = A*x; where A and x are both posits.


// matnorm
    // std::cout << "||A|| = " << matnorm(Aw) << std::endl;
    // std::cout << "K = " << condest(Aw) << std::endl;


*/

// Print Results
    // std::cout << "L = \n" << L << std::endl;
    // std::cout << "U = \n" << U << std::endl;
    /*
    for(int i = 0;i < 10;++i){
        for(int j = 0; j<10;++j){
            if(L(i,j) != 0){
                // std::cout <<  L(i,j) << std::endl;
            }
        }
    }


    // Factor A = LU
    // auto [Pl, Ll, Ul] = plu(Al);  // Factor low precision
    // Mw L(Ll), U(Ul);              // Coerce to working precision


    */