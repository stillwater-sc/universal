#pragma once

#include<algorithm>
#include<universal/blas/matrix.hpp>
#include<universal/blas/vector.hpp>
#include<universal/blas/operators.hpp>
#include<universal/blas/solvers.hpp>
#include<universal/blas/generators.hpp>
#include<universal/blas/blas_l1.hpp>

const double k = 0.0000001;
namespace sw::universal::blas {  

            template<typename Scalar>
            void qr(const matrix<Scalar>& A, matrix<Scalar>& Q, matrix<Scalar>& R) {
                /*
                using value_type = typename matrix<Scalar>::value_type;
                using size_type = typename matrix<Scalar>::size_type;

                size_type ncols = num_cols(A), nrows = num_rows(A);
//                static_assert(nrows < ncols, "Required Columns <= Rows");
                matrix<Scalar> A_tmp(A),tmp;
                vector<matrix<Scalar>> qi(nrows);
                for(size_t i=0;i<ncols && i<nrows-1; ++i){
                    vector<Scalar> e(nrows),x(nrows);
                    Scalar a;
                    tmp=minor(A_tmp, i);
                    get_col(tmp, x, i);
                    a=norm(x, 2);
                    if(A[nrows][nrows]>0) a-=a;
                    for(size_t j=0; j<e.size();++j){
                        e[j]=(j==nrows) ? 1:0;
                    }
                    for(size_t j=0;j<e.size();++j) e[j]=x[j]+a*e[j];
                    Scalar f=norm(e);
                    for(size_t j=0;j<e.size();++j) e[j]/=f;
                    householder_factors(qi[i], e);
                    A_tmp=qi[i]*tmp;
                }
                Q=qi[0];
                for(size_t i=1;i<ncols && i<nrows-1;++i){
                    tmp=qi[i]*Q;
                    Q=tmp;
                }
                R=Q*A;
                Q.transpose();
                */
            }

            template<typename Scalar>
            std::pair<matrix<Scalar>, matrix<Scalar>> qr(const matrix<Scalar>& A) {
                //R is the upper triangular matrix
                //Q is the orthogonal matrix
                matrix<Scalar> Q(num_rows(A), num_cols(A)), R(A);
                qr(A, Q, R);
                return std::make_pair(Q, R);
            }


            template<typename Scalar>
            void householder_factors(matrix<Scalar>& A, const vector<Scalar>& v){
                size_t n=num_cols(A);
                for(size_t i=0;i<n;++i){
                    for(size_t j=0;j<n;++j){
                        A[i][j]=-2*v[i]*v[j];
                    }
                }
                for(size_t i=0;i<n;++i) A[i][i]+=1;
            }

template<typename Scalar>
std::tuple<matrix<Scalar>,matrix<Scalar>, matrix<Scalar>> randsvd(const matrix<Scalar>& A) {
    size_t k = std::min(num_cols(A), num_rows(A));
    size_t n = num_cols(A), m = num_rows(A);                
    matrix<Scalar> omega(n, k),Y(m, k), B(k, n);
    double mean = 1.0;
    double stddev = 0.5;
    gaussian_random(omega, mean, stddev);
    Y = A * omega;
    matrix<Scalar> Q(n, k), R(n, n);
    std::tie(Q, R) = qr(Y);
    Q.transpose();
    B = Q * A;
    matrix<Scalar> S(n, k), V(n, n), D(n, n);
    std::tie(S, V, D) = svd(B,k);
    return std::make_tuple(S, V, D);
}

} // namespace sw::universal::blas