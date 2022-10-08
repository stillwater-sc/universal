#pragma once
// blas_l3.hpp: BLAS Level 3 functions
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal { namespace blas {

// sum entire matrix (dim == 0), all rows (dim == 1), or all columns (dim == 2)
template<typename Matrix>
vector<typename Matrix::value_type> sum(Matrix& A, int dim = 0) {
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


// A times B = C fused matrix-vector product
template<size_t nbits, size_t es>
matrix< sw::universal::posit<nbits, es> > fmm(const matrix< sw::universal::posit<nbits, es> >& A, const matrix< sw::universal::posit<nbits, es> >& B) {
	// preconditions
	assert(A.cols() == B.rows());

	constexpr size_t capacity = 20; // FDP for vectors < 1,048,576 elements
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

}}} // namespace sw::universal::blas
