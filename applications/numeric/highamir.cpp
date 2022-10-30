// highamir: Iterative Refinement Experiment following Higham (see reference list)
//    Addresses fundamental important problem of solving Ax = b.
//      
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
// @jamesquinlan
// Modified: 2022-10-08
// 
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// References
/*
Higham, N. J., & Mary, T. (2019). A new preconditioner that exploits 
    low-rank approximations to factorization error. SIAM Journal on 
    Scientific Computing, 41(1), A59-A82.
*/

// Build Directory
// universal/build/applications/numeric

// Environmental Configurations
#include<universal/utility/directives.hpp>
#include<universal/utility/long_double.hpp>
#include<universal/utility/bit_cast.hpp>
#include <universal/utility/number_system_properties.hpp>

#define CFLOAT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/cfloat/cfloat.hpp>

// Higher Order Libraries
#include <universal/blas/vector.hpp>
#include <universal/blas/matrix.hpp>
#include <universal/blas/blas.hpp>
#include <universal/blas/utes/matnorm.hpp>
#include <universal/blas/utes/condest.hpp>

// Support Packages
//#include <universal/blas/solvers/jacobi.hpp>
// #include <universal/blas/solvers/lu.hpp>  // solve()
#include <universal/blas/solvers/plu.hpp>
#include <universal/blas/solvers/backsub.hpp>
#include <universal/blas/solvers/forwsub.hpp>
#include <universal/blas/squeeze.hpp>
// #include <universal/blas/nnz.hpp>

// Matrix Test Suite
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
#include <universal/blas/matrices/fs_183_3.hpp>       // 
#include <universal/blas/matrices/faires74x3.hpp>     // Example Burden and Faires 3x3 Ill-conditioned
#include <universal/blas/matrices/rand4.hpp>            // Random 4x4 (low condition) for testing
#include <universal/blas/matrices/cage3.hpp>
#include <universal/blas/matrices/bwm200.hpp>           // Chem. simulation 1e3.
#include <universal/blas/matrices/gre_343.hpp>          // Directed Weighted Graph

