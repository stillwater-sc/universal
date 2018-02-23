// lu_decomposition.cpp example program comparing float vs posit equation solver
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// enable the mathematical constants in cmath: old-style preprocessor magic which isn't best practice anymore
#include "stdafx.h"

#include <vector>
#include <posit>
#include "math_functions.hpp"

using namespace std;
using namespace sw::unum;

// can the ratio a/b be represented exactly
bool isRepresentable(int a, int b) {
	if (b == 0) return false;
	while (b % 2 == 0) { b /= 2; }
	while (b % 5 == 0) { b /= 5; }
	return a % b == 0;
}

// These functions print matrices and vectors in a nice format
template<typename Ty>
void coutMatrix(const std::string& name, const std::vector<Ty>& m) {
	int d = (int)std::sqrt(m.size());
	cout << "Matrix: " << name << " is " << d << "x" << d << endl;
	for (int i = 0; i<d; ++i) {
		for (int j = 0; j<d; ++j) cout << setw(14) << m[i*d + j];
		cout << endl;
	}
}

template<typename Ty>
void coutVector(const std::string& name, const std::vector<Ty>& v) {
	int d = (int)v.size();
	cout << "Vector: " << name << " is of size " << d << " elements" << endl;
	for (int j = 0; j<d; ++j)cout << setw(14) << v[j];
	cout << endl;
}

// The following compact LU factorization schemes are described
// in Dahlquist, Bjorck, Anderson 1974 "Numerical Methods".
//
// S and D are d by d matrices.  However, they are stored in
// memory as 1D arrays of length d*d.  Array indices are in
// the C style such that the first element of array A is A[0]
// and the last element is A[d*d-1].
//
// These routines are written with separate source S and
// destination D matrices so the source matrix can be retained
// if desired.  However, the compact schemes were designed to
// perform in-place computations to save memory.  In
// other words, S and D can be the SAME matrix.  

// Crout implements an in-place LU decomposition, that is, S and D can be the same
// Crout uses unit diagonals for the upper triangle
template<typename Ty>
void Crout(int d, std::vector<Ty>& S, std::vector<Ty>& D) {
	for (int k = 0; k < d; ++k) {
		for (int i = k; i<d; ++i) {
			Ty sum = 0.;
			for (int p = 0; p<k; ++p) sum += D[i*d + p] * D[p*d + k];
			D[i*d + k] = S[i*d + k] - sum; // not dividing by diagonals
		}
		for (int j = k + 1; j < d; ++j) {
			Ty sum = 0.;
			for (int p = 0; p<k; ++p) sum += D[k*d + p] * D[p*d + j];
			D[k*d + j] = (S[k*d + j] - sum) / D[k*d + k];
		}
	}
}

// SolveCrout takes an LU decomposition, LU, and a right hand side vector, b, and produces a result, x.
template<typename Ty>
void SolveCrout(const std::vector<Ty>& LU, const std::vector<Ty>& b, std::vector<Ty>& x) {
	int d = (int)b.size();
	std::vector<Ty> y(d);
	for (int i = 0; i < d; ++i) {
		Ty sum = 0.0;
		for (int k = 0; k < i; ++k) sum += LU[i*d + k] * y[k];
		y[i] = (b[i] - sum) / LU[i*d + i];

	}
	for (int i = d - 1; i >= 0; --i) {
		Ty sum = 0.0;
		for (int k = i + 1; k < d; ++k) {
			cout << "lu[] = " << LU[i*d+k] << " x[" << k << "] = " << x[k] << endl;
			sum += LU[i*d + k] * x[k];
		}
		cout << "sum " << sum << endl;
		x[i] = (y[i] - sum); // not dividing by diagonals
	}
}

template<size_t nbits, size_t es, size_t capacity = 10>
void CroutFDP(int d, std::vector< posit<nbits, es> >& S, std::vector< posit<nbits, es> >& D) {
	for (int k = 0; k < d; ++k) {
		for (int i = k; i < d; ++i) {
			quire<nbits, es, capacity> q = 0.0;
			//for (int p = 0; p < k; ++p) q += D[i*d + p] * D[p*d + k];   if we had expression templates for the quire
			for (int p = 0; p < k; ++p) q += quire_mul(D[i*d + p], D[p*d + k]);
			posit<nbits, es> sum;
			sum.convert(q.to_value());   // one and only rounding step of the fused-dot product
			D[i*d + k] = S[i*d + k] - sum; // not dividing by diagonals
		}
		for (int j = k + 1; j < d; ++j) {
			quire<nbits, es, capacity> q = 0.0;
			//for (int p = 0; p < k; ++p) q += D[k*d + p] * D[p*d + j];   if we had expression templates for the quire
			for (int p = 0; p < k; ++p) q += quire_mul(D[k*d + p], D[p*d + j]);
			posit<nbits, es> sum;
			sum.convert(q.to_value());   // one and only rounding step of the fused-dot product
			D[k*d + j] = (S[k*d + j] - sum) / D[k*d + k];
		}
	}
}

