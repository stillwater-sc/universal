// luir: Iterative Refinement Experiment
//      This focuses on the fundamentally important problem of solving Ax = b.
//      It factors A into a low-precision LU, then solves for x.
//      This estimate is then refined by solving (LU)d = r = b - Ax, and
//      overwriting x with x + d, that is, x = x + d.  
//      The process is repeated until convergence.  
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
// @jamesquinlan
// Modified: 2022-09-30
// 
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// See Notes at EOF.

// Environmental Configurations
#include<universal/utility/directives.hpp>
#include<universal/utility/long_double.hpp>
#include<universal/utility/bit_cast.hpp>
#include <universal/utility/number_system_properties.hpp>

// Configure the posit library with arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit/posit.hpp>

#define CFLOAT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/cfloat/cfloat.hpp>

// Higher Order Libraries
#include <universal/blas/vector.hpp>
#include <universal/blas/matrix.hpp>
#include <universal/blas/blas.hpp>

// Support Packages
//#include <universal/blas/solvers/jacobi.hpp>
//#include <universal/blas/solvers/lu.hpp>
#include <universal/blas/solvers/plu.hpp>
#include <universal/blas/solvers/backsub.hpp>
#include <universal/blas/solvers/forwsub.hpp>
// #include <universal/blas/squeeze.hpp>
// #include <universal/blas/nnz.hpp>

// Matrix Test Suite
// #include <universal/blas/matrices/q3.hpp>            // 3 x 3 test matrix
// #include <universal/blas/matrices/q4.hpp>            // 4 x 4 test matrix
// #include <universal/blas/matrices/lu4.hpp>           // 4 x 4 test matrix
// #include <universal/blas/matrices/s4.hpp>            // 4 x 4 test matrix
// #include <universal/blas/matrices/west0132.hpp>      //
// #include <universal/blas/matrices/west0167.hpp>      //
// #include <universal/blas/matrices/steam1.hpp>        //
// #include <universal/blas/matrices/steam3.hpp>        //
// #include <universal/blas/matrices/fs_183_1.hpp>      //
//#include <universal/blas/matrices/fs_183_3.hpp>         // 
//#include <universal/blas/matrices/faires74x3.hpp>       // Example Burden and Faires 3x3 Ill-conditioned
#include <universal/blas/matrices/rand4.hpp>            // Random 4x4 (low condition) for testing

namespace sw { namespace universal { namespace blas {  
template<typename Scalar>
std::tuple<vector<Scalar>, vector<Scalar>> luir(const matrix<Scalar>& A){ 
    using Vector = sw::universal::blas::vector<Scalar>;
    Vector xx(10,1); // vector length 10 of values 1 = ones(10,1);
    //std::cout << x << std::endl;
    Vector r = {1,1,1,1}; // 
    return std::make_tuple(xx,r); 
}
}}} // namespaces


// Currently not used
template<typename Real>
void test(const Real &a, const Real &b){
    std::cout << "Test TBD\n";
}


// Run Posit
template<size_t nbits, size_t es>
void runPosit()
{
    using namespace sw::universal;
    posit<nbits, es> p, q, mp;  // mp = minpos
    p.maxpos();
    q.maxneg();
    mp.minpos();

    // test<posit<nbits, es>>(q, p);
     std::cout << "Maxpos = " << p << '\n';
     std::cout << "Minpos = " << mp << '\n';
     double dminpos = double(mp);
     std::cout << "Double min real = " << dminpos << '\n';
     std::cout << "Dynamic range " << dynamic_range<posit<nbits,es>>() << '\n';
     std::cout << "Dynamic Double " << dynamic_range<double>() << '\n';
}

// Run Cfloat function - NOT USED
template<size_t nbits, size_t es>
void runCfloat()
{
    using namespace sw::universal;
    using Cfloat = cfloat<nbits,es, uint16_t, true, false, false>;
    Cfloat a, b;
    a.minpos();
    b.maxpos();
    std::cout << b << std::endl;

    //test<Cfloat>(a,b);
}


