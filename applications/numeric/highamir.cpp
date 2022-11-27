// ************************************************************************
// highamir: Iterative Refinement following Higham (see references)
//    Addresses fundamental important problem of solving Ax = b.
//      
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
// Author: James Quinlan
// Modified: 2022-11-05 (see history)
// 
// This file is part of the universal numbers project, 
// which is released under an MIT Open Source license.

// ************************************************************************
// Build Directory
// universal/build/applications/numeric

// Environmental Configurations
#include<universal/utility/directives.hpp>
#include<universal/utility/long_double.hpp>
#include<universal/utility/bit_cast.hpp>
// #include <universal/utility/number_system_properties.hpp>

#define CFLOAT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/cfloat/cfloat.hpp>

// Higher Order Libraries
#include <universal/blas/vector.hpp>
#include <universal/blas/matrix.hpp>
#include <universal/blas/blas.hpp>
#include <universal/blas/utes/matnorm.hpp>
#include <universal/blas/utes/condest.hpp>

// Support Packages
#include <universal/blas/solvers/luq.hpp>
#include <universal/blas/solvers/plu.hpp>
#include <universal/blas/solvers/backsub.hpp>
#include <universal/blas/solvers/forwsub.hpp>
#include <universal/blas/squeeze.hpp>
// #include <universal/blas/nnz.hpp>

// Matrix Test Suite
#include <universal/blas/matrices/h3.hpp>            // 3 x 3 test matrix
#include <universal/blas/matrices/q3.hpp>            // 3 x 3 test matrix
#include <universal/blas/matrices/q4.hpp>            // 4 x 4 test matrix
#include <universal/blas/matrices/q5.hpp>            // 4 x 4 test matrix
#include <universal/blas/matrices/lu4.hpp>           // 4 x 4 test matrix
#include <universal/blas/matrices/s4.hpp>            // 4 x 4 test matrix
#include <universal/blas/matrices/west0132.hpp>      //
#include <universal/blas/matrices/west0167.hpp>      //
#include <universal/blas/matrices/steam1.hpp>        //
#include <universal/blas/matrices/steam3.hpp>        //
#include <universal/blas/matrices/fs_183_1.hpp>      //
#include <universal/blas/matrices/fs_183_3.hpp>      // 
#include <universal/blas/matrices/faires74x3.hpp>    // Burden Faires 3x3 Ill-conditioned
#include <universal/blas/matrices/rand4.hpp>         // Random 4x4 (low condition) for testing
#include <universal/blas/matrices/cage3.hpp>         //
#include <universal/blas/matrices/bwm200.hpp>        // Chem. simulation 1e3.
#include <universal/blas/matrices/gre_343.hpp>       // Directed Weighted Graph
#include <universal/blas/matrices/int3.hpp>          // 3x3 integer test matrix (low condition number)
#include <universal/blas/matrices/b1_ss.hpp>         // 7x7 Chemical Process Simulation Problem
#include <universal/blas/matrices/cage3.hpp>         // 
#include <universal/blas/matrices/pores_1.hpp>       // 30x30 Computational Fluid Dynamics
#include <universal/blas/matrices/Stranke94.hpp>     // 10 x 10 Undirected Weighted Graph
#include <universal/blas/matrices/Trefethen_20.hpp>  // 20x20 Combinatorial Problem
#include <universal/blas/matrices/bcsstk01.hpp>      // 48x48
#include <universal/blas/matrices/bcsstk03.hpp>      // 112 x 112
#include <universal/blas/matrices/bcsstk04.hpp>      // 132 x 132
#include <universal/blas/matrices/bcsstk05.hpp>      // 153 x 153
#include <universal/blas/matrices/bcsstk22.hpp>      // 138 x 138


// File I/O
#include <iostream>
#include <fstream>


