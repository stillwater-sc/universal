// squeeze.cpp: Driver code to squeeze a matrix to lower precision
// 				see also: squeeze.hpp
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
// Author: James Quinlan
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include<universal/utility/directives.hpp>
#include<universal/utility/long_double.hpp>

// Standard library
#include <float.h>

// Configure the posit library with arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1

// Dependencies
#include <universal/number/posit/posit.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/blas/blas.hpp>
#include <universal/blas/generators.hpp>
#include <universal/blas/solvers/jacobi.hpp>
#include <universal/blas/solvers/lu.hpp>
#include <universal/blas/solvers/plu.hpp>

#include <universal/blas/squeeze.hpp>
#include <universal/blas/nnz.hpp>
#include <universal/utility/number_system_properties.hpp>



// Matrix Test Suite
#include <universal/blas/matrices/q3.hpp>        // 3 x 3 test matrix
#include <universal/blas/matrices/q4.hpp>        // 4 x 4 test matrix
#include <universal/blas/matrices/lu4.hpp>        // 4 x 4 test matrix
#include <universal/blas/matrices/s4.hpp>        // 4 x 4 test matrix
#include <universal/blas/matrices/west0132.hpp>
#include <universal/blas/matrices/west0167.hpp>
#include <universal/blas/matrices/steam1.hpp>
#include <universal/blas/matrices/steam3.hpp>
#include <universal/blas/matrices/fs_183_1.hpp>
#include <universal/blas/matrices/fs_183_3.hpp>

// Min. Element of Matrix
template<typename Scalar>
Scalar minelement(const sw::universal::blas::matrix<Scalar> &A){
	size_t m = num_rows(A);
	size_t n = num_cols(A);
	Scalar Min = std::numeric_limits<Scalar>::max();
	for (size_t i = 0; i < m; ++i){
		for (size_t j = 0; j < n; ++j){
			if (abs(A(i,j)) > 0){
				if (Min > abs(A(i,j))){Min = abs(A(i,j));}
			}
		}
	}
	return Min;
}

// Max. Element of Matrix
template<typename Scalar>
Scalar maxelement(const sw::universal::blas::matrix<Scalar> &A){
	size_t m = num_rows(A);
	size_t n = num_cols(A);
	Scalar Max = 0; // std::numeric_limits<Scalar>::min();
	for (size_t i = 0; i < m; ++i){
		for (size_t j = 0; j < n; ++j){
			if (Max < abs(A(i,j))){Max = abs(A(i,j));}
		}
	}
	return Max;
}







