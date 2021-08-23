// l3_fused_mm.cpp: example program showing a fused matrix-matrix product
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#ifdef _MSC_VER
#pragma warning(disable : 4100) // argc/argv unreferenced formal parameter
#pragma warning(disable : 4514 4571)
#pragma warning(disable : 4625 4626) // 4625: copy constructor was implicitly defined as deleted, 4626: assignment operator was implicitely defined as deleted
#pragma warning(disable : 5025 5026 5027 5045)
#pragma warning(disable : 4710 4774)
#pragma warning(disable : 4820)
#endif
// enable the following define to show the intermediate steps in the fused-dot product
// #define POSIT_VERBOSE_OUTPUT
#define POSIT_TRACE_MUL
#define QUIRE_TRACE_ADD
// enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit/posit.hpp>
#define BLAS_TRACE_ROUNDING_EVENTS 1
#include <universal/blas/blas.hpp>

template<typename Scalar>
std::string conditional_fdp(const sw::universal::blas::vector< Scalar >& a, const sw::universal::blas::vector< Scalar >& b) {
	return std::string("no FDP for non-posit value_type");
}
template<size_t nbits, size_t es>
std::string conditional_fdp(const sw::universal::blas::vector< sw::universal::posit<nbits, es> >& a, const sw::universal::blas::vector< sw::universal::posit<nbits, es> >& b) {
	std::stringstream ss;
	ss << sw::universal::fdp(a, b);
	return ss.str();
}

template<typename Scalar>
void check_precision() {
	Scalar a1 = 3.2e8;
	Scalar a2 = 1;
	Scalar a3 = -1;
	Scalar a4 = 8e7;

	Scalar b1 = 4.0e7;
	Scalar b2 = 1;
	Scalar b3 = -1;
	Scalar b4 = -1.6e8;

	std::cout << a1 << " * " << b1 << " = " << a1 * b1 << '\n';
	std::cout << a2 << " * " << b2 << " = " << a2 * b2 << '\n';
	std::cout << a3 << " * " << b3 << " = " << a3 * b3 << '\n';
	std::cout << a4 << " * " << b4 << " = " << a4 * b4 << '\n';

	std::cout << a1 << " * " << b4 << " = " << a1 * b4 << '\n';
	std::cout << a2 << " * " << b3 << " = " << a2 * b3 << '\n';
	std::cout << a3 << " * " << b2 << " = " << a3 * b2 << '\n';
	std::cout << a4 << " * " << b1 << " = " << a4 * b1 << '\n';

	sw::universal::blas::vector<Scalar> a = { a1, a2, a3, a4 };
	sw::universal::blas::vector<Scalar> b_v1 = { b1, b2, b3, b4 };
	sw::universal::blas::vector<Scalar> b_v2 = { b4, b3, b2, b1 };

	std::cout << "dot(a,b)         " << sw::universal::blas::dot(a, b_v1) << '\n';
	std::cout << "dot(a,b_flipped) " << sw::universal::blas::dot(a, b_v2) << '\n';
	std::cout << "fdp(a,b)         " << conditional_fdp(a, b_v1) << '\n';
	std::cout << "fdp(a,b_flipped) " << conditional_fdp(a, b_v2) << '\n';
}

template<typename Scalar>
void catastrophicCancellationTest() {
	std::cout << "\nScalar type : " << typeid(Scalar).name() << '\n';
	using Matrix = sw::universal::blas::matrix<Scalar>;

	Scalar a1 = 3.2e8;
	Scalar a2 = 1;
	Scalar a3 = -1;
	Scalar a4 = 8e7;
	Matrix A = { 
		{ a1, a2, a3, a4 }, 
		{ a4, a3, a2, a1 },
	};
	Scalar b1 = 4.0e7;
	Scalar b2 = 1;
	Scalar b3 = -1;
	Scalar b4 = -1.6e8;
	Matrix B = {
		{ b1, b4 },
		{ b2, b3 },
		{ b3, b2 },
		{ b4, b1 }
	};

	std::cout << std::setprecision(10);
	std::cout << "matrix A: \n" << A << '\n';
	std::cout << "matrix B: \n" << B << '\n';
	auto C = A * B;
	std::cout << "matrix C: \n" << C << '\n';
	if (C[0][0] == 2 && C[1][1] == 2) {
		std::cout << "PASS\n";
	}
	else {
		std::cout << "FAIL\n";
	}
}

int main(int argc, char** argv)
try {
	using namespace sw::universal::blas;

	catastrophicCancellationTest<float>();  // FAILS due to catastrophic cancellation
	catastrophicCancellationTest<double>(); // FAILS due to catastrophic cancellation
	catastrophicCancellationTest< sw::universal::posit<32,2> >(); // PASSES due to FDP
	catastrophicCancellationTest< sw::universal::posit<64, 3> >(); // PASSES due to FDP

//	check_precision<float>();
//	check_precision< sw::universal::posit<32, 2> >();

	{
		sw::universal::blas::matrix< sw::universal::posit<32, 2> > A(4, 4);
		A[0][0] = 1;
		std::cout << A << '\n';
	}

	{
		sw::universal::blas::matrix< sw::universal::posit<32, 2> > A(SIZE_1K, SIZE_1K);
		A[0][0] = 1;
		std::cout << "A(0,0) = " << A[0][0] << " A(SIZE_1K-1, SIZE_1K-1) = " << A[SIZE_1K - 1][SIZE_1K - 1] << '\n';
	}

	{
		using Real = sw::universal::posit<32,2>;
		sw::universal::blas::vector<Real> a = { 1, 2 };
		sw::universal::blas::vector<Real> b = { 2, 1 };
		std::cout << "fdp = " << sw::universal::fdp(a, b) << '\n';
	}

	try {
		sw::universal::blas::matrix<float> A(2, 3), B(2, 3);
		auto C = A * B;
	}
	catch (const sw::universal::blas::matmul_incompatible_matrices& err) {
		std::cerr << "Correctly caught incompatible matrix exeption:\n" << err.what() << std::endl;
	}
	catch (const std::runtime_error& err) {
		std::cerr << "Unexcpected runtime exception: " << err.what() << std::endl;
		return EXIT_FAILURE;
	}
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
