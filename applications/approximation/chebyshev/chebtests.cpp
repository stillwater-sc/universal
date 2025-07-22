// ChebyTESTS
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
// Author: James Quinlan
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Author: James Quinlan
#include <universal/utility/directives.hpp>

// Dependencies
#include<cmath> /* sin, cos, etc. */
// using namespace sw::universal::blas;
// using Matrix = sw::universal::blas::matrix<Scalar>;
/*

Mathematical 	C++ Symbol	Decimal Representation
Expression
pi				M_PI		3.14159265358979323846
pi/2			M_PI_2		1.57079632679489661923
pi/4			M_PI_4		0.785398163397448309616
1/pi			M_1_PI		0.318309886183790671538
2/pi			M_2_PI		0.636619772367581343076
2/sqrt(pi)		M_2_SQRTPI	1.12837916709551257390
sqrt(2)			M_SQRT2		1.41421356237309504880
1/sqrt(2)		M_SQRT1_2	0.707106781186547524401
e				M_E			2.71828182845904523536
log_2(e)		M_LOG2E		1.44269504088896340736
log_10(e)		M_LOG10E	0.434294481903251827651
log_e(2)		M_LN2		0.693147180559945309417
log_e(10)		M_LN10		2.30258509299404568402

*/
// Configure the posit library with arithmetic exceptions
// enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit/posit.hpp>
// Stillwater BLAS library
#include <blas/blas.hpp>
#include "chebpts.hpp"
//#include "prod.hpp"
//#include "diff.hpp"
#include "meandistance.hpp"
//#include "linscale.hpp"
//#include "ones.hpp"
#include "rpad.hpp"
#include "chebpoly.hpp"
#include "chebfun.hpp"
#include "chebmat.hpp"

int main()
try {
	using namespace sw::universal;
	using namespace sw::universal::blas;
	using namespace chebyshev;
	
	#define USE_POSIT 0
	#if USE_POSIT
		constexpr size_t nbits = 32;
		constexpr size_t es = 1;
		using Scalar = posit<nbits, es>;
		std::cout << "\nUsing POSIT<" << nbits << "," <<  es << ">\n\n";
	#else	  
		using Scalar = double;
		std::cout << "\nUsing DOUBLE " << Scalar(0.0) << "\n\n";
	#endif

	// TESTS
	// ---------------------------------------------------

	// 1. Test Chebpts
	//auto x = chebpts<Scalar>(5,1); 							// x = chebypts(n,1,[a,b])
	//std::cout << "Chebyshev 1st kind = " << x << std::endl;	 

	
	 // auto y = chebyshev::chebpts<Scalar>(10); 							// x = chebypts(n,2,[a,b])
	//std::cout << "Chebyshev 2nd kind = " << y << std::endl;	 

	//auto z = chebpts<Scalar>(-3,1); 						// x = chebypts(n,kind,[a,b])
	//std::cout << "Chebpts called with incorrect parameters = " << z << std::endl;				 


	// 2. Test prod(x)
	//blas::vector<Scalar> a(5);
	//a(0)=1;a(1)=2;a(2)=3;a(3)=4;a(4)=5; 
	
	 //auto b = prod<Scalar>(a);
	 //std::cout << b << std::endl;

	// 3. Checking abs and sqrt functions
	// std::cout << "Abs = " << abs(b) << " , SQRT =  " << sqrt(b) << std::endl;

	// 4. Test diff(x,y)
	//auto c = diff<Scalar>(x,y); 
	//std::cout << c << std::endl;

	// 5. Test meandistance(x)
	//auto d = chebyshev::meandistance<Scalar>(y);
	//std::cout << d << std::endl;

	// 6. Ones vector
	// blas::vector<Scalar> a(5,1);
	// std::cout << "a = " << a << std::endl;


	// 7. Test linear shift and scale interval 
		//blas::vector<Scalar> a(5);
		//a(0)=2;a(1)=3;a(2)=4;a(3)=5;a(4)=6; 
		//std::cout << "a = " << a << std::endl;

		//auto b = linscale<Scalar>(a,-M_PI,M_PI);
		//std::cout << "b = " << b << std::endl;

	// 8. Ones vector
	// auto b = ones<Scalar>(8);
	// std::cout << "b = " << b << std::endl;

	// 9. Norm test
	// blas::vector<Scalar> a(5);
	// a(0)=1;a(1)=2;a(2)=3;a(3)=4;a(4)=5; 
	// std::cout << "norm(a) = " << norm(a,1) << std::endl;

	// int INF = std::numeric_limits<int>::max();
	// std::cout << "norm(a) = " << norm(a,INF) << std::endl;

	// 10. pad right
	// blas::vector<Scalar> a(5);
	// a(0)=1;a(1)=2;a(2)=3;a(3)=4;a(4)=5;
	// std::cout << "padded vector = " << rpad(a,4) << std::endl;

	// 11. element-wise add
	//blas::vector<Scalar> a(5);
	//a(0)=1;a(1)=2;a(2)=3;a(3)=4;a(4)=5;

	//blas::vector<Scalar> b(5);
	//b(0)=3;b(1)=5;b(2)=7;b(3)=9;b(4)=11;
	//std::cout << "element-wise difference = " << a - b << std::endl;

	// 12. Chebpoly
	// size_t n = 1;
	//std::cout << "Chebyshev Polynomial T_0" << " = " << chebpoly<Scalar>(0) << std::endl;
	//std::cout << "Chebyshev Polynomial T_1" << " = " << chebpoly<Scalar>(1) << std::endl;
	//std::cout << "Chebyshev Polynomial T_2" << " = " << chebpoly<Scalar>(2) << std::endl;
	//std::cout << "Chebyshev Polynomial T_3" << " = " << chebpoly<Scalar>(3) << std::endl;

	//std::cout << chebpoly<Scalar>(3) + rpad<Scalar>(chebpoly<Scalar>(1), 3-1) << std::endl;
	//chebpoly<Scalar>(n);

	// 13.
	
//	auto f = chebyshev::chebfun<float>([]<typename T>(T x) {return sin(x);});
//	auto f = chebyshev::chebfun<float>(std::sin);
	auto M = chebyshev::chebmat<float>(5);
	std::cout << M << std::endl;








	return EXIT_SUCCESS;
}

// Runtime error statements
catch (char const* msg) {
	std::cerr << msg << std::endl;
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