/* TEST FUNCTION 
---------------------------------------------------------
- Define system Ax = b
- Squeeze matrix/vector

*/
template<typename Scalar>
void test(const Scalar & maxneg, const Scalar & maxpos){
	using namespace sw::universal;
	using namespace sw::universal::blas;

	using SourceScalar = double;
	using SourceMatrix = sw::universal::blas::matrix<SourceScalar>; 
	using SourceVector = sw::universal::blas::vector<SourceScalar>;

	using TargetScalar = Scalar;
	using TargetMatrix = sw::universal::blas::matrix<TargetScalar>;
	using TargetVector = sw::universal::blas::vector<TargetScalar>;

    // Literals are int, double 1.0 (default), or single 1.0f, long double 1.0l 
		
	// /*
	SourceMatrix A = s4;  // lu4, west0167, steam1, steam3, fs_183_1, fs_183_3
	size_t N = num_cols(A); 
	SourceVector b(N); 
	// uniform_random(b,0.001,1); // = { 6.0, 25.0, -11.0,15.0 };  
	SourceVector x(N);  // = {  0,0,0,0 };
	SourceVector _x(N);
	_x = 1.0; 
	b = A*_x;
	// -----------------------------------
	// Low Precision
	TargetMatrix D(N,N);   // copy of A
	TargetMatrix B(N,N);   // 3,3 squeeze mat.
	TargetVector b_(N);
	TargetVector x_(N);

	// copy A to D
	for (size_t i = 0; i < N; ++i) {
        for (size_t j = 0; j < N; ++j) {
            Scalar dij = A(i, j);
            D(i,j) = dij;
        }
    }

	// Compute A_p (Round A to lower precision)
	for (size_t i = 0; i < N; ++i) {
        for (size_t j = 0; j < N; ++j) {
            Scalar aij = A(i, j);
            B(i,j) = aij;
        }
    }
	// Compute the low precision version of b
	for (size_t i =0; i < N; ++i){
		Scalar bi = b(i);
		b_(i) = bi;
	}
	// posit<16,2> c = 2.0;
	// cfloat<16,3> c = 2.0;
	// /* 
	//std::cout << "A = \n" << A << '\n';
	//std::cout << "B = \n" << B << '\n';

	//std::cout << "b = \n" << b << '\n';
	//std::cout << "b_p = \n" << b_ << '\n';
	//std::tuple<float,float,float>foo; 
	
	// auto PLU = plu<SourceScalar>(A);
	// std::cout << "P = \n" << PLU(0) << '\n';
	//TargetMatrix P(N,N);
	//TargetMatrix L(N,N);
	//TargetMatrix U(N,N);
	// plu(D, P, L, U);
	auto [P, L, U] = plu(D); // note: D is a <Scalar> copy of A.

	std::cout << std::setprecision(15) << '\n';

	std::cout << "A = \n" << A << '\n';
	std::cout << "P = \n" << P << '\n';
	std::cout << "L = \n" << L << '\n';
	std::cout << "U = \n" << U << '\n';

	//sw::universal::blas::Crout(D,B);
	//std::cout << "A = \n" << B << std::endl;



	// std::cout << triu(A) << std::endl;

	//std::cout << std::setprecision(34);
	//std::cout << "sqrt(2) = " << sqrt(c) << '\n';
	// */
	// 1.414213562373095 048801688724209698 0785696718753769480731766797379907324784621
	// 1.414 0625
	// 1.41421356 0521602630615234375
	// 1.414213562373095 145474621858738828
	// 1.414213562373095 048801688724209698
	// */

	// Check if nnz(A) works
	//std::cout << "NNZ = " << nnz(A) << '\n';
	//std::cout << "Min = " << minelement(A) << '\n';
	//std::cout << "Max = " << maxelement(A) << '\n';
	//std::cout << "Range = " << sw::universal::dynamic_range<Scalar>() << '\n';
	// Check if external variable works
	// std::cout << "import = " << A << '\n';

	//std::cout << "Max. Positive Value = " << maxpos << '\n';
	//std::cout << "Max. Neg Value = " << maxneg << '\n';

	//squeeze(A,B, maxneg, maxpos);
	//squeeze(b,b_, maxneg, maxpos);

	//std::cout << "NNZ = " << nnz(B) << '\n';
	//std::cout << "A = \n" << A << '\n';
	//std::cout << "2*A = \n" << 2.*A << '\n';
	//std::cout << "Squeezed A = \n" << B << '\n';
	//std::cout << b << '\n';
	//std::cout << b_ << '\n';
	// NOTE: Jacobi converges IFF rho(D^{-1}(L+U)) < 1
	// iterations = Jacobi(A, b, x);
	// std::cout << "solution in " << iterations << " iterations\n";
	// std::cout << "x = " << x << '\n';
	// std::cout << "b_ = " << b_ << '\n';
	// std::cout << A * x << " = " << b << '\n';
	// std::cout << '\n' << std::endl;
	
	// x = solve(A, b);
	//std::cout << "A = LU: x = " << solve(A,b) << std::endl;
	//std::cout << "A = \n" << A << '\n';
	//std::cout << "--------------- " << '\n';
	// iterations = Jacobi(B, b_, x_);
	// std::cout << iterations << " iterations\n\n";
	// std::cout << "x = " << x_ << '\n';
	// std::cout << B * x_ << " = " << b_ << '\n';
	//DVector X_;
	TargetVector y_(N);
	TargetVector r(N);
	TargetVector d(N);
	//std::cout << "A =\n " << A << '\n' << "B = \n" << B << std::endl;
	//y_ = solve(B,b_);
	// std::cout << "x (posit solution) =  " << y_ << std::endl;
	/*
	for (size_t i=1;i<3;++i){
		r = posit<64,4>(b_) - posit<64,4>(B*y_);
		d = solve(B,r);
		y_ = y_ + d;	
		std::cout << "r =  " << r << std::endl;
		std::cout << "d =  " << d << std::endl;
		std::cout << "x (posit solution) =  " << y_ << std::endl;
	}
	*/
	for (size_t i=1;i<3;++i){
		//posit<64,2> bj = b_(i);
		Scalar bj = b_(i);
		r = bj;
	}
	// std::cout << "r<64,2> = " << r << std::endl;

	//y_ = solve(B,b_); 
	//std::cout << "b_ =  " << b_ << std::endl;
	
	 
	//DVector y(N);
	//DVector a(N);
	//Expand(B*y_,a);
	//Expand(y_, y);  // put back to double
	
	//std::cout << "norm(Bx - Ax,inf) =  " << normLinf(a - A*x) << std::endl; 
	//std::cout << "norm(x - y,inf) =  " << normLinf(x - y) << std::endl;
	//std::cout << "x (posit solution) =  " << y_ << std::endl;
	std::cout << "x (fp64 solution) =  " << x << std::endl;
	
	//std::cout << "B*y_ =  " << B*y_ << " ?= " << A*x << std::endl;
	//std::cout << "r =  " << b_ - B*y_ << std::endl;

	// std::cout << "Norm(x, inf) = " << normLinf(solve(B, b_)) << std::endl;
}







