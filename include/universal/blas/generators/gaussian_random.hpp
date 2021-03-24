#pragma once
// uniform_random.hpp: uniform random matrix generator
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <random>
#include <algorithm>

namespace sw{ 
    namespace universal{ 
        namespace blas{            
            template<typename Scalar, typename Mean, typename Stddev>
            inline void gaussian_random(matrix<Scalar>& omega, Mean mean = 100.00, Stddev stddev = 6.00){
                using value_type = typename matrix::value_type;
                using size_type = typename matrix::size_type;
                const size_type ncols = num_cols(omega), nrows = num_rows(omega);                
                std::random_device rd;
                std::mt19937 rng(rd());                
                for(size_t i=0; i<nrows; ++i){
                    for(size_t j=0; j<ncols; ++j){
                            std::normal_distribution<double> s(mean, stddev);
                            omega[i][j]=value_type(s(rng));
                        }
                    }
                }
            }
        }
    } // namespace sw::universal::blas