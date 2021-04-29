
// ChebyTESTS
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Author: James Quinlan


// Dependencies
#include<cmath> /* sin, cos, etc. */
// using namespace sw::universal::blas;
//	using Matrix = sw::universal::blas::matrix<Scalar>;
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
#include <universal/number/posit/posit>
#include <universal/blas/blas>
#include "chebpts.hpp"
#include "prod.hpp"


int main()
try {
	using namespace std;
	using namespace sw::universal;
	using namespace sw::universal::blas;
	
	#define USE_POSIT 0
	#if USE_POSIT
		constexpr size_t nbits = 16;
		constexpr size_t es = 1;
		using Scalar = posit<nbits, es>;
		cout << "\nUsing POSIT<" << nbits << "," <<  es << ">\n" <<  endl;
	#else	  
		using Scalar = double;
	#endif

	// 

	// 1. Test Chebpts
	auto x = chebpts<Scalar>(5,1); 				// x = chebypts(n,kind,[a,b])
	std::cout << "Chebyshev 1st kind = " << x << std::endl;				// Print results
	
	auto y = chebpts<Scalar>(5); 				// x = chebypts(n,kind,[a,b])
	std::cout << "Chebyshev 2nd kind = " << y << std::endl;				// Print results

	auto z = chebpts<Scalar>(-3,1); 				// x = chebypts(n,kind,[a,b])
	std::cout << "Chebyshev WRONG = " << z << std::endl;				// Print results


	// 2. Calculate mean distance
	//auto y = prod<Scalar>(x);
	// std::cout << y << std::endl;

	













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