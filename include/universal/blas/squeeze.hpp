/** **********************************************************************
 * Squeeze.hpp: Squeeze elements of a matrix for solving Ax = b
 *              using low-precision representations.
 *
 * @author:     James Quinlan
 * @date:       2022-12-20
 * @copyright:  Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
 * @license:    MIT Open Source license 
 * 
 * This file is part of the Universal Number Library project.
 * ***********************************************************************
 */

#pragma once
#include <universal/blas/blas.hpp>
 
namespace sw{namespace universal{

/**
 * ***********************************************************************
 * Helper functions
 *  - row/column scaling
 *  - generate matrices R and S (see Higham) 
 * ***********************************************************************
 */

template<typename Scalar>
void getR(blas::matrix<Scalar>& A, blas::vector<Scalar>& R, unsigned &n){
    Scalar M;
    for (unsigned i = 0; i < n; ++i){
        M = 0;
        for (unsigned j = 0; j < n; ++j){
            M = (abs(A(i,j)) > M) ? abs(A(i,j)) : M;
        }
        R(i) = 1/M;
    }  
} // Get Row scaler
 
template<typename Scalar>
void getS(blas::matrix<Scalar>& A, blas::vector<Scalar>& S, unsigned &n){
    Scalar M;
    for (unsigned j = 0; j < n; ++j){
        M = 0;
        for (unsigned i = 0; i < n; ++i){
            M = (abs(A(i,j)) > M) ? abs(A(i,j)) : M;
        }
        S(j) = 1/M;        
    }  
} // Get Column scaler
 

template<typename Scalar>
void rowScale(blas::vector<Scalar>& R, blas::matrix<Scalar>& A ,unsigned &n){
    for (unsigned i = 0; i < n; ++i){
        for (unsigned j = 0; j < n; ++j){
            A(i,j) = R(i)*A(i,j);
        }
    }  
} // Scale Rows of A

template<typename Scalar>
void colScale(blas::matrix<Scalar>& A, blas::vector<Scalar>& S, unsigned &n){
    for (unsigned j = 0; j < n; ++j){
        for (unsigned i = 0; i < n; ++i){
            A(i,j) = S(j)*A(i,j);
        }
    }
} // Scale Columns of A



/** 
 * ***********************************************************************
 * Squeeze Methods
 *  - round and replace:  
 *  - scale, then round:
 *  - two-sided scaling (row/column equilabration), then round
 * ***********************************************************************
*/
template<typename Working, typename Low>
void roundReplace(blas::matrix<Working>& A, blas::matrix<Low>& Al, unsigned &n){
    /* Algo 21: round then replace infinities */
    Al = A;
    Low maxpos(SpecificValue::maxpos);
    for (unsigned i = 0; i < n; ++i){
        for (unsigned j = 0; j < n; ++j){
            Low sgn = (Al(i,j) > 0) ? 1 : ((Al(i,j) < 0) ? -1 : 0);
            if (isinf(abs(Al(i,j)))){
                Al(i,j) = sgn*(maxpos);   
            }
        }
    }
} // Round and Replace


template<typename Working, typename Low>
void scaleRound(blas::matrix<Working>& A, 
                    blas::matrix<Low>& Al, 
                    Working T, 
                    Working &mu){
    /* Algo 22:  scale by scalar, then round */
    Working Amax = maxelement(A);
    Low xmax(SpecificValue::maxpos);
    Working Xmax(xmax);
    
    #define CFLOAT 0   // 0 = POSITS
    // /** 
    #if CFLOAT 
        mu =(T*Xmax) / Amax;  // use for cfloats
    #else
        mu = T / Amax;  // use for posits
    #endif
    
    A = mu*A;  // Scale A
    Al = A;    // Round A = fl(A)
    // std::cout << "Al (after scaling)  = \n" << Al << std::endl;
    // std::cout << "--------------------------------------------" << std::endl;
    // std::cout << Xmax << "\t" << Amax << "\t  \t" << T << "\t" << mu << "\n" << std::endl;
    /* 
    std::cout << "A (after scaling)  = \n" << A << std::endl;
    std::cout << "Xmax \t  Amax \t      T \t     mu " << std::endl;
    std::cout << "--------------------------------------------" << std::endl;
    std::cout << Xmax << "\t" << Amax << "\t  \t" << T << "\t" << mu << "\n" << std::endl;
    */
} // Scale and Round


template<typename Working, typename Low>
void twosideScaleRound(blas::matrix<Working>& A, 
                       blas::matrix<Low>& Al, 
                       blas::vector<Working>& R, 
                       blas::vector<Working>& S, 
                       Working T,
                       Working &mu,
                       unsigned &n, 
                       size_t algo = 24){
        
    if (algo == 24){xyyEQU(R,A,S,n);}
    if (algo == 25){ 
        // nothing here to see
    }
    scaleRound(A, Al, T, mu);
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
void xyyEQU(blas::vector<Scalar>& R, 
            blas::matrix<Scalar>& A, 
            blas::vector<Scalar>& S, 
            unsigned &n){
    /* Algo 24: construct R and S */
    /* Algo 24: row and column equilibration */
    bool print = false;

    getR(A,R,n);          // Lines:1-4
    if (print){std::cout << "R = \n" << R << std::endl;}
    
    rowScale(R,A,n);      // Line: 5,  A is row equilibrated
    if (print){std::cout << "RA = \n" << A << std::endl;}
    
    getS(A,S,n);          // Lines: 6 - 9
    if (print){std::cout << "S = \n" << S << std::endl;}
    
    colScale(A,S,n);
    if (print){std::cout << "RAS = \n" << A << std::endl;}
} // Construct R and S

}} // namespace