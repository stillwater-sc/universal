#pragma once
// uniform_random.hpp: uniform random matrix generator
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstdint>
#include <random>

namespace sw { namespace blas {

	// generate a uniform random N element vector
	template<typename Scalar>
	vector<Scalar> uniform_random_vector(unsigned N, double lowerbound = 0.0, double upperbound = 1.0) {
		vector<Scalar> v(N);
		return uniform_random(v, lowerbound, upperbound);
	}

	// fill a dense vector with random values between [lowerbound, upperbound]
	template <typename Scalar>
	vector<Scalar>& uniform_random(vector<Scalar>& v, double lowerbound = 0.0, double upperbound = 1.0)
	{
		// Use random_device to generate a seed for Mersenne twister engine.
		std::random_device rd{};
		// Use Mersenne twister engine to generate pseudo-random numbers.
		std::mt19937 engine{ rd() };
		// "Filter" MT engine's output to generate pseudo-random double values,
		// **uniformly distributed** on the closed interval [lowerbound, upperbound].
		// (Note that the range is [inclusive, inclusive].)
		std::uniform_real_distribution<double> dist{ lowerbound, upperbound };
		// Pattern to generate pseudo-random number.
		// double rnd_value = dist(engine);

		// generate and insert random values in A
		for (unsigned i = 0; i < v.size(); ++i) {
			v[i] = Scalar(dist(engine));
		}
		return v;
	}

	// fill a dense matrix with random values between [lowerbound, upperbound]
	template <typename Scalar>
	matrix<Scalar>& uniform_random(matrix<Scalar>& A, double lowerbound = 0.0, double upperbound = 1.0)
	{
		// Use random_device to generate a seed for Mersenne twister engine.
		std::random_device rd{};
		// Use Mersenne twister engine to generate pseudo-random numbers.
		std::mt19937 engine{ rd() };
		// "Filter" MT engine's output to generate pseudo-random double values,
		// **uniformly distributed** on the closed interval [lowerbound, upperbound].
		// (Note that the range is [inclusive, inclusive].)
		std::uniform_real_distribution<double> dist{ lowerbound, upperbound };
		// Pattern to generate pseudo-random number.
		// double rnd_value = dist(engine);

		using Matrix = matrix<Scalar>;
		typedef typename Matrix::value_type    value_type;
		typedef typename Matrix::size_type     size_type;

		// inserters add to the elements, so we need to set the value to 0 before we begin
		A = 0.0;
		// generate and insert random values in A
		for (size_type r = 0; r < num_rows(A); r++) {
			for (size_type c = 0; c < num_cols(A); c++) {
				A[r][c] = value_type(dist(engine));
			}
		}
		return A;
	}

	// generate a uniform random MxN matrix
	template<typename Scalar>
	matrix<Scalar> uniform_random_matrix(unsigned M, unsigned N, double lowerbound = 0.0, double upperbound = 1.0) {
		matrix<Scalar> A(M, N);
		return uniform_random(A, lowerbound, upperbound);
	}

}} // namespace sw::blas