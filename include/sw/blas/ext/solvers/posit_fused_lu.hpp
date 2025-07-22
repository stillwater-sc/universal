#pragma once
// posit_fused_lu.hpp: fused LU decomposition and solver routines for posits
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <string>
#include <universal/number/posit/posit_fwd.hpp>
#include <numeric/containers.hpp>

namespace sw { namespace universal { namespace blas {


///////////////////////////////////////////////////////////////////////////////////
/// CroutFDP with sw::universal::blas data structures

template<unsigned nbits, unsigned es, unsigned capacity = 10>
void CroutFDP(sw::universal::blas::matrix< sw::universal::posit<nbits, es> >& S, 
	      sw::universal::blas::matrix< sw::universal::posit<nbits, es> >& D) {
	assert(num_rows(S) == num_rows(D));
	assert(num_cols(S) == num_cols(D));
	size_t N = num_rows(S);
	for (size_t k = 0; k < N; ++k) {
		for (size_t i = k; i < N; ++i) {
			quire<nbits, es, capacity> q;
			q.reset();
			//for (int p = 0; p < k; ++p) q += D[i][p] * D[p][k];   if we had expression templates for the quire
			for (size_t p = 0; p < k; ++p) q += quire_mul(D[i][p], D[p][k]);
			posit<nbits, es> sum;
			convert(q.to_value(), sum);     // one and only rounding step of the fused-dot product
			// TODO: can we add the difference to the quire operation?
			D[i][k] = S[i][k] - sum; // not dividing by diagonals

#if BLAS_TRACE_ROUNDING_EVENTS
			quire<nbits, es, capacity> qsum(sum);
			q -= qsum;
			if (!q.iszero()) {
				sw::universal::posit<nbits, es> roundingError;
				convert(q.to_value(), roundingError);
				std::cout << "D[" << i << "," << k << "] rounding error: " << roundingError << std::endl;
			}
#endif
		}
		for (size_t j = k + 1; j < N; ++j) {
			quire<nbits, es, capacity> q;
			q.reset();
			//for (size_t p = 0; p < k; ++p) q += D[k][p] * D[p][j];   if we had expression templates for the quire
			for (size_t p = 0; p < k; ++p) q += quire_mul(D[k][p], D[p][j]);
			posit<nbits, es> sum;
			convert(q.to_value(), sum);   // one and only rounding step of the fused-dot product
			D[k][j] = (S[k][j] - sum) / D[k][k];

#if BLAS_TRACE_ROUNDING_EVENTS
			quire<nbits, es, capacity> qsum(sum);
			q -= qsum;
			if (!q.iszero()) {
				sw::universal::posit<nbits, es> roundingError;
				convert(q.to_value(), roundingError);
				std::cout << "D[" << k << "," << j << "] rounding error: " << roundingError << std::endl;
			}
#endif

		}
	}
}

// SolveCrout takes an LU decomposition, LU, and a right hand side vector, b, and produces a result, x.
template<unsigned nbits, unsigned es, unsigned capacity = 10>
void SolveCroutFDP(const matrix< sw::universal::posit<nbits, es> >& LU, 
		   const sw::universal::blas::vector< sw::universal::posit<nbits, es> >& b, 
                   sw::universal::blas::vector< sw::universal::posit<nbits, es> >& x) {
	size_t N = size(b);
	std::vector< posit<nbits, es> > y(N);
	for (size_t i = 0; i < N; ++i) {
		quire<nbits, es, capacity> q;
		// for (int k = 0; k < i; ++k) q += LU[i][k] * y[k];   if we had expression templates for the quire
		for (size_t k = 0; k < i; ++k) q += quire_mul(LU[i][k], y[k]);
		posit<nbits, es> sum;
		convert(q.to_value(), sum);   // one and only rounding step of the fused-dot product
		y[i] = (b[i] - sum) / LU[i][i];
	}
	for (long i = long(N) - 1; i >= 0; --i) {
		quire<nbits, es, capacity> q;
		// for (size_t k = i + 1; k < d; ++k) q += LU[i][k] * x[k];   if we had expression templates for the quire
		for (size_t k = i + 1; k < N; ++k) {
			//cout << "lu[] = " << LU[i][k] << " x[" << k << "] = " << x[k] << endl;
			q += quire_mul(LU[i][k], x[k]);
		}
		posit<nbits, es> sum;
		convert(q.to_value(), sum);  // one and only rounding step of the fused-dot product
		// cout << "sum " << sum << endl;
		x[i] = (y[i] - sum); // not dividing by diagonals
	}
}

// in-place LU decomposition using partial pivoting with implicit pivoting applied
template<unsigned nbits, unsigned es, unsigned capacity = 10>
int ludcmp(matrix< sw::universal::posit<nbits, es> >& A, vector<size_t>& indx) {
	using namespace std;
	using std::fabs;
	using namespace sw::universal;
	const size_t N = num_rows(A);
	if (N != num_cols(A)) {
		std::cerr << "matrix argument to ludcmp is not square: (" << num_rows(A) << " x " << num_cols(A) << ")\n";
		return 1;
	}
	indx.resize(N);
	indx = 0;
	using Scalar = sw::universal::posit<nbits, es>;
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
			sw::universal::quire<nbits, es, capacity> q(A(i, j));
			for (size_t k = 0; k < i; ++k) q -= quire_mul(A(i, k), A(k, j));
			convert(q.to_value(), sum);     // one and only rounding step of the fused-dot product
			A(i, j) = sum;
		}
		Scalar pivot = 0; // initialize for search for largest pivot element
		for (size_t i = j; i < N; ++i) {
			sw::universal::quire<nbits, es, capacity> q(A(i, j));
			for (size_t k = 0; k < j; ++k) q -= quire_mul(A(i, k), A(k, j));
			convert(q.to_value(), sum);     // one and only rounding step of the fused-dot product
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
template<unsigned nbits, unsigned es, unsigned capacity = 10>
vector< sw::universal::posit<nbits, es> > lubksb(const matrix< sw::universal::posit<nbits, es> >& A, const vector<size_t>& indx, const vector<sw::universal::posit<nbits, es> >& b) {
	using Scalar = sw::universal::posit<nbits, es>;
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
	Scalar sum = 0;
	// forward substitution
	for (size_t i = 0; i < N; ++i) {
		size_t ip = indx(i);
		quire<nbits, es, capacity> q(x(ip));
		x(ip) = x(i);
		for (size_t j = 0; j < i; ++j) {
			q -= quire_mul(A(i, j), x(j));
		}
		convert(q.to_value(), sum);
		x(i) = sum;
	}
	// backsubstitution
	for (size_t i = N; i >= 1; --i) {
		quire<nbits, es, capacity> q(x(i - 1));
		for (size_t j = i; j < N; ++j) {
			q -= quire_mul(A(i - 1, j), x(j));
		}
		convert(q.to_value(), sum);
		x(i - 1) = sum / A(i - 1, i - 1);
	}
	return x;
}

// solve the system of equations A x = b using partial pivoting LU
template<unsigned nbits, unsigned es, unsigned capacity = 10>
vector<sw::universal::posit<nbits, es> > solve(const matrix<sw::universal::posit<nbits, es> >& _A, const vector<sw::universal::posit<nbits, es>>& _b) {
	using namespace std;
	using std::fabs;
	const size_t N = num_rows(_A);
	if (N != num_cols(_A)) {
		cerr << "matrix is not square: (" << num_rows(_A) << " x " << num_cols(_A) << ")\n";
		return 1;
	}
	if (N != size(_b)) {
		cerr << "matrix shape (" << num_rows(_A) << " x " << num_cols(_A) << ") is not congruous with vector size (" << size(_b) << ")\n";
		return 1;
	}
	using Scalar = sw::universal::posit<nbits, es>;
	//cerr << typeid(Scalar).name() << " specialization of LU decomposition solver with fused-dot-product operators" << endl;
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
			quire<nbits, es, capacity> q(A(i, j));
			for (size_t k = 0; k < i; ++k) q -= quire_mul(A(i, k), A(k, j));
			convert(q.to_value(), sum);     // one and only rounding step of the fused-dot product
			A(i, j) = sum;
		}
		Scalar pivot = 0; // initialize for search for largest pivot element
		for (size_t i = j; i < N; ++i) {
			quire<nbits, es, capacity> q(A(i, j));
			for (size_t k = 0; k < j; ++k) q -= quire_mul(A(i, k), A(k, j));
			convert(q.to_value(), sum);     // one and only rounding step of the fused-dot product
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
		quire<nbits, es, capacity> q(x(ip));
		x(ip) = x(i);
		for (size_t j = 0; j < i; ++j) q -= quire_mul(A(i, j), x(j));
		Scalar sum;
		convert(q.to_value(), sum);     // one and only rounding step of the fused-dot product
		x(i) = sum;
	}
	//	cout << "y\n" << x << endl;
	// backsubstitution
	for (size_t i = N; i >= 1; --i) {
		quire<nbits, es, capacity> q(x(i - 1));
		for (size_t j = i; j < N; ++j) q -= quire_mul(A(i - 1, j), x(j));
		Scalar sum;
		convert(q.to_value(), sum);     // one and only rounding step of the fused-dot product
		x(i - 1) = sum / A(i - 1, i - 1);
	}
	return x;
}

}}} // namespace sw::universal::blas
