/** **********************************************************************
 * Estimated Condition number of matrix 
 *
 * @author:     James Quinlan
 * @date:       2023-02-11
 * @copyright:  Copyright (c) 2022 Stillwater Supercomputing, Inc.
 * @license:    MIT Open Source license 
 * 
 * This file is part of the universal numbers project.
 * ***********************************************************************
 */
#pragma once
#include <tuple>

#include <blas/blas.hpp>
 
namespace sw { namespace blas {  

	using namespace sw::numeric::containers;

	// LU decomposition with partial pivoting
    template<typename Scalar>
    std::tuple<matrix<Scalar>, matrix<Scalar>> plu(const matrix<Scalar>& A){ 

        using Matrix = matrix<Scalar>;
        using namespace std;

        Scalar x;
        size_t n = num_rows(A);

        Matrix L(n,n);
        Matrix U(n,n);

        L = 1;
        U = A;

        // Elimination Process
        for (size_t i = 0; i < n-1; ++i){ // i-th row
            Scalar absmax = abs(U(i,i)); 
            size_t argmax = i;

            // Select k >= i to maximize |U(k,i)| 
            for (size_t k = i + 1; k < n; ++k){ // subsequent row (ele. in column k)
                if (abs(U(k,i)) > absmax){
                    absmax = abs(U(k,i));
                    argmax = k;
                }
            }
            // Check for necessary swaps
            if (argmax != i){
                // Swap rows loop
                for (size_t j = i; j < n;++j){
                    x = U(i,j);
                    U(i,j) = U(argmax,j);
                    U(argmax,j) = x;
                }
           
                // Permuate entries in L to match P
                for (size_t j = 0; j < i; ++j){
                    x = L(i,j);
                    L(i,j) = L(argmax,j);
                    L(argmax,j) = x;
                }
            }
            // Continue with row reduction
            for (size_t k = i + 1; k < n; ++k){  // objective row
                        L(k,i) = U(k,i) / U(i,i);
                for (size_t j = i; j < n; ++j){
                    U(k,j) = U(k,j) - L(k,i)*U(i,j);
                }
            }
        }
        U = triu(U);
        return std::make_tuple(L,U); 
    } // LU

    template<typename Scalar>
    Scalar condest(const sw::universal::blas::matrix<Scalar>& A) {
        /**
         * After changing from [P,L,U] = plu(A) to inplace version of plu,
         * condest has stopped working.  Need to fix, however, as long as
         * showCondest = false; LUIR.cpp will run.
         */

        Scalar Na = matnorm(A, 2);    // || A ||
        Scalar Ni = 1;               // || A^{-1} ||
        vector<Scalar> b(num_cols(A), 1);

        auto [L, U] = plu(A);
        auto Ut = U;
        auto Lt = L;
        auto z = forwsub(Ut.transpose(), b);
        auto x = backsub(Lt.transpose(), z);
        auto y = backsub(U, forwsub(L, x));

        Scalar infNormY = 0;
        for (size_t k = 0; k < num_cols(A); k++) infNormY = (abs(y(k)) > infNormY) ? abs(y(k)) : infNormY;
        Scalar infNormX = 0;
        for (size_t k = 0; k < num_cols(A); k++) infNormX = (abs(x(k)) > infNormX) ? abs(x(k)) : infNormX;
        Ni = infNormY / infNormX;

        return Ni * Na;

    } // end function

    // Reference 
    // Equations (4.3) and (4.4) p. 372 
    /**
    Cline, A. K., Moler, C. B., Stewart, G. W., & Wilkinson, J. H. (1979).
    An estimate for the condition number of a matrix. SIAM Journal on
    Numerical Analysis, 16(2), 368-375.

    @article{cline1979estimate,
      title={An estimate for the condition number of a matrix},
      author={Cline, Alan K and Moler, Cleve B and Stewart, George W and Wilkinson, James H},
      journal={SIAM Journal on Numerical Analysis},
      volume={16},
      number={2},
      pages={368--375},
      year={1979},
      publisher={SIAM}
    }
    */

}} // namespace sw::blas