int main(){
try {
    std::cout << std::setprecision(16) << std::endl;
	using namespace sw::universal;
	using namespace sw::universal::blas;

    // ----------------------------------------------------- //
    // Configurations
    // ----------------------------------------------------- //
    constexpr unsigned wbits = 64;
    constexpr unsigned wes = 11;

    constexpr unsigned lbits = 16;
    constexpr unsigned les = 5;
    
    constexpr unsigned hbits = 128;
    constexpr unsigned hes = 15;

    // Squeeze Selection 0, 21, 22, 24
    // 0 Round
    // 21 
    // 22
    // 24 - Two-sided Scaling
    size_t algo = 24; // See Higham 2019 Squeeze

    bool print = false;
    // -----------------------------------------------------//

    // Write Configurations
    /*
    std::ofstream MyFile("configs_highamir.txt", std::ios_base::app); // Create and open a text file
    MyFile << "-------------------------\n";
    MyFile << "Algo = \t" << algo << "\n";         // Write to the file
    MyFile << "--------------------------\n\n";
    MyFile << "(hbits, hes) = (" << hbits << ", " << hes << ") \n";
    MyFile << "(wbits, wes) = (" << wbits << ", " << wes << ") \n";
    MyFile << "(lbits, les) = (" << lbits << ", " << les << ") \n";
    MyFile.close();                  // Close the file
    */

    // Precision Templates
    using WorkingPrecision = cfloat<wbits,wes,uint32_t, true, false, false>;
    using LowPrecision  = cfloat<lbits,les,uint32_t, true, false, false>;
    using HighPrecision  =  cfloat<hbits,hes,uint32_t, true, false, false>;

    // Matrix and Vector Type alias
    using Mh = sw::universal::blas::matrix<HighPrecision>;
    using Vh = sw::universal::blas::vector<HighPrecision>;
    using Mw = sw::universal::blas::matrix<WorkingPrecision>;
    using Vw = sw::universal::blas::vector<WorkingPrecision>;
    using Ml = sw::universal::blas::matrix<LowPrecision>;

    // View Numerical Properties of Configuration
    LowPrecision m, M;
    m.minpos();
    M.maxpos();
    std::cout << "Numeric Bounds fp<" << lbits << "," << les << "> = (" << m << ", " << M << ")" << std::endl;
    // std::cout << "Dynamic range " << dynamic_range<LowPrecision>() << '\n';
    // std::cout << "Precision = " << ULP  <<  std::endl;    

    // ---------------------------------------------------------------------------- 
    /*
    Let A be n x n ("working precision") nonsingular matrix.
        Test Matrices in suite:
        int3, rand4, lu4, west0167, steam1, steam3, fs_183_1, fs_183_3, faires74x3
        q3, q4, q5, h3, pores_1, Stranke94, bcsstk05  ...
    */ 
    // ----------------------------------------------------------------------------
    Mw A = pores_1;
    if(print){std::cout << "A = \n" << A << std::endl;}
    unsigned n = num_cols(A);
    // Met-A data
    std::cout << "Condition estimate: " << condest(A) << std::endl;
    std::cout << "Size: (" << n << ", " << n  << ")\n" << std::endl;


    Ml Al; //(A);  // Declare low precision matrix to store A
   
    // Test getRow(i,A);
    // if(print){std::cout << "getRow(i,A) = " << getRow(2,A) << std::endl;}


    
    // ---------------------------------------------------------------------------- 
    /*
        Squeezing Matrix:  
    */ 
    // ----------------------------------------------------------------------------
    WorkingPrecision T = 0.1;  // \in (0,1]
    WorkingPrecision mu = 1.0;
    Vw R(num_rows(A),1);  // Row Squeezer
    Vw S(num_rows(A),1);  // Column Squeezer

    // Round, then replace inf (overflow)
    if (algo == 21){ //Round, then replace
        squeezeRoundReplace(A, Al);
        if(print){std::cout << "Algorithm: Round, then replace infinities.\n" << std::endl;}
    
    // Scale and Round
    }else if(algo == 22){
        squeezeScaleRound<WorkingPrecision, LowPrecision>(A, Al, T, mu);
        if(print){std::cout << "Algorithm: Scale, then round.\n" << std::endl;}
    
    }else if(algo == 23 || algo == 24 || algo == 25){
        twosideScaleRound<WorkingPrecision, LowPrecision>(A, Al, R, S, T, mu, algo);
        if(print){std::cout << "Algorithm: Two-sided squeezing, RAS." << std::endl;}
    }else{
        // Do nothing
        Al = A;
        if(print){std::cout << "Algorithm: Round only, i.e., A --> A (low)." << std::endl;}
    }
    if(print){
        std::cout << "A (modified) = \n" << A << std::endl;
        std::cout << "Al (low precision) = \n" << Al << std::endl;
    }
    // ----------------------------------------------------------------------------



    // LU Factorization of Low Precision
    auto [P, L, U] = plu(Al);
    if(print){std::cout << "PLU = \n" << P << "\n" << L << "\n" << U << std::endl;}

    // store in working precision
    Mw Pw(P);
    Mw Lw(L);
    Mw Uw(U);
    
    // Compute new (permuted) A
    A = Pw*A; 
    if(print){
        std::cout << "PA = \n" << A << std::endl;
        std::cout << "LU (low precision)= \n" << L*U << std::endl;
        std::cout << "LU (working precision) = \n" << Lw*Uw << std::endl;
    }
    // matvec(b,A,x);  // quire-enabled

    // Create high precision version 
    Mh Ah(A);       // High precision A
    Vh X(n,1);      // X is exact solution = [1, 1, 1, ..., 1]
    Vh b = Ah*X;    // Generate b vector in high precision.

    // Store working
    Vw x(X);  
    Vw bw(b); // Note: also try b = P*mu*R*(AX), where A is original matrix.

    // 1. Solve Ax = b in low-precision, then store x in working
    auto xn = backsub(Uw,forwsub(Lw,bw));

    // Results Header
    std::cout << "#" << "      "  << " ||x - xn||   " << '\n'; 
    std::cout << "----------------------------------"  << '\n';

    // Iterative Refinement 
    // Stratagem: compute a quantity by adding a small correction to previous approximation.
    Vh r; // High precision residual vector
    size_t niters = 0;
    while(((x - xn).norm() > 1e-7) && (niters < 25)){
        niters += 1;
        // ----------------------------------------------------------- 
        // Residual Calculation (high precision)
        Vh xh(xn);
        r = b - Ah*xh;
        Vw rn(r); // Store in working presicion

        // Solve Ad = r where A = LU (low precision)
        auto d = backsub(Uw,forwsub(Lw,rn));  // Stored d in working precision         
        xn += d;  // update solution vector with corrector
            
        // Print Results
        std::cout << niters << "\t"  << (x - xn).norm() << '\n';
    } //wend

    // Print solution vector
    std::cout << "----------------------------------"  << '\n';
    std::cout << "Showing first few elements of solution vector..." << '\n';
    std::cout << "x = " << '\n';
    for(size_t i=0;i < 3;++i){
        std::cout << xn(i) << '\n';
    }
    // ----------------------------------------------------------------------------

	int nrOfFailedTestCases = 0;
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
2022-11-05 
 - Created an in-place LU decomposition to save storage.
 - Updated forsub.hpp to in-place.  No longer requires lower triangular matrix
    Added input parameter of type boolean to indicate whether it is lower tri.
    In that case, we assume diagonal is all ones.  
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


   /* 
    Is there a difference between C++ compiler and MATLAB implementation???
    Below is a test for the case of subnormals in Higham's squeeze paper.
    This code can run independently.   

    NOTE: Subnormals done in software. 
    using LowPrecisionP1  = cfloat<lbits+1,les,uint32_t, true, false, false>;
    LowPrecisionP1 Ap(Al(0,1));
    std::cout << "Binary A(0,1) = " << to_binary(A(0,1)) << std::endl;
    std::cout << "Binary A(0,1) = " << to_binary(Al(0,1)) << std::endl;
    std::cout << "Binary Ap(0,1) = " << to_binary(Ap) << " : " << Ap << std::endl;
    std::cout << "Binary Ap/2(0,1) = " << to_binary(Ap/2) << " : " << Ap/2 << std::endl;
    
    cfloat<32,8,uint32_t, true, false, false> aa(SpecificValue::minpos);
    std::cout << "Binary of Single Precision " << to_binary(aa) << " : " << aa << std::endl;
    float f = float(aa);
    // 1.401298464324817e-45
    std::cout << "Binary of Single Precision " << to_binary(f) << " : " << f << std::endl;
    float g = f*3/4;
    std::cout << "Binary of g " << to_binary(g) << " : " << g << std::endl;
    */