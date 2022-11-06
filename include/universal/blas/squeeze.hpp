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
// #include <universal/blas/vector.hpp>
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


template<typename Working, typename Low>
void squeezeScaleRound(blas::matrix<Working>& A, Working T = 1.0){
        /* scale by scalar, then round */
        Low xmax(SpecificValue::maxpos);
        //std::cout << "X max = " << xmax << std::endl;
        Working Xmax(xmax);
        Working Amax = maxelement(A);
        Working mu =(T*Xmax) / Amax;
        A = mu*A;  //Scale A
        // Al = A;
        // std::cout << Xmax << "\t" << Amax << "\t" << T << "\t" << mu << "\n" << std::endl;

} // Scale and Round



template<typename Working, typename Low>
void twosideScaleRound(blas::matrix<Working>& A, Working T = 1.0){
        /* two-sided scale, then round */
        blas::matrix<Working> R(num_rows(A),num_cols(A));
        blas::matrix<Working> S(num_rows(A),num_cols(A));
        R = 1;
        S = 1;

        Low xmax(SpecificValue::maxpos);
        Working Xmax(xmax);
        Working beta = maxelement(R*A*S);
        Working mu = (T*Xmax) / beta;
        A = mu*A;  //Scale A
        // std::cout << Xmax << "\t" << beta << "\t" << T << "\t" << mu << "\n" << std::endl;

} // Scale and Round


//template<typename Scalar>
//blas::matrix<Scalar> squeezeTest(blas::matrix<Scalar>& A){
        //  / * scale by scalar, then round */
//        Scalar maxpos(SpecificValue::maxpos);
//        Scalar mu = maxpos / maxelement(A);
//        blas::matrix<Scalar> B = mu*A;
//        return B;
//} // Scale and Round 


//template<typename Scalar>
//blas::matrix<Scalar> squeezeTest(blas::matrix<Scalar>& A){
        //  / * scale by scalar, then round */
//        Scalar maxpos(SpecificValue::maxpos);
//        Scalar mu = maxpos / maxelement(A);
//        blas::matrix<Scalar> B = mu*A;
//        return B;
//} // Scale and Round 



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