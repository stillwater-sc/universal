/** ************************************************************************
* Solve System of Equation Tests
* 
*  @author:     James Quinlan
*  @date:       2022-12-13
*  @copyright:  Copyright (c) 2017 Stillwater Supercomputing, Inc.
*  @license:    MIT Open Source license 
* 
*  SPDX-License-Identifier: MIT
* 
* This file is part of the Universal Number Library project. 
* *************************************************************************
*/
// Build Directory = universal/build/applications/numeric
#include <universal/utility/directives.hpp>

// #define POSIT_VERBOSE_OUTPUT
#define POSIT_TRACE_MUL
#define QUIRE_TRACE_ADD

// enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include<universal/number/posit/posit.hpp>
// enable cfloat arithmetic exceptions
#define CFLOAT_THROW_ARITHMETIC_EXCEPTION 1
#include<universal/number/cfloat/cfloat.hpp>

// Higher Order Libraries
#define BLAS_TRACE_ROUNDING_EVENTS 1
#include<blas/blas.hpp>
#include<blas/utes/matnorm.hpp>
#include<blas/utes/condest.hpp>
#include<blas/utes/nbe.hpp>      // Normwise Backward Error

// Support Packages
#include<blas/solvers/lu.hpp>
#include<blas/solvers/plu.hpp>
#include<blas/solvers/qr.hpp>

#include<blas/solvers/backsub.hpp>
#include<blas/solvers/forwsub.hpp>
#include<blas/matrices/testsuite.hpp>

#include <chrono>


template<typename Real>
sw::universal::blas::vector<Real> ei(const size_t &ii, const size_t &n) {
    sw::universal::blas::vector<Real> e(n); //,x(n);
    for(size_t j=0; j< n; ++j){
        e[j]=(j == ii - 1) ? 1 : 0;
    }
    return e;
}

  
template<typename Real>
sw::universal::blas::matrix<Real> submat(sw::universal::blas::matrix<Real> &A, 
                                         const size_t &r1, 
                                         const size_t &r2, 
                                         const size_t &c1, 
                                         const size_t &c2) {
    // Returns submatrix A(r1:r2,c1:c2)
    // Note: 1-indexed (i.e., MATLAB)
    size_t m = r2 - r1 + 1;
    size_t n = c2 - c1 + 1;
    sw::universal::blas::matrix<Real> S(m,n); //,x(n);
    for(size_t i=0; i < m; ++i){
        for(size_t j=0; j < n; ++j){
            S(i,j)=A(r1 - 1 + i,c1 - 1 + j);
        }
    }
    return S;
}
 