// SolveCrout takes an LU decomposition, LU, and a right hand side vector, b, and produces a result, x.
template<size_t nbits, size_t es, size_t capacity = 10>
void SolveCroutFDP(const std::vector< posit<nbits, es> >& LU, const std::vector< posit<nbits, es> >& b, std::vector< posit<nbits, es> >& x) {
	int d = (int)b.size();
	std::vector< posit<nbits, es> > y(d);
	for (int i = 0; i < d; ++i) {
		quire<nbits, es, capacity> q = 0.0;
		// for (int k = 0; k < i; ++k) q += LU[i*d + k] * y[k];   if we had expression templates for the quire
		for (int k = 0; k < i; ++k) q += quire_mul(LU[i*d + k], y[k]);
		posit<nbits, es> sum;
		sum.convert(q.to_value());   // one and only rounding step of the fused-dot product
		y[i] = (b[i] - sum) / LU[i*d + i];
	}
	for (int i = d - 1; i >= 0; --i) {
		quire<nbits, es, capacity> q = 0.0;
		// for (int k = i + 1; k < d; ++k) q += LU[i*d + k] * x[k];   if we had expression templates for the quire
		for (int k = i + 1; k < d; ++k) {
			cout << "lu[] = " << LU[i*d + k] << " x[" << k << "] = " << x[k] << endl;
			q += quire_mul(LU[i*d + k], x[k]);
		}
		posit<nbits, es> sum;
		sum.convert(q.to_value());   // one and only rounding step of the fused-dot product
		cout << "sum " << sum << endl;
		x[i] = (y[i] - sum); // not dividing by diagonals
	}
}

// Doolittle uses unit diagonals for the lower triangle
template<typename Ty>
void Doolittle(int d, std::vector<Ty>& S, std::vector<Ty>& D) {
	for (int k = 0; k < d; ++k) {
		for (int j = k; j < d; ++j) {
			Ty sum = 0.;
			for (int p = 0; p < k; ++p) sum += D[k*d + p] * D[p*d + j];
			D[k*d + j] = (S[k*d + j] - sum); // not dividing by diagonals
		}
		for (int i = k + 1; i < d; ++i) {
			Ty sum = 0.;
			for (int p = 0; p < k; ++p) sum += D[i*d + p] * D[p*d + k];
			D[i*d + k] = (S[i*d + k] - sum) / D[k*d + k];
		}
	}
}
// SolveDoolittle takes an LU decomposition, LU, and a right hand side vector, b, and produces a result, x.
template<typename Ty>
void SolveDoolittle(const std::vector<Ty>& LU, const std::vector<Ty>& b, std::vector<Ty>& x) {
	int d = (int)b.size();
	std::vector<Ty> y(d);
	for (int i = 0; i<d; ++i) {
		Ty sum = 0.;
		for (int k = 0; k<i; ++k)sum += LU[i*d + k] * y[k];
		y[i] = (b[i] - sum); // not dividing by diagonals
	}
	for (int i = d - 1; i >= 0; --i) {
		Ty sum = 0.;
		for (int k = i + 1; k<d; ++k)sum += LU[i*d + k] * x[k];
		x[i] = (y[i] - sum) / LU[i*d + i];
	}
}

