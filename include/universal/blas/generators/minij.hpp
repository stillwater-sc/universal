#pragma once
// minij.hpp: uniform random matrix generator
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include<algorithm>

namespace sw{ 
    namespace universal{ 
        namespace blas{            
            template<typename Scalar>
            inline matrix<Scalar> minij(std::size_t n){//returns the n-by-n symmetric positive definite matrix with entries A(i,j) = min(i,j).
                matrix<Scalar> A(n, n);
                using value_type = typename matrix::value_type;
                using size_type = typename matrix::size_type;
                for(std::size_t i=0; i<n; ++i){
                    for(std::size_t j=0; j<n; ++j){
                            A(i,j)=value_type(std::min(i,j));
                        }
                    }
                }
            }
        }
    } // namespace sw::universal::blas