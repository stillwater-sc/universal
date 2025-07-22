// jacobi.cpp: Jacobi iterative method
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
// Authors: Theodore Omtzigt, Allan Leal
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/long_double.hpp>

// Configure the posit library to use arithmetic exceptions
// enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit/posit.hpp>
// Configure the cfloat library to use arithmetic exceptions
#define CFLOAT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/cfloat/cfloat.hpp>
// bring in the linear algebra constructs
#include <blas/blas.hpp>

// specialized for native floating-point
template<typename Scalar,
	typename = typename std::enable_if<std::is_floating_point<Scalar>::value>::type>
Scalar normL1(const sw::universal::blas::vector<float>& v) {
	float L1Norm{ 0 };
	for (auto e : v) {
		L1Norm += std::abs(e);
	}
	return L1Norm;
}

template<typename Scalar>
void Test() {
	using Matrix = sw::universal::blas::matrix<Scalar>;
	using Vector = sw::universal::blas::vector<Scalar>;

	std::cout << "Jacobi iteration on Scalar type: " << typeid(Scalar).name() << '\n';
	std::cout << sw::universal::dynamic_range<Scalar>() << '\n';

	// Initialize 'A' 'b' & intial guess 'x' * _
	Matrix A = {
		{ 5, -2,  3,  0},
		{-3,  9,  1, -2},
		{ 2, -1, -7,  1},
		{ 4,  3, -5,  7} };
	Vector b = { -1, 2, 3, 0.5 };
	Vector x = { 0, 0, 0, 0 };


	std::cout << std::fixed << A << std::defaultfloat << '\n';
	std::cout << b << '\n';
	// solve to arithmetic type precision, defined by epsilon()
	Scalar tolerance = std::numeric_limits<Scalar>::epsilon();
	size_t iterations = sw::universal::blas::Jacobi(A, b, x, tolerance);
	std::cout << "solution in " << iterations << " iterations\n";
	std::cout << "solution is " << x << '\n';
	std::cout << A * x << " vs actual " << b << '\n';
	std::cout << "-----------------------\n";
}

int main()
try {
	using namespace sw::universal;
	using namespace sw::universal::blas;

	Test<float>();

	Test<cfloat<32, 8, uint32_t>>();

	Test<posit<32, 2>>();


	return EXIT_SUCCESS;
}
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
