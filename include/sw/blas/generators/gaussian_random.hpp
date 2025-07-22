#pragma once
// uniform_random.hpp: uniform random matrix generator
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <random>
#include <numeric/containers.hpp>

namespace sw { namespace blas {  
	using namespace sw::numeric::containers;

    //////////////////////////////////////////////////////////////////
    // generate a gaussian random vector
    //////////////////////////////////////////////////////////////////
 
    // seed an STL vector
    template<typename Scalar>
    inline std::vector<Scalar>& gaussian_random(std::vector<Scalar>& v, double mean = 100.00, double stddev = 6.00) {
        using value_type = typename vector<Scalar>::value_type;
        using size_type = typename vector<Scalar>::size_type;
        const size_type nrElements = size(v);
        std::random_device rd;
        std::mt19937 rng(rd());
        for (size_type i = 0; i < nrElements; ++i) {
            std::normal_distribution<double> s(mean, stddev);
            v[i] = value_type(s(rng));
        }
        return v;
    }

    template<typename Scalar>
    inline vector<Scalar>& gaussian_random(vector<Scalar>& v, double mean = 100.00, double stddev = 6.00) {
        using value_type = typename vector<Scalar>::value_type;
        using size_type = typename vector<Scalar>::size_type;
        const size_type nrElements = size(v);
        std::random_device rd;
        std::mt19937 rng(rd());
        for (size_type i = 0; i < nrElements; ++i) {
            std::normal_distribution<double> s(mean, stddev);
            v[i] = value_type(s(rng));
        }
        return v;
    }


    // generate a gaussian random N element vector
    template<typename Scalar>
    vector<Scalar> gaussian_random_vector(unsigned N, double mean = 100.00, double stddev = 6.00) {
        vector<Scalar> v(N);
        return gaussian_random(v, mean, stddev);

    }



    //////////////////////////////////////////////////////////////////
	// generate a gaussian random MxN matrix
    //////////////////////////////////////////////////////////////////

    template<typename Scalar>
    inline matrix<Scalar>& gaussian_random(matrix<Scalar>& A, double mean = 100.00, double stddev = 6.00){
        using value_type = typename matrix<Scalar>::value_type;
        using size_type = typename matrix<Scalar>::size_type;
        const size_type ncols = num_cols(A), nrows = num_rows(A);                
        std::random_device rd;
        std::mt19937 rng(rd());                
        for(size_t i=0; i<nrows; ++i) {
            for(size_t j=0; j<ncols; ++j) {
                std::normal_distribution<double> s(mean, stddev);
                A[i][j]=value_type(s(rng));
            }
        }
        return A;
    }

    // generate a gaussian random MxN matrix
    template<typename Scalar>
    matrix<Scalar> gaussian_random_matrix(unsigned M, unsigned N, double mean = 100.00, double stddev = 6.00) {
        matrix<Scalar> A(M, N);
        return gaussian_random(A, mean, stddev);
    }

}} // namespace sw::blas