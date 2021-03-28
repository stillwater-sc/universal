#pragma once

#include<algorithm>
#include<vector>
#include<universal/blas/operators.hpp>
#include<universal/blas/solvers.hpp>
#include<universal/blas/generators.hpp>
#include<universal/include/universal/blas/blas_l1.hpp>
const double EPS = 1E-9;
const double k = 0.0000001;
namespace sw {
    namespace universal {
        namespace blas {

            template<typename Scalar>
            size_t find_rank(const matrix<Scalar>& A) {
                size_t n = num_rows(A);
                size_t m = num_cols(A);

                size_t rank = 0;
                vector<bool> row(n, false);
                for (size_t i = 0; i < m; ++i) {
                    for (size_t j = 0; j < n; ++j) {
                        if (abs(A[j][i]) > EPS && row[j]!=Scalar(0)) break;
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
            inline void qr(const matrix<Scalar>& A, matrix<Scalar>& Q, matrix<Scalar>& R) {
                using value_type = typename matrix::value_type;
                using size_type = typename matrix::size_type;
                using Magnitude<value_type>::type = typename matrix::magnitude_type;

                size_type ncols = num_cols(A), nrows = num_rows(A);
                mini = ncols == nrows ? ncols - 1 : (nrows >= ncols ? ncols : nrows);
                magnitude_type  factor = magnitude_type(2);
                //static_assert(nrows<ncols,"Required Columns<=Rows");
                matrix<Scalar> A_tmp(A),tmp;
                vector<matrix<Scalar>> qi(nrows);
                for(size_t i=0;i<ncols && i<nrows-1; ++i){
                    vector<Scalar> e(nrows),x(nrows);
                    Scalar a;
                    tmp=minor(A_tmp, i);
                    get_col(tmp, x, i);
                    a=norm(x, "two_norm");
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
            }//completed QR-decomposition
            template<typename Scalar>
            std::pair<matrix<Scalar>, matrix<Scalar>>
                inline qr(const matrix<Scalar>& A) {
                //R is the upper triangular matrix
                //Q is the orthogonal matrix
                matrix<Scalar> Q(num_rows(A), num_cols(A)), R(A);
                qr(A, Q, R);
                return std::make_pair(Q, R);
            }
            template<Scalar, Vector>
            void householder_factors(matrix<Scalar>& A, const Vector& v){
                Scalar n=num_cols(A);
                for(size_t i=0;i<n;++i){
                    for(size_t j=0;j<n;++j){
                        A[i][j]=-2*v[i]*v[j];
                    }
                }
                for(size_t i=0;i<n;++i) A[i][i]+=1;
            }
            template<typename Scalar, typename Tolerance>
            inline void svd(const matrix<Scalar>& A, matrix<Scalar>& S, matrix<Scalar>& V, matrix<Scalar>& D, Tolerance tol = 10e-10) {
                using value_type = typename matrix::value_type;
                using size_type = typename matrix::size_type;
                size_type        ncols = num_cols(A), nrows = num_rows(A);
                value_type       ref, zero = math::zero(ref), one = math::one(ref);
                double 	     err(std::numeric_limits<double>::max()), e, f;
                if (nrows != ncols) std::swap(row, col);
                matrix<Scalar> Q(nrows, nrows), R(nrows, ncols), VT(nrows, ncols), E(nrows, ncols),QT(ncols, ncols), RT(ncols, nrows);
                size_type l = 100 * std::max(nrows, ncols);
                S = one; D = one; E = zero;
                for (size_type i = 0; err > tol && i < l; ++i) {
                    std::tie(QT, RT) = qr(V);
                    S *= QT;
                    VT = RT.transpose();
                    std::tie(Q, R) = qr(VT);
                    D *= Q;
                    E = triu(R, 1);
                    V = trans(R);
                    //have to implement when upper(R)=0
                }
            }
            template<typename Scalar, typename Tolerance>
            std::tuple<matrix<Scalar>, matrix<Scalar>, matrix<Scalar>>
            inline svd(const matrix<Scalar>& A, Tolerance tol= 10e-10) {
                typedef typename Collection<Matrix>::size_type    size_type;
                size_type    ncols = num_cols(A), nrows = num_rows(A);
                if (nrows != ncols) std::swap(row, col);
                matrix<Scalar> ST(ncols, ncols), V(A), D(nrows, nrows);
                svd(A, ST, V, D, tol);
                return std::make_tuple(ST, V, D);

            }                        
            template<typename Scalar>
            std::tuple<matrix<Scalar>,matrix<Scalar>, matrix<Scalar>>
                inline randsvd(const matrix<Scalar>& A) {
                size_t k = min(num_cols(A), num_rows(A));
                size_t n = num_cols(A), m = num_row(A);                
                matrix<Scalar> omega(n, k),Y(m, k), B(k, n);                
                gaussian_random(omega);
                Y = A * omega;
                tie(Q, R) = qr(Y);
                Q.transpose();
                B = Q * A;
                std::tie(S, V, D) = svd(B,k);
                return std::make_tuple(S, V, D);
            }
        }
    }
}