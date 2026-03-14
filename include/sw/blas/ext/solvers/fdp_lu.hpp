#pragma once
// fdp_lu.hpp: fused LU decomposition and solver routines using generalized quire
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <string>
// Consumer must include the appropriate quire/fdp headers for their Scalar type before this header
#include <numeric/containers.hpp>
#include <blas/exceptions.hpp>

namespace sw { namespace blas { namespace fdp {
	using namespace sw::numeric::containers;
	using namespace sw::universal;

///////////////////////////////////////////////////////////////////////////////////
/// CroutFDP with sw::universal::blas data structures

template<typename Scalar, unsigned capacity = 10>
void CroutFDP(matrix<Scalar>& S, matrix<Scalar>& D) {
	assert(num_rows(S) == num_rows(D));
	assert(num_cols(S) == num_cols(D));
	size_t N = num_rows(S);
	for (size_t k = 0; k < N; ++k) {
		for (size_t i = k; i < N; ++i) {
			quire<Scalar, capacity> q;
			q.reset();
			for (size_t p = 0; p < k; ++p) q += quire_mul(D[i][p], D[p][k]);
			Scalar sum = quire_resolve(q);
			D[i][k] = S[i][k] - sum;

#if BLAS_TRACE_ROUNDING_EVENTS
			quire<Scalar, capacity> qsum(sum);
			q -= qsum;
			if (!q.iszero()) {
				Scalar roundingError = quire_resolve(q);
				std::cout << "D[" << i << "," << k << "] rounding error: " << roundingError << std::endl;
			}
#endif
		}
		for (size_t j = k + 1; j < N; ++j) {
			quire<Scalar, capacity> q;
			q.reset();
			for (size_t p = 0; p < k; ++p) q += quire_mul(D[k][p], D[p][j]);
			Scalar sum = quire_resolve(q);
			D[k][j] = (S[k][j] - sum) / D[k][k];

#if BLAS_TRACE_ROUNDING_EVENTS
			quire<Scalar, capacity> qsum(sum);
			q -= qsum;
			if (!q.iszero()) {
				Scalar roundingError = quire_resolve(q);
				std::cout << "D[" << k << "," << j << "] rounding error: " << roundingError << std::endl;
			}
#endif

		}
	}
}

// SolveCrout takes an LU decomposition, LU, and a right hand side vector, b, and produces a result, x.
template<typename Scalar, unsigned capacity = 10>
void SolveCroutFDP(const matrix<Scalar>& LU,
		   const vector<Scalar>& b,
                   vector<Scalar>& x) {
	assert(num_rows(LU) == num_cols(LU));
	assert(num_rows(LU) == size(b));
	size_t N = size(b);
	std::vector<Scalar> y(N);
	for (size_t i = 0; i < N; ++i) {
		quire<Scalar, capacity> q;
		for (size_t k = 0; k < i; ++k) q += quire_mul(LU[i][k], y[k]);
		Scalar sum = quire_resolve(q);
		y[i] = (b[i] - sum) / LU[i][i];
	}
	for (long i = long(N) - 1; i >= 0; --i) {
		quire<Scalar, capacity> q;
		for (size_t k = i + 1; k < N; ++k) {
			q += quire_mul(LU[i][k], x[k]);
		}
		Scalar sum = quire_resolve(q);
		x[i] = (y[i] - sum); // not dividing by diagonals
	}
}

// in-place LU decomposition using partial pivoting with implicit pivoting applied
template<typename Scalar, unsigned capacity = 10>
int ludcmp(matrix<Scalar>& A, vector<size_t>& indx) {
	using namespace std;
	using std::fabs;
	const size_t N = num_rows(A);
	if (N != num_cols(A)) {  // LCOV_EXCL_START
		std::cerr << "matrix argument to ludcmp is not square: (" << num_rows(A) << " x " << num_cols(A) << ")\n";
		return 1;
	}  // LCOV_EXCL_STOP
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
		if (pivot == 0) {  // LCOV_EXCL_START
			std::cerr << "LU argument matrix is singular\n";
			return 2;
		}  // LCOV_EXCL_STOP
		implicitScale[i] = Scalar(1.0) / pivot; // save the scaling factor for that row
	}
	//int nrOfRowExchanges = 0;
	size_t imax = 0;
	for (size_t j = 0; j < N; ++j) { // loop over columns of Crout's method
		Scalar sum = 0;
		for (size_t i = 0; i < j; ++i) {
			sw::universal::quire<Scalar, capacity> q(A(i, j));
			for (size_t k = 0; k < i; ++k) q -= quire_mul(A(i, k), A(k, j));
			sum     = quire_resolve(q);
			A(i, j) = sum;
		}
		Scalar pivot = 0; // initialize for search for largest pivot element
		for (size_t i = j; i < N; ++i) {
			sw::universal::quire<Scalar, capacity> q(A(i, j));
			for (size_t k = 0; k < j; ++k) q -= quire_mul(A(i, k), A(k, j));
			sum        = quire_resolve(q);
			A(i, j)    = sum;
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
		indx[j] = imax;
		if (A(j, j) == 0) A(j, j) = std::numeric_limits<Scalar>::epsilon();
		if (j != N) {
			Scalar dum = Scalar(1) / A(j, j);
			for (size_t i = j + 1; i < N; ++i) A(i, j) *= dum;
		}
	}
	return 0; // success
}

// backsubstitution of an LU decomposition: Matrix A is in (L + U) form
template<typename Scalar, unsigned capacity = 10>
vector<Scalar> lubksb(const matrix<Scalar>& A, const vector<size_t>& indx, const vector<Scalar>& b) {
	const size_t N = num_rows(A);
	// LCOV_EXCL_START
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
	// LCOV_EXCL_STOP
	vector<Scalar> x(b);
	Scalar sum = 0;
	// forward substitution
	for (size_t i = 0; i < N; ++i) {
		size_t ip = indx(i);
		quire<Scalar, capacity> q(x(ip));
		x(ip) = x(i);
		for (size_t j = 0; j < i; ++j) {
			q -= quire_mul(A(i, j), x(j));
		}
		sum  = quire_resolve(q);
		x(i) = sum;
	}
	// backsubstitution
	for (size_t i = N; i >= 1; --i) {
		quire<Scalar, capacity> q(x(i - 1));
		for (size_t j = i; j < N; ++j) {
			q -= quire_mul(A(i - 1, j), x(j));
		}
		sum      = quire_resolve(q);
		x(i - 1) = sum / A(i - 1, i - 1);
	}
	return x;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////
// solve the system of equations A x = b using partial pivoting LU
template<typename Scalar, unsigned capacity = 10>
vector<Scalar> solve(const matrix<Scalar>& _A, const vector<Scalar>& _b) {
	using namespace std;
	using std::fabs;
	const size_t N = num_rows(_A);
	// LCOV_EXCL_START
	if (N != num_cols(_A)) {
		cerr << "matrix is not square: (" << num_rows(_A) << " x " << num_cols(_A) << ")\n";
		return vector<Scalar>{};
	}
	if (N != size(_b)) {
		cerr << "matrix shape (" << num_rows(_A) << " x " << num_cols(_A) << ") is not congruous with vector size (" << size(_b) << ")\n";
		return vector<Scalar>{};
	}
	// LCOV_EXCL_STOP
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
		if (pivot == 0) {  // LCOV_EXCL_START
			std::cerr << "LU argument matrix is singular\n";
			return vector<Scalar>{};
		}  // LCOV_EXCL_STOP
		implicitScale[i] = Scalar(1.0) / pivot; // save the scaling factor for that row
	}
	//int nrOfRowExchanges = 0;
	size_t imax = 0;
	for (size_t j = 0; j < N; ++j) { // loop over columns of Crout's method
		Scalar sum = 0;
		for (size_t i = 0; i < j; ++i) {
			quire<Scalar, capacity> q(A(i, j));
			for (size_t k = 0; k < i; ++k) q -= quire_mul(A(i, k), A(k, j));
			sum     = quire_resolve(q);
			A(i, j) = sum;
		}
		Scalar pivot = 0; // initialize for search for largest pivot element
		for (size_t i = j; i < N; ++i) {
			quire<Scalar, capacity> q(A(i, j));
			for (size_t k = 0; k < j; ++k) q -= quire_mul(A(i, k), A(k, j));
			sum        = quire_resolve(q);
			A(i, j)    = sum;
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
		if (A(j, j) == 0) {  // LCOV_EXCL_START
			std::cerr << "injecting tiny value to replace 0" << std::endl;
			A(j, j) = std::numeric_limits<Scalar>::epsilon();
		}  // LCOV_EXCL_STOP
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
		quire<Scalar, capacity> q(x(ip));
		x(ip) = x(i);
		for (size_t j = 0; j < i; ++j) q -= quire_mul(A(i, j), x(j));
		Scalar sum;
		sum  = quire_resolve(q);
		x(i) = sum;
	}
	//	cout << "y\n" << x << endl;
	// backsubstitution
	for (size_t i = N; i >= 1; --i) {
		quire<Scalar, capacity> q(x(i - 1));
		for (size_t j = i; j < N; ++j) q -= quire_mul(A(i - 1, j), x(j));
		Scalar sum;
		sum      = quire_resolve(q);
		x(i - 1) = sum / A(i - 1, i - 1);
	}
	return x;
}

}}} // namespace sw::blas::fdp
