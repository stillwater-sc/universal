#pragma once

#include<algorithm>
#include<vector>
#include<tuple>
#include<universal/blas/matrix.hpp>
#include<mtl/operation/qr.hpp>
#include<mtl/operation/svd.hpp>
const double EPS = 1E-9;
double k = 0.0000001;
namespace sw {
    namespace universal {
        namespace blas {

            template<typename Scalar>
            size_t find_rank(const matrix<Scalar> A) {
                size_t n = num_rows(A);
                size_t m = num_cols(A);

                size_t rank = 0;
                vector<bool> row(n, false);
                for (size_t i = 0; i < m; ++i) {
                    for (size_t j = 0; j < n; ++j) {
                        if (abs(A[j][i]) > EPS && !row[j]) break;
                    }

                    if (j != n) {
                        ++rank;
                        row[j] = true;
                        for (size_t p = i + 1; p < m; ++p) A[j][p] /= A[j][i];
                        for (size_t k = 0; k < n; ++k) {
                            if (abs(A[k][i]) > EPS && k != j) {
                                for (size_t p = i + 1; p < m; ++p)
                                    A[k][p] -= A[j][p] * A[k][i];
                            }
                        }
                    }
                }
                return rank;
            }
            template<typename Scalar>
            void qr(const matrix<Scalar> A, matrix<Scalar> Q, matrix<Scalar> R) {
            		
            }
            
            template<typename Scalar>
            tuple randsvd(const matrix<Scalar> &A) {
                size_t k = min(num_cols(A), num_rows(A));
                size_t n = num_cols(A), m = num_row(A);
                //generate a gaussian random matrix of size n x k omega in this case
                matrix<Scalar> omega,Y,Q,R,B,S,V,D;
                //omega(n x k) x A(m x n) == Y(m x k)
                Y = A * omega;
                tie(Q, R) = qr(Y);
                //implement qr decomposition & svd here
                Q = transpose();
                B = Q * A;
                std::tie(S, V, D) = svd(B,k);

                return std::make_tuple(S, V, D);
            }
        }
    }
}