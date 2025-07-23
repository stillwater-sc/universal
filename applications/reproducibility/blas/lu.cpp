// lu.cpp: example program comparing float vs posit Gaussian Elimination (LU Decomposition) equation solver
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the HPRBLAS project, which is released under an MIT Open Source license.
#include <chrono>
#include <universal/utility/directives.hpp>
#include <universal/native/ieee754.hpp>
// enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
// enable fast posits
#define POSIT_FAST_SPECIALIZATION
#include <universal/number/posit/posit.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <blas/blas.hpp>
#include <blas/generators.hpp>
#include <blas/ext/posit_fused_blas.hpp>   // addition of fdp, fmv, and fmm functions
#include <blas/ext/solvers/posit_fused_lu.hpp>

using namespace sw::universal;
using namespace sw::numeric::containers;
using namespace sw::blas;
using namespace sw::blas::solvers;

template<unsigned nbits, unsigned es, unsigned capacity = 10>
void BenchmarkLUDecomposition(matrix< posit<nbits, es> >& A, vector< posit<nbits, es> >& x, vector< posit<nbits, es> >& b) {
	assert(num_rows(A) == num_cols(A));

	size_t N = num_cols(A);
	matrix< posit<nbits, es> > LU(N,N);

	{
		using namespace std::chrono;
		steady_clock::time_point t1 = steady_clock::now();
		Crout(A, LU);
		steady_clock::time_point t2 = steady_clock::now();
		duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
		double elapsed = time_span.count();
		std::cout << "Crout took " << elapsed << " seconds.\n";
		std::cout << "Performance " << (uint32_t)(N*N*N / (1000 * elapsed)) << " KOPS/s\n";

		SolveCrout(LU, b, x);
		std::cout << "Crout LU\n" << LU << '\n';
		std::cout << "Solution\n" << x << '\n';
		std::cout << "RHS\n" << b << '\n';
	}

	{
		using namespace std::chrono;
		steady_clock::time_point t1 = steady_clock::now();
		CroutFDP(A, LU);
		steady_clock::time_point t2 = steady_clock::now();
		duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
		double elapsed = time_span.count();
		std::cout << "CroutFDP took " << elapsed << " seconds.\n";
		std::cout << "Performance " << (uint32_t)(N * N * N / (1000 * elapsed)) << " KOPS/s\n";

		SolveCroutFDP(LU, b, x);
		std::cout << "CroutFDP LU\n" << LU << '\n';
		std::cout << "Solution\n" << x << '\n';
		std::cout << "RHS\n" << b << '\n';
	}

	std::cout << std::endl;
}

template<unsigned nbits, unsigned es>
void GaussianEliminationTest() {
	using Scalar = posit<nbits, es>;
	using Vector = vector<Scalar>;
	using Matrix = matrix<Scalar>;
	Scalar a;
	std::cout << "Using " << dynamic_range(a) << '\n';


	Matrix U = {     // define the upper triangular matrix
		{ 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0 },
		{ 0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0 },
		{ 0.0, 0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0 },
		{ 0.0, 0.0, 0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0 },
		{ 0.0, 0.0, 0.0, 0.0, 1.0, 2.0, 3.0, 4.0, 5.0 },
		{ 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 2.0, 3.0, 4.0 },
		{ 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 2.0, 3.0 },
		{ 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 2.0 },
		{ 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0 },
	};
	Matrix L = transpose(U);

	auto A = L * U;   // construct the A matrix to solve
	std::cout << "L\n" << L << '\n';
	std::cout << "U\n" << U << '\n';
	std::cout << "A\n" << A << '\n';
	size_t N = num_cols(A);
	// define a difficult solution
	Scalar epsplus = Scalar(1) + std::numeric_limits<Scalar>::epsilon();
	Vector x(N);
	x = epsplus;
	auto b = fmv(A, x);   // construct the right hand side
	std::cout << "b" << b << '\n';
	std::cout << "\n>>>>>>>>>>>>>>>>\n";

	BenchmarkLUDecomposition(A, x, b);
}

