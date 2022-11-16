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
#include <universal/blas/vector.hpp>
#include <universal/blas/blas.hpp>  // this includes matrix/vector (are the above needed?)

namespace sw{namespace universal{

 
template<typename Scalar>
void getR(blas::matrix<Scalar>& A, blas::vector<Scalar>& R){
    for (unsigned i = 0; i < num_rows(A); ++i){
        blas::vector<Scalar> localvec(num_rows(A),1);
        for (unsigned j = 0; j < num_cols(A); ++j){
            localvec(j) = A(i,j);
        }
        R(i) = 1/normLinf(localvec);
    }  
} // Get Row scaler
 


 
template<typename Scalar>
void getS(blas::matrix<Scalar>& A, blas::vector<Scalar>& S){
    for (unsigned j = 0; j < num_rows(A); ++j){
        blas::vector<Scalar> localvec(num_rows(A),1);
        for (unsigned i = 0; i < num_cols(A); ++i){
            localvec(i) = A(i,j);
        }
        S(j) = 1/normLinf(localvec);
    }  
} // Get Column scaler
 


template<typename Scalar>
void rowScale(blas::vector<Scalar>& R, blas::matrix<Scalar>& A){
    for (unsigned i = 0; i < num_rows(A); ++i){
        for (unsigned j = 0; j < num_cols(A); ++j){
            A(i,j) = R(i)*A(i,j);
        }
    }  
} // Scale Rows of A

template<typename Scalar>
void colScale(blas::matrix<Scalar>& A, blas::vector<Scalar>& S){
    for (unsigned j = 0; j < num_rows(A); ++j){
        for (unsigned i = 0; i < num_cols(A); ++i){
            A(i,j) = S(j)*A(i,j);
        }
    }
} // Scale Columns of A

template<typename Working, typename Low>
void squeezeRoundReplace(blas::matrix<Working>& A, blas::matrix<Low>& Al){
        /* Algo 21: round then replace infinities */
        Al = A;
        Low maxpos(SpecificValue::maxpos);
        for (size_t i = 0; i < num_rows(A); ++i){
            for (size_t j = 0; j < num_cols(A); ++j){
                Low sgn = (Al(i,j) > 0) ? 1 : ((Al(i,j) < 0) ? -1 : 0);
                if (isinf(abs(Al(i,j)))){
                    Al(i,j) = sgn*(maxpos);   
                }
            }
        }
} // Round and Replace


template<typename Working, typename Low>
void squeezeScaleRound(blas::matrix<Working>& A, blas::matrix<Low>& Al, Working T = 1.0){
        /* Algo 22:  scale by scalar, then round */
        Working Amax = maxelement(A);
        Low xmax(SpecificValue::maxpos);
        Working Xmax(xmax);
        Working mu =(T*Xmax) / Amax;
        A = mu*A;  //Scale A
        //std::cout << "A (after scaling)  = \n" << A << std::endl;
        Al = A;
        //std::cout << "Al (after scaling)  = \n" << B << std::endl;
        //std::cout << "--------------------------------------------" << std::endl;
        //std::cout << Xmax << "\t" << Amax << "\t  \t" << T << "\t" << mu << "\n" << std::endl;
        /* 
        std::cout << "A (after scaling)  = \n" << A << std::endl;
        std::cout << "Xmax \t  Amax \t      T \t     mu " << std::endl;
        std::cout << "--------------------------------------------" << std::endl;
        std::cout << Xmax << "\t" << Amax << "\t  \t" << T << "\t" << mu << "\n" << std::endl;
        */
} // Scale and Round



template<typename Working, typename Low>
void twosideScaleRound(blas::matrix<Working>& A, blas::matrix<Low>& Al, Working T = 1.0, size_t algo = 23){
        blas::vector<Working> R(num_rows(A),1);
        blas::vector<Working> S(num_rows(A),1);
        
        if (algo == 24){xyyEQU(R,A,S);}
        if (algo == 25){
            // 
        }
        squeezeScaleRound(A, Al, T );
        /* Algo 23: general two-sided scaling, then round*/
        /*
        Low xmax(SpecificValue::maxpos);
        Working Xmax(xmax);
        Working beta = maxelement(A);
        Working mu = (T*Xmax) / beta;
        A = mu*A;   // Scale A
        B = A;     // Round
        */
        // std::cout << Xmax << "\t" << beta << "\t" << T << "\t" << mu << "\n" << std::endl;
} // Two-sided Scale and Round


template<typename Scalar>
void xyyEQU(blas::vector<Scalar>& R, blas::matrix<Scalar>& A, blas::vector<Scalar>& S){
        /* Algo 24: construct R and S */
        /* Algo 24: row and column equilibration */
        getR(A,R);          // Lines:1-4
        std::cout << "R = \n" << R << std::endl;
        rowScale(R,A);      // Line: 5,  A is row equilibrated
        std::cout << "RA = \n" << A << std::endl;
        getS(A,S);          // Lines: 6 - 9
        std::cout << "S = \n" << S << std::endl;
        colScale(A,S);
        std::cout << "RAS = \n" << A << std::endl;

} // Construct R and S






}} // namespace