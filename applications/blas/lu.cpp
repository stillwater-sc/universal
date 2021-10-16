// lu.cpp: example program comparing float vs posit Gaussian Elimination (LU Decomposition) equation solver
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the HPRBLAS project, which is released under an MIT Open Source license.
#ifdef _MSC_VER
#pragma warning(disable : 4514)   // unreferenced inline function has been removed
#pragma warning(disable : 4710)   // 'int sprintf_s(char *const ,const size_t,const char *const ,...)': function not inlined
#pragma warning(disable : 4820)   // 'sw::universal::value<23>': '3' bytes padding added after data member 'sw::universal::value<23>::_sign'
#pragma warning(disable : 5045)   // Compiler will insert Spectre mitigation for memory load if /Qspectre switch specified
#endif

#include <chrono>
// enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
// enable fast posits
#define POSIT_FAST_SPECIALIZATION
#include <universal/blas/blas.hpp>
#include <universal/blas/generators.hpp>
#include <universal/functions/isrepresentable.hpp>

template<size_t nbits, size_t es, size_t capacity = 10>
void BenchmarkLUDecomposition(sw::universal::blas::matrix< sw::universal::posit<nbits, es> >& A, sw::universal::blas::vector< sw::universal::posit<nbits, es> >& x, sw::universal::blas::vector< sw::universal::posit<nbits, es> >& b) {
	using namespace sw::universal;
	using namespace sw::universal::blas;
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

template<size_t nbits, size_t es>
void GaussianEliminationTest() {
	using namespace sw::universal;
	using namespace sw::universal::blas;
	using Scalar = sw::universal::posit<nbits, es>;
	using Vector = sw::universal::blas::vector<Scalar>;
	using Matrix = sw::universal::blas::matrix<Scalar>;
	std::cout << "Using " << dynamic_range<nbits, es>() << '\n';

	// repeat set up for posits
	std::cout << "Posit inputs\n";
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
	using Matrix = sw::universal::blas::matrix<Scalar>;

	Matrix A = {
	{ 5, 4, 3, 2, 1 },
	{ 4, 4, 3, 2, 1 },
	{ 0, 3, 3, 2, 1 },
	{ 0, 0, 2, 2, 1 },
	{ 0, 0, 0, 1, 1 }
	};

	auto LU = lu(A);
	std::cout << "\n---------------- result ------------------\n";
	std::cout << "Combined matrix\n" << LU << '\n';
	auto D = diag(diag(LU));
	auto L = tril(LU) - D + sw::universal::blas::eye<Scalar>(num_cols(A));
	auto U = triu(LU);
	std::cout << "Lower Triangular matrix\n" << L << '\n';
	std::cout << "Upper Triangular matrix\n" << U << '\n';
}

template<typename Scalar>
void FrankMatrixTest() {
	using Vector = sw::universal::blas::vector<Scalar>;
	using Matrix = sw::universal::blas::matrix<Scalar>;

	Matrix A = {
	{ 5, 4, 3, 2, 1 },
	{ 4, 4, 3, 2, 1 },
	{ 0, 3, 3, 2, 1 },
	{ 0, 0, 2, 2, 1 },
	{ 0, 0, 0, 1, 1 }
	};

	Vector x(5);
	x = Scalar(1);  // vector of 1's
	Vector b(5);
	b = A * x;
	// now solve for b should yield a vector of 1's
	sw::universal::blas::vector<size_t> p;
	ludcmp(A, p);
	auto xx = lubksb(A, p, b);
	auto e = xx - x;
	Scalar infnorm = -1;
	for (auto v : e) {
		if (fabs(v) > infnorm) {
			infnorm = fabs(v);
		}
	}
	std::cout << "\nSolution vector for type " << std::setw(30) << typeid(Scalar).name() << " is [" << xx << "]" << " infinity norm of error " << infnorm << '\n';
}

void FrankMatrix() {
	using namespace sw::universal;
	std::cout << "Frank matrix solver\n";
	FrankMatrixTest<float>();
	FrankMatrixTest<double>();
	FrankMatrixTest<long double>();
	FrankMatrixTest< posit<16, 1> >();
	FrankMatrixTest< posit<20, 1> >();
	FrankMatrixTest< posit<28, 1> >();
	FrankMatrixTest< posit<28, 2> >();    // <---- this has the same number of fraction bits at 1 as IEEE single precision
	FrankMatrixTest< posit<32, 2> >();
	FrankMatrixTest< posit<64, 3> >();
}

template<typename Scalar>
void MagicSquareTest(int N) {
	using namespace sw::universal::blas;
	using Vector = sw::universal::blas::vector<Scalar>;
	using Matrix = sw::universal::blas::matrix<Scalar>;

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
		if (fabs(v - 1) > 0.00001) {
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

int main(int argc, char** argv)
try {
	using namespace sw::universal;
	using namespace sw::universal::blas;

	// We want to solve the system Ax=b
	// GaussianEliminationTest<32, 2>();

	if (argc == 1) std::cout << argv[0] << '\n';
	int nrOfFailedTestCases = 0;

	FrankMatrix();

	/*
	MagicSquareTest<float>(5);
	MagicSquareTest<float>(51);
	MagicSquareTest<float>(251);
	MagicSquareTest<float>(501);
	MagicSquareTest<double>(501);

	MagicSquareTest<posit<32, 2> >(51);
	MagicSquareTest<posit<32, 2> >(251);
	*/

	// basic structure from MATLAB
	//	[L U P] = lu(A);
	//	y = L\(P*b);
	//	x = U\y;

#if 0
	constexpr float eps = std::numeric_limits<float>::epsilon();
	constexpr float epsminus = 1.0f - eps;
	constexpr float epsplus = 1.0f + eps;

	cout << setprecision(std::numeric_limits<float>::max_digits10);
	cout << "posit<25,2>\n";
	cout << "1.0 - FLT_EPSILON = " << epsminus << " converts to posit value " << posit<25, 2>(epsminus) << endl;
	cout << "1.0 + FLT_EPSILON = " << epsplus << " converts to posit value " << posit<25, 2>(epsplus) << endl;
	cout << "posit<26,2>\n";
	cout << "1.0 - FLT_EPSILON = " << epsminus << " converts to posit value " << posit<26, 2>(epsminus) << endl;
	cout << "1.0 + FLT_EPSILON = " << epsplus << " converts to posit value " << posit<26, 2>(epsplus) << endl;
	cout << "posit<27,2>\n";
	cout << "1.0 - FLT_EPSILON = " << epsminus << " converts to posit value " << posit<27, 2>(epsminus) << endl;
	cout << "1.0 + FLT_EPSILON = " << epsplus << " converts to posit value " << posit<27, 2>(epsplus) << endl;
	cout << "posit<28,2>\n";
	cout << "1.0 - FLT_EPSILON = " << epsminus << " converts to posit value " << posit<28, 2>(epsminus) << endl;
	cout << "1.0 + FLT_EPSILON = " << epsplus << " converts to posit value " << posit<28, 2>(epsplus) << endl;
	cout << "posit<29,2>\n";
	cout << "1.0 - FLT_EPSILON = " << epsminus << " converts to posit value " << posit<29, 2>(epsminus) << endl;
	cout << "1.0 + FLT_EPSILON = " << epsplus << " converts to posit value " << posit<29, 2>(epsplus) << endl;
	cout << "posit<30,2>\n";
	cout << "1.0 - FLT_EPSILON = " << epsminus << " converts to posit value " << posit<30, 2>(epsminus) << endl;
	cout << "1.0 + FLT_EPSILON = " << epsplus << " converts to posit value " << posit<30, 2>(epsplus) << endl;
	cout << "posit<31,2>\n";
	cout << "1.0 - FLT_EPSILON = " << epsminus << " converts to posit value " << posit<31, 2>(epsminus) << endl;
	cout << "1.0 + FLT_EPSILON = " << epsplus << " converts to posit value " << posit<31, 2>(epsplus) << endl;
	cout << "posit<32,2>\n";
	cout << "1.0 - FLT_EPSILON = " << epsminus << " converts to posit value " << posit<32, 2>(epsminus) << endl;
	cout << "1.0 + FLT_EPSILON = " << epsplus << " converts to posit value " << posit<32, 2>(epsplus) << endl;

	cout << endl << endl;
	// is there a representational difference between 1+FLT_EPSILON in float versus posit<32,2>?
	double depsplus = epsplus;
	double pepsplus = double(posit<32, 2>(epsplus));
	cout << "1.0 + FLT_EPSILON = " << depsplus << " converts to posit value " << epsplus << endl;
	cout << "1.0 + FLT_EPSILON = " << depsplus << " converts to posit value " << pepsplus << endl;

	cout << "1.0 + FLT_EPSILON = " << epsplus << " converts to posit value " << hex_format(posit<32, 2>(epsplus)) << endl;
	cout << "1.0 + FLT_EPSILON = " << depsplus << " converts to posit value " << hex_format(posit<32, 2>(pepsplus)) << endl;

#endif

	return (nrOfFailedTestCases == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
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
catch (std::runtime_error& err) {
	std::cerr << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