template<typename Scalar>
void LUTest() {
	using Matrix = matrix<Scalar>;

	Matrix A = {
	{ 5, 4, 3, 2, 1 },
	{ 4, 4, 3, 2, 1 },
	{ 0, 3, 3, 2, 1 },
	{ 0, 0, 2, 2, 1 },
	{ 0, 0, 0, 1, 1 }
	};

	std::cout << "---------------- LUTest ------------------\n";
	std::cout << "Original matrix\n" << A << '\n';
	auto LU = lu(A);
	std::cout << "---------------- result ------------------\n";
	std::cout << "Combined matrix\n" << LU << '\n';
	auto D = diag(diag(LU));
	auto L = tril(LU) - D + eye<Matrix>(num_cols(A));
	auto U = triu(LU);
	std::cout << "Lower Triangular matrix\n" << L << '\n';
	std::cout << "Upper Triangular matrix\n" << U << '\n';
}

template<typename Scalar>
void LUwithoutQuire() {
	using Vector = vector<Scalar>;
	using Matrix = matrix<Scalar>;

	const unsigned N = 5;
	Matrix A = {
	{ 5, 4, 3, 2, 1 },
	{ 4, 4, 3, 2, 1 },
	{ 0, 3, 3, 2, 1 },
	{ 0, 0, 2, 2, 1 },
	{ 0, 0, 0, 1, 1 }
	};
	Vector x(N), b(N);
	// define a difficult solution
	Scalar epsplus = Scalar(1) + std::numeric_limits<Scalar>::epsilon();
	x = epsplus;
	matvec(b, A, x);
	std::cout << "reference x = " << x << '\n';
	x = solve(A, b);
	std::cout << "solution  x = " << x << '\n';
}

template<typename Scalar>
void FrankMatrixTest() {
	using std::abs;
	using Vector = vector<Scalar>;
	using Matrix = matrix<Scalar>;

	Matrix A = {
		{ 9, 8, 7, 6, 5, 4, 3, 2, 1 },
		{ 8, 8, 7, 6, 5, 4, 3, 2, 1 },
		{ 0, 7, 7, 6, 5, 4, 3, 2, 1 },
		{ 0, 0, 6, 6, 5, 4, 3, 2, 1 },
		{ 0, 0, 0, 5, 5, 4, 3, 2, 1 },
		{ 0, 0, 0, 0, 4, 4, 3, 2, 1 },
		{ 0, 0, 0, 0, 0, 3, 3, 2, 1 },
		{ 0, 0, 0, 0, 0, 0, 2, 2, 1 },
		{ 0, 0, 0, 0, 0, 0, 0, 1, 1 }
	};

	Vector x(9);
	x = Scalar(1);  // vector of 1's
	Vector b(9);
	b = A * x;
	// now solve for b should yield a vector of 1's
	vector<size_t> p;  // Clang fix that treats size_t and std::uint64_t as two different types
	ludcmp(A, p);
	auto xx = lubksb(A, p, b);
	auto e = xx - x;
	Scalar infnorm = -1;
	for (auto v : e) {
		if (abs(v) > infnorm) {
			infnorm = abs(v);
		}
	}
	std::cout << "Solution vector for type " << std::setw(32) << typeid(Scalar).name() << " is [" << xx << "]" << " infinity norm of error " << infnorm << '\n';
}

void FrankMatrix() {
	std::cout << "Frank matrix solver\n";
	FrankMatrixTest<float>();
	FrankMatrixTest<double>();
	FrankMatrixTest<long double>();
	FrankMatrixTest< posit<16, 1> >();
	FrankMatrixTest< posit<28, 2> >();    // <---- this has the same number of fraction bits at 1 as IEEE single precision
	FrankMatrixTest< posit<32, 2> >();
	FrankMatrixTest< posit<40, 2> >();
	FrankMatrixTest< posit<48, 2> >();
	FrankMatrixTest< posit<56, 2> >();
	FrankMatrixTest< posit<64, 3> >();
}

