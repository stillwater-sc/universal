// jacobi.cpp: Jacobi iterative method
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
// Authors: Theodore Omtzigt, Allan Leal
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#ifdef _MSC_VER
#pragma warning(disable : 4514)   // unreferenced inline function has been removed
#pragma warning(disable : 4710)   // 'int sprintf_s(char *const ,const size_t,const char *const ,...)': function not inlined
#pragma warning(disable : 4820)   // 'sw::universal::value<23>': '3' bytes padding added after data member 'sw::universal::value<23>::_sign'
#pragma warning(disable : 5045)   // Compiler will insert Spectre mitigation for memory load if /Qspectre switch specified
#endif

// standard library
#include <limits>
// Configure the posit library to use arithmetic exceptions
// enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit/posit.hpp>
// Configure the cfloat library to use arithmetic exceptions
#define CFLOAT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/cfloat/cfloat.hpp>
// bring in the linear algebra constructs
#include <universal/blas/blas.hpp>
#include <universal/blas/generators.hpp>
#include <universal/blas/solvers/jacobi.hpp>


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
	std::cout << "Jacobi iteration on Scalar type: " << typeid(Scalar).name() << '\n';
	using Matrix = sw::universal::blas::matrix<Scalar>;
	using Vector = sw::universal::blas::vector<Scalar>;

	// Initialize 'A' 'b' & intial guess 'x' * _
	Matrix A = {
		{ 5, -2,  3,  0},
		{-3,  9,  1, -2},
		{ 2, -1, -7,  1},
		{ 4,  3, -5,  7} };
	Vector b = { -1, 2, 3, 0.5 };
	Vector x = { 0, 0, 0, 0 };

	std::cout << A << '\n';
	std::cout << b << '\n';
	size_t iterations = sw::universal::blas::Jacobi(A, b, x);
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
