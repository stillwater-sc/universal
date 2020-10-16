#pragma once
// hilbert.hpp: generate a Hilbert matrix and its exact inverse
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstdint>
#include <random>
#include <algorithm>
#include <universal/functions/binomial.hpp>

namespace sw { namespace unum { namespace blas {

// Generate the scaling factor of a Hilbert matrix so that its elements are representable
// that is, no infinite expensions of rationals, such as 1/3, 1/10, etc.
template<typename IntegerType>
IntegerType HilbertScalingFactor(IntegerType N) {
	vector<IntegerType> coef;
	for (IntegerType i = 2; i <= N; ++i) coef.push_back(i);
	for (IntegerType j = 2; j <= N; ++j) coef.push_back(N + j - IntegerType(1));
	return sw::function::findlcm(coef);
}

// Generate a scaled/unscaled Hilbert matrix depending on the bScale parameter
template<typename Scalar>
size_t GenerateHilbertMatrix(matrix<Scalar>& M, bool bScale = true) {
	assert(num_rows(M) == num_cols(M)); // needs to be square
	size_t N = num_rows(M);
	size_t lcm = HilbertScalingFactor(N); // always calculate the Least Common Multiplier
	Scalar scale = bScale ? Scalar(lcm) : Scalar(1);
	for (int i = 1; i <= N; ++i) {
		for (int j = 1; j <= N; ++j) {
			M[i - 1][j - 1] = scale / Scalar(i + j - 1);
		}
	}
	return lcm;
}

template<typename Scalar>
void GenerateHilbertMatrixInverse(matrix<Scalar>& M, Scalar scale = Scalar(1.0)) {
	assert(num_rows(M) == num_cols(M)); // needs to be square
	size_t N = num_rows(M);
	for (int i = 1; i <= N; ++i) {
		for (int j = 1; j <= N; ++j) {
			Scalar sign = ((i + j) % 2) ? Scalar(-1) : Scalar(1);
			Scalar factor1 = Scalar(i + j - 1);
			Scalar factor2 = Scalar(sw::function::binomial<uint64_t>(N + i - 1, N - j));
			Scalar factor3 = Scalar(sw::function::binomial<uint64_t>(N + j - 1, N - i));
			Scalar factor4 = Scalar(sw::function::binomial<uint64_t>(i + j - 2, i - 1));
			M[i - 1][j - 1] = Scalar(sign * factor1 * factor2 * factor3 * factor4 * factor4);
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

}}} // namespace sw::unum::blas
