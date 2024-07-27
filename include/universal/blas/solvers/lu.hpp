#pragma once
// lu.hpp: dense matrix LU decomposition and backsubstitution to solve systems of equations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <universal/blas/matrix.hpp>

#if defined(_MSC_VER)
#pragma warning(disable : 26451) //arithmetic overflow: operator+ on 4byte value and casting to 8 bytes may overflow
#endif

// compilation flags
// BLAS_TRACE_ROUNDING_EVENTS
// when set traces the quire operations
#ifndef BLAS_TRACE_ROUNDING_EVENTS
#define BLAS_TRACE_ROUNDING_EVENTS 0
#endif

namespace sw { namespace universal { namespace blas {

/*
template<typename Scalar>
class LUP {
public:
	void decompose(const matrix<Scalar>& A) {
		auto LU = lu(A);
		L = tril(LU);
		U = triu(LU);
	}
	vector<value_type> solve(const vector<Scalar>& b) {
		auto y = solve(L, (P * b));
		auto x = solve(U, y);
		return x;
	}

private:
	matrix<Scalar> L;
	matrix<Scalar> U;
	vector<size_t> P;
};
*/

// non-pivoting Gaussian Elimination
// The following compact LU factorization schemes are described
// in Dahlquist, Bjorck, Anderson 1974 "Numerical Methods".
//
//
// These routines are written with separate source S and
// destination D matrices so the source matrix can be retained
// if desired.  However, the compact schemes were designed to
// perform in-place computations to save memory.  In
// other words, S and D can be the SAME matrix.  

// Crout implements an in-place LU decomposition, that is, S and D can be the same
// Crout uses unit diagonals for the upper triangle


/////////////////////////////////////////////////////////////////////////////////////////////////////
/// Crout using MTL data structures

// Crout method using MTL data structures
template<typename Matrix>
void Crout(const Matrix& S, Matrix& D) {
	assert(num_rows(S) == num_rows(D));
	assert(num_cols(S) == num_cols(D));
	using value_type = typename Matrix::value_type;
	size_t N = num_rows(S);
	for (size_t k = 0; k < N; ++k) {
		for (size_t i = k; i < N; ++i) {
			value_type sum = 0.;
			for (size_t p = 0; p < k; ++p) sum += D[i][p] * D[p][k];
			D[i][k] = S[i][k] - sum; // not dividing by diagonals
		}
		for (size_t j = k + 1; j < N; ++j) {
			value_type sum = 0.;
			for (size_t p = 0; p < k; ++p) sum += D[k][p] * D[p][j];
			D[k][j] = (S[k][j] - sum) / D[k][k];
		}
	}
}

// SolveCrout: given an LU matrix, solve the equation: LU * x = b, through back substitution
template<typename Matrix, typename Vector>
void SolveCrout(const Matrix& LU, const Vector& b, Vector& x) {
	assert(num_cols(LU) == size(b));
	unsigned N = size(b);
	using value_type = typename Matrix::value_type;
	sw::universal::blas::vector<value_type> y(N);
	for (unsigned i = 0; i < N; ++i) {
		value_type sum = 0.0;
		for (size_t k = 0; k < size_t(i); ++k) sum += LU[i][k] * y[k];
		y[i] = (b[i] - sum) / LU[i][i];

	}
	for (int i = static_cast<int>(N) - 1; i >= 0; --i) {
		value_type sum = 0.0;
		for (unsigned k = i + 1; k < static_cast<int>(N); ++k) {
			//cout << "lu[] = " << LU[i][k] << " x[" << k << "] = " << x[k] << endl;
			sum += LU[i][k] * x[k];
		}
		//cout << "sum " << sum << endl;
		x[i] = (y[i] - sum); // not dividing by diagonals
	}
}


// in-place LU decomposition using partial pivoting with implicit pivoting applied
template<typename Scalar>
int ludcmp(matrix<Scalar>& A, vector<size_t>& indx) {
	using namespace std;
	using std::fabs;
	const size_t N = num_rows(A);
	if (N != num_cols(A)) {
		std::cerr << "matrix argument to ludcmp is not square: (" << num_rows(A) << " x " << num_cols(A) << ")\n";
		return 1;
	}
	indx.resize(N);
	indx = 0;
	// implicit pivoting pre-calculation
	vector<Scalar> implicitScale(N);	
	for (size_t i = 0; i < N; ++i) { // for each row
		Scalar pivot = 0;
		for (size_t j = 0; j < N; ++j) { // scan the columns for the biggest abs value
			Scalar e = fabs(A(i, j));
			if (e > pivot) pivot = e;
		}
		if (pivot == 0) {
			std::cerr << "LU argument matrix is singular\n";
			return 2;
		}
		implicitScale[i] = Scalar(1.0) / pivot; // save the scaling factor for that row
	}
	//int nrOfRowExchanges = 0;
	size_t imax = 0;
	for (size_t j = 0; j < N; ++j) { // loop over columns of Crout's method
		Scalar sum = 0;
		for (size_t i = 0; i < j; ++i) {
			sum = A(i, j);
			for (size_t k = 0; k < i; ++k) sum -= A(i, k) * A(k, j);
			A(i, j) = sum;
		}
		Scalar pivot = 0; // initialize for search for largest pivot element
		for (size_t i = j; i < N; ++i) {
			sum = A(i, j);
			for (size_t k = 0; k < j; ++k) sum -= A(i, k) * A(k, j);
			A(i, j) = sum;
			Scalar dum = implicitScale[i] * fabs(sum);
			if (dum >= pivot) { // is figure of merit better than the best so far?
				pivot = dum;
				imax = i;
			}
		}
		if (j != imax) {
			for (size_t k = 0; k < N; ++k) std::swap(A(imax, k), A(j, k));
			//++nrOfRowExchanges;
			implicitScale[imax] = implicitScale[j]; // interchange scaling factor
		}
//		std::cout << "scaling\n" << implicitScale << std::endl;
		indx[j] = imax;
		if (A(j, j) == 0) A(j, j) = std::numeric_limits<Scalar>::epsilon();
		if (j != N) {
			Scalar dum = Scalar(1) / A(j, j);
			for (size_t i = j + 1; i < N; ++i) A(i, j) *= dum;
		}
	}
//	cout << "index array\n" << indx << endl;
	return 0; // success
}

/*
// Solve the system LU . x = b
template<typename Scalar>
void lubksb(const matrix<Scalar>& LU, const vector<int>& permutation, const vector<Scalar>& _b, vector<Scalar>& x) {
	using namespace std;
	const size_t N = num_rows(LU);
	if (N != size(_b)) {
		std::cerr << "LU decomposition size is not congruent with size of right hand side\n";
		return;
	}
	using Vector = vector<Scalar>;
	Vector b(_b);
	Scalar sum = 0;
	size_t ii = 0;
	for (size_t i = 0; i < N; ++i) {
		ip = indx(i);
		sum = b(ip);
		b(ip) = b(i);
		if (ii) {
			for (size_t j = ii; j <= i; ++j) {
				sum -= LU(i, j) * b(j);
			}
		}
		else {
			if (sum) ii = i;
		}
	}

	for (size_t i = N; i >= 1; --i) {
		sum = b(i);
		for (size_t j = i + 1; j <= N; ++j) {
			sum -= LU(i, j) = b(j);
		}
		b(i) = sum / LU(i, i);
	}
}
*/


// LU decomposition using partial pivoting with implicit pivoting applied
template<typename Scalar>
matrix<Scalar> lu(const matrix<Scalar>& A) {
	using namespace std;
	const size_t N = num_rows(A);
	if (N != num_cols(A)) {
		std::cerr << "matrix argument is not square: (" << num_rows(A) << " x " << num_cols(A) << ")\n";
		return matrix<Scalar>{};
	}
	matrix<Scalar> B(A); 
	vector<size_t> p;
	if (ludcmp(B, p) == 1) {
		std::cerr << "LU decomposition failed\n";
		return matrix<Scalar>{};
	}
	return B;
}

// backsubstitution of an LU decomposition: Matrix A is in (L + U) form
template<typename Scalar>
vector<Scalar> lubksb(const matrix<Scalar>& A, const vector<size_t>& indx, const vector<Scalar>& b) {
	const size_t N = num_rows(A);
	if (N != num_cols(A)) {
		std::cerr << "matrix argument to lubksb is not square: (" << num_rows(A) << " x " << num_cols(A) << ")\n";
		return vector<Scalar>{};
	}
	if (N != size(indx)) {
		std::cerr << "permutation vector does not match size of LU decomposition: (" << N << " x " << N << ") !-> " << size(indx) << std::endl;
		return vector<Scalar>{};
	}
	if (N != size(b)) {
		std::cerr << "rhs vector does not match size of LU decomposition" << std::endl;
		return vector<Scalar>{};
	}
	vector<Scalar> x(b);
	// forward substitution
	for (size_t i = 0; i < N; ++i) {
		size_t ip = indx(i);
		Scalar sum = x(ip);
		x(ip) = x(i);
		for (size_t j = 0; j < i; ++j) {
			sum -= A(i, j) * x(j);
		}		
		x(i) = sum;
	}
	// backsubstitution
	for (size_t i = N; i >= 1; --i) {
		Scalar sum = x(i - 1);
		for (size_t j = i; j < N; ++j) {
			sum -= A(i - 1, j) * x(j);
		}
		x(i - 1) = sum / A(i - 1, i - 1);
	}
	return x;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

// solve the system of equations A x = b using partial pivoting LU
template<typename Scalar>
vector<Scalar> solve(const matrix<Scalar>& _A, const vector<Scalar>& _b) {
	using namespace std;
	using std::fabs;
	const size_t N = num_rows(_A);
	if (N != num_cols(_A)) {
		std::cerr << "matrix is not square: (" << num_rows(_A) << " x " << num_cols(_A) << ")\n";
		return 1;
	}
	if (N != size(_b)) {
		std::cerr << "matrix shape (" << num_rows(_A) << " x " << num_cols(_A) << ") is not congruous with vector size (" << size(_b) << ")\n";
		return 1;
	}
	matrix<Scalar> A(_A);
	// implicit pivoting pre-calculation
	vector<Scalar> implicitScale(N);
	vector<size_t> indx(N);
	for (size_t i = 0; i < N; ++i) { // for each row
		Scalar pivot = 0;
		for (size_t j = 0; j < N; ++j) { // scan the columns for the biggest abs value
			Scalar e = fabs(A(i, j));
			if (e > pivot) pivot = e;
		}
		if (pivot == 0) {
			std::cerr << "LU argument matrix is singular\n";
			return 2;
		}
		implicitScale[i] = Scalar(1.0) / pivot; // save the scaling factor for that row
	}
	//int nrOfRowExchanges = 0;
	size_t imax = 0;
	for (size_t j = 0; j < N; ++j) { // loop over columns of Crout's method
		Scalar sum = 0;
		for (size_t i = 0; i < j; ++i) {
			sum = A(i, j);
			for (size_t k = 0; k < i; ++k) sum -= A(i, k) * A(k, j);
			A(i, j) = sum;
		}
		Scalar pivot = 0; // initialize for search for largest pivot element
		for (size_t i = j; i < N; ++i) {
			sum = A(i, j);
			for (size_t k = 0; k < j; ++k) sum -= A(i, k) * A(k, j);
			A(i, j) = sum;
			Scalar dum = implicitScale[i] * fabs(sum);
			if (dum >= pivot) { // is figure of merit better than the best so far?
				pivot = dum;
				imax = i;
			}
		}
		if (j != imax) {
			for (size_t k = 0; k < N; ++k) std::swap(A(imax, k), A(j, k));
			//++nrOfRowExchanges;
			implicitScale[imax] = implicitScale[j]; // interchange scaling factor
		}
//		cout << "indx: " << indx << endl;
		indx[j] = imax;
//		cout << "      " << indx << endl;
		if (A(j, j) == 0) {
			std::cerr << "injecting tiny value to replace 0" << std::endl;
			A(j, j) = std::numeric_limits<Scalar>::epsilon();
		}
		if (j != N) {
			Scalar dum = Scalar(1) / A(j, j);
			for (size_t i = j + 1; i < N; ++i) A(i, j) *= dum;
		}
	}
//	cout << "index array\n" << indx << endl;
//	cout << "A\n" << A << endl;

	vector<Scalar> x(_b);
	// forward substitution
	for (size_t i = 0; i < N; ++i) {
		size_t ip = indx(i);
		Scalar sum = x(ip);
		x(ip) = x(i);
		for (size_t j = 0; j < i; ++j) {
			sum -= A(i, j) * x(j);
		}
		x(i) = sum;
	}
//	cout << "y\n" << x << endl;
	// backsubstitution
	for (size_t i = N; i >= 1; --i) {
		Scalar sum = x(i-1);
		for (size_t j = i; j < N; ++j) {
			sum -= A(i-1, j) * x(j);
		}
		x(i-1) = sum / A(i-1, i-1);
	}
	return x;
}



} } }  // namespace sw::universal::blas
