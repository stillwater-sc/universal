#pragma once
// uniform_random.hpp: uniform random matrix generator
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <random>

namespace sw { namespace universal { namespace blas {  

    // generate a gaussian random N element vector
    template<typename Scalar>
    vector<Scalar> gaussian_random_vector(unsigned N, double mean = 100.00, double stddev = 6.00) {
        vector<Scalar> v(N);
        return gaussian_random(v, mean, stddev);
    }

    template<typename Scalar>
    inline vector<Scalar>& gaussian_random(vector<Scalar>& omega, double mean = 100.00, double stddev = 6.00) {
        using value_type = typename vector<Scalar>::value_type;
        using size_type = typename vector<Scalar>::size_type;
        const size_type nrElements = size(omega);
        std::random_device rd;
        std::mt19937 rng(rd());
        for (size_type i = 0; i < nrElements; ++i) {
            std::normal_distribution<double> s(mean, stddev);
            omega[i] = value_type(s(rng));
        }
        return omega;
    }

    // generate a gaussian random MxN matrix
    template<typename Scalar>
    matrix<Scalar> gaussian_random_matrix(unsigned M, unsigned N, double mean = 100.00, double stddev = 6.00) {
        matrix<Scalar> A(M, N);
        return gaussian_random(A, mean, stddev);
    }

    template<typename Scalar>
    inline matrix<Scalar> gaussian_random(matrix<Scalar>& omega, double mean = 100.00, double stddev = 6.00){
        using value_type = typename matrix<Scalar>::value_type;
        using size_type = typename matrix<Scalar>::size_type;
        const size_type ncols = num_cols(omega), nrows = num_rows(omega);                
        std::random_device rd;
        std::mt19937 rng(rd());                
        for(size_t i=0; i<nrows; ++i) {
            for(size_t j=0; j<ncols; ++j) {
                std::normal_distribution<double> s(mean, stddev);
                omega[i][j]=value_type(s(rng));
            }
        }
        return omega;
    }

}}} // namespace sw::universal::blas