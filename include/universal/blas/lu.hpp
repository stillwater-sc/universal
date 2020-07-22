#pragma once
// lu.hpp: super-simple dense matrix implementation
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <universal/blas/matrix.hpp>

namespace sw { namespace unum { namespace blas {

	struct no_pivoting {};
	struct partial_pivoting {};
	struct full_pivoting {};

template<typename Matrix, typename PivotingStrategy>
class LU {
public:
	typedef typename Matrix::value_type value_type;

	void compute(const Matrix& A) {

	}
	vector<value_type> solve(const vector<value_type>& b) {
		vector<value_type> x(n);
		// ...
		return x;
	}

private:
	size_t m, n; // rows, cols
};

// partial pivoting Gaussian Elimination


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
	size_t N = size(b);
	using value_type = typename Matrix::value_type;
	sw::unum::blas::vector<value_type> y(N);
	for (size_t i = 0; i < N; ++i) {
		value_type sum = 0.0;
		for (size_t k = 0; k < size_t(i); ++k) sum += LU[i][k] * y[k];
		y[i] = (b[i] - sum) / LU[i][i];

	}
	for (long i = long(N) - 1; i >= 0; --i) {
		value_type sum = 0.0;
		for (size_t k = i + 1; k < N; ++k) {
			//cout << "lu[] = " << LU[i][k] << " x[" << k << "] = " << x[k] << endl;
			sum += LU[i][k] * x[k];
		}
		//cout << "sum " << sum << endl;
		x[i] = (y[i] - sum); // not dividing by diagonals
	}
}


///////////////////////////////////////////////////////////////////////////////////
/// CroutFDP with sw::unum::blas data structures

template<size_t nbits, size_t es, size_t capacity = 10>
void CroutFDP(sw::unum::blas::matrix< sw::unum::posit<nbits, es> >& S, sw::unum::blas::matrix< sw::unum::posit<nbits, es> >& D) {
	size_t d = num_rows(S);
	assert(size(S) == size(D));
	using namespace sw::unum;
	for (size_t k = 0; k < d; ++k) {
		for (size_t i = k; i < d; ++i) {
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
				sw::unum::posit<nbits, es> roundingError;
				convert(q.to_value(), roundingError);
				std::cout << "D[" << i << "," << k << "] rounding error: " << roundingError << std::endl;
			}
#endif
		}
		for (size_t j = k + 1; j < d; ++j) {
			quire<nbits, es, capacity> q;
			q.reset();
			//for (size_t p = 0; p < k; ++p) q += D[k][p] * D[p][j];   if we had expression templates for the quire
			for (int p = 0; p < k; ++p) q += quire_mul(D[k][p], D[p][j]);
			posit<nbits, es> sum;
			convert(q.to_value(), sum);   // one and only rounding step of the fused-dot product
			D[k][j] = (S[k][j] - sum) / D[k][k];

#if BLAS_TRACE_ROUNDING_EVENTS
			quire<nbits, es, capacity> qsum(sum);
			q -= qsum;
			if (!q.iszero()) {
				sw::unum::posit<nbits, es> roundingError;
				convert(q.to_value(), roundingError);
				std::cout << "D[" << k << "," << j << "] rounding error: " << roundingError << std::endl;
			}
#endif

		}
	}
}

// SolveCrout takes an LU decomposition, LU, and a right hand side vector, b, and produces a result, x.
template<size_t nbits, size_t es, size_t capacity = 10>
void SolveCroutFDP(const sw::unum::blas::matrix< sw::unum::posit<nbits, es> >& LU, const sw::unum::blas::vector< sw::unum::posit<nbits, es> >& b, sw::unum::blas::vector< sw::unum::posit<nbits, es> >& x)
{
	using namespace sw::unum;

	size_t d = size(b);
	std::vector< posit<nbits, es> > y(d);
	for (size_t i = 0; i < d; ++i) {
		quire<nbits, es, capacity> q;
		// for (int k = 0; k < i; ++k) q += LU[i][k] * y[k];   if we had expression templates for the quire
		for (size_t k = 0; k < i; ++k) q += quire_mul(LU[i][k], y[k]);
		posit<nbits, es> sum;
		convert(q.to_value(), sum);   // one and only rounding step of the fused-dot product
		y[i] = (b[i] - sum) / LU[i][i];
	}
	for (long i = long(d) - 1; i >= 0; --i) {
		quire<nbits, es, capacity> q;
		// for (size_t k = i + 1; k < d; ++k) q += LU[i][k] * x[k];   if we had expression templates for the quire
		for (size_t k = i + 1; k < d; ++k) {
			//cout << "lu[] = " << LU[i][k] << " x[" << k << "] = " << x[k] << endl;
			q += quire_mul(LU[i][k], x[k]);
		}
		posit<nbits, es> sum;
		convert(q.to_value(), sum);  // one and only rounding step of the fused-dot product
									 //cout << "sum " << sum << endl;
		x[i] = (y[i] - sum); // not dividing by diagonals
	}
}

} } }  // namespace sw::unum::blas
