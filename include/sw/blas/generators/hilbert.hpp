#pragma once
// hilbert.hpp: generate a Hilbert matrix and its exact inverse
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstdint>
#include <random>
#include <math/functions/binomial.hpp>

namespace sw { namespace blas {

// Generate the scaling factor of a Hilbert matrix so that its elements are representable
// that is, no infinite expensions of rationals, such as 1/3, 1/10, etc.
template<typename IntegerType>
IntegerType HilbertScalingFactor(IntegerType N) {
	vector<IntegerType> coef;
	for (IntegerType i = 2; i <= IntegerType(N); ++i) coef.push_back(i);
	for (IntegerType j = 2; j <= IntegerType(N); ++j) coef.push_back(N + j - IntegerType(1));
	return sw::function::findlcm(coef);
}

// Generate a scaled/unscaled Hilbert matrix depending on the bScale parameter
template<typename Scalar>
size_t GenerateHilbertMatrix(matrix<Scalar>& M, bool bScale = true) {
	assert(num_rows(M) == num_cols(M)); // needs to be square
	size_t N = num_rows(M);
	size_t lcm = HilbertScalingFactor(N); // always calculate the Least Common Multiplier
	Scalar scale = bScale ? Scalar(lcm) : Scalar(1);
	for (size_t i = 0; i < N; ++i) {
		for (size_t j = 0; j < N; ++j) {
			M[i][j] = scale / Scalar(i + j + 1ul);
		}
	}
	return lcm;
}

template<typename Scalar>
void GenerateHilbertMatrixInverse(matrix<Scalar>&M) {
	assert(num_rows(M) == num_cols(M)); // needs to be square
	size_t N = num_rows(M);
	for (size_t i = 0; i < N; ++i) {
		for (size_t j = 0; j < N; ++j) {
			Scalar sign = ((i + j) % 2) ? Scalar(-1) : Scalar(1);
			Scalar factor1 = Scalar(i + j + 1ul);
			Scalar factor2 = Scalar(sw::function::binomial<uint64_t>(N + i, N - j - 1ul));
			Scalar factor3 = Scalar(sw::function::binomial<uint64_t>(N + j, N - i - 1ul));
			Scalar factor4 = Scalar(sw::function::binomial<uint64_t>(i + j, i));
			M[i][j] = Scalar(sign * factor1 * factor2 * factor3 * factor4 * factor4);
			/* for tracing dynamic range failures
			std::cout << "element " << i << "," << j << std::endl;
			std::cout << "sign    " << sign << std::endl;
			std::cout << "factor1 " << factor1 << std::endl;
			std::cout << "factor2 " << factor2 << std::endl;
			std::cout << "factor3 " << factor3 << std::endl;
			std::cout << "factor4 " << factor4 << std::endl;
			*/
		}
	}
}

// generate a standard hilbert matrix of size N
template<typename Scalar>
matrix<Scalar> hilbert(size_t N, bool bScale = true) {
	using Matrix = matrix<Scalar>;
	Matrix H(N, N);
	GenerateHilbertMatrix(H, bScale);
	return H;
}

}} // namespace sw::blas
