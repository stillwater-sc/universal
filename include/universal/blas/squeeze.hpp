#pragma once
// squeeze.hpp: Squeeze elements of a matrix for solving Ax = b
//              using low-precision representations.
//
// @author:     James Quinlan
// Copyright (C) 2022 James Quinlan
// SPDX-License-Identifier: MIT 
//
// Created : 2022-12-20
// Modified: 2022-10-30
//
// This file is part of the Mixed Precision Iterative Refinement project.
#include <universal/blas/blas.hpp>
 
namespace sw { namespace universal {

/**
 * ***********************************************************************
 * Helper functions
 *  - row/column scaling
 *  - generate matrices R and S (see Higham) 
 * ***********************************************************************
 */

/// <summary>
///  Get row scaler
/// </summary>
/// <typeparam name="Scalar"></typeparam>
/// <param name="A">matrix to generate row scale factors for</param>
/// <param name="R"></param>
template<typename Scalar>
void getR(const blas::matrix<Scalar>& A, blas::vector<Scalar>& R) {
    unsigned n = static_cast<unsigned>(A.rows()); // assuming square matrix
    for (unsigned i = 0; i < n; ++i) {
        Scalar M{ 0 };
        for (unsigned j = 0; j < n; ++j) {
            M = (abs(A(i,j)) > M) ? abs(A(i,j)) : M;
        }
        R(i) = 1/M;
    }  
}

/// <summary>
/// Get column scaler
/// </summary>
/// <typeparam name="Scalar"></typeparam>
/// <param name="A">matrix to generate column scale factors for</param>
/// <param name="S">scale factors</param>
template<typename Scalar>
void getS(const blas::matrix<Scalar>& A, blas::vector<Scalar>& S) {
    unsigned n = static_cast<unsigned>(A.rows()); // assuming square matrix
    for (unsigned j = 0; j < n; ++j) {
        Scalar M{ 0 };
        for (unsigned i = 0; i < n; ++i) {
            M = (abs(A(i,j)) > M) ? abs(A(i,j)) : M;
        }
        S(j) = 1/M;
    }  
}
 
/// <summary>
/// Scale Rows of A via R*A where R is a vector of scale factors
/// </summary>
/// <typeparam name="Scalar"></typeparam>
/// <param name="R">scale factors to apply</param>
/// <param name="A">matrix to scale</param>
template<typename Scalar>
void rowScale(blas::vector<Scalar>& R, const blas::matrix<Scalar>& A) {
    unsigned n = static_cast<unsigned>(A.rows()); // assuming square matrix
    for (unsigned i = 0; i < n; ++i) {
        for (unsigned j = 0; j < n; ++j) {
            A(i,j) = R(i)*A(i,j);
        }
    }  
}

/// <summary>
/// Scale Columns of A via A*S where S is a vector of scale factors
/// </summary>
/// <typeparam name="Scalar"></typeparam>
/// <param name="A">matrix to scale</param>
/// <param name="S">scale factors to apply</param>
template<typename Scalar>
void colScale(blas::matrix<Scalar>& A, blas::vector<Scalar>& S) {
    unsigned n = static_cast<unsigned>(A.rows()); // assuming square matrix
    for (unsigned j = 0; j < n; ++j) {
        for (unsigned i = 0; i < n; ++i) {
            A(i,j) = S(j)*A(i,j);
        }
    }
}


/** 
 * ***********************************************************************
 * Squeeze Methods
 *  - round and replace:  
 *  - scale, then round:
 *  - two-sided scaling (row/column equilabration), then round
 * ***********************************************************************
*/


/// <summary>
/// RoundAndReplace: round and replace infinities
/// </summary>
/// <typeparam name="Working"></typeparam>
/// <typeparam name="Low"></typeparam>
/// <param name="A">Working precision matrix</param>
/// <param name="Al">Low precision matrix</param>
template<typename Working, typename Low>
void RoundAndReplace(const blas::matrix<Working>& Aw, const blas::matrix<Low>& Al){
    constexpr bool Verbose = false;

    unsigned m = static_cast<unsigned>(Aw.rows());
    unsigned n = static_cast<unsigned>(Aw.cols());
    Low maxpos(SpecificValue::maxpos);
    for (unsigned i = 0; i < m; ++i) {
        double maxval{ 0 };
        for (unsigned j = 0; j < n; ++j) {
            if (abs(Aw(i,j)) > maxval) maxval = double(abs(Aw(i,j)));
            Low sgn = (Aw(i,j) > 0) ? 1 : ((Aw(i,j) < 0) ? -1 : 0);
            if (isinf(Al(i,j))) {
                Al(i,j) = sgn*(maxpos);   
            }
        }
        if constexpr (Verbose) std::cout << "maxval row[" << std::setw(4) << i << "] = " << maxval << std::endl;
    }
} 


template<typename WorkingPrecision, typename LowPrecision>
void ScaleAndRound(blas::matrix<WorkingPrecision>& Aw, blas::matrix<LowPrecision>& Al, WorkingPrecision& T, WorkingPrecision& mu) {
    constexpr bool Trace = false;
    /* Algo 22: scale by scalar, then round */
    WorkingPrecision Amax = maxelement(Aw);
    LowPrecision xmax(SpecificValue::maxpos);
    WorkingPrecision Xmax(xmax);
    
    // 
    #if defined(CFLOAT) 
        mu =(T*Xmax) / Amax;  // use for cfloats
    #else
        mu = T / Amax;  // use for posits
    #endif
    
    Aw *= mu; // Scale Aw
    Al = Aw;  // Round Aw to lower precision

    if constexpr (Trace) {
        std::cout << "Al (after scaling)  = \n" << Al << '\n';
        std::cout << "--------------------------------------------" << '\n';
        std::cout << Xmax << "\t" << Amax << "\t  \t" << T << "\t" << mu << "\n" << '\n';

        std::cout << "Aw (after scaling)  = \n" << Aw << '\n';
        std::cout << "Xmax \t  Amax \t      T \t     mu " << '\n';
        std::cout << "--------------------------------------------" << '\n';
        std::cout << Xmax << "\t" << Amax << "\t  \t" << T << "\t" << mu << "\n" << '\n';
    } 
}


template<typename Scalar>
void xyyEQU(blas::vector<Scalar>& R, blas::matrix<Scalar>& A, blas::vector<Scalar>& S) 
{
    /* Algo 24: construct R and S */
    /* Algo 24: row and column equilibration */
    constexpr bool Trace = false;

    getR(A, R);          // Lines:1-4
    if constexpr (Trace) { std::cout << "R = \n" << R << std::endl; }

    rowScale(R, A);      // Line: 5,  A is row equilibrated
    if constexpr (Trace) { std::cout << "RA = \n" << A << std::endl; }

    getS(A, S);          // Lines: 6 - 9
    if constexpr (Trace) { std::cout << "S = \n" << S << std::endl; }

    colScale(A, S);
    if constexpr (Trace) { std::cout << "RAS = \n" << A << std::endl; }
}

/// <summary>
/// general two-sided scaling, then round
/// </summary>
/// <typeparam name="WorkingPrecision"></typeparam>
/// <typeparam name="LowPrecision"></typeparam>
/// <param name="A"></param>
/// <param name="Al"></param>
/// <param name="T"></param>
/// <param name="mu"></param>
template<typename WorkingPrecision, typename LowPrecision>
void TwoSidedScaleAndRound(blas::matrix<WorkingPrecision>& Aw, blas::matrix<LowPrecision>& Al, WorkingPrecision& T, WorkingPrecision& mu)
{
    blas::vector<WorkingPrecision> R(num_rows(Aw), 1);  // Row Squeezer
    blas::vector<WorkingPrecision> S(num_rows(Aw), 1);  // Column Squeezer

    xyyEQU(R,Aw,S);
    ScaleAndRound(Aw, Al, T, mu);
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
}

}} // namespace sw::universal