/* MAIN PROGRAM 
----------------------------
Control code:

	Calls `test` method

*/
int main()
try {
	using namespace sw::universal;
	using namespace sw::universal::blas;

	#define NUMERIC 5
	// 1 = POSIT, 2 = CFLOAT, 3 = Float, 4 = FIXPT.
	constexpr size_t nbits = 64;
	constexpr size_t es = 2; // different for cfloat
	constexpr size_t rbits = 8;
	constexpr size_t ebits = 5;

	// Table of Dyanmic Ranges
	{
	/*
	std::cout << "cfloat<16,5> = " << dynamic_range<cfloat<16,5,uint16_t, true, true, false>>() << '\n';
	std::cout << "cfloat<16,6> = " << dynamic_range<cfloat<16,6,uint16_t, true, true, false>>() << '\n';
	std::cout << "cfloat<16,7> = " << dynamic_range<cfloat<16,7,uint16_t, true, true, false>>() << '\n';
	std::cout << "cfloat<16,8> = " << dynamic_range<cfloat<16,8,uint16_t, true, true, false>>() << '\n';

	std::cout << "--------------- " << '\n';

	std::cout << "cfloat<16,5> = " << dynamic_range<cfloat<16,5,uint16_t, false, false, false>>() << '\n';
	std::cout << "cfloat<16,6> = " << dynamic_range<cfloat<16,6,uint16_t, false, false, false>>() << '\n';
	std::cout << "cfloat<16,7> = " << dynamic_range<cfloat<16,7,uint16_t, false, false, false>>() << '\n';
	std::cout << "cfloat<16,8> = " << dynamic_range<cfloat<16,8,uint16_t, false, false, false>>() << '\n';

	std::cout << "--------------- " << '\n';

	std::cout << "posit<16,0> = " << dynamic_range<posit<16,0>>() << '\n';
	std::cout << "posit<16,1> = " << dynamic_range<posit<16,1>>() << '\n';
	std::cout << "posit<16,2> = " << dynamic_range<posit<16,2>>() << '\n';
	std::cout << "posit<16,3> = " << dynamic_range<posit<16,3>>() << '\n';


	std::cout << "--------------- " << '\n';

	std::cout << "posit<8,0> = " << dynamic_range<posit<8,0>>() << '\n';
	std::cout << "posit<8,1> = " << dynamic_range<posit<8,1>>() << '\n';
	std::cout << "posit<8,2> = " << dynamic_range<posit<8,2>>() << '\n';
	std::cout << "posit<8,3> = " << dynamic_range<posit<8,3>>() << '\n';
	std::cout << "posit<8,4> = " << dynamic_range<posit<8,4>>() << '\n';
	
	std::cout << "cfloat<17,5> = " << dynamic_range<cfloat<17,5,uint16_t, false, false, false>>() << '\n';
	std::cout << "cfloat<17,6> = " << dynamic_range<cfloat<17,6,uint16_t, false, false, false>>() << '\n';
	std::cout << "cfloat<17,7> = " << dynamic_range<cfloat<17,7,uint16_t, false, false, false>>() << '\n';
	std::cout << "cfloat<17,8> = " << dynamic_range<cfloat<17,8,uint16_t, false, false, false>>() << '\n';

	std::cout << "--------------- " << '\n';

	std::cout << "posit<17,0> = " << dynamic_range<posit<17,0>>() << '\n';
	std::cout << "posit<17,1> = " << dynamic_range<posit<17,1>>() << '\n';
	std::cout << "posit<17,2> = " << dynamic_range<posit<17,2>>() << '\n';
	std::cout << "posit<17,3> = " << dynamic_range<posit<17,3>>() << '\n';
	*/
	}
	// Squeeze to Posit
	#if NUMERIC == 1 
	{
		posit<nbits, es> p, q, mp;  // mp = minpos
		p.maxpos();
		q.maxneg();
		mp.minpos();

		test<posit<nbits, es>>(q, p);
		std::cout << "Maxpos = " << p << '\n';
		std::cout << "Minpos = " << mp << '\n';
	}


	// Squeeze to Cfloat
	#elif NUMERIC == 2
	{
		using Cfloat = cfloat<nbits,ebits, uint16_t, true, false, false>;
		Cfloat a, b;
		a.maxneg();
		b.maxpos();

		test<Cfloat>(a,b);
	}
	

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
		test<double>(DBL_MAX,DBL_MIN);
	}
	#endif


	int nrOfFailedTestCases = 0;
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}


