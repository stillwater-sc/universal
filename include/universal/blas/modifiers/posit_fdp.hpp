#pragma once
// posit_fdp.hpp: type specific fused dot product overloads for the matrix class
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <string>
#include <universal/number/posit/posit_fwd.hpp>
#include <universal/blas/vector.hpp>
#include <universal/blas/matrix.hpp>

namespace sw { namespace universal { namespace blas {


	// generate a posit format ASCII format nbits.esxNN...NNp
	template<unsigned nbits, unsigned es>
	inline std::string hex_format(const vector< sw::universal::posit<nbits, es> >& v) {
		// we need to transform the posit into a string
		std::stringstream ss;
		for (size_t j = 0; j < size(v); ++j) ss << hex_format(v[j]) << " ";
		return ss.str();
	}

	// generate a posit format ASCII format nbits.esxNN...NNp
	template<unsigned nbits, unsigned es>
	inline std::string hex_format(const matrix< sw::universal::posit<nbits, es> >& A) {
		using Scalar = sw::universal::posit<nbits, es>;
		using size_type = typename matrix<Scalar>::size_type;
		std::stringstream ostr;
		size_type m = A.rows();
		size_type n = A.cols();
		for (size_type i = 0; i < m; ++i) {
			for (size_type j = 0; j < n; ++j) {
				ostr << hex_format(A(i, j)) << " ";
			}
			ostr << '\n';
		}
		return ostr.str();
	}

// fused dot product operator*() overload for posit vectors
template<unsigned nbits, unsigned es>
sw::universal::posit<nbits, es> operator*(const vector< sw::universal::posit<nbits, es> >& a, const vector< sw::universal::posit<nbits, es> >& b) {
	using Scalar = posit<nbits, es>;
	//	std::cout << "fused dot product for " << typeid(Scalar).name() << std::endl;
	size_t N = size(a);
	if (size(a) != size(b)) {
		std::cerr << "vector sizes are different: " << N << " vs " << size(b) << '\n';
		return Scalar{ 0 };
	}
	constexpr unsigned capacity = 20;
	sw::universal::quire<nbits, es, capacity> sum{ 0 };
	for (size_t i = 0; i < N; ++i) {
		sum += sw::universal::quire_mul(a(i), b(i));
	}
	Scalar p;
	convert(sum.to_value(), p);
	return p;
}

// overload for posits to use fused dot products
template<unsigned nbits, unsigned es>
vector< sw::universal::posit<nbits, es> > operator*(const matrix< sw::universal::posit<nbits, es> >& A, const vector< sw::universal::posit<nbits, es> >& x) {
	constexpr unsigned capacity = 20; // FDP for vectors < 1,048,576 elements
	vector< sw::universal::posit<nbits, es> > b(A.rows());
	for (unsigned i = 0; i < A.rows(); ++i) {
		sw::universal::quire<nbits, es, capacity> q;
		for (unsigned j = 0; j < A.cols(); ++j) {
			q += quire_mul(A(i, j), x[j]);
		}
		convert(q.to_value(), b[i]); // one and only rounding step of the fused-dot product
	}
	return b;
}

// overload for posits uses fused dot products
template<unsigned nbits, unsigned es>
matrix< sw::universal::posit<nbits, es> > operator*(const matrix< sw::universal::posit<nbits, es> >& A, const matrix< sw::universal::posit<nbits, es> >& B) {
	using size_type = typename matrix< sw::universal::posit<nbits, es> >::size_type;
	constexpr unsigned capacity = 20; // FDP for vectors < 1,048,576 elements
	if (A.cols() != B.rows()) throw matmul_incompatible_matrices(incompatible_matrices(A.rows(), A.cols(), B.rows(), B.cols(), "*").what());
	auto rows = A.rows();
	auto cols = B.cols();
	auto dots = A.cols();
	matrix< posit<nbits, es> > C(rows, cols);
	for (size_type i = 0; i < rows; ++i) {
		for (size_type j = 0; j < cols; ++j) {
			sw::universal::quire<nbits, es, capacity> q;
			for (size_type k = 0; k < dots; ++k) {
				q += quire_mul(A(i, k), B(k, j));
			}
			convert(q.to_value(), C(i, j)); // one and only rounding step of the fused-dot product
		}
	}
	std::cout << "fused dot product matrix-matrix multiplication\n";
	return C;
}

///////////////////////////////////////////////////////////////////////////////////
// fused matrix-vector product
//  
// TODO: how would you generalize this to posits, cfloats, lns, integer, or even native int and float?
//
// A times x = b fused matrix-vector product
template<unsigned nbits, unsigned es>
sw::universal::blas::vector< sw::universal::posit<nbits, es> > fmv(const sw::universal::blas::matrix< sw::universal::posit<nbits, es> >& A, const sw::universal::blas::vector< sw::universal::posit<nbits, es> >& x) {
	// preconditions
	assert(A.cols() == size(x));
	sw::universal::blas::vector< sw::universal::posit<nbits, es> > b(size(x));

#if BLAS_TRACE_ROUNDING_EVENTS
	unsigned errors = 0;
#endif
	size_t nr = size(b);
	size_t nc = size(x);
	for (size_t i = 0; i < nr; ++i) {
		sw::universal::quire<nbits, es> q(0);
		for (size_t j = 0; j < nc; ++j) {
			q += sw::universal::quire_mul(A(i, j), x[j]);
		}
		sw::universal::convert(q.to_value(), b[i]);     // one and only rounding step of the fused-dot product
#if BLAS_TRACE_ROUNDING_EVENTS
		sw::universal::quire<nbits, es> qdiff = q;
		sw::universal::quire<nbits, es> qsum = b[i];
		qdiff -= qsum;
		if (!qdiff.iszero()) {
			++errors;
			std::cout << "q    : " << q << std::endl;
			std::cout << "qsum : " << qsum << std::endl;
			std::cout << "qdiff: " << qdiff << std::endl;
			sw::universal::posit<nbits, es> roundingError;
			convert(qdiff.to_value(), roundingError);
			std::cout << "matvec b[" << i << "] = " << hex_format(b[i]) << " rounding error: " << hex_format(roundingError) << " " << roundingError << std::endl;
		}
#endif
	}
#if BLAS_TRACE_ROUNDING_EVENTS
	if (errors) {
		std::cout << "Universal-BLAS: tracing found " << errors << " rounding errors in matvec operation\n";
	}
#endif
	return b;
}

///////////////////////////////////////////////////////////////////////////////////
// fused matrix-matrix product
//  
// TODO: how to generalize this to posit, cfloat, lns, integer, etc.
//
// A times B = C fused matrix-vector product
template<unsigned nbits, unsigned es>
matrix< sw::universal::posit<nbits, es> > fmm(const matrix< sw::universal::posit<nbits, es> >& A, const matrix< sw::universal::posit<nbits, es> >& B) {
	// preconditions
	assert(A.cols() == B.rows());

	constexpr unsigned capacity = 20; // FDP for vectors < 1,048,576 elements
	if (A.cols() != B.rows()) throw matmul_incompatible_matrices(incompatible_matrices(A.rows(), A.cols(), B.rows(), B.cols(), "*").what());
	size_t rows = A.rows();
	size_t cols = B.cols();
	size_t dots = A.cols();
	matrix< posit<nbits, es> > C(rows, cols);
	for (size_t i = 0; i < rows; ++i) {
		for (size_t j = 0; j < cols; ++j) {
			quire<nbits, es, capacity> q;
			for (size_t k = 0; k < dots; ++k) {
				q += quire_mul(A(i, k), B(k, j));
			}
			convert(q.to_value(), C(i, j)); // one and only rounding step of the fused-dot product
		}
	}
	return C;
}


///////////////////////////////////////////////////////////////////////////////////
/// CroutFDP with sw::universal::blas data structures

template<unsigned nbits, unsigned es, unsigned capacity = 10>
void CroutFDP(sw::universal::blas::matrix< sw::universal::posit<nbits, es> >& S, sw::universal::blas::matrix< sw::universal::posit<nbits, es> >& D) {
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
void SolveCroutFDP(const matrix< sw::universal::posit<nbits, es> >& LU, const sw::universal::blas::vector< sw::universal::posit<nbits, es> >& b, sw::universal::blas::vector< sw::universal::posit<nbits, es> >& x) {
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