int main()
try {
	
    using namespace sw::universal;
	using namespace sw::universal::blas;
    using namespace std::chrono; // used for timing
    
    // NOTE: higher precision wasn't correct
    using Real  = cfloat<64,11,uint32_t, true, false, false>; // float; // double;
    // using Real  = posit<64,2>;
    // Once matrix is squeeze, there is no need for dynamic range.  

    // Matrix and Vector Type alias
    using Matrix = sw::universal::blas::matrix<Real>;
    //using Vector = sw::universal::blas::vector<Real>;
    //using MatH = sw::universal::blas::matrix<Hirez>;
    //using VecH = sw::universal::blas::vector<Hirez>;

    // using value_type = typename Matrix::value_type; // what do these do?
    // using size_type = typename Matrix::size_type;
    //std::cout << value_type << std::endl;
    //std::cout << size_type << std::endl;

    // size_t n = 3;
    Matrix A(4,4);
    A = {
        { 1,   3,    5,   1 }, 
        { 2,  -1,    2,   1 }, 
        { 1,   4,    6,   1 }, 
        { 4,   5,   10,   1 }
    };
    /*
    A = {
        { 0.8147,   0.0975 , 0.1576 }, 
        { 0.9058,   0.2785,  0.9706 }, 
        { 0.1270,   0.5469,  0.9572 }, 
        { 0.9134,   0.9575,  0.4854 },
        { 0.6324,   0.9649,  0.8003 }
    };

    A = {
        {0.75126707, 0.75126707, 0.75126706, 0.75126706},
        {0.25509512, 0.25509512, 0.25509512, 0.25509512},
        {0.50595705, 0.50595706, 0.50595705, 0.50595706},
        {0.69907672, 0.69907673, 0.69907673, 0.69907673},
        {0.89090326, 0.89090326, 0.89090326, 0.89090326}
    };
     
    A = {
        { 1,  -1 , 4 }, 
        { 1,   4, -2 }, 
        { 1,   4,  2 }, 
        { 1,  -1,  0 }
    };

    A = {
    { 1, 2, 2},
    { 2,-2, 1},
    { 2, 1, -2}
   };
     
   A = {
    { 2, -3, 1},
    { 1, -2, 1},
    { 1, -3, 2}
   };
   
  A = {
        { 1,  -2 , -1 }, 
        { 2,   0,   1 }, 
        { 2,  -4,   2 }, 
        { 4,   0,   0 }
    };
    // std::cout << "Submat = \n" <<  submat(A,2,4,1,2) << std::endl;

    //Matrix Q(3,3);
    //Matrix R(3,3);
*/
    // std::cout << std::setprecision(7);
    auto [Q,R] = qr(A,4);
    
    std::cout << "Q = \n" << Q << std::endl; // disp(Q);
   std::cout << "R = \n"; disp(R);
    std::cout << "QR = \n" << Q*R << std::endl; // disp(Q*R);
    
    /* 
    size_t n = 3;
    Matrix A(n,n);
    A = {
        { 1,  2 , 1 }, 
        { 3, -1, -3 }, 
        { 2,  3,  1 } 
    };
    */ 

    //Matrix A = getTestMatrix("cage3");
    //size_t n = num_cols(A);
    //std::cout << "Condest = " << condest(A) << std::endl;

    
    // Matrix B = uniform_random_matrix<Real>(n,n);
    /* 
    Matrix B(n,n);
    B = {
        { 1,  2 , 1 }, 
        { 3, -1, -3 }, 
        { 2,  3,  1 } 
    };
    Vector c = {3, -1, 4};
    std::cout << B*ei<Real>(2,3) << std::endl;
    std::cout << submat(B,1,3,2,2) << std::endl;

    // Vector X(n,1);
    // auto y = solve(B,c);
    // std::cout << "y = " << y << std::endl;
    
    Vector X(n,1);
    //Matrix B = getTestMatrix("cage3");
    //Vector X = uniform_random_vector<Real>(n);
    Vector b(n);
    b = A*X;
    // std::cout << "b = " << b << std::endl;

    sw::universal::blas::matrix<size_t> P(n-1,2);

     // TIMING
    steady_clock::time_point beginLU = steady_clock::now();
	plu(A,P);
    
    
    
    //std::cout << "A = \n" << A << std::endl;
    // std::cout << "P = \n" << P << std::endl;

    // Permute vector b to match PA = LU
    // Ax = b
    // PAx = 
    for (size_t ii = 0; ii < n-1; ++ii){
        // std::cout << "ii,n = \n" << ii << ", " << n << std::endl;
        if(P(ii,0) != P(ii,1)){
                auto bi = b(P(ii,0));
                b(P(ii,0)) = b(P(ii,1));
                b(P(ii,1)) = bi;
        }
    }
    
    //MatH Ah(A);
    //VecH bh(b);
    auto x = backsub(A,forwsub(A,b));
    steady_clock::time_point endLU = steady_clock::now();
	duration<double> time_spanLU = duration_cast<duration<double>> (endLU - beginLU);
	double elapsed_timeLU = time_spanLU.count();
    // END TIMING
	std::cout << "Solve from plu.hpp = " << elapsed_timeLU << " sec "  << std::endl;




    //std::cout << "Approx = " << x << std::endl;
    //std::cout << "Exact = " << X << std::endl;

    // TIMING using Solve
    //   
    steady_clock::time_point begin = steady_clock::now();
	auto y = solve(B,b);
    steady_clock::time_point end = steady_clock::now();
	duration<double> time_span = duration_cast<duration<double>> (end - begin);
	double elapsed_time = time_span.count();
    // std::cout << "y = " << y << std::endl;
	std::cout << "Solve from lu.hpp = " << elapsed_time << " sec "  << std::endl;
    Matrix v(3,1);
    v = {
        {3}, 
        {-1}, 
        {4}
    };
    auto vt = v.transpose();
    std::cout << v.size() << ", " << num_rows(v) << std::endl;
    std::cout << "v = " << v <<  "v' = " <<  v.transpose() << "vt = "<< vt <<  std::endl; 
    
    // std::cout << v*(v.transpose()) << std::endl;  // <------ BUG HERE
    
    std::fixed – Fixed Floating-point notation : 
    It writes floating-point values in fixed-point notation. 
    The value is represented with exactly as many digits in the decimal part as 
    specified by the precision field (precision) and with no exponent part.

    std::scientific – Scientific floating-point notation : 
    It writes floating-point values in Scientific-point notation. 
    The value is represented always with only one digit before the decimal point, 
    followed by the decimal point and as many decimal digits as the precision field (precision). 
    Finally, this notation always includes an exponential part consisting on the 
    letter “e” followed by an optional sign and three exponential digits.

    */ 


	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << "Caught exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::quire_exception& err) {
	std::cerr << "Uncaught quire exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_internal_exception& err) {
	std::cerr << "Uncaught posit internal exception: " << err.what() << std::endl;
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

/*
A = {
        { 0.45368292, 0.19382865}, 
        { 0.70364726, 0.52104011} 
    };

A = {
        { 3.3330, 15920.0,  -10.333}, 
	    { 2.2220, 16.710,    9.6120},
	    { 1.5611,  5.1791,   1.6852} 
    };

A = {
        { 0.40563526, 0.26686200 }, 
        { 0.73033346, 0.48047658 } 
    };
     
A = {
        { 1,  2 , 1 }, 
        { 3, -1, -3 }, 
        { 2,  3,  1 } 
    };
    b = {3, -1, 4};
*/