int main(){
try {
    std::cout << std::setprecision(16) << std::endl;
	using namespace sw::universal;
	using namespace sw::universal::blas;

	#define NUMERIC 5
	// 1 = POSIT, 2 = CFLOAT, 3 = Float, 4 = FIXPT.
   
	// Squeeze to Posit
	#if NUMERIC == 1 
    runPosit<64,2>();

	// Squeeze to Cfloat
	#elif NUMERIC == 2
	runCfloat<32,8>();

	// Squeeze to Float (base case)
	#elif NUMERIC == 3
	{
		test<float>(FLT_MAX,FLT_MIN);	
	}
	


	// Squeeze to Fix Point
	#elif NUMERIC == 4
	{
		test<fixpnt<nbits,rbits>>();
	}

	#elif NUMERIC == 5
	{
        // Configurations
        constexpr size_t hbits = 64;
        constexpr size_t hes = 0;
        constexpr size_t lnbits = 8;
        constexpr size_t les = 0;

        // Precision Templates
        using WorkingPrecision = posit<hbits,hes>; // cfloat<hbits,hes,uint16_t, true, false, false>;
        using FactorPrecision  =  posit<lnbits,les>; // cfloat<lbits,les,uint16_t, true, false, false>;
        
        // Matrix and Vector Types
        // Type alias
        using Mw = sw::universal::blas::matrix<WorkingPrecision>; // Working precision
        using Vw = sw::universal::blas::vector<WorkingPrecision>;
        using Mf = sw::universal::blas::matrix<FactorPrecision>;
        using Vf = sw::universal::blas::vector<FactorPrecision>;

       
        
        // Initialize Variables
        size_t niters = 0;

        
        // Let A be n x n ("working precision") nonsingular matrix.
        Mw A = rand4;  // lu4, west0167, steam1, steam3, fs_183_1, fs_183_3, faires74x3
        size_t n = num_cols(A);

        Vw X(n,1);    // X is exact solution = [1, 1, 1, ..., 1]
        Vw b = A*X;   // Generate b vector in working precision.  

        // Store A and b in Low Precision
        Mf B(A);
        Vf c(b);

        // Print Results
        std::cout << "A = " << A << std::endl;
        std::cout << "B = " << B << std::endl;
        std::cout << "b = " << b << std::endl;
        std::cout << "c = " << c << std::endl;


        // Factor A = LU
        auto [P, L, U] = plu(B);  // Factor low precision
        Mw Lw(L), Uw(U);          // Refining low precision LU

        // Print Results
        std::cout << "L = " << L << std::endl;
        std::cout << "U = " << U << std::endl;


        // Header
        std::cout << "#" << "      "  << "Residual   " << '\n';
        std::cout << "-------------------------------"  << '\n';

        // 1. Solve Ax0 = b in low-precision, then store x0 in working
        // std::cout << "L \\ c " <<  forwsub(L,c) << std::endl;
        auto x = backsub(U,forwsub(L,c));
        //std::cout << "x = " <<  x << std::endl;
        
        // Make working precision version of x and b. 
        Vw y(x), r(b);  

        // Print Results
        // std::cout << "x = " << x << std::endl;
        // std::cout << "y = " << y << std::endl;
        // std::cout << "b = " << b << std::endl;
        // std::cout << "r = " << r << std::endl;
         

        // Iterative Refinement 
        while(((y - X).norm() > 1e-5) && (niters < 50)){
            niters += 1;
            // y = x;
            // y.disp();

            // ----------------------------------------------------------- 
            // Residual Calculation
            // A, b, y in "Working" precision (r in "higher")
            // r is using the quire to defer rounding to working precision
            // until assignment.  
            for (size_t i = 0; i < n; ++i) {
                sw::universal::quire<hbits, hes> q(r(i));
                for (size_t j = 0; j < n; ++j) {
                    q -= sw::universal::quire_mul(A(i,j), y(j));
                }
                sw::universal::convert(q.to_value(), r(i));  
            }
            //std::cout << "r = " << r << '\n';
            // ----------------------------------------------------------- 



            // ----------------------------------------------------------- 
            // Solve Ad = r where A = LU (low precision)
            // Solved in HIGH PRECESION using LU.
            
            auto d = backsub(Uw,forwsub(Lw,r));
            //std::cout << "d = " << d << '\n';
            
            y += d;  // y = y + d
            //std::cout << "x = " << y << '\n';
            // ----------------------------------------------------------- 
            
            // Results
            std::cout << niters <<  "      "    << (X - y).norm() << '\n';
        }
	}
	#endif


	int nrOfFailedTestCases = 0;
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}

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
}

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