// Errors
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

/*
Test Suite - Small
---------------------------------------------------------
	DMatrix A = { 
		{ 10.0, -1.0, 2.0, 0.0}, 
		{ -1.0, 11.0, -1.0, 3.0},
		{ 2.0, -1.0, 10.0, -1.0},
		{ 0.0, 3.0, -1.0, 8.0 } };
	DVector b = { 6.0, 25.0, -11.0,15.0 };
	DVector x = {  0,0,0,0 };

	Matrix B(4,4);   // 3,3 squeeze mat.
	Vector b_(4);
	Vector x_(4);
 


	DMatrix A = frank<double>(N); 
	DVector b = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
	DVector x(N);  // zero vector

*/


/*
	DMatrix A = { 
		{ 10.0, -1.0, 2.0, 0.0}, 
		{ -1.0, 11.0, -1.0, 3.0},
		{ 2.0, -1.0, 10.0, -1.0},
		{ 0.0, 3.0, -1.0, 8.0 } };
	DVector b = { 6.0, 25.0, -11.0,15.0 };
	DVector x = {  0,0,0,0 };
	size_t N = num_cols(A); 
	Matrix B(N,N);   // 3,3 squeeze mat.
	Vector b_(N);
	Vector x_(N);
	*/


	// size_t iterations = 0;