// Cholesky requires the matrix to be symmetric positive-definite
template<typename Ty>
void Cholesky(int d, std::vector<Ty>& S, std::vector<Ty>& D) {
	for (int k = 0; k<d; ++k) {
		Ty sum = 0.;
		for (int p = 0; p<k; ++p)sum += D[k*d + p] * D[k*d + p];
		D[k*d + k] = sqrt(S[k*d + k] - sum);
		for (int i = k + 1; i<d; ++i) {
			Ty sum = 0.;
			for (int p = 0; p<k; ++p)sum += D[i*d + p] * D[k*d + p];
			D[i*d + k] = (S[i*d + k] - sum) / D[k*d + k];
		}
	}
}
// This version could be more efficient on some architectures
// Use solveCholesky for both Cholesky decompositions
template<typename Ty>
void CholeskyRow(int d, std::vector<Ty>& S, std::vector<Ty>& D) {
	for (int k = 0; k<d; ++k) {
		for (int j = 0; j<d; ++j) {
			Ty sum = 0.;
			for (int p = 0; p<j; ++p) sum += D[k*d + p] * D[j*d + p];
			D[k*d + j] = (S[k*d + j] - sum) / D[j*d + j];
		}
		Ty sum = 0.;
		for (int p = 0; p<k; ++p) sum += D[k*d + p] * D[k*d + p];
		D[k*d + k] = sqrt(S[k*d + k] - sum);
	}
}
// SolveCholesky takes an LU decomposition, LU, and a right hand side vector, b, and produces a result, x.
template<typename Ty>
void SolveCholesky(const std::vector<Ty>& LU, const std::vector<Ty>& b, std::vector<Ty>& x) {
	int d = (int)b.size();
	std::vector<Ty> y(d);
	for (int i = 0; i<d; ++i) {
		Ty sum = 0.;
		for (int k = 0; k<i; ++k) sum += LU[i*d + k] * y[k];
		y[i] = (b[i] - sum) / LU[i*d + i];
	}
	for (int i = d - 1; i >= 0; --i) {
		Ty sum = 0.;
		for (int k = i + 1; k<d; ++k) sum += LU[k*d + i] * x[k];
		x[i] = (y[i] - sum) / LU[i*d + i];
	}
}

void GenerateTestCase(int a, int b) {
	std::cout << std::setw(3) << a << "/" << std::setw(3) << b << (isRepresentable(a, b) ? " is    " : " is not") << " representable " << (a / double(b)) << std::endl;
}

void EnumerateTestCases() {
	for (int i = 0; i < 30; i += 3) {
		for (int j = 0; j < 70; j += 7) {
			GenerateTestCase(i, j);
		}
	}
}

template<typename Ty>
void CompareDecompositions(bool useQuire, int d, std::vector<Ty>& A, std::vector<Ty>& b) {
	std::vector<Ty> x(d);
	std::vector<Ty> LU(d*d);

	if (useQuire) {
		CroutFDP(d, A, LU);
		SolveCroutFDP(LU, b, x);
		coutMatrix("CroutFDP LU", LU);
		coutVector("Solution", x);

		cout << endl;
#if 0
		DoolittleFDP(d, A, LU);
		SolveDoolittleFDP(LU, b, x);
		coutMatrix("DoolittleFDP LU", LU);
		coutVector("Solution", x);

		cout << endl;

		CholeskyFDP(d, A, LU);
		SolveCholeskyFDP(LU, b, x);
		coutMatrix("CholeskyFDP Decomposition", LU);
		coutVector("Solution", x);
#endif

	}
	else {
		Crout(d, A, LU);
		SolveCrout(LU, b, x);
		coutMatrix("Crout LU", LU);
		coutVector("Solution", x);

		cout << endl;
#if 0
		Doolittle(d, A, LU);
		SolveDoolittle(LU, b, x);
		coutMatrix("Doolittle LU", LU);
		coutVector("Solution", x);

		cout << endl;

		Cholesky(d, A, LU);
		SolveCholesky(LU, b, x);
		coutMatrix("Cholesky Decomposition", LU);
		coutVector("Solution", x);
#endif
	}

}

int main(int argc, char** argv)
try {
	const size_t nbits = 28;
	const size_t es = 1;

	//typedef float MyType;
	typedef posit<nbits, es> MyType;
	MyType p;
	cout << "Using " << spec_to_string(p) << endl;

	// We want to solve the system Ax=b
	int d = 5;
	std::vector<MyType> A = { 
		2.,1.,1.,3.,2.,
		1.,2.,2.,1.,1.,
		1.,2.,9.,1.,5.,
		3.,1.,1.,7.,1.,
		2.,1.,5.,1.,8. };

	std::vector<MyType> b = { 9, 7, 18, 13, 17 }; //  { -2.0, 4.0, 3.0, -5.0, 1.0 };
	cout << "LinearSolve WITHOUT fused-dot product" << endl;
	CompareDecompositions(false, d, A, b); 
	cout << endl << ">>>>>>>>>>>>>>>>" << endl;
	cout << "LinearSolve WITH fused-dot product" << endl;
	CompareDecompositions(true, d, A, b);

	int nrOfFailedTestCases = 0;
	return nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
catch (char const* msg) {
	cerr << msg << endl;
	return EXIT_FAILURE;
}
catch (...) {
	cerr << "Caught unknown exception" << endl;
	return EXIT_FAILURE;
}
