#pragma once
// blas_l3.hpp: BLAS Level 3 functions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal { namespace blas {

// sum entire matrix (dim == 0), all rows (dim == 1), or all columns (dim == 2)
template<typename Matrix>
vector<typename Matrix::value_type> sumOfElements(Matrix& A, int dim = 0) {
	using value_type = typename Matrix::value_type;
	using size_type = typename Matrix::size_type;

	size_type rows = num_rows(A);
	size_type cols = num_cols(A);

	switch (dim) {
	case 0:
	{
		value_type sum{ 0 };
		for (size_type i = 0; i < rows; ++i) {
			for (size_type j = 0; j < cols; ++j) {
				sum += A(i, j);
			}
		}
		return vector<value_type>{sum};
	}

	case 1: 
	{
		vector<value_type> rowSums(rows);
		for (size_type i = 0; i < rows; ++i) {
			for (size_type j = 0; j < cols; ++j) {
				rowSums[i] += A(i, j);
			}
		}
		return rowSums;
	}

	case 2:
	{
		vector<value_type> colSums(cols);
		for (size_type i = 0; i < rows; ++i) {
			for (size_type j = 0; j < cols; ++j) {
				colSums[j] += A(i, j);
			}
		}
		return colSums;
	}

	default:
		break;
	}

	return vector<value_type>{0};
}

enum NormalizationMethod {
	Norm2,
	Center,
	Zscore,
	Scale,
	Range
};

// normalize entire matrix (dim == 0), all rows (dim == 1), or all columns (dim == 2)
template<typename Matrix>
void normalize(Matrix& A, int dim = 0) {
	using value_type = typename Matrix::value_type;
	using size_type = typename Matrix::size_type;

	size_type rows = num_rows(A);
	size_type cols = num_cols(A);

	switch (dim) {
	case 0:
		{
			value_type sos{ 0 };
			for (size_type i = 0; i < rows; ++i) {
				for (size_type j = 0; j < cols; ++j) {
					sos += A(i, j) * A(i, j);
				}
			}
			for (size_type i = 0; i < rows; ++i) {
				for (size_type j = 0; j < cols; ++j) {
					A(i, j) /= sqrt(sos);
				}
			}
		}
		break;
	case 1:
		{
			vector<value_type> rowSos(rows);
			for (size_type i = 0; i < rows; ++i) {
				for (size_type j = 0; j < cols; ++j) {
					rowSos[i] += A(i, j) * A(i, j);
				}
			}
			for (size_type i = 0; i < rows; ++i) {
				for (size_type j = 0; j < cols; ++j) {
					A(i, j) /= sqrt(rowSos[i]);
				}
			}
		}
		break;
	case 2:
		{
			vector<value_type> colSos(cols);
			for (size_type i = 0; i < rows; ++i) {
				for (size_type j = 0; j < cols; ++j) {
					colSos[j] += A(i, j) * A(i, j);
				}
			}
			for (size_type i = 0; i < rows; ++i) {
				for (size_type j = 0; j < cols; ++j) {
					A(i, j) /= sqrt(colSos[j]);
				}
			}
		}
		break;
	default:
		break;
	}
}

// 2-norm of entire matrix (dim == 0), each row (dim == 1), or each column (dim == 2)
template<typename Matrix>
vector<typename Matrix::value_type> matrixNorm(Matrix& A, int dim = 0) {
	using value_type = typename Matrix::value_type;
	using size_type = typename Matrix::size_type;

	size_type rows = num_rows(A);
	size_type cols = num_cols(A);

	switch (dim) {
	case 0:
	{
		value_type sos{ 0 };
		for (size_type i = 0; i < rows; ++i) {
			for (size_type j = 0; j < cols; ++j) {
				sos += A(i, j) * A(i,j);
			}
		}
		return vector<value_type>{sqrt(sos)};
	}

	case 1:
	{
		vector<value_type> rowSoS(rows);
		for (size_type i = 0; i < rows; ++i) {
			for (size_type j = 0; j < cols; ++j) {
				rowSoS[i] += A(i, j) * A(i, j);
			}
			rowSoS[i] = sqrt(rowSoS[i]);
		}
		return rowSoS;
	}

	case 2:
	{
		vector<value_type> colSoS(cols);
		for (size_type i = 0; i < rows; ++i) {
			for (size_type j = 0; j < cols; ++j) {
				colSoS[j] += A(i, j) * A(i, j);
			}
		}
		for (size_type j = 0; j < cols; ++j) {
			colSoS[j] = sqrt(colSoS[j]);
		}
		return colSoS;
	}

	default:
		break;
	}
	return vector<value_type>{-1};
}

// xyt is outer product x*y'
template<typename Scalar>
matrix<Scalar> xyt(const vector<Scalar>& x, const vector<Scalar>& y) {
	size_t m = size(x);
	size_t n = size(y);
	matrix<Scalar> A(m,n);
	
	for (size_t i = 0; i < m; i++) {
		for (size_t j = 0; j < n; j++) {
			A(i,j) = x(i)*x(j);
		}
	}
	return A;
}

}}} // namespace sw::universal::blas
