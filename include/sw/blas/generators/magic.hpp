#pragma once
// magric.hpp: generate a magic square matrix 
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstdint>
#include <random>

namespace sw { namespace blas {
	using namespace sw::numeric::containers;

// fill a dense (N, N) matrix with linear index values in row order
template <typename Scalar>
matrix<Scalar> magic(unsigned N) {
	using Matrix = matrix<Scalar>;
	// precondition tests
	if (N == 0) return matrix<Scalar>{};
	if (N % 2 == 0) {
		std::cerr << "matrix size N is even, must be odd" << std::endl;
		return matrix<Scalar>{};
	}

	Matrix A(N, N);

	// generate a magic square matrix where all the rows, columns, and main diagonals sum to the same value
	// Three conditions:
	// 1- position of next number is calculated by rowIndex-1, colIndex+1, modulo N-1
	// 2- if number exists at new position, redo calculation as rowIndex+2, colIndex-2
	// 3- if row is 1 and column is N, new position is (0, n-2)
	//
	int n = static_cast<int>(N);
	int nsqr = n*n;
	int i = n / 2;
	int j = n - 1;

	// generate the indices
	for (int e = 1; e <= nsqr; /* increment in body */) {
		if (i == -1 && j == n) {
			i = 0;
			j = n - 2;
		}
		else {
			// first condition helper if next row index wraps around
			if (i < 0) i = n - 1;;
			// first condition helper if next column index wraps around
			if (j == n) j = 0;
		}
		if (A(i, j) > Scalar(0)) { // second condition
			++i; j -= 2;
			continue;
		}
		else {
			A(i, j) = Scalar(e++);
		}
		// first condition
		--i;
		++j;
	}

	return A;
}

}} // namespace sw::blas
