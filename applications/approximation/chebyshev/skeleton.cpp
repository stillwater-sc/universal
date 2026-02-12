// skeleton.cpp: chebyshev skeleton
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
// Configure the posit library with arithmetic exceptions
// enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit/posit.hpp>
//#include <universal/number/fixpnt/fixpnt.hpp> 
// Stillwater BLAS library
#include <blas/blas.hpp>

// skeleton environment to experiment with Chebyshev polynomials and approximations
int main()
try {
	using namespace sw::universal;
	using namespace sw::blas;

	std::cout << "Chebyshev polynomial test skeleton\n";

//	using Scalar = fixpnt<32,16, Modulo, uint32_t>;
	using Scalar = posit<32, 2>;
	Scalar PI{ 3.14159265358979323846 };  // best practice for C++
	constexpr int N = 12;
	auto k = arange<Scalar>(0, N);
	std::cout << "k       = " << k << '\n';
	auto cosines = -cos(k * PI / N);
	std::cout << "cosines = " << cosines << '\n';

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
