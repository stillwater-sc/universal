// Squeeze.hpp: Squeeze elements of a matrix for solving Ax = b
//              using low-precision representations.
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
// Author: James Quinlan
//
// File is part of the universal numbers project. 
// License: MIT Open Source license.

// Modified: 2022-10-30
// --------------------------------------------------------------- //
#pragma once
#include <universal/blas/matrix.hpp>
#include <universal/blas/blas.hpp>

namespace sw{namespace universal{

template<typename Scalar>
void squeezeRoundReplace(blas::matrix<Scalar>& A){
        /* round then replace infinities */
        Scalar maxpos(SpecificValue::maxpos);
        for (size_t i = 0; i < num_rows(A); ++i){
            for (size_t j = 0; j < num_cols(A); ++j){
                Scalar sgn = (A(i,j) > 0) ? 1 : ((A(i,j) < 0) ? -1 : 0);
                if (isinf(abs(A(i,j)))){
                    A(i,j) = sgn*(maxpos);   
                }
            }
        }
} // Round and Replace


template<typename Scalar>
void squeezeScaleRound(blas::matrix<Scalar>& A, Scalar T = 1){
        /* scale by scalar, then round */
        Scalar maxpos(SpecificValue::maxpos);
        Scalar mu = T*maxpos / maxelement(A);
        A = mu*A;
} // Scale and Round


}} // namespace

template<typename Scalar>
void squeeze(sw::universal::blas::matrix<Scalar>& A, size_t algo = 1, Scalar T = 1) {
    /**
    * A is n x n matrix. 
    * algo = Which squeezing technique? See higham2019squeezing
    * T = Theta in Algorithm 2.2
    */
    
    Scalar M;

    if (algo == 1){ 
        /* do nothing to A */
    }
    else if (algo == 21) // Algo 2.1
    {
        /* round then replace infinities */
        // Ml Al(A);
        for (size_t i = 0; i < num_rows(A); ++i){
            for (size_t j = 0; j < num_cols(A); ++j){
                Scalar sgn = (A(i,j) > 0) ? 1 : ((A(i,j) < 0) ? -1 : 0);
                if (isinf(abs(A(i,j)))){
                    A(i,j) = sgn*(M.maxpos());   
                }
            }
        }
    }
    else if (algo == 22) // Algo 2.2
    {
        /* scale by scalar, then round */
        Scalar mu = T*(M.maxpos()) / maxelement(A);
        //std::cout << A << std::endl;
        //std::cout << "T = " << T << ", M = " << M.maxpos() << ", Amax = " << maxelement(A) << std::endl;
        A = mu*A;
        // Ml Al(A);
    }
    
    

} // end squeeze