template<typename Scalar>
void MagicSquareTest(unsigned N) {
	using std::abs;
	using Vector = vector<Scalar>;
	using Matrix = matrix<Scalar>;

	Matrix A = magic<Scalar>(N);
	Scalar magicSum = sum(diag(A));
	Vector b(N), x(N); 
	b = magicSum;
	using namespace std::chrono;
	steady_clock::time_point t1 = steady_clock::now();
	x = solve(A, b);
	steady_clock::time_point t2 = steady_clock::now();
//	std::cout << "solution x\n" << x << '\n';
	bool bFail = false;
	for (auto v : x) {
		if (abs(v - 1) > 0.00001) {
			std::cout << v << " outside of range 1.0+-0.00001\n";
			bFail = true;
			break;
		}
	}
	if (bFail) std::cout << "FAIL for " << typeid(Scalar).name() << " when N = " << N << '\n';
	else std::cout << "PASS for N = " << typeid(Scalar).name() << " when N = " << N << '\n';
	duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
	double elapsed = time_span.count();
	std::cout << "solve took " << elapsed << " seconds: ";
	double nrOps = double(N) * double(N) * double(N) / (3.0 * elapsed);
	std::cout << "performance ";
	if (nrOps > 1000000.0) {
		std::cout << (uint32_t)(nrOps * 1e-6) << " MOPS/s\n";
	}
	else {
		std::cout << (uint32_t)(nrOps * 1e-3) << " KOPS/s\n";
	}
}

void MagicSquareMatrix() {
	std::cout << "Magic Square matrix solver\n";
	MagicSquareTest<float>(5);
	MagicSquareTest<float>(51);
	MagicSquareTest<float>(251);
	MagicSquareTest<float>(501);
	MagicSquareTest<double>(501);
	MagicSquareTest<posit<32, 2> >(51);
	// MagicSquareTest<posit<32, 2> >(251);
}

template<typename Posit>
void PrintPositsAroundOne() {
	constexpr float eps = std::numeric_limits<float>::epsilon();
	constexpr float epsminus = 1.0f - eps;
	constexpr float epsplus = 1.0f + eps;
	Posit pepsminus(epsminus), pepsplus(epsplus);
	std::string tag = type_tag(pepsminus);
	std::cout << tag << '\n';
	std::cout << "1.0 - FLT_EPSILON:\n";
	std::cout << "         float       : "   << to_binary(epsminus) << " : " << epsminus << '\n';
	std::cout << "         " << tag << " : " << color_print(pepsminus) << " : " << pepsminus << '\n';
	std::cout << "1.0 + FLT_EPSILON:\n";
	std::cout << "         float       : "   << to_binary(epsplus) << " : " << epsplus << '\n';
	std::cout << "         " << tag << " : " << color_print(pepsplus) << " : " << pepsplus << '\n';
}

void FloatVsPositAroundOne() {
	using namespace sw::universal;

	PrintPositsAroundOne < posit<26, 2> >();
	PrintPositsAroundOne < posit<27, 2> >();
	PrintPositsAroundOne < posit<28, 2> >();  // => equivalent to float around 1.0
}

int main()
try {

	// We want to solve the system Ax=b
	GaussianEliminationTest<32, 2>();

	LUwithoutQuire<cfloat<40, 8, uint32_t, true, false, false>>();

	std::cout << std::setprecision(std::numeric_limits<float>::max_digits10);
	FloatVsPositAroundOne();

	std::cout << '\n';
	FrankMatrix();

	std::cout << '\n';
	MagicSquareMatrix();

	// basic workflow used in MATLAB
	//	[L U P] = lu(A);
	//	y = L\(P*b);
	//	x = U\y;

	std::cout << '\n';
	LUTest<posit<32, 2>>();

	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const universal_internal_exception& err) {
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