int main(){
try {
    std::cout << std::setprecision(16) << std::endl;
	using namespace sw::universal;
	using namespace sw::universal::blas;

    // Configurations
    constexpr unsigned wbits = 64;
    constexpr unsigned wes = 11;

    constexpr unsigned lbits = 16;
    constexpr unsigned les = 6;
    
    constexpr unsigned hbits = 128;
    constexpr unsigned hes = 15;

    // Squeeze Selection
    size_t algo = 22;

    // Precision Templates
    using WorkingPrecision = cfloat<wbits,wes,uint32_t, true, false, false>;
    using LowPrecision  = cfloat<lbits,les,uint32_t, true, false, false>;
    using HighPrecision  =  cfloat<hbits,hes,uint32_t, true, false, false>;

    // View Numerical Properties of Configuration
    // std::cout << "Dynamic range " << dynamic_range<LowPrecision>() << '\n';
    LowPrecision m, M;
    m.minpos();
    M.maxpos();
    std::cout << "Numeric Bounds = (" << m << ", " << M << ")" << std::endl;
    // std::cout << "Max = " <<  M << std::endl;


    // Matrix and Vector Types
    // Type alias
    using Mh = sw::universal::blas::matrix<HighPrecision>;
    using Vh = sw::universal::blas::vector<HighPrecision>;
    using Mw = sw::universal::blas::matrix<WorkingPrecision>; // Working precision
    using Vw = sw::universal::blas::vector<WorkingPrecision>;
    using Ml = sw::universal::blas::matrix<LowPrecision>;
    //using Vl = sw::universal::blas::vector<LowPrecision>;
     
    // Initialize Variables
    size_t niters = 0;
        
    // Let A be n x n ("working precision") nonsingular matrix.
    Mw Aw = q5;  // rand4, lu4, west0167, steam1, steam3, fs_183_1, fs_183_3, faires74x3
    size_t n = num_cols(Aw);
    Mh A(Aw);

    // matnorm
    // std::cout << "||A|| = " << matnorm(Aw) << std::endl;
    std::cout << "K = " << condest(Aw) << std::endl;
    
    
    
    Vh X(n,1);    // X is exact solution = [1, 1, 1, ..., 1]
    Vw x(X);
    Vh b = A*X;   // Generate b vector in high precision.  
    Vw bw(b);

    // Store A in Low Precision
    WorkingPrecision T = 0.0001;
    
    Ml Al(Aw);
    if (algo == 21){
        squeezeScaleRound(Al);
    }
    {
        /* code */
    }
    
    squeeze(Al,algo);
    
    // std::cout << "Max. Element = " << maxelement(Al) << std::endl;    
    // std::cout << "Min. Element = " << minelement(Al) << std::endl;

    
    std::cout << "Al = \n" << Al << std::endl;
    // Ml I(n,n); // Identity Matrix
    // I = 1;

    

    // Print Results
    // std::cout << "A = " << A << std::endl;
    // std::cout << "Al = " << Al << std::endl;
    // std::cout << "b = " << b << std::endl;
    // std::cout << "bw = " << bw << std::endl;

    // Factor A = LU
    auto [Pl, Ll, Ul] = plu(Al);  // Factor low precision
    Mw L(Ll), U(Ul);              // Coerce to working precision

    // Is P identity?
    /*
    for(int i = 0;i < n;++i){
        for(int j = 0; j<n;++j){
            if(Pl(i,j) != I(i,j)){
                std::cout <<  "Not Identity" << std::endl;
            }
        }
    }
    */


    // Print Results
    // std::cout << "L = \n" << L << std::endl;
    // std::cout << "U = \n" << U << std::endl;
    ///*
    for(int i = 0;i < 10;++i){
        for(int j = 0; j<10;++j){
            if(L(i,j) != 0){
                // std::cout <<  L(i,j) << std::endl;
            }
        }
    }
    //*/



    // 1. Solve Ax0 = b in low-precision, then store x0 in working
    // std::cout << "L \\ b " << forwsub(L,bw) << std::endl;
    auto xn = backsub(U,forwsub(L,bw));
    // std::cout << "x = " <<  xn << std::endl;
    // xn.disp();
        
    // High precision r. 
    Vh r;  

    // Iterative Refinement 
    // ``The general stratagem in numerical calculations is that it is best to compute a 
    //   quantity by adding a small correction term to a (previous) approximation." Kincaid-Cheny, p.76.
    //   e.g., In Bisection method, the midpoint c is computed as c <— a + (b — a)/2 rather than 
    //   as c <— (a + b)/2 to adhere to the stratagem above.  

     // Header
    std::cout << "#" << "      "  << " ||x - xn||   " << '\n';
    std::cout << "-------------------------------"  << '\n';
    while(((x - xn).norm() > 1e-7) && (niters < 25)){
        niters += 1;
        // std::cout << niters <<  "\t"  << "x = " << xn << "\t"  << (xn - x).norm() << '\n';
        // ----------------------------------------------------------- 
        // Residual Calculation
        // A, b, y in "Working" precision (r in "higher")
        // r is using the quire to defer rounding to working precision
        // until assignment. 
        Vh xh(xn);
        r = b - A*xh;
        
        //std::cout << "r = " << r << '\n';
        // ----------------------------------------------------------- 

        // ----------------------------------------------------------- 
        // Solve Ad = r where A = LU (low precision)
        // Solved in Working PRECESION using LU.
        Vw rn(r);
        auto d = backsub(U,forwsub(L,rn));            
        xn += d;  // corrector
        // ----------------------------------------------------------- 
            
        // Results
        std::cout << niters << "\t"  << (x - xn).norm() << '\n';
        // std::cout << niters <<  "\t"  << "x = " << xn << "\t"  << (xn - x).norm() << '\n';
    } // wend

    // Print solution vector
    std::cout << "-------------------------------"  << '\n';
    std::cout << "x = " << '\n';
    for(size_t i=0;i < 3;++i){
        std::cout << xn(i) << '\n';
    }
    // xn.disp();


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

// NOTES
/*
